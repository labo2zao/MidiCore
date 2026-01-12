#include "Services/ain/ain.h"
#include "Services/ui/ui.h"
#include "Hal/ainser64_hw/hal_ainser64_hw_step.h"
#include "cmsis_os2.h"
#include <string.h>
#include <math.h>

typedef enum { ST_IDLE =0, ST_ARMED, ST_DOWN } key_state_t;

typedef struct {
  uint16_t cal_min, cal_max;
  uint16_t filt;
  uint16_t pos, pos_prev;
  uint32_t t1_ms;
  uint16_t vb_ema;
  key_state_t st;
} key_ctx_t;

static key_ctx_t g_keys[AIN_NUM_KEYS];

// Debug snapshots (updated in process_key()).
static uint16_t g_dbg_raw[AIN_NUM_KEYS];
static uint16_t g_dbg_filt[AIN_NUM_KEYS];
static uint16_t g_dbg_pos14[AIN_NUM_KEYS];

// thresholds on pos (0..16383)
static const uint16_t T1   = 1200;
static const uint16_t T2   = 6500;
static const uint16_t TOFF = 4200;
static const uint16_t HYS  = 250;

// velocity mapping params
static const uint32_t DT_MIN_MS = 10;
static const uint32_t DT_MAX_MS = 160;
static const float    GAMMA     = 1.4f;
static const float    WA        = 0.7f;

#define EVQ_SIZE 64
static ain_event_t evq[EVQ_SIZE];
static volatile uint8_t evq_w = 0, evq_r = 0;

static inline void evq_push(const ain_event_t* e) {
  uint8_t next = (uint8_t)((evq_w + 1) & (EVQ_SIZE - 1));
  if (next == evq_r) return;
  evq[evq_w] = *e;
  evq_w = next;
}

uint8_t ain_pop_event(ain_event_t* ev) {
  if (evq_r == evq_w) return 0;
  *ev = evq[evq_r];
  evq_r = (uint8_t)((evq_r + 1) & (EVQ_SIZE - 1));
  return 1;
}

static inline uint16_t clamp_u16(int32_t v, uint16_t lo, uint16_t hi) {
  if (v < (int32_t)lo) return lo;
  if (v > (int32_t)hi) return hi;
  return (uint16_t)v;
}

static uint16_t normalize(uint16_t raw, uint16_t mn, uint16_t mx) {
  if (mx <= mn + 8) return 0;
  int32_t num = (int32_t)(raw - mn) * 16383;
  int32_t den = (int32_t)(mx - mn);
  int32_t p = num / den;
  return clamp_u16(p, 0, 16383);
}

static inline uint32_t now_ms(void) { return (uint32_t)osKernelGetTickCount(); }

static uint8_t map_velocity_A(uint32_t dt_ms) {
  if (dt_ms <= DT_MIN_MS) return 127;
  if (dt_ms >= DT_MAX_MS) return 1;
  float x = (float)(dt_ms - DT_MIN_MS) / (float)(DT_MAX_MS - DT_MIN_MS);
  float y = powf(x, GAMMA);
  float v = 127.0f - (y * 126.0f);
  if (v < 1.0f) v = 1.0f;
  if (v > 127.0f) v = 127.0f;
  return (uint8_t)(v + 0.5f);
}

static uint8_t map_velocity_B(uint16_t vb_ema) {
  const uint16_t VB_MIN = 5;
  const uint16_t VB_MAX = 400;
  if (vb_ema <= VB_MIN) return 1;
  if (vb_ema >= VB_MAX) return 127;
  uint32_t v = (uint32_t)(vb_ema - VB_MIN) * 126u / (uint32_t)(VB_MAX - VB_MIN) + 1u;
  if (v > 127u) v = 127u;
  return (uint8_t)v;
}

static inline uint8_t fuse_vel(uint8_t vA, uint8_t vB) {
  // Use integer arithmetic instead of floating point for performance
  uint32_t vf = ((uint32_t)vA * 70u + (uint32_t)vB * 30u) / 100u;
  if (vf < 1u) vf = 1u;
  if (vf > 127u) vf = 127u;
  return (uint8_t)vf;
}

static void process_key(uint8_t key, uint16_t raw) {
  key_ctx_t* k = &g_keys[key];

  // calibrate bounds (keep enabled for bring-up; you may freeze later)
  if (raw < k->cal_min) k->cal_min = raw;
  if (raw > k->cal_max) k->cal_max = raw;

  // EMA filter: adaptive
  uint8_t shift = (k->st == ST_DOWN) ? 3 : 2;
  k->filt = (uint16_t)(k->filt + ((int32_t)raw - (int32_t)k->filt) / (1<<shift));

  k->pos_prev = k->pos;
  k->pos = normalize(k->filt, k->cal_min, k->cal_max);

  // Debug snapshots
  g_dbg_raw[key] = raw;
  g_dbg_filt[key] = k->filt;
  g_dbg_pos14[key] = k->pos;

  if (k->st == ST_IDLE) {
    if (k->pos > T1) {
      k->st = ST_ARMED;
      k->t1_ms = now_ms();
      k->vb_ema = 0;
    }
  } else if (k->st == ST_ARMED) {
    uint16_t dpos = (k->pos > k->pos_prev) ? (k->pos - k->pos_prev) : 0;
    k->vb_ema = (uint16_t)(k->vb_ema + ((int32_t)dpos - (int32_t)k->vb_ema) / 2);

    if (k->pos > T2) {
      uint32_t dt = now_ms() - k->t1_ms;
      uint8_t vA = map_velocity_A(dt);
      uint8_t vB = map_velocity_B(k->vb_ema);
      uint8_t v  = fuse_vel(vA, vB);

      // Note on (raw key; chord handled in AinMIDI task)
      ain_event_t e = { .key = key, .type = AIN_EV_NOTE_ON, .pos = k->pos, .velocity = v };
      evq_push(&e);

      k->st = ST_DOWN;
    }
  } else { // DOWN
    uint16_t th = (TOFF > HYS) ? (TOFF - HYS) : 0;
    if (k->pos < th) {
      // Note off (raw key; chord handled in AinMIDI task)
      ain_event_t e = { .key = key, .type = AIN_EV_NOTE_OFF, .pos = k->pos, .velocity = 0 };
      evq_push(&e);
      k->st = ST_IDLE;
    }
  }
}

static uint8_t g_bank = 0;
static uint8_t g_step = 0;

// MIOS32-compatible port re-ordering (matches the wiring on the classic
// MBHP_AINSER64 PCB).
// If your PCB/wiring differs, adjust this table.
static const uint8_t k_mux_port_map[8] = { 0, 5, 2, 7, 4, 1, 6, 3 };

#ifndef AINSER64_NUM_MODULES
#define AINSER64_NUM_MODULES 1
#endif

void ain_init(void) {
  memset(g_keys, 0, sizeof(g_keys));
  memset(g_dbg_raw, 0, sizeof(g_dbg_raw));
  memset(g_dbg_filt, 0, sizeof(g_dbg_filt));
  memset(g_dbg_pos14, 0, sizeof(g_dbg_pos14));
  for (uint8_t i=0;i<AIN_NUM_KEYS;i++) {
    g_keys[i].cal_min = 0;
    g_keys[i].cal_max = 4095;
    g_keys[i].st = ST_IDLE;
  }
}

void ain_debug_get_raw(uint16_t* dst, uint16_t len) {
  if (!dst) return;
  if (len > AIN_NUM_KEYS) len = AIN_NUM_KEYS;
  memcpy(dst, g_dbg_raw, len * sizeof(uint16_t));
}

void ain_debug_get_filt(uint16_t* dst, uint16_t len) {
  if (!dst) return;
  if (len > AIN_NUM_KEYS) len = AIN_NUM_KEYS;
  memcpy(dst, g_dbg_filt, len * sizeof(uint16_t));
}

void ain_debug_get_pos14(uint16_t* dst, uint16_t len) {
  if (!dst) return;
  if (len > AIN_NUM_KEYS) len = AIN_NUM_KEYS;
  memcpy(dst, g_dbg_pos14, len * sizeof(uint16_t));
}

void ain_tick_5ms(void) {
  uint16_t vals[8] = {0};
  if (hal_ainser64_read_bank_step(g_bank, g_step, vals) == 0) {
    uint8_t port = k_mux_port_map[g_step & 7u];
    for (uint8_t ch=0; ch<8; ch++) {
      // Key mapping:
      // - step selects the port group (J6..J13), possibly reordered by k_mux_port_map
      // - MCP3208 channel (0..7) corresponds to A0..A7, reversed to match MIOS32
      (void)g_bank; // currently only one module supported in this project
      uint8_t key = (uint8_t)(port * 8u + (uint8_t)(7u - ch));
      process_key(key, vals[ch]);
    }
  }
  g_step++;
  if (g_step >= 8) {
    g_step = 0;
    if (AINSER64_NUM_MODULES > 1) {
      g_bank = (uint8_t)((g_bank + 1) % AINSER64_NUM_MODULES);
    } else {
      g_bank = 0;
    }
  }
}
