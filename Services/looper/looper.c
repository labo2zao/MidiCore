#include "Services/looper/looper.h"
#include "Services/midi/midi_delayq.h"
#include "Services/instrument/instrument_cfg.h"
#include "Services/humanize/humanize.h"
#include "cmsis_os2.h"
#include "ff.h"
#include <string.h>
#include <stdint.h>

#ifndef LOOPER_PPQN
#define LOOPER_PPQN 96u
#endif

#ifndef LOOPER_MAX_EVENTS
#define LOOPER_MAX_EVENTS 512u
#endif

#define LOOPER_MAGIC 0x4C4F4F50u /* 'LOOP' */
#define LOOPER_FMT_V1 1u

typedef struct {
  uint32_t tick;
  uint8_t  len;
  uint8_t  b0, b1, b2;
} looper_evt_t;

typedef struct {
  looper_state_t st;

  uint32_t loop_len_ticks;
  uint16_t loop_beats;
  looper_quant_t quant;
  uint8_t mute;

  uint32_t write_tick;
  uint32_t play_tick;
  uint32_t next_idx;
  uint32_t count;
  looper_evt_t ev[LOOPER_MAX_EVENTS];

  uint8_t active_notes[16][128];
} looper_track_t;

// NOTE: place the looper tracks in CCMRAM on STM32F4 (0x1000_0000)
// to preserve main SRAM for other subsystems.
// CCMRAM is NOT DMA accessible, which is fine here (pure CPU data).
static looper_track_t g_tr[LOOPER_TRACKS] __attribute__((section(".ccmram")));
static looper_transport_t g_tp = { .bpm=120, .ts_num=4, .ts_den=4, .auto_loop=1 };

static uint32_t g_ticks_per_ms_q16 = 0;
static uint32_t g_acc_q16 = 0;

static osMutexId_t g_mutex;

static void update_rate(void) {
  uint32_t bpm = g_tp.bpm;
  if (bpm < 20) bpm = 20;
  if (bpm > 300) bpm = 300;
  uint32_t ticks_per_sec_num = bpm * (uint32_t)LOOPER_PPQN;
  g_ticks_per_ms_q16 = (ticks_per_sec_num * 65536u) / 60000u;
  if (g_ticks_per_ms_q16 == 0) g_ticks_per_ms_q16 = 1;
}

static inline uint32_t beats_to_ticks(uint16_t beats) {
  return (uint32_t)beats * (uint32_t)LOOPER_PPQN;
}

static inline uint32_t quant_step_ticks(looper_quant_t q) {
  // Optimized with bit shifts for power-of-2 divisions
  if (q == LOOPER_QUANT_1_16) return (uint32_t)LOOPER_PPQN >> 2u;
  if (q == LOOPER_QUANT_1_8)  return (uint32_t)LOOPER_PPQN >> 1u;
  if (q == LOOPER_QUANT_1_4)  return (uint32_t)LOOPER_PPQN;
  return 0; // LOOPER_QUANT_OFF or default
}

static inline uint32_t quantize_tick(uint32_t t, uint32_t step) {
  if (!step) return t;
  uint32_t r = t % step;
  uint32_t half_step = step >> 1u;
  uint32_t down = t - r;
  return (r < half_step) ? down : (down + step);
}

static void clear_track(looper_track_t* t) {
  t->count = 0;
  t->loop_len_ticks = 0;
  t->write_tick = 0;
  t->play_tick = 0;
  t->next_idx = 0;
  memset(t->active_notes, 0, sizeof(t->active_notes));
}

static void sort_events(looper_track_t* t) {
  for (uint32_t i=1;i<t->count;i++) {
    looper_evt_t key = t->ev[i];
    uint32_t j = i;
    while (j>0 && t->ev[j-1].tick > key.tick) {
      t->ev[j] = t->ev[j-1];
      j--;
    }
    t->ev[j] = key;
  }
}

void looper_init(void) {
  memset(g_tr, 0, sizeof(g_tr));
  for (uint8_t i=0;i<LOOPER_TRACKS;i++) {
    g_tr[i].quant = LOOPER_QUANT_OFF;
    g_tr[i].loop_beats = 4;
    g_tr[i].mute = 0;
    g_tr[i].st = LOOPER_STATE_STOP;
  }
  const osMutexAttr_t attr = { .name = "looper" };
  g_mutex = osMutexNew(&attr);
  update_rate();
}

void looper_set_transport(const looper_transport_t* t) {
  if (!t) return;
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  g_tp = *t;
  if (g_tp.ts_num == 0) g_tp.ts_num = 4;
  if (g_tp.ts_den == 0) g_tp.ts_den = 4;
  update_rate();
  if (g_mutex) osMutexRelease(g_mutex);
}

void looper_get_transport(looper_transport_t* out) {
  if (!out) return;
  *out = g_tp;
}

void looper_set_tempo(uint16_t bpm) {
  looper_transport_t t = g_tp;
  t.bpm = bpm;
  looper_set_transport(&t);
}
uint16_t looper_get_tempo(void) { return g_tp.bpm; }

void looper_set_loop_beats(uint8_t track, uint16_t beats) {
  if (track >= LOOPER_TRACKS) return;
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  g_tr[track].loop_beats = beats;
  if (g_mutex) osMutexRelease(g_mutex);
}
uint16_t looper_get_loop_beats(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return g_tr[track].loop_beats;
}

void looper_set_quant(uint8_t track, looper_quant_t q) {
  if (track >= LOOPER_TRACKS) return;
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  g_tr[track].quant = q;
  if (g_mutex) osMutexRelease(g_mutex);
}
looper_quant_t looper_get_quant(uint8_t track) {
  if (track >= LOOPER_TRACKS) return LOOPER_QUANT_OFF;
  return g_tr[track].quant;
}

void looper_set_mute(uint8_t track, uint8_t mute) {
  if (track >= LOOPER_TRACKS) return;
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  g_tr[track].mute = mute ? 1 : 0;
  if (g_mutex) osMutexRelease(g_mutex);
}
uint8_t looper_get_mute(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return g_tr[track].mute;
}

void looper_clear(uint8_t track) {
  if (track >= LOOPER_TRACKS) return;
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  clear_track(&g_tr[track]);
  if (g_mutex) osMutexRelease(g_mutex);
}

looper_state_t looper_get_state(uint8_t track) {
  if (track >= LOOPER_TRACKS) return LOOPER_STATE_STOP;
  return g_tr[track].st;
}

static uint32_t ensure_loop_len(looper_track_t* t) {
  if (t->loop_len_ticks) return t->loop_len_ticks;
  if (t->loop_beats) {
    t->loop_len_ticks = beats_to_ticks(t->loop_beats);
    if (t->loop_len_ticks < LOOPER_PPQN) t->loop_len_ticks = LOOPER_PPQN;
    return t->loop_len_ticks;
  }
  return 0;
}

void looper_set_state(uint8_t track, looper_state_t st) {
  if (track >= LOOPER_TRACKS) return;
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);

  looper_track_t* t = &g_tr[track];
  looper_state_t prev = t->st;

  if (st == LOOPER_STATE_REC) {
    clear_track(t);
    (void)ensure_loop_len(t);
  } else if (prev == LOOPER_STATE_REC && st == LOOPER_STATE_STOP) {
    if (t->loop_len_ticks == 0) {
      uint32_t len = t->write_tick;
      if (len < LOOPER_PPQN) len = LOOPER_PPQN;
      t->loop_len_ticks = len;
    }
    sort_events(t);
    t->play_tick = 0;
    t->next_idx = 0;
    memset(t->active_notes, 0, sizeof(t->active_notes));
  } else if (st == LOOPER_STATE_PLAY) {
    if (ensure_loop_len(t) == 0) st = LOOPER_STATE_STOP;
    t->play_tick = 0;
    t->next_idx = 0;
    memset(t->active_notes, 0, sizeof(t->active_notes));
  } else if (st == LOOPER_STATE_OVERDUB) {
    if (ensure_loop_len(t) == 0) st = LOOPER_STATE_STOP;
  }

  t->st = st;
  if (g_mutex) osMutexRelease(g_mutex);
}

static uint8_t is_note_on(uint8_t st, uint8_t v) { return ((st & 0xF0) == 0x90) && v != 0; }
static uint8_t is_note_off(uint8_t st, uint8_t v) { return ((st & 0xF0) == 0x80) || (((st & 0xF0) == 0x90) && v == 0); }

void looper_on_router_msg(uint8_t in_node, const router_msg_t* msg) {
  (void)in_node;
  if (!msg) return;

  uint8_t len = 0;
  if (msg->type == ROUTER_MSG_2B) len = 2;
  else if (msg->type == ROUTER_MSG_3B) len = 3;
  else return;

  uint8_t status = msg->b0;
  if ((status & 0x80) == 0) return;

  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  for (uint8_t tr=0; tr<LOOPER_TRACKS; tr++) {
    looper_track_t* t = &g_tr[tr];
    if (t->st != LOOPER_STATE_REC && t->st != LOOPER_STATE_OVERDUB) continue;
    if (t->count >= LOOPER_MAX_EVENTS) continue;

    uint32_t tick = (t->st == LOOPER_STATE_REC) ? t->write_tick : t->play_tick;
    uint32_t step = quant_step_ticks(t->quant);
    tick = quantize_tick(tick, step);

    if (t->loop_len_ticks) tick %= t->loop_len_ticks;

    looper_evt_t* e = &t->ev[t->count++];
    e->tick = tick;
    e->len = len;
    e->b0 = msg->b0;
    e->b1 = msg->b1;
    e->b2 = msg->b2;
  }
  if (g_mutex) osMutexRelease(g_mutex);
}

static void emit_msg3(uint8_t b0, uint8_t b1, uint8_t b2) {
  router_msg_t m;
  m.type = ROUTER_MSG_3B;
  m.b0 = b0; m.b1 = b1; m.b2 = b2;
  const instrument_cfg_t* cfg = instrument_cfg_get();
  int8_t j = humanize_time_ms(cfg, HUMAN_APPLY_LOOPER);
  uint16_t d = (j < 0) ? 0u : (uint16_t)j;
  midi_delayq_send(ROUTER_NODE_LOOPER, &m, d);
}
static void emit_msg2(uint8_t b0, uint8_t b1) {
  router_msg_t m;
  m.type = ROUTER_MSG_2B;
  m.b0 = b0; m.b1 = b1; m.b2 = 0;
  const instrument_cfg_t* cfg = instrument_cfg_get();
  int8_t j = humanize_time_ms(cfg, HUMAN_APPLY_LOOPER);
  uint16_t d = (j < 0) ? 0u : (uint16_t)j;
  midi_delayq_send(ROUTER_NODE_LOOPER, &m, d);
}

static void send_all_note_off(looper_track_t* t) {
  for (uint8_t ch=0; ch<16; ch++) {
    for (uint16_t note=0; note<128; note++) {
      if (t->active_notes[ch][note]) {
        emit_msg3((uint8_t)(0x80 | ch), (uint8_t)note, 0);
        t->active_notes[ch][note] = 0;
      }
    }
  }
}

static void note_tracker_update(looper_track_t* t, uint8_t b0, uint8_t b1, uint8_t b2) {
  uint8_t ch = b0 & 0x0F;
  if (is_note_on(b0, b2)) t->active_notes[ch][b1] = 1;
  else if (is_note_off(b0, b2)) t->active_notes[ch][b1] = 0;
}

static void emit_due_events(looper_track_t* t) {
  while (t->next_idx < t->count && t->ev[t->next_idx].tick == t->play_tick) {
    looper_evt_t* e = &t->ev[t->next_idx];
    if (!t->mute) {
      if (e->len == 2) emit_msg2(e->b0, e->b1);
      else emit_msg3(e->b0, e->b1, e->b2);
      if (e->len == 3) note_tracker_update(t, e->b0, e->b1, e->b2);
    }
    t->next_idx++;
  }
}

void looper_tick_1ms(void) {
  g_acc_q16 += g_ticks_per_ms_q16;
  uint32_t adv = g_acc_q16 >> 16;
  if (!adv) return;
  g_acc_q16 &= 0xFFFFu;

  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);

  for (uint8_t tr=0; tr<LOOPER_TRACKS; tr++) {
    looper_track_t* t = &g_tr[tr];

    if (t->st == LOOPER_STATE_REC) {
      t->write_tick += adv;

      if (g_tp.auto_loop && t->loop_len_ticks) {
        if (t->write_tick >= t->loop_len_ticks) {
          sort_events(t);
          t->st = LOOPER_STATE_PLAY;
          t->play_tick = 0;
          t->next_idx = 0;
          t->write_tick = t->loop_len_ticks;
          memset(t->active_notes, 0, sizeof(t->active_notes));
        }
      }
      if (t->write_tick > 0x7FFFFFFFu) t->write_tick = 0x7FFFFFFFu;
    }

    if (t->st == LOOPER_STATE_PLAY || t->st == LOOPER_STATE_OVERDUB) {
      if (t->loop_len_ticks == 0) continue;

      for (uint32_t k=0; k<adv; k++) {
        emit_due_events(t);
        t->play_tick++;
        if (t->play_tick >= t->loop_len_ticks) {
          send_all_note_off(t);
          t->play_tick = 0;
          t->next_idx = 0;
        }
      }
    }
  }

  if (g_mutex) osMutexRelease(g_mutex);
}

// ---------- Persistence (binary) ----------
typedef struct __attribute__((packed)) {
  uint32_t magic;
  uint16_t fmt;
  uint16_t ppqn;
  uint16_t bpm;
  uint16_t loop_beats;
  uint32_t loop_len_ticks;
  uint32_t count;
  uint8_t  quant;
  uint8_t  mute;
  uint8_t  ts_num;
  uint8_t  ts_den;
} looper_file_hdr_t;

int looper_save_track(uint8_t track, const char* filename) {
  if (track >= LOOPER_TRACKS || !filename) return -1;
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);

  looper_track_t* t = &g_tr[track];
  FIL f;
  if (f_open(&f, filename, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
    if (g_mutex) osMutexRelease(g_mutex);
    return -2;
  }

  looper_file_hdr_t hdr;
  hdr.magic = LOOPER_MAGIC;
  hdr.fmt = LOOPER_FMT_V1;
  hdr.ppqn = (uint16_t)LOOPER_PPQN;
  hdr.bpm = g_tp.bpm;
  hdr.loop_beats = t->loop_beats;
  hdr.loop_len_ticks = t->loop_len_ticks;
  hdr.count = t->count;
  hdr.quant = (uint8_t)t->quant;
  hdr.mute = t->mute;
  hdr.ts_num = g_tp.ts_num;
  hdr.ts_den = g_tp.ts_den;

  UINT bw=0;
  if (f_write(&f, &hdr, sizeof(hdr), &bw) != FR_OK || bw != sizeof(hdr)) {
    f_close(&f);
    if (g_mutex) osMutexRelease(g_mutex);
    return -3;
  }

  for (uint32_t i=0;i<t->count;i++) {
    if (f_write(&f, &t->ev[i], sizeof(looper_evt_t), &bw) != FR_OK || bw != sizeof(looper_evt_t)) {
      f_close(&f);
      if (g_mutex) osMutexRelease(g_mutex);
      return -4;
    }
  }

  f_close(&f);
  if (g_mutex) osMutexRelease(g_mutex);
  return 0;
}

int looper_load_track(uint8_t track, const char* filename) {
  if (track >= LOOPER_TRACKS || !filename) return -1;

  FIL f;
  if (f_open(&f, filename, FA_READ) != FR_OK) return -2;

  looper_file_hdr_t hdr;
  UINT br=0;
  if (f_read(&f, &hdr, sizeof(hdr), &br) != FR_OK || br != sizeof(hdr)) {
    f_close(&f);
    return -3;
  }
  if (hdr.magic != LOOPER_MAGIC || hdr.fmt != LOOPER_FMT_V1) {
    f_close(&f);
    return -4;
  }
  if (hdr.count > LOOPER_MAX_EVENTS) {
    f_close(&f);
    return -5;
  }

  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);

  looper_track_t* t = &g_tr[track];
  clear_track(t);
  t->loop_beats = hdr.loop_beats;
  t->loop_len_ticks = hdr.loop_len_ticks;
  t->count = hdr.count;
  t->quant = (looper_quant_t)hdr.quant;
  t->mute = hdr.mute;

  g_tp.bpm = hdr.bpm;
  g_tp.ts_num = hdr.ts_num ? hdr.ts_num : 4;
  g_tp.ts_den = hdr.ts_den ? hdr.ts_den : 4;
  update_rate();

  for (uint32_t i=0;i<t->count;i++) {
    if (f_read(&f, &t->ev[i], sizeof(looper_evt_t), &br) != FR_OK || br != sizeof(looper_evt_t)) {
      f_close(&f);
      if (g_mutex) osMutexRelease(g_mutex);
      return -6;
    }
  }
  sort_events(t);
  t->st = LOOPER_STATE_STOP;

  if (g_mutex) osMutexRelease(g_mutex);
  f_close(&f);
  return 0;
}


// ---- UI/Debug helpers ----
uint32_t looper_get_loop_len_ticks(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  uint32_t v = 0;
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  v = g_tr[track].loop_len_ticks;
  if (g_mutex) osMutexRelease(g_mutex);
  return v;
}

uint32_t looper_export_events(uint8_t track, looper_event_view_t* out, uint32_t max) {
  if (track >= LOOPER_TRACKS || !out || max == 0) return 0;
  uint32_t n = 0;
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  looper_track_t* t = &g_tr[track];
  n = t->count;
  if (n > max) n = max;
  for (uint32_t i=0; i<n; i++) {
    out[i].idx = i;
    out[i].tick = t->ev[i].tick;
    out[i].len  = t->ev[i].len;
    out[i].b0   = t->ev[i].b0;
    out[i].b1   = t->ev[i].b1;
    out[i].b2   = t->ev[i].b2;
  }
  if (g_mutex) osMutexRelease(g_mutex);
  return n;
}

static void sort_events(looper_track_t* t); // forward

int looper_edit_event(uint8_t track, uint32_t idx, uint32_t new_tick,
                      uint8_t len, uint8_t b0, uint8_t b1, uint8_t b2) {
  if (track >= LOOPER_TRACKS) return -1;
  if (len != 2 && len != 3) return -2;

  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  looper_track_t* t = &g_tr[track];
  if (idx >= t->count) {
    if (g_mutex) osMutexRelease(g_mutex);
    return -3;
  }

  if (t->loop_len_ticks) new_tick %= t->loop_len_ticks;

  t->ev[idx].tick = new_tick;
  t->ev[idx].len = len;
  t->ev[idx].b0 = b0;
  t->ev[idx].b1 = b1;
  t->ev[idx].b2 = b2;

  // keep events ordered after edit
  sort_events(t);

  if (g_mutex) osMutexRelease(g_mutex);
  return 0;
}


int looper_add_event(uint8_t track, uint32_t tick, uint8_t len, uint8_t b0, uint8_t b1, uint8_t b2) {
  if (track >= LOOPER_TRACKS) return -1;
  if (len != 2 && len != 3) return -2;

  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  looper_track_t* t = &g_tr[track];
  if (t->count >= LOOPER_MAX_EVENTS) {
    if (g_mutex) osMutexRelease(g_mutex);
    return -3;
  }
  if (t->loop_len_ticks) tick %= t->loop_len_ticks;

  looper_evt_t* e = &t->ev[t->count++];
  e->tick = tick;
  e->len = len;
  e->b0 = b0; e->b1 = b1; e->b2 = b2;

  sort_events(t);

  if (g_mutex) osMutexRelease(g_mutex);
  return 0;
}

int looper_delete_event(uint8_t track, uint32_t idx) {
  if (track >= LOOPER_TRACKS) return -1;
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  looper_track_t* t = &g_tr[track];
  if (idx >= t->count) {
    if (g_mutex) osMutexRelease(g_mutex);
    return -2;
  }
  for (uint32_t i=idx; i+1<t->count; i++) {
    t->ev[i] = t->ev[i+1];
  }
  t->count--;
  if (t->next_idx > t->count) t->next_idx = t->count;
  sort_events(t);
  if (g_mutex) osMutexRelease(g_mutex);
  return 0;
}

// ---- Song Mode / Scene Management ----

// Scene storage: snapshot of track states
typedef struct {
  uint8_t has_clip;
  uint16_t loop_beats;
  uint16_t loop_len_ticks;
  looper_state_t saved_state;
  // For simplicity, we don't copy the full event buffer here
  // Just track metadata. In a full implementation, you'd need
  // to save/restore event data or reference stored clips.
} scene_slot_t;

static scene_slot_t g_scenes[LOOPER_SCENES][LOOPER_TRACKS];
static uint8_t g_current_scene = 0;

/**
 * @brief Get clip info for a specific scene and track
 */
looper_scene_clip_t looper_get_scene_clip(uint8_t scene, uint8_t track) {
  looper_scene_clip_t clip = {0, 0};
  if (scene >= LOOPER_SCENES || track >= LOOPER_TRACKS) return clip;
  
  clip.has_clip = g_scenes[scene][track].has_clip;
  clip.loop_beats = g_scenes[scene][track].loop_beats;
  return clip;
}

/**
 * @brief Set current scene
 */
void looper_set_current_scene(uint8_t scene) {
  if (scene < LOOPER_SCENES) {
    g_current_scene = scene;
  }
}

/**
 * @brief Get current scene
 */
uint8_t looper_get_current_scene(void) {
  return g_current_scene;
}

/**
 * @brief Copy current track state to a scene slot
 */
void looper_save_to_scene(uint8_t scene, uint8_t track) {
  if (scene >= LOOPER_SCENES || track >= LOOPER_TRACKS) return;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  looper_track_t* t = &g_tr[track];
  scene_slot_t* slot = &g_scenes[scene][track];
  
  // Save metadata
  slot->has_clip = (t->count > 0 || t->loop_beats > 0) ? 1 : 0;
  slot->loop_beats = t->loop_beats;
  slot->loop_len_ticks = t->loop_len_ticks;
  slot->saved_state = t->st;
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Load a scene's track state to current
 */
void looper_load_from_scene(uint8_t scene, uint8_t track) {
  if (scene >= LOOPER_SCENES || track >= LOOPER_TRACKS) return;
  
  scene_slot_t* slot = &g_scenes[scene][track];
  if (!slot->has_clip) return;  // Nothing to load
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  looper_track_t* t = &g_tr[track];
  
  // Restore metadata (in a full implementation, restore event data too)
  t->loop_beats = slot->loop_beats;
  t->loop_len_ticks = slot->loop_len_ticks;
  // Note: Not changing state automatically - user controls playback
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Trigger scene playback (load all tracks from scene)
 */
void looper_trigger_scene(uint8_t scene) {
  if (scene >= LOOPER_SCENES) return;
  
  g_current_scene = scene;
  
  // Load all tracks from this scene and start playback
  for (uint8_t track = 0; track < LOOPER_TRACKS; track++) {
    scene_slot_t* slot = &g_scenes[scene][track];
    
    if (slot->has_clip) {
      looper_load_from_scene(scene, track);
      // Start playback if there's a clip
      looper_set_state(track, LOOPER_STATE_PLAY);
    } else {
      // Stop tracks that don't have clips in this scene
      looper_set_state(track, LOOPER_STATE_STOP);
    }
  }
}

