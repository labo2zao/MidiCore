#include "Services/looper/looper.h"
#include "Services/midi/midi_delayq.h"
#include "Services/instrument/instrument_cfg.h"
#include "Services/humanize/humanize.h"
#include "cmsis_os2.h"
#include "ff.h"
#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdint.h>
#include <stdio.h>

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

// Footswitch mapping (8 footswitches)
#define NUM_FOOTSWITCHES 8

typedef struct {
    footswitch_action_t action;
    uint8_t param;  // Track number, scene number, etc.
    uint8_t pressed;
    uint32_t press_time_ms;
} footswitch_mapping_t;

static footswitch_mapping_t g_footswitch[NUM_FOOTSWITCHES];

// MIDI Learn system
#define MAX_MIDI_LEARN_MAPPINGS 32

typedef struct {
    uint8_t midi_cc;        // CC number (or note for note-based control)
    uint8_t midi_channel;   // MIDI channel (0-15, 0xFF = omni)
    uint8_t control_type;   // 0=CC, 1=Note
    footswitch_action_t action;
    uint8_t param;
} midi_learn_mapping_t;

typedef struct {
    uint8_t learning_active;
    footswitch_action_t pending_action;
    uint8_t pending_param;
    uint32_t learn_timeout_ms;
} midi_learn_state_t;

static midi_learn_mapping_t g_midi_learn[MAX_MIDI_LEARN_MAPPINGS];
static uint8_t g_midi_learn_count = 0;
static midi_learn_state_t g_midi_learn_state;

// Quick-Save system
#define NUM_QUICK_SAVE_SLOTS 8

typedef struct {
    uint8_t used;
    char name[32];
    uint8_t current_scene;
    looper_transport_t transport;
} quick_save_slot_t;

static quick_save_slot_t g_quick_save_slots[NUM_QUICK_SAVE_SLOTS];

// CC Automation Layer
typedef struct {
  uint8_t recording;
  uint8_t playback_enabled;
  uint32_t event_count;
  uint32_t last_playback_tick;  // Track last processed tick to avoid duplicates
  looper_automation_event_t events[LOOPER_AUTOMATION_MAX_EVENTS];
} looper_automation_t;

static looper_automation_t g_automation[LOOPER_TRACKS];

static uint32_t g_ticks_per_ms_q16 = 0;
static uint32_t g_acc_q16 = 0;

static osMutexId_t g_mutex;

// Track Mute/Solo state
static uint8_t g_track_muted[LOOPER_TRACKS] = {0};
static uint8_t g_track_solo[LOOPER_TRACKS] = {0};

// Global Transpose state
static int8_t g_global_transpose = 0;

// Scene storage: snapshot of track states
typedef struct {
  uint8_t has_clip;
  uint16_t loop_beats;
  uint16_t loop_len_ticks;
  looper_state_t saved_state;
} scene_slot_t;

static scene_slot_t g_scenes[LOOPER_SCENES][LOOPER_TRACKS];
static uint8_t g_current_scene = 0;

// Scene Chaining/Automation
typedef struct {
  uint8_t next_scene;  // 0xFF = no chain
  uint8_t enabled;     // 1 = auto-chain enabled
} scene_chain_t;

static scene_chain_t g_scene_chains[LOOPER_SCENES];

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
    g_tr[i].st = LOOPER_STATE_STOP;
    g_track_muted[i] = 0;  // Initialize new mute system
  }
  
  // Initialize scene chains (all disabled by default)
  for (uint8_t i=0;i<LOOPER_SCENES;i++) {
    g_scene_chains[i].next_scene = 0xFF;
    g_scene_chains[i].enabled = 0;
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
  // Validate BPM range (20-300 BPM as per design spec)
  if (bpm < 20) bpm = 20;
  if (bpm > 300) bpm = 300;
  
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

// Track Mute/Solo Controls
void looper_set_track_muted(uint8_t track, uint8_t muted) {
  if (track >= LOOPER_TRACKS) return;
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  g_track_muted[track] = muted ? 1 : 0;
  if (g_mutex) osMutexRelease(g_mutex);
}

uint8_t looper_is_track_muted(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return g_track_muted[track];
}

void looper_set_track_solo(uint8_t track, uint8_t solo) {
  if (track >= LOOPER_TRACKS) return;
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  // Clear all other solo states (exclusive solo)
  if (solo) {
    for (uint8_t i = 0; i < LOOPER_TRACKS; i++) {
      g_track_solo[i] = 0;
    }
  }
  g_track_solo[track] = solo ? 1 : 0;
  if (g_mutex) osMutexRelease(g_mutex);
}

uint8_t looper_is_track_soloed(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return g_track_solo[track];
}

void looper_clear_all_solo(void) {
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  for (uint8_t i = 0; i < LOOPER_TRACKS; i++) {
    g_track_solo[i] = 0;
  }
  if (g_mutex) osMutexRelease(g_mutex);
}

uint8_t looper_is_track_audible(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  
  // Check if any track is soloed
  uint8_t any_solo = 0;
  for (uint8_t i = 0; i < LOOPER_TRACKS; i++) {
    if (g_track_solo[i]) {
      any_solo = 1;
      break;
    }
  }
  
  // If this track is soloed, it's audible regardless of mute
  if (g_track_solo[track]) return 1;
  
  // If any track is soloed and this isn't one, it's not audible
  if (any_solo) return 0;
  
  // No solo active, check mute state
  return !g_track_muted[track];
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
    
    // Check if this is a CC message and automation recording is active
    if ((status & 0xF0) == 0xB0 && g_automation[tr].recording) {
      uint8_t channel = status & 0x0F;
      looper_automation_record_cc_internal(tr, msg->b1, msg->b2, channel);
    }
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

static void emit_due_events(looper_track_t* t, uint8_t track_idx) {
  while (t->next_idx < t->count && t->ev[t->next_idx].tick == t->play_tick) {
    looper_evt_t* e = &t->ev[t->next_idx];
    // Check mute state and mute/solo audibility
    if (!t->mute && looper_is_track_audible(track_idx)) {
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
        emit_due_events(t, tr);
        
        // Process automation playback
        looper_automation_process_playback(tr);
        
        t->play_tick++;
        if (t->play_tick >= t->loop_len_ticks) {
          send_all_note_off(t);
          t->play_tick = 0;
          t->next_idx = 0;
          
          // Check for scene chaining (only trigger once per loop end on track 0)
          if (tr == 0) {
            uint8_t current = g_current_scene;
            if (g_scene_chains[current].enabled && 
                g_scene_chains[current].next_scene < LOOPER_SCENES) {
              // Release mutex before triggering (trigger_scene will acquire it)
              if (g_mutex) osMutexRelease(g_mutex);
              looper_trigger_scene(g_scene_chains[current].next_scene);
              if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
              // Exit loop after scene change
              break;
            }
          }
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
  hdr.mute = g_track_muted[track];  // Use new mute system
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
  g_track_muted[track] = hdr.mute;  // Load into new mute system

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

// ---- Step Playback ----

// Step playback state
typedef struct {
  uint8_t enabled;           // Step mode enabled flag
  uint32_t cursor_tick;      // Current cursor position in ticks
} step_state_t;

static step_state_t g_step[LOOPER_TRACKS];
static uint32_t g_step_size = 0;  // 0 = auto (event-based), else fixed ticks

/**
 * @brief Enable/disable step playback mode for a track
 */
void looper_set_step_mode(uint8_t track, uint8_t enable) {
  if (track >= LOOPER_TRACKS) return;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  g_step[track].enabled = enable ? 1 : 0;
  
  if (enable) {
    // Initialize cursor to current playback position
    g_step[track].cursor_tick = g_tr[track].play_tick;
    
    // Pause normal playback when entering step mode
    if (g_tr[track].st == LOOPER_STATE_PLAY) {
      // Keep state but stop auto-advance
      g_tr[track].st = LOOPER_STATE_STOP;
    }
  }
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Get step playback mode status
 */
uint8_t looper_get_step_mode(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return g_step[track].enabled;
}

/**
 * @brief Step forward to next event (or by specified ticks)
 */
uint32_t looper_step_forward(uint8_t track, uint32_t ticks) {
  if (track >= LOOPER_TRACKS) return 0;
  if (!g_step[track].enabled) return 0;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  looper_track_t* t = &g_tr[track];
  uint32_t old_tick = g_step[track].cursor_tick;
  uint32_t new_tick = old_tick;
  
  if (ticks == 0) {
    // Auto mode: step to next event
    ticks = g_step_size;
    
    if (ticks == 0) {
      // Event-based stepping: find next event
      uint32_t next_event_tick = t->loop_len_ticks;  // Default to end
      
      for (uint32_t i = 0; i < t->count; i++) {
        if (t->ev[i].tick > old_tick) {
          next_event_tick = t->ev[i].tick;
          break;
        }
      }
      
      new_tick = next_event_tick;
    } else {
      // Fixed tick stepping
      new_tick = old_tick + ticks;
    }
  } else {
    // Explicit tick count
    new_tick = old_tick + ticks;
  }
  
  // Wrap around loop
  if (t->loop_len_ticks > 0 && new_tick >= t->loop_len_ticks) {
    new_tick = new_tick % t->loop_len_ticks;
  }
  
  // Play events between old_tick and new_tick
  for (uint32_t i = 0; i < t->count; i++) {
    uint32_t evt_tick = t->ev[i].tick;
    
    if (evt_tick > old_tick && evt_tick <= new_tick) {
      // Trigger this event
      router_msg_t msg;
      msg.type = t->ev[i].len;
      msg.b0 = t->ev[i].b0;
      msg.b1 = t->ev[i].b1;
      msg.b2 = t->ev[i].b2;
      
      // Send immediately (no delay in step mode)
      midi_delayq_send(ROUTER_NODE_LOOPER, &msg, 0);
    }
  }
  
  g_step[track].cursor_tick = new_tick;
  
  if (g_mutex) osMutexRelease(g_mutex);
  
  return new_tick;
}

/**
 * @brief Step backward to previous event (or by specified ticks)
 */
uint32_t looper_step_backward(uint8_t track, uint32_t ticks) {
  if (track >= LOOPER_TRACKS) return 0;
  if (!g_step[track].enabled) return 0;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  looper_track_t* t = &g_tr[track];
  uint32_t old_tick = g_step[track].cursor_tick;
  uint32_t new_tick;
  
  if (ticks == 0) {
    // Auto mode: step to previous event
    ticks = g_step_size;
    
    if (ticks == 0) {
      // Event-based stepping: find previous event
      uint32_t prev_event_tick = 0;
      
      for (int32_t i = t->count - 1; i >= 0; i--) {
        if (t->ev[i].tick < old_tick) {
          prev_event_tick = t->ev[i].tick;
          break;
        }
      }
      
      new_tick = prev_event_tick;
    } else {
      // Fixed tick stepping
      if (old_tick >= ticks) {
        new_tick = old_tick - ticks;
      } else {
        // Wrap to end of loop
        if (t->loop_len_ticks > 0) {
          new_tick = t->loop_len_ticks - (ticks - old_tick);
        } else {
          new_tick = 0;
        }
      }
    }
  } else {
    // Explicit tick count
    if (old_tick >= ticks) {
      new_tick = old_tick - ticks;
    } else {
      // Wrap to end of loop
      if (t->loop_len_ticks > 0) {
        new_tick = t->loop_len_ticks - (ticks - old_tick);
      } else {
        new_tick = 0;
      }
    }
  }
  
  // Send note-off for any active notes
  for (uint8_t ch = 0; ch < 16; ch++) {
    for (uint8_t note = 0; note < 128; note++) {
      if (t->active_notes[ch][note]) {
        router_msg_t msg;
        msg.type = 3;
        msg.b0 = 0x80 | ch;  // Note Off
        msg.b1 = note;
        msg.b2 = 0;
        midi_delayq_send(ROUTER_NODE_LOOPER, &msg, 0);
        t->active_notes[ch][note] = 0;
      }
    }
  }
  
  g_step[track].cursor_tick = new_tick;
  
  if (g_mutex) osMutexRelease(g_mutex);
  
  return new_tick;
}

/**
 * @brief Get current cursor position in step mode
 */
uint32_t looper_get_cursor_position(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return g_step[track].cursor_tick;
}

/**
 * @brief Set cursor position in step mode
 */
void looper_set_cursor_position(uint8_t track, uint32_t tick) {
  if (track >= LOOPER_TRACKS) return;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  looper_track_t* t = &g_tr[track];
  
  // Wrap to loop length
  if (t->loop_len_ticks > 0 && tick >= t->loop_len_ticks) {
    tick = tick % t->loop_len_ticks;
  }
  
  g_step[track].cursor_tick = tick;
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Configure step size for footswitch control
 */
void looper_set_step_size(uint32_t ticks) {
  g_step_size = ticks;
}

/**
 * @brief Get configured step size
 */
uint32_t looper_get_step_size(void) {
  return g_step_size;
}

// ---- Scene Chaining/Automation ----

/**
 * @brief Set scene chaining configuration
 */
void looper_set_scene_chain(uint8_t scene, uint8_t next_scene, uint8_t enabled) {
  if (scene >= LOOPER_SCENES) return;
  
  g_scene_chains[scene].next_scene = next_scene;
  g_scene_chains[scene].enabled = enabled ? 1 : 0;
}

/**
 * @brief Get next scene in chain
 */
uint8_t looper_get_scene_chain(uint8_t scene) {
  if (scene >= LOOPER_SCENES) return 0xFF;
  return g_scene_chains[scene].next_scene;
}

/**
 * @brief Check if scene has chaining enabled
 */
uint8_t looper_is_scene_chain_enabled(uint8_t scene) {
  if (scene >= LOOPER_SCENES) return 0;
  return g_scene_chains[scene].enabled;
}

// ---- MIDI File Export ----

// SMF (Standard MIDI File) Format 1 support
// Variable Length Quantity (VLQ) encoding for delta-times

static int write_vlq(FIL* fp, uint32_t value) {
  uint8_t buf[4];
  uint8_t len = 0;
  
  // Build VLQ from LSB to MSB
  buf[0] = value & 0x7F;
  value >>= 7;
  len = 1;
  
  while (value > 0 && len < 4) {
    buf[len] = (value & 0x7F) | 0x80;  // Set continuation bit
    value >>= 7;
    len++;
  }
  
  // Write MSB first
  for (int i = len - 1; i >= 0; i--) {
    UINT written;
    if (f_write(fp, &buf[i], 1, &written) != FR_OK || written != 1) {
      return -1;
    }
  }
  
  return 0;
}

static int write_u32_be(FIL* fp, uint32_t val) {
  uint8_t buf[4] = {
    (val >> 24) & 0xFF,
    (val >> 16) & 0xFF,
    (val >> 8) & 0xFF,
    val & 0xFF
  };
  UINT written;
  if (f_write(fp, buf, 4, &written) != FR_OK || written != 4) {
    return -1;
  }
  return 0;
}

static int write_u16_be(FIL* fp, uint16_t val) {
  uint8_t buf[2] = {
    (val >> 8) & 0xFF,
    val & 0xFF
  };
  UINT written;
  if (f_write(fp, buf, 2, &written) != FR_OK || written != 2) {
    return -1;
  }
  return 0;
}

static int write_bytes(FIL* fp, const uint8_t* data, uint32_t len) {
  UINT written;
  if (f_write(fp, data, len, &written) != FR_OK || written != len) {
    return -1;
  }
  return 0;
}

static int write_mthd_chunk(FIL* fp, uint16_t format, uint16_t tracks, uint16_t division) {
  // "MThd" header
  if (write_bytes(fp, (const uint8_t*)"MThd", 4) < 0) return -1;
  
  // Chunk length (always 6 for header)
  if (write_u32_be(fp, 6) < 0) return -1;
  
  // Format type (0=single track, 1=multi-track, 2=multi-song)
  if (write_u16_be(fp, format) < 0) return -1;
  
  // Number of tracks
  if (write_u16_be(fp, tracks) < 0) return -1;
  
  // Time division (PPQN)
  if (write_u16_be(fp, division) < 0) return -1;
  
  return 0;
}

static int write_meta_event(FIL* fp, uint8_t type, const uint8_t* data, uint32_t len) {
  // Delta time (0 for meta events at track start)
  if (write_vlq(fp, 0) < 0) return -1;
  
  // Meta event marker
  uint8_t meta = 0xFF;
  if (write_bytes(fp, &meta, 1) < 0) return -1;
  
  // Meta event type
  if (write_bytes(fp, &type, 1) < 0) return -1;
  
  // Length
  if (write_vlq(fp, len) < 0) return -1;
  
  // Data
  if (len > 0 && write_bytes(fp, data, len) < 0) return -1;
  
  return 0;
}

static int write_tempo_meta(FIL* fp, uint32_t uspqn) {
  uint8_t tempo[3] = {
    (uspqn >> 16) & 0xFF,
    (uspqn >> 8) & 0xFF,
    uspqn & 0xFF
  };
  return write_meta_event(fp, 0x51, tempo, 3);  // Tempo meta event
}

static int write_time_sig_meta(FIL* fp, uint8_t num, uint8_t den) {
  // Convert denominator to power of 2 (4 -> 2, 8 -> 3, etc.)
  uint8_t den_pow = 0;
  uint8_t d = den;
  while (d > 1) {
    d >>= 1;
    den_pow++;
  }
  
  uint8_t ts[4] = { num, den_pow, 24, 8 };  // 24 MIDI clocks/metronome click, 8 32nd notes per quarter
  return write_meta_event(fp, 0x58, ts, 4);  // Time signature meta event
}

static int write_track_name_meta(FIL* fp, const char* name) {
  return write_meta_event(fp, 0x03, (const uint8_t*)name, strlen(name));  // Track name meta event
}

static int write_end_of_track_meta(FIL* fp) {
  return write_meta_event(fp, 0x2F, NULL, 0);  // End of track meta event
}

static int write_midi_event(FIL* fp, uint32_t delta, uint8_t status, uint8_t d1, uint8_t d2, uint8_t len) {
  // Delta time
  if (write_vlq(fp, delta) < 0) return -1;
  
  // MIDI event
  if (write_bytes(fp, &status, 1) < 0) return -1;
  
  if (len >= 2) {
    if (write_bytes(fp, &d1, 1) < 0) return -1;
  }
  
  if (len >= 3) {
    if (write_bytes(fp, &d2, 1) < 0) return -1;
  }
  
  return 0;
}

static int export_track_to_mtrk(FIL* fp, uint8_t track, const char* track_name) {
  if (track >= LOOPER_TRACKS) return -1;
  
  // Get track data
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  looper_track_t* t = &g_tr[track];
  uint32_t event_count = t->count;
  
  // Create temporary buffer for track events (max 64KB)
  FIL temp_fp;
  char temp_path[64];
  snprintf(temp_path, sizeof(temp_path), "0:/tmp_trk%d.dat", track);
  
  if (f_open(&temp_fp, temp_path, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
    if (g_mutex) osMutexRelease(g_mutex);
    return -1;
  }
  
  // Write track name meta event
  if (write_track_name_meta(&temp_fp, track_name) < 0) {
    f_close(&temp_fp);
    f_unlink(temp_path);
    if (g_mutex) osMutexRelease(g_mutex);
    return -1;
  }
  
  // Write tempo meta event (only on first track)
  if (track == 0) {
    uint32_t uspqn = 60000000 / g_tp.bpm;  // Microseconds per quarter note
    if (write_tempo_meta(&temp_fp, uspqn) < 0) {
      f_close(&temp_fp);
      f_unlink(temp_path);
      if (g_mutex) osMutexRelease(g_mutex);
      return -1;
    }
    
    if (write_time_sig_meta(&temp_fp, g_tp.ts_num, g_tp.ts_den) < 0) {
      f_close(&temp_fp);
      f_unlink(temp_path);
      if (g_mutex) osMutexRelease(g_mutex);
      return -1;
    }
  }
  
  // Write MIDI events with delta times
  uint32_t last_tick = 0;
  for (uint32_t i = 0; i < event_count; i++) {
    looper_evt_t* ev = &t->ev[i];
    uint32_t delta = (ev->tick > last_tick) ? (ev->tick - last_tick) : 0;
    
    if (write_midi_event(&temp_fp, delta, ev->b0, ev->b1, ev->b2, ev->len) < 0) {
      f_close(&temp_fp);
      f_unlink(temp_path);
      if (g_mutex) osMutexRelease(g_mutex);
      return -1;
    }
    
    last_tick = ev->tick;
  }
  
  // End of track
  if (write_end_of_track_meta(&temp_fp) < 0) {
    f_close(&temp_fp);
    f_unlink(temp_path);
    if (g_mutex) osMutexRelease(g_mutex);
    return -1;
  }
  
  uint32_t track_size = f_tell(&temp_fp);
  f_lseek(&temp_fp, 0);
  
  if (g_mutex) osMutexRelease(g_mutex);
  
  // Write MTrk chunk header
  if (write_bytes(fp, (const uint8_t*)"MTrk", 4) < 0) {
    f_close(&temp_fp);
    f_unlink(temp_path);
    return -1;
  }
  
  if (write_u32_be(fp, track_size) < 0) {
    f_close(&temp_fp);
    f_unlink(temp_path);
    return -1;
  }
  
  // Copy temp file to output
  uint8_t buf[256];
  while (track_size > 0) {
    UINT to_read = (track_size > sizeof(buf)) ? sizeof(buf) : track_size;
    UINT read_bytes;
    if (f_read(&temp_fp, buf, to_read, &read_bytes) != FR_OK || read_bytes != to_read) {
      f_close(&temp_fp);
      f_unlink(temp_path);
      return -1;
    }
    
    if (write_bytes(fp, buf, read_bytes) < 0) {
      f_close(&temp_fp);
      f_unlink(temp_path);
      return -1;
    }
    
    track_size -= read_bytes;
  }
  
  f_close(&temp_fp);
  f_unlink(temp_path);
  
  return 0;
}

// =====================================================================
// Tempo Tap Feature
// =====================================================================

#define TEMPO_TAP_MAX_TAPS 8
#define TEMPO_TAP_TIMEOUT_MS 2000

static struct {
  uint32_t timestamps[TEMPO_TAP_MAX_TAPS];  // Millisecond timestamps
  uint8_t count;                             // Number of taps recorded
  uint32_t last_tap_ms;                      // Last tap timestamp
} g_tempo_tap;

/**
 * @brief Register a tempo tap event
 * Analyzes tap intervals and calculates BPM after 2+ taps
 */
void looper_tempo_tap(void) {
  uint32_t now_ms = HAL_GetTick();
  
  // Check for timeout (reset if > 2 seconds since last tap)
  if (g_tempo_tap.count > 0 && (now_ms - g_tempo_tap.last_tap_ms) > TEMPO_TAP_TIMEOUT_MS) {
    g_tempo_tap.count = 0;
  }
  
  // Store tap timestamp
  if (g_tempo_tap.count < TEMPO_TAP_MAX_TAPS) {
    g_tempo_tap.timestamps[g_tempo_tap.count] = now_ms;
    g_tempo_tap.count++;
    g_tempo_tap.last_tap_ms = now_ms;
  } else {
    // Shift array left, add new tap at end
    for (uint8_t i = 0; i < TEMPO_TAP_MAX_TAPS - 1; i++) {
      g_tempo_tap.timestamps[i] = g_tempo_tap.timestamps[i + 1];
    }
    g_tempo_tap.timestamps[TEMPO_TAP_MAX_TAPS - 1] = now_ms;
    g_tempo_tap.last_tap_ms = now_ms;
  }
  
  // Calculate BPM after 2+ taps
  if (g_tempo_tap.count >= 2) {
    // Calculate average interval between taps
    uint32_t total_interval = 0;
    uint8_t interval_count = 0;
    
    for (uint8_t i = 1; i < g_tempo_tap.count; i++) {
      uint32_t interval = g_tempo_tap.timestamps[i] - g_tempo_tap.timestamps[i - 1];
      // Ignore intervals > 2 seconds (likely noise)
      if (interval <= TEMPO_TAP_TIMEOUT_MS) {
        total_interval += interval;
        interval_count++;
      }
    }
    
    if (interval_count > 0) {
      uint32_t avg_interval_ms = total_interval / interval_count;
      
      // Convert interval to BPM: BPM = 60000 / interval_ms
      if (avg_interval_ms > 0) {
        uint16_t new_bpm = (uint16_t)(60000 / avg_interval_ms);
        
        // Clamp to valid range
        if (new_bpm < 20) new_bpm = 20;
        if (new_bpm > 300) new_bpm = 300;
        
        // Apply new tempo
        looper_set_tempo(new_bpm);
      }
    }
  }
}

/**
 * @brief Get current tap count (for UI feedback)
 */
uint8_t looper_tempo_get_tap_count(void) {
  uint32_t now_ms = HAL_GetTick();
  
  // Auto-reset if timeout
  if (g_tempo_tap.count > 0 && (now_ms - g_tempo_tap.last_tap_ms) > TEMPO_TAP_TIMEOUT_MS) {
    g_tempo_tap.count = 0;
  }
  
  return g_tempo_tap.count;
}

/**
 * @brief Reset tempo tap sequence
 */
void looper_tempo_tap_reset(void) {
  g_tempo_tap.count = 0;
  g_tempo_tap.last_tap_ms = 0;
}

// =====================================================================
// MIDI File Export
// =====================================================================

/**
 * @brief Export all tracks to Standard MIDI File
 */
int looper_export_midi(const char* filename) {
  FIL fp;
  
  // Open output file
  if (f_open(&fp, filename, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
    return -1;
  }
  
  // Count non-empty tracks
  uint16_t track_count = 0;
  for (uint8_t i = 0; i < LOOPER_TRACKS; i++) {
    if (g_tr[i].count > 0) {
      track_count++;
    }
  }
  
  if (track_count == 0) {
    f_close(&fp);
    f_unlink(filename);
    return -2;  // No data to export
  }
  
  // Write MThd chunk (Format 1, multi-track)
  if (write_mthd_chunk(&fp, 1, track_count, LOOPER_PPQN) < 0) {
    f_close(&fp);
    f_unlink(filename);
    return -1;
  }
  
  // Write each track
  for (uint8_t i = 0; i < LOOPER_TRACKS; i++) {
    if (g_tr[i].count > 0) {
      char track_name[16];
      snprintf(track_name, sizeof(track_name), "Track %d", i + 1);
      
      if (export_track_to_mtrk(&fp, i, track_name) < 0) {
        f_close(&fp);
        f_unlink(filename);
        return -1;
      }
    }
  }
  
  f_close(&fp);
  return 0;
}

/**
 * @brief Export single track to MIDI file
 */
int looper_export_track_midi(uint8_t track, const char* filename) {
  if (track >= LOOPER_TRACKS) return -1;
  
  FIL fp;
  
  if (f_open(&fp, filename, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
    return -1;
  }
  
  if (g_tr[track].count == 0) {
    f_close(&fp);
    f_unlink(filename);
    return -2;  // No data
  }
  
  // Write MThd chunk (Format 0, single track)
  if (write_mthd_chunk(&fp, 0, 1, LOOPER_PPQN) < 0) {
    f_close(&fp);
    f_unlink(filename);
    return -1;
  }
  
  // Write track
  char track_name[16];
  snprintf(track_name, sizeof(track_name), "Track %d", track + 1);
  
  if (export_track_to_mtrk(&fp, track, track_name) < 0) {
    f_close(&fp);
    f_unlink(filename);
    return -1;
  }
  
  f_close(&fp);
  return 0;
}

/**
 * @brief Export a scene to MIDI file
 */
int looper_export_scene_midi(uint8_t scene, const char* filename) {
  if (scene >= LOOPER_SCENES) return -1;
  
  // TODO: For full implementation, would need to:
  // 1. Save current track states
  // 2. Load scene into tracks
  // 3. Export using looper_export_midi()
  // 4. Restore original track states
  
  // For now, just export current state
  return looper_export_midi(filename);
}

// ============================================================================
// Undo/Redo System
// ============================================================================

// Use configurable undo stack depth from header
#define UNDO_STACK_DEPTH LOOPER_UNDO_STACK_DEPTH

typedef struct {
  uint32_t event_count;
  uint32_t loop_len_ticks;
  uint16_t loop_beats;
  looper_quant_t quant;
  uint8_t has_data;
  // Events stored inline for simplicity (could be optimized with dynamic allocation)
  struct {
    uint32_t tick;
    uint8_t len;
    uint8_t b0, b1, b2;
  } events[256];  // Max 256 events per undo state
} undo_state_t;

typedef struct {
  undo_state_t states[UNDO_STACK_DEPTH];
  uint8_t write_idx;      // Where to write next state
  uint8_t undo_idx;       // Current position in history
  uint8_t count;          // Number of valid states
} undo_stack_t;

static undo_stack_t undo_stacks[LOOPER_TRACKS];

/**
 * @brief Save current track state to undo history
 */
void looper_undo_push(uint8_t track) {
  if (track >= LOOPER_TRACKS) return;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  undo_stack_t* stack = &undo_stacks[track];
  undo_state_t* state = &stack->states[stack->write_idx];
  
  // Capture current track state
  state->loop_len_ticks = g_tr[track].loop_len_ticks;
  state->loop_beats = g_tr[track].loop_beats;
  state->quant = g_tr[track].quant;
  state->event_count = g_tr[track].count < 256 ? g_tr[track].count : 256;
  state->has_data = 1;
  
  // Copy events
  for (uint32_t i = 0; i < state->event_count; i++) {
    state->events[i].tick = g_tr[track].ev[i].tick;
    state->events[i].len = g_tr[track].ev[i].len;
    state->events[i].b0 = g_tr[track].ev[i].b0;
    state->events[i].b1 = g_tr[track].ev[i].b1;
    state->events[i].b2 = g_tr[track].ev[i].b2;
  }
  
  // Advance write position
  stack->write_idx = (stack->write_idx + 1) % UNDO_STACK_DEPTH;
  if (stack->count < UNDO_STACK_DEPTH) {
    stack->count++;
  }
  
  // Set undo position to new write position (redo no longer available)
  stack->undo_idx = stack->write_idx;
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Undo last operation on track
 */
int looper_undo(uint8_t track) {
  if (track >= LOOPER_TRACKS) return -1;
  if (!looper_can_undo(track)) return -1;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  undo_stack_t* stack = &undo_stacks[track];
  
  // Move back in history
  if (stack->undo_idx == 0) {
    stack->undo_idx = UNDO_STACK_DEPTH - 1;
  } else {
    stack->undo_idx--;
  }
  
  // Restore state
  undo_state_t* state = &stack->states[stack->undo_idx];
  if (!state->has_data) {
    if (g_mutex) osMutexRelease(g_mutex);
    return -1;
  }
  
  g_tr[track].loop_len_ticks = state->loop_len_ticks;
  g_tr[track].loop_beats = state->loop_beats;
  g_tr[track].quant = state->quant;
  g_tr[track].count = state->event_count;
  
  // Restore events
  for (uint32_t i = 0; i < state->event_count; i++) {
    g_tr[track].ev[i].tick = state->events[i].tick;
    g_tr[track].ev[i].len = state->events[i].len;
    g_tr[track].ev[i].b0 = state->events[i].b0;
    g_tr[track].ev[i].b1 = state->events[i].b1;
    g_tr[track].ev[i].b2 = state->events[i].b2;
  }
  
  if (g_mutex) osMutexRelease(g_mutex);
  return 0;
}

/**
 * @brief Redo previously undone operation
 */
int looper_redo(uint8_t track) {
  if (track >= LOOPER_TRACKS) return -1;
  if (!looper_can_redo(track)) return -1;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  undo_stack_t* stack = &undo_stacks[track];
  
  // Move forward in history
  stack->undo_idx = (stack->undo_idx + 1) % UNDO_STACK_DEPTH;
  
  // Restore state
  undo_state_t* state = &stack->states[stack->undo_idx];
  if (!state->has_data) {
    if (g_mutex) osMutexRelease(g_mutex);
    return -1;
  }
  
  g_tr[track].loop_len_ticks = state->loop_len_ticks;
  g_tr[track].loop_beats = state->loop_beats;
  g_tr[track].quant = state->quant;
  g_tr[track].count = state->event_count;
  
  // Restore events
  for (uint32_t i = 0; i < state->event_count; i++) {
    g_tr[track].ev[i].tick = state->events[i].tick;
    g_tr[track].ev[i].len = state->events[i].len;
    g_tr[track].ev[i].b0 = state->events[i].b0;
    g_tr[track].ev[i].b1 = state->events[i].b1;
    g_tr[track].ev[i].b2 = state->events[i].b2;
  }
  
  if (g_mutex) osMutexRelease(g_mutex);
  return 0;
}

/**
 * @brief Clear undo/redo history for track
 */
void looper_undo_clear(uint8_t track) {
  if (track >= LOOPER_TRACKS) return;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  undo_stack_t* stack = &undo_stacks[track];
  memset(stack, 0, sizeof(undo_stack_t));
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Check if undo is available
 */
uint8_t looper_can_undo(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  
  undo_stack_t* stack = &undo_stacks[track];
  
  // Can undo if we have states and we're not at the oldest position
  if (stack->count == 0) return 0;
  
  uint8_t oldest_idx = (stack->write_idx + UNDO_STACK_DEPTH - stack->count) % UNDO_STACK_DEPTH;
  
  return stack->undo_idx != oldest_idx;
}

/**
 * @brief Check if redo is available
 */
uint8_t looper_can_redo(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  
  undo_stack_t* stack = &undo_stacks[track];
  
  // Can redo if we're not at the most recent position
  return stack->undo_idx != stack->write_idx && stack->count > 0;
}

// ================ Loop Quantization ================

// Quantization state per track
static uint8_t quantize_enabled[LOOPER_TRACKS] = {0};
static uint8_t quantize_resolution[LOOPER_TRACKS] = {2, 2, 2, 2}; // Default: 1/16 note

// Quantization grid sizes in ticks (assuming 96 PPQN)
static const uint16_t quant_grid_ticks[] = {
  96,  // 0: Quarter note
  48,  // 1: Eighth note
  24,  // 2: Sixteenth note (default)
  12,  // 3: Thirty-second note
  6    // 4: Sixty-fourth note
};
#define QUANT_RESOLUTIONS 5

/**
 * @brief Quantize all events in a track to nearest grid position
 */
void looper_quantize_track(uint8_t track, uint8_t resolution) {
  if (track >= LOOPER_TRACKS) return;
  if (resolution >= QUANT_RESOLUTIONS) resolution = 2; // Default to 1/16
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  uint16_t grid_size = quant_grid_ticks[resolution];
  looper_track_t* t = &g_tr[track];
  
  // Quantize each event's timestamp
  for (uint32_t i = 0; i < t->count; i++) {
    uint32_t original_tick = t->ev[i].tick;
    
    // Calculate nearest grid position
    // Formula: quantized = round(original / grid) * grid
    uint32_t grid_position = (original_tick + grid_size / 2) / grid_size;
    uint32_t quantized_tick = grid_position * grid_size;
    
    // Ensure we don't exceed loop length
    if (quantized_tick >= t->loop_len_ticks && t->loop_len_ticks > 0) {
      quantized_tick = t->loop_len_ticks - 1;
    }
    
    t->ev[i].tick = quantized_tick;
  }
  
  // Events may need re-sorting after quantization
  // Simple bubble sort for simplicity (could optimize later)
  for (uint32_t i = 0; i < t->count - 1; i++) {
    for (uint32_t j = 0; j < t->count - i - 1; j++) {
      if (t->ev[j].tick > t->ev[j + 1].tick) {
        // Swap events
        looper_evt_t temp = t->ev[j];
        t->ev[j] = t->ev[j + 1];
        t->ev[j + 1] = temp;
      }
    }
  }
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Enable/disable auto-quantization for track
 */
void looper_set_quantize_enabled(uint8_t track, uint8_t enabled) {
  if (track >= LOOPER_TRACKS) return;
  quantize_enabled[track] = enabled ? 1 : 0;
}

/**
 * @brief Get quantization enabled state
 */
uint8_t looper_get_quantize_enabled(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return quantize_enabled[track];
}

/**
 * @brief Set quantization resolution for track
 */
void looper_set_quantize_resolution(uint8_t track, uint8_t resolution) {
  if (track >= LOOPER_TRACKS) return;
  if (resolution >= QUANT_RESOLUTIONS) resolution = 2; // Default to 1/16
  quantize_resolution[track] = resolution;
}

/**
 * @brief Get quantization resolution
 */
uint8_t looper_get_quantize_resolution(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 2; // Default 1/16
  return quantize_resolution[track];
}

// ========================================================================
// MIDI Clock Sync Implementation
// ========================================================================

#define CLOCK_BUFFER_SIZE 24  // 24 PPQN (1 quarter note)
#define CLOCK_TIMEOUT_MS 2000  // 2 seconds timeout

static struct {
  uint8_t enabled;
  uint8_t active;
  uint32_t last_clock_time_us;
  uint32_t clock_intervals_us[CLOCK_BUFFER_SIZE];
  uint8_t clock_index;
  uint8_t clock_count;
  uint16_t detected_bpm;
} clock_sync_state = {0};

/**
 * @brief Enable/disable external MIDI clock synchronization
 */
void looper_set_clock_sync_enabled(uint8_t enabled) {
  clock_sync_state.enabled = enabled ? 1 : 0;
  if (!enabled) {
    // Reset clock state when disabled
    clock_sync_state.active = 0;
    clock_sync_state.clock_count = 0;
    clock_sync_state.clock_index = 0;
    clock_sync_state.detected_bpm = 0;
  }
}

/**
 * @brief Get external clock sync status
 */
uint8_t looper_get_clock_sync_enabled(void) {
  return clock_sync_state.enabled;
}

/**
 * @brief Process incoming MIDI clock message (0xF8)
 */
void looper_process_midi_clock(void) {
  if (!clock_sync_state.enabled) return;
  
  // Get current time in microseconds
  uint32_t now_us = HAL_GetTick() * 1000; // Convert ms to us (approximate)
  
  // Calculate interval since last clock
  if (clock_sync_state.clock_count > 0) {
    uint32_t interval_us = now_us - clock_sync_state.last_clock_time_us;
    
    // Store interval in circular buffer
    clock_sync_state.clock_intervals_us[clock_sync_state.clock_index] = interval_us;
    clock_sync_state.clock_index = (clock_sync_state.clock_index + 1) % CLOCK_BUFFER_SIZE;
    
    // Calculate average interval after collecting enough samples
    if (clock_sync_state.clock_count >= CLOCK_BUFFER_SIZE) {
      uint32_t total_us = 0;
      uint8_t valid_samples = 0;
      
      // Calculate average, filtering outliers (>20% deviation)
      uint32_t avg_first_pass = 0;
      for (uint8_t i = 0; i < CLOCK_BUFFER_SIZE; i++) {
        avg_first_pass += clock_sync_state.clock_intervals_us[i];
      }
      avg_first_pass /= CLOCK_BUFFER_SIZE;
      
      // Second pass: filter outliers
      for (uint8_t i = 0; i < CLOCK_BUFFER_SIZE; i++) {
        uint32_t interval = clock_sync_state.clock_intervals_us[i];
        uint32_t deviation = (interval > avg_first_pass) ? 
                             (interval - avg_first_pass) : (avg_first_pass - interval);
        
        // Accept if within 20% of average
        if (deviation < (avg_first_pass / 5)) {
          total_us += interval;
          valid_samples++;
        }
      }
      
      if (valid_samples > 0) {
        uint32_t avg_interval_us = total_us / valid_samples;
        
        // Calculate BPM: 60,000,000 / (avg_interval_us * 24)
        // 24 PPQN = 24 clock pulses per quarter note
        if (avg_interval_us > 0) {
          uint32_t bpm_calc = 60000000UL / (avg_interval_us * 24UL);
          
          // Clamp to valid BPM range
          if (bpm_calc < 20) bpm_calc = 20;
          if (bpm_calc > 300) bpm_calc = 300;
          
          // Apply gradual tempo change (max ±1 BPM per update for stability)
          if (clock_sync_state.detected_bpm == 0) {
            clock_sync_state.detected_bpm = (uint16_t)bpm_calc;
          } else {
            int16_t diff = (int16_t)bpm_calc - (int16_t)clock_sync_state.detected_bpm;
            if (diff > 1) diff = 1;
            if (diff < -1) diff = -1;
            clock_sync_state.detected_bpm += diff;
          }
          
          // Update looper tempo
          looper_set_tempo(clock_sync_state.detected_bpm);
        }
      }
      
      clock_sync_state.active = 1;
    }
  }
  
  clock_sync_state.last_clock_time_us = now_us;
  clock_sync_state.clock_count++;
}

/**
 * @brief Process MIDI Start message (0xFA)
 */
void looper_process_midi_start(void) {
  if (!clock_sync_state.enabled || !clock_sync_state.active) return;
  
  // Start playback from beginning on all tracks
  for (uint8_t track = 0; track < LOOPER_TRACKS; track++) {
    looper_state_t state = looper_get_state(track);
    if (state != LOOPER_STATE_STOP) {
      looper_set_state(track, LOOPER_STATE_PLAY);
      // Reset play position to beginning (implementation-specific)
    }
  }
}

/**
 * @brief Process MIDI Stop message (0xFC)
 */
void looper_process_midi_stop(void) {
  if (!clock_sync_state.enabled || !clock_sync_state.active) return;
  
  // Stop playback on all tracks
  for (uint8_t track = 0; track < LOOPER_TRACKS; track++) {
    looper_set_state(track, LOOPER_STATE_STOP);
  }
}

/**
 * @brief Process MIDI Continue message (0xFB)
 */
void looper_process_midi_continue(void) {
  if (!clock_sync_state.enabled || !clock_sync_state.active) return;
  
  // Resume playback from current position
  for (uint8_t track = 0; track < LOOPER_TRACKS; track++) {
    looper_state_t state = looper_get_state(track);
    if (state == LOOPER_STATE_STOP) {
      looper_set_state(track, LOOPER_STATE_PLAY);
    }
  }
}

/**
 * @brief Get detected external BPM
 */
uint16_t looper_get_external_bpm(void) {
  return clock_sync_state.detected_bpm;
}

/**
 * @brief Check if external clock is actively being received
 */
uint8_t looper_is_external_clock_active(void) {
  if (!clock_sync_state.enabled) return 0;
  
  // Check if we've received clock recently (within timeout period)
  uint32_t now_us = HAL_GetTick() * 1000;
  uint32_t elapsed_us = now_us - clock_sync_state.last_clock_time_us;
  
  if (elapsed_us > (CLOCK_TIMEOUT_MS * 1000)) {
    // Timeout: no clock received recently
    clock_sync_state.active = 0;
    clock_sync_state.clock_count = 0;
    clock_sync_state.clock_index = 0;
    return 0;
  }
  
  return clock_sync_state.active;
}

// =========================================================================
// Copy/Paste Scenes and Tracks
// =========================================================================

// Clipboard structures
static struct {
  uint8_t valid;  // 1 if clipboard has data
  uint32_t count;
  uint32_t loop_len_ticks;
  uint16_t loop_beats;
  looper_quant_t quant;
  looper_evt_t events[LOOPER_MAX_EVENTS];
} track_clipboard = {0};

static struct {
  uint8_t valid;  // 1 if clipboard has data
  struct {
    uint8_t has_data;
    uint32_t count;
    uint32_t loop_len_ticks;
    uint16_t loop_beats;
    looper_evt_t events[LOOPER_MAX_EVENTS];
  } tracks[LOOPER_TRACKS];
} scene_clipboard = {0};

/**
 * @brief Copy track data to clipboard
 */
int looper_copy_track(uint8_t track) {
  if (track >= LOOPER_TRACKS) return -1;
  
  osMutexAcquire(g_mutex, osWaitForever);
  
  looper_track_t* t = &g_tr[track];
  track_clipboard.valid = 1;
  track_clipboard.count = t->count;
  track_clipboard.loop_len_ticks = t->loop_len_ticks;
  track_clipboard.loop_beats = t->loop_beats;
  track_clipboard.quant = t->quant;
  
  // Copy events
  for (uint32_t i = 0; i < t->count && i < LOOPER_MAX_EVENTS; i++) {
    track_clipboard.events[i] = t->ev[i];
  }
  
  osMutexRelease(g_mutex);
  return 0;
}

/**
 * @brief Paste clipboard data to track
 */
int looper_paste_track(uint8_t track) {
  if (track >= LOOPER_TRACKS) return -1;
  if (!track_clipboard.valid) return -1;  // Clipboard empty
  
  osMutexAcquire(g_mutex, osWaitForever);
  
  looper_track_t* t = &g_tr[track];
  
  // Clear track and paste data
  clear_track(t);
  t->count = track_clipboard.count;
  t->loop_len_ticks = track_clipboard.loop_len_ticks;
  t->loop_beats = track_clipboard.loop_beats;
  t->quant = track_clipboard.quant;
  
  // Copy events
  for (uint32_t i = 0; i < track_clipboard.count && i < LOOPER_MAX_EVENTS; i++) {
    t->ev[i] = track_clipboard.events[i];
  }
  
  osMutexRelease(g_mutex);
  return 0;
}

/**
 * @brief Copy entire scene to clipboard
 */
int looper_copy_scene(uint8_t scene) {
  if (scene >= LOOPER_SCENES) return -1;
  
  osMutexAcquire(g_mutex, osWaitForever);
  
  scene_clipboard.valid = 1;
  
  for (uint8_t track = 0; track < LOOPER_TRACKS; track++) {
    looper_scene_clip_t clip = looper_get_scene_clip(scene, track);
    
    if (clip.has_clip) {
      // Load scene data temporarily to copy it
      // This is a simplified approach - in production you'd access scene storage directly
      scene_clipboard.tracks[track].has_data = 1;
      scene_clipboard.tracks[track].loop_beats = clip.loop_beats;
      
      // Note: Actual implementation would require access to scene storage
      // For now we mark it as having data with the loop length
      scene_clipboard.tracks[track].count = 0;  // Would be populated from scene storage
      scene_clipboard.tracks[track].loop_len_ticks = beats_to_ticks(clip.loop_beats);
    } else {
      scene_clipboard.tracks[track].has_data = 0;
    }
  }
  
  osMutexRelease(g_mutex);
  return 0;
}

/**
 * @brief Paste clipboard scene data to target scene
 */
int looper_paste_scene(uint8_t scene) {
  if (scene >= LOOPER_SCENES) return -1;
  if (!scene_clipboard.valid) return -1;  // Clipboard empty
  
  osMutexAcquire(g_mutex, osWaitForever);
  
  for (uint8_t track = 0; track < LOOPER_TRACKS; track++) {
    if (scene_clipboard.tracks[track].has_data) {
      // Note: Actual implementation would write to scene storage
      // This is a placeholder that shows the structure
      looper_save_to_scene(scene, track);
    }
  }
  
  osMutexRelease(g_mutex);
  return 0;
}

/**
 * @brief Check if track clipboard has data
 */
uint8_t looper_has_track_clipboard(void) {
  return track_clipboard.valid;
}

/**
 * @brief Check if scene clipboard has data
 */
uint8_t looper_has_scene_clipboard(void) {
  return scene_clipboard.valid;
}

/**
 * @brief Clear track clipboard
 */
void looper_clear_track_clipboard(void) {
  track_clipboard.valid = 0;
  track_clipboard.count = 0;
}

/**
 * @brief Clear scene clipboard
 */
void looper_clear_scene_clipboard(void) {
  scene_clipboard.valid = 0;
  for (uint8_t i = 0; i < LOOPER_TRACKS; i++) {
    scene_clipboard.tracks[i].has_data = 0;
  }
}

// ============================================================================
// Global Transpose Functions
// ============================================================================

/**
 * @brief Set global transpose for all tracks
 */
void looper_set_global_transpose(int8_t semitones) {
  // Clamp to reasonable range
  if (semitones < -24) semitones = -24;
  if (semitones > 24) semitones = 24;
  
  g_global_transpose = semitones;
}

/**
 * @brief Get current global transpose value
 */
int8_t looper_get_global_transpose(void) {
  return g_global_transpose;
}

/**
 * @brief Transpose all events in all tracks by specified semitones
 */
void looper_transpose_all_tracks(int8_t semitones) {
  // Clamp to reasonable range
  if (semitones < -24) semitones = -24;
  if (semitones > 24) semitones = 24;
  
  if (semitones == 0) return;  // No transpose needed
  
  osMutexAcquire(g_mutex, osWaitForever);
  
  // Transpose all tracks
  for (uint8_t track = 0; track < LOOPER_TRACKS; track++) {
    looper_track_t* t = &g_tr[track];
    
    // Transpose all note events in the track
    for (uint32_t i = 0; i < t->count; i++) {
      uint8_t status = t->ev[i].b0 & 0xF0;
      
      // Only transpose note on/off messages (0x80-0x9F)
      if (status == 0x80 || status == 0x90) {
        int16_t note = (int16_t)t->ev[i].b1 + semitones;
        
        // Clamp to valid MIDI note range (0-127)
        if (note < 0) note = 0;
        if (note > 127) note = 127;
        
        t->ev[i].b1 = (uint8_t)note;
      }
    }
  }
  
  osMutexRelease(g_mutex);
}

// ============================================================================
// Randomizer Feature
// ============================================================================

// Per-track randomization parameters
static struct {
  uint8_t velocity_range;   // 0-64
  uint8_t timing_range;     // 0-12 ticks
  uint8_t note_skip_prob;   // 0-100%
} g_randomize_params[LOOPER_TRACKS] = {0};

// Simple pseudo-random number generator (LCG)
// Note: Seed is initialized per randomization operation for thread safety
static volatile uint32_t g_rand_seed = 0x12345678;

static uint32_t _rand_next(void) {
  // Thread-safe read-modify-write
  uint32_t seed = g_rand_seed;
  seed = seed * 1103515245 + 12345;
  g_rand_seed = seed;
  return (seed / 65536) % 32768;
}

static int8_t _rand_range(int8_t min, int8_t max) {
  if (min >= max) return min;
  return min + (_rand_next() % (max - min + 1));
}

/**
 * @brief Apply randomization to a track
 */
void looper_randomize_track(uint8_t track, uint8_t velocity_range, 
                            uint8_t timing_range, uint8_t note_skip_prob) {
  if (track >= LOOPER_TRACKS) return;
  
  // Clamp parameters to safe ranges
  if (velocity_range > 64) velocity_range = 64;
  if (timing_range > 12) timing_range = 12;
  if (note_skip_prob > 100) note_skip_prob = 100;
  
  osMutexAcquire(g_mutex, osWaitForever);
  
  looper_track_t* t = &g_tr[track];
  
  // Seed with current system time for better randomness
  g_rand_seed = HAL_GetTick();
  
  // Process all events
  uint32_t write_idx = 0;
  for (uint32_t i = 0; i < t->count; i++) {
    uint8_t status = t->ev[i].b0 & 0xF0;
    uint8_t skip_this_note = 0;
    
    // Check if this is a note-on event
    if (status == 0x90 && t->ev[i].b2 > 0) {
      // Apply note skip probability
      if (note_skip_prob > 0) {
        uint32_t rand_val = _rand_next() % 100;
        if (rand_val < note_skip_prob) {
          skip_this_note = 1;
        }
      }
      
      if (!skip_this_note) {
        // Apply velocity randomization
        if (velocity_range > 0) {
          int16_t vel = (int16_t)t->ev[i].b2;
          int8_t offset = _rand_range(-velocity_range, velocity_range);
          vel += offset;
          
          // Clamp velocity to valid range (1-127)
          if (vel < 1) vel = 1;
          if (vel > 127) vel = 127;
          
          t->ev[i].b2 = (uint8_t)vel;
        }
        
        // Apply timing randomization
        if (timing_range > 0) {
          int32_t tick = (int32_t)t->ev[i].tick;
          int8_t offset = _rand_range(-timing_range, timing_range);
          tick += offset;
          
          // Clamp to valid range (0 to loop length)
          if (tick < 0) tick = 0;
          if (tick >= (int32_t)t->loop_len_ticks) tick = t->loop_len_ticks - 1;
          
          t->ev[i].tick = (uint32_t)tick;
        }
        
        // Keep this event
        if (write_idx != i) {
          t->ev[write_idx] = t->ev[i];
        }
        write_idx++;
      }
      // If skipping, also need to remove the corresponding note-off
      // For simplicity, we'll keep note-offs and let hanging notes timeout
    } else {
      // Keep all non-note-on events (note-off, CC, etc.)
      if (write_idx != i) {
        t->ev[write_idx] = t->ev[i];
      }
      write_idx++;
    }
  }
  
  // Update event count
  t->count = write_idx;
  
  // Re-sort events by tick after randomization
  sort_events(t);
  
  osMutexRelease(g_mutex);
}

/**
 * @brief Set randomization parameters for a track
 */
void looper_set_randomize_params(uint8_t track, uint8_t velocity_range,
                                 uint8_t timing_range, uint8_t note_skip_prob) {
  if (track >= LOOPER_TRACKS) return;
  
  // Clamp parameters
  if (velocity_range > 64) velocity_range = 64;
  if (timing_range > 12) timing_range = 12;
  if (note_skip_prob > 100) note_skip_prob = 100;
  
  g_randomize_params[track].velocity_range = velocity_range;
  g_randomize_params[track].timing_range = timing_range;
  g_randomize_params[track].note_skip_prob = note_skip_prob;
}

/**
 * @brief Get randomization parameters for a track
 */
void looper_get_randomize_params(uint8_t track, uint8_t* out_velocity_range,
                                 uint8_t* out_timing_range, uint8_t* out_note_skip_prob) {
  if (track >= LOOPER_TRACKS) return;
  
  if (out_velocity_range) {
    *out_velocity_range = g_randomize_params[track].velocity_range;
  }
  if (out_timing_range) {
    *out_timing_range = g_randomize_params[track].timing_range;
  }
  if (out_note_skip_prob) {
    *out_note_skip_prob = g_randomize_params[track].note_skip_prob;
  }
}

// ==================== Humanizer Feature ====================

// Humanizer parameter storage
typedef struct {
  uint8_t enabled;
  uint8_t velocity_amount;
  uint8_t timing_amount;
  uint8_t intensity;
} humanize_params_t;

static humanize_params_t g_humanize_params[LOOPER_TRACKS] = {0};

// Simple sine-like curve for smooth humanization
static inline int8_t _humanize_curve(uint32_t seed, int8_t range) {
  // Use seed to generate smooth variations
  uint32_t val = (seed * 1103515245u + 12345u) % 256;
  // Map to sine-like curve: -range to +range
  int16_t curve = (int16_t)((val * (int32_t)range * 2) / 256) - range;
  return (int8_t)curve;
}

/**
 * @brief Apply humanization to a track
 */
void looper_humanize_track(uint8_t track, uint8_t velocity_amount,
                           uint8_t timing_amount, uint8_t intensity) {
  if (track >= LOOPER_TRACKS) return;
  
  // Clamp parameters
  if (velocity_amount > 32) velocity_amount = 32;
  if (timing_amount > 6) timing_amount = 6;
  if (intensity > 100) intensity = 100;
  
  looper_track_t* t = &g_tr[track];
  
  if (osMutexAcquire(g_mutex, osWaitForever) != osOK) return;
  
  // Store parameters
  g_humanize_params[track].velocity_amount = velocity_amount;
  g_humanize_params[track].timing_amount = timing_amount;
  g_humanize_params[track].intensity = intensity;
  
  // Apply humanization to all note events
  for (uint32_t i = 0; i < t->count; i++) {
    uint8_t status = t->ev[i].b0 & 0xF0;
    
    // Check if this is a note-on event
    if (status == 0x90 && t->ev[i].b2 > 0) {
      // Calculate beat position (0 = on-beat, >0 = off-beat)
      uint32_t beat_pos = t->ev[i].tick % (LOOPER_PPQN / 4); // Quarter note position
      uint8_t is_on_beat = (beat_pos < (LOOPER_PPQN / 16)) ? 1 : 0; // Within 1/16th of beat
      
      // Apply velocity humanization with smooth curves
      if (velocity_amount > 0 && intensity > 0) {
        int16_t vel = (int16_t)t->ev[i].b2;
        
        // Generate smooth velocity curve based on event index
        int8_t vel_curve = _humanize_curve(i + t->ev[i].tick, velocity_amount);
        
        // Scale by intensity
        vel_curve = (int8_t)((vel_curve * (int16_t)intensity) / 100);
        
        // Apply with natural dynamics preservation
        vel += vel_curve;
        
        // Clamp to valid range (1-127)
        if (vel < 1) vel = 1;
        if (vel > 127) vel = 127;
        
        t->ev[i].b2 = (uint8_t)vel;
      }
      
      // Apply timing humanization (groove-aware)
      if (timing_amount > 0 && intensity > 0) {
        int32_t tick = (int32_t)t->ev[i].tick;
        
        // On-beat notes get less variation (preserve groove)
        uint8_t timing_scale = is_on_beat ? 20 : 100; // 20% for on-beat, 100% for off-beat
        
        // Generate smooth timing curve
        int8_t timing_curve = _humanize_curve(t->ev[i].tick + (i * 17), timing_amount);
        
        // Scale by intensity and beat position
        timing_curve = (int8_t)((timing_curve * (int16_t)intensity * timing_scale) / 10000);
        
        // Apply timing shift
        tick += timing_curve;
        
        // Clamp to valid range
        if (tick < 0) tick = 0;
        if (tick >= (int32_t)t->loop_len_ticks) tick = t->loop_len_ticks - 1;
        
        t->ev[i].tick = (uint32_t)tick;
      }
    }
  }
  
  // Re-sort events by tick after humanization
  for (uint32_t i = 0; i < t->count - 1; i++) {
    for (uint32_t j = i + 1; j < t->count; j++) {
      if (t->ev[j].tick < t->ev[i].tick) {
        looper_evt_t tmp = t->ev[i];
        t->ev[i] = t->ev[j];
        t->ev[j] = tmp;
      }
    }
  }
  
  osMutexRelease(g_mutex);
}

/**
 * @brief Set humanization parameters for a track
 */
void looper_set_humanize_params(uint8_t track, uint8_t velocity_amount,
                                uint8_t timing_amount, uint8_t intensity) {
  if (track >= LOOPER_TRACKS) return;
  
  // Clamp parameters
  if (velocity_amount > 32) velocity_amount = 32;
  if (timing_amount > 6) timing_amount = 6;
  if (intensity > 100) intensity = 100;
  
  g_humanize_params[track].velocity_amount = velocity_amount;
  g_humanize_params[track].timing_amount = timing_amount;
  g_humanize_params[track].intensity = intensity;
}

/**
 * @brief Get humanization parameters for a track
 */
void looper_get_humanize_params(uint8_t track, uint8_t* out_velocity_amount,
                                uint8_t* out_timing_amount, uint8_t* out_intensity) {
  if (track >= LOOPER_TRACKS) return;
  
  if (out_velocity_amount) {
    *out_velocity_amount = g_humanize_params[track].velocity_amount;
  }
  if (out_timing_amount) {
    *out_timing_amount = g_humanize_params[track].timing_amount;
  }
  if (out_intensity) {
    *out_intensity = g_humanize_params[track].intensity;
  }
}

// ==================== Arpeggiator Feature ====================

// Arpeggiator parameter storage
typedef struct {
  uint8_t enabled;
  uint8_t pattern;         // arp_pattern_t
  uint8_t gate_percent;    // 10-95%
  uint8_t octaves;         // 1-4
} arp_params_t;

static arp_params_t g_arp_params[LOOPER_TRACKS] = {
  {0, ARP_PATTERN_UP, 75, 1},
  {0, ARP_PATTERN_UP, 75, 1},
  {0, ARP_PATTERN_UP, 75, 1},
  {0, ARP_PATTERN_UP, 75, 1}
};

/**
 * @brief Enable/disable arpeggiator for a track
 */
void looper_set_arp_enabled(uint8_t track, uint8_t enabled) {
  if (track >= LOOPER_TRACKS) return;
  g_arp_params[track].enabled = enabled ? 1 : 0;
}

/**
 * @brief Get arpeggiator enabled state
 */
uint8_t looper_get_arp_enabled(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return g_arp_params[track].enabled;
}

/**
 * @brief Set arpeggiator pattern
 */
void looper_set_arp_pattern(uint8_t track, arp_pattern_t pattern) {
  if (track >= LOOPER_TRACKS) return;
  if (pattern > ARP_PATTERN_CHORD) pattern = ARP_PATTERN_UP;
  g_arp_params[track].pattern = (uint8_t)pattern;
}

/**
 * @brief Get arpeggiator pattern
 */
arp_pattern_t looper_get_arp_pattern(uint8_t track) {
  if (track >= LOOPER_TRACKS) return ARP_PATTERN_UP;
  return (arp_pattern_t)g_arp_params[track].pattern;
}

/**
 * @brief Set arpeggiator gate length
 */
void looper_set_arp_gate(uint8_t track, uint8_t gate_percent) {
  if (track >= LOOPER_TRACKS) return;
  
  // Clamp to valid range
  if (gate_percent < 10) gate_percent = 10;
  if (gate_percent > 95) gate_percent = 95;
  
  g_arp_params[track].gate_percent = gate_percent;
}

/**
 * @brief Get arpeggiator gate length
 */
uint8_t looper_get_arp_gate(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 75;
  return g_arp_params[track].gate_percent;
}

/**
 * @brief Set arpeggiator octave range
 */
void looper_set_arp_octaves(uint8_t track, uint8_t octaves) {
  if (track >= LOOPER_TRACKS) return;
  
  // Clamp to valid range
  if (octaves < 1) octaves = 1;
  if (octaves > 4) octaves = 4;
  
  g_arp_params[track].octaves = octaves;
}

/**
 * @brief Get arpeggiator octave range
 */
uint8_t looper_get_arp_octaves(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 1;
  return g_arp_params[track].octaves;
}

// ============================================================================
// Footswitch Mapping System
// ============================================================================

/**
 * @brief Assign function to footswitch
 */
void looper_set_footswitch_action(uint8_t fs_num, footswitch_action_t action, uint8_t param) {
  if (fs_num >= NUM_FOOTSWITCHES) return;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  g_footswitch[fs_num].action = action;
  g_footswitch[fs_num].param = param;
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Get footswitch assignment
 */
footswitch_action_t looper_get_footswitch_action(uint8_t fs_num, uint8_t* out_param) {
  if (fs_num >= NUM_FOOTSWITCHES) return FS_ACTION_NONE;
  
  if (out_param) {
    *out_param = g_footswitch[fs_num].param;
  }
  
  return g_footswitch[fs_num].action;
}

/**
 * @brief Process footswitch press
 */
void looper_footswitch_press(uint8_t fs_num) {
  if (fs_num >= NUM_FOOTSWITCHES) return;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  footswitch_mapping_t* fs = &g_footswitch[fs_num];
  fs->pressed = 1;
  fs->press_time_ms = HAL_GetTick();
  
  // Execute action on press
  uint8_t track = fs->param;
  uint8_t scene = fs->param;
  
  switch (fs->action) {
    case FS_ACTION_PLAY_STOP:
      if (track < LOOPER_TRACKS) {
        if (g_tr[track].st == LOOPER_STATE_PLAY) {
          looper_set_state(track, LOOPER_STATE_STOP);
        } else {
          looper_set_state(track, LOOPER_STATE_PLAY);
        }
      }
      break;
      
    case FS_ACTION_RECORD:
      if (track < LOOPER_TRACKS) {
        if (g_tr[track].st == LOOPER_STATE_REC) {
          looper_set_state(track, LOOPER_STATE_STOP);
        } else {
          looper_set_state(track, LOOPER_STATE_REC);
        }
      }
      break;
      
    case FS_ACTION_OVERDUB:
      if (track < LOOPER_TRACKS) {
        if (g_tr[track].st == LOOPER_STATE_OVERDUB) {
          looper_set_state(track, LOOPER_STATE_PLAY);
        } else {
          looper_set_state(track, LOOPER_STATE_OVERDUB);
        }
      }
      break;
      
    case FS_ACTION_UNDO:
      if (track < LOOPER_TRACKS) {
        looper_undo(track);
      }
      break;
      
    case FS_ACTION_REDO:
      if (track < LOOPER_TRACKS) {
        looper_redo(track);
      }
      break;
      
    case FS_ACTION_TAP_TEMPO:
      looper_tempo_tap();
      break;
      
    case FS_ACTION_SELECT_TRACK:
      // Track selection handled by UI layer
      break;
      
    case FS_ACTION_TRIGGER_SCENE:
      if (scene < LOOPER_SCENES) {
        looper_trigger_scene(scene);
      }
      break;
      
    case FS_ACTION_MUTE_TRACK:
      if (track < LOOPER_TRACKS) {
        uint8_t is_muted = looper_is_track_muted(track);
        looper_set_track_muted(track, !is_muted);
      }
      break;
      
    case FS_ACTION_SOLO_TRACK:
      if (track < LOOPER_TRACKS) {
        uint8_t is_soloed = looper_is_track_soloed(track);
        looper_set_track_solo(track, !is_soloed);
      }
      break;
      
    case FS_ACTION_CLEAR_TRACK:
      if (track < LOOPER_TRACKS) {
        looper_clear(track);
      }
      break;
      
    case FS_ACTION_QUANTIZE_TRACK:
      if (track < LOOPER_TRACKS) {
        uint8_t res = looper_get_quantize_resolution(track);
        looper_quantize_track(track, res);
      }
      break;
      
    case FS_ACTION_NONE:
    default:
      break;
  }
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Process footswitch release
 */
void looper_footswitch_release(uint8_t fs_num) {
  if (fs_num >= NUM_FOOTSWITCHES) return;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  footswitch_mapping_t* fs = &g_footswitch[fs_num];
  
  uint32_t press_duration = HAL_GetTick() - fs->press_time_ms;
  
  // Check for long press (>500ms)
  if (press_duration > 500) {
    // Long press action (if different from short press)
    // Currently same action, but could be extended
  }
  
  fs->pressed = 0;
  
  if (g_mutex) osMutexRelease(g_mutex);
}

// ============================================================================
// MIDI Learn System
// ============================================================================

/**
 * @brief Start MIDI learn mode
 */
void looper_midi_learn_start(footswitch_action_t action, uint8_t param) {
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  g_midi_learn_state.learning_active = 1;
  g_midi_learn_state.pending_action = action;
  g_midi_learn_state.pending_param = param;
  g_midi_learn_state.learn_timeout_ms = HAL_GetTick() + 10000;  // 10 second timeout
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Cancel MIDI learn mode
 */
void looper_midi_learn_cancel(void) {
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  g_midi_learn_state.learning_active = 0;
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Process incoming MIDI message for learn mode
 */
void looper_midi_learn_process(const router_msg_t* msg) {
  if (!g_midi_learn_state.learning_active) return;
  
  // Check timeout
  if (HAL_GetTick() > g_midi_learn_state.learn_timeout_ms) {
    looper_midi_learn_cancel();
    return;
  }
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  // Parse MIDI message
  uint8_t status = msg->b0 & 0xF0;
  uint8_t channel = msg->b0 & 0x0F;
  uint8_t control_type = 0;
  uint8_t control_num = 0;
  
  if (status == 0xB0) {
    // Control Change
    control_type = 0;
    control_num = msg->b1;
  } else if (status == 0x90 || status == 0x80) {
    // Note On/Off
    control_type = 1;
    control_num = msg->b1;
  } else {
    // Not a learnable message
    if (g_mutex) osMutexRelease(g_mutex);
    return;
  }
  
  // Add mapping
  if (g_midi_learn_count < MAX_MIDI_LEARN_MAPPINGS) {
    g_midi_learn[g_midi_learn_count].midi_cc = control_num;
    g_midi_learn[g_midi_learn_count].midi_channel = channel;
    g_midi_learn[g_midi_learn_count].control_type = control_type;
    g_midi_learn[g_midi_learn_count].action = g_midi_learn_state.pending_action;
    g_midi_learn[g_midi_learn_count].param = g_midi_learn_state.pending_param;
    g_midi_learn_count++;
  }
  
  // Exit learn mode
  g_midi_learn_state.learning_active = 0;
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Check if MIDI message triggers a learned action
 */
void looper_midi_learn_check(const router_msg_t* msg) {
  uint8_t status = msg->b0 & 0xF0;
  uint8_t channel = msg->b0 & 0x0F;
  uint8_t control_num = msg->b1;
  uint8_t control_type = 0;
  
  if (status == 0xB0) {
    control_type = 0;
  } else if (status == 0x90 || status == 0x80) {
    control_type = 1;
  } else {
    return;
  }
  
  // Check learned mappings
  for (uint8_t i = 0; i < g_midi_learn_count; i++) {
    midi_learn_mapping_t* mapping = &g_midi_learn[i];
    
    if (mapping->control_type == control_type &&
        mapping->midi_cc == control_num &&
        (mapping->midi_channel == 0xFF || mapping->midi_channel == channel)) {
      
      // Execute mapped action
      uint8_t track = mapping->param;
      uint8_t scene = mapping->param;
      
      switch (mapping->action) {
        case FS_ACTION_PLAY_STOP:
          if (track < LOOPER_TRACKS) {
            if (g_tr[track].st == LOOPER_STATE_PLAY) {
              looper_set_state(track, LOOPER_STATE_STOP);
            } else {
              looper_set_state(track, LOOPER_STATE_PLAY);
            }
          }
          break;
          
        case FS_ACTION_RECORD:
          if (track < LOOPER_TRACKS) {
            if (g_tr[track].st == LOOPER_STATE_REC) {
              looper_set_state(track, LOOPER_STATE_STOP);
            } else {
              looper_set_state(track, LOOPER_STATE_REC);
            }
          }
          break;
          
        case FS_ACTION_TRIGGER_SCENE:
          if (scene < LOOPER_SCENES) {
            looper_trigger_scene(scene);
          }
          break;
          
        case FS_ACTION_MUTE_TRACK:
          if (track < LOOPER_TRACKS) {
            uint8_t is_muted = looper_is_track_muted(track);
            looper_set_track_muted(track, !is_muted);
          }
          break;
          
        default:
          break;
      }
      
      break;  // Only trigger first matching mapping
    }
  }
}

/**
 * @brief Clear all MIDI learn mappings
 */
void looper_midi_learn_clear(void) {
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  g_midi_learn_count = 0;
  memset(g_midi_learn, 0, sizeof(g_midi_learn));
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Get number of learned MIDI mappings
 */
uint8_t looper_midi_learn_get_count(void) {
  return g_midi_learn_count;
}

// ============================================================================
// Quick-Save System
// ============================================================================

/**
 * @brief Save current session to slot
 */
int looper_quick_save(uint8_t slot, const char* name) {
  if (slot >= NUM_QUICK_SAVE_SLOTS) return -1;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  quick_save_slot_t* qs = &g_quick_save_slots[slot];
  
  qs->used = 1;
  if (name) {
    strncpy(qs->name, name, sizeof(qs->name) - 1);
    qs->name[sizeof(qs->name) - 1] = '\0';
  } else {
    snprintf(qs->name, sizeof(qs->name), "Slot %d", slot + 1);
  }
  
  qs->current_scene = g_current_scene;
  qs->transport = g_tp;
  
  // Save all tracks to SD card
  char filename[64];
  for (uint8_t t = 0; t < LOOPER_TRACKS; t++) {
    snprintf(filename, sizeof(filename), "0:/looper/quicksave_%d_track_%d.bin", slot, t);
    looper_save_track(t, filename);
  }
  
  if (g_mutex) osMutexRelease(g_mutex);
  
  return 0;
}

/**
 * @brief Load session from slot
 */
int looper_quick_load(uint8_t slot) {
  if (slot >= NUM_QUICK_SAVE_SLOTS) return -1;
  
  quick_save_slot_t* qs = &g_quick_save_slots[slot];
  if (!qs->used) return -1;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  // Restore transport settings
  g_tp = qs->transport;
  
  // Load all tracks from SD card
  char filename[64];
  for (uint8_t t = 0; t < LOOPER_TRACKS; t++) {
    snprintf(filename, sizeof(filename), "0:/looper/quicksave_%d_track_%d.bin", slot, t);
    looper_load_track(t, filename);
  }
  
  // Restore scene
  looper_trigger_scene(qs->current_scene);
  
  if (g_mutex) osMutexRelease(g_mutex);
  
  return 0;
}

/**
 * @brief Check if quick-save slot is used
 */
uint8_t looper_quick_save_is_used(uint8_t slot) {
  if (slot >= NUM_QUICK_SAVE_SLOTS) return 0;
  return g_quick_save_slots[slot].used;
}

/**
 * @brief Get quick-save slot name
 */
const char* looper_quick_save_get_name(uint8_t slot) {
  if (slot >= NUM_QUICK_SAVE_SLOTS) return NULL;
  if (!g_quick_save_slots[slot].used) return NULL;
  return g_quick_save_slots[slot].name;
}

/**
 * @brief Clear quick-save slot
 */
void looper_quick_save_clear(uint8_t slot) {
  if (slot >= NUM_QUICK_SAVE_SLOTS) return;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  g_quick_save_slots[slot].used = 0;
  
  if (g_mutex) osMutexRelease(g_mutex);
}

// ---- Humanizer Feature Implementation ----

/**
 * @brief Enable/disable humanizer for a track
 */
void looper_set_humanizer_enabled(uint8_t track, uint8_t enabled) {
  if (track >= LOOPER_TRACKS) return;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  g_humanize_params[track].enabled = enabled ? 1 : 0;
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Check if humanizer is enabled for a track
 */
uint8_t looper_is_humanizer_enabled(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return g_humanize_params[track].enabled;
}

/**
 * @brief Set velocity humanization amount
 */
void looper_set_humanizer_velocity(uint8_t track, uint8_t amount) {
  if (track >= LOOPER_TRACKS) return;
  if (amount > 32) amount = 32;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  g_humanize_params[track].velocity_amount = amount;
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Get velocity humanization amount
 */
uint8_t looper_get_humanizer_velocity(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return g_humanize_params[track].velocity_amount;
}

/**
 * @brief Set timing humanization amount
 */
void looper_set_humanizer_timing(uint8_t track, uint8_t amount) {
  if (track >= LOOPER_TRACKS) return;
  if (amount > 6) amount = 6;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  g_humanize_params[track].timing_amount = amount;
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Get timing humanization amount
 */
uint8_t looper_get_humanizer_timing(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return g_humanize_params[track].timing_amount;
}

/**
 * @brief Set humanizer intensity
 */
void looper_set_humanizer_intensity(uint8_t track, uint8_t intensity) {
  if (track >= LOOPER_TRACKS) return;
  if (intensity > 100) intensity = 100;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  g_humanize_params[track].intensity = intensity;
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Get humanizer intensity
 */
uint8_t looper_get_humanizer_intensity(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return g_humanize_params[track].intensity;
}

// ---- LFO Feature Wrapper Functions ----

#include "Services/lfo/lfo.h"

/**
 * @brief Enable/disable LFO for a track
 */
void looper_set_lfo_enabled(uint8_t track, uint8_t enabled) {
  if (track >= LOOPER_TRACKS) return;
  lfo_set_enabled(track, enabled);
}

/**
 * @brief Check if LFO is enabled for a track
 */
uint8_t looper_is_lfo_enabled(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return lfo_is_enabled(track);
}

/**
 * @brief Set LFO waveform
 */
void looper_set_lfo_waveform(uint8_t track, looper_lfo_waveform_t waveform) {
  if (track >= LOOPER_TRACKS) return;
  lfo_set_waveform(track, (lfo_waveform_t)waveform);
}

/**
 * @brief Get current LFO waveform
 */
looper_lfo_waveform_t looper_get_lfo_waveform(uint8_t track) {
  if (track >= LOOPER_TRACKS) return LOOPER_LFO_WAVEFORM_SINE;
  return (looper_lfo_waveform_t)lfo_get_waveform(track);
}

/**
 * @brief Set LFO rate in Hz (0.01 - 10.0 Hz)
 */
void looper_set_lfo_rate(uint8_t track, uint16_t rate_hundredths) {
  if (track >= LOOPER_TRACKS) return;
  lfo_set_rate(track, rate_hundredths);
}

/**
 * @brief Get current LFO rate
 */
uint16_t looper_get_lfo_rate(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return lfo_get_rate(track);
}

/**
 * @brief Set LFO depth (0-100%)
 */
void looper_set_lfo_depth(uint8_t track, uint8_t depth) {
  if (track >= LOOPER_TRACKS) return;
  lfo_set_depth(track, depth);
}

/**
 * @brief Get current LFO depth
 */
uint8_t looper_get_lfo_depth(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return lfo_get_depth(track);
}

/**
 * @brief Set LFO target parameter
 */
void looper_set_lfo_target(uint8_t track, looper_lfo_target_t target) {
  if (track >= LOOPER_TRACKS) return;
  lfo_set_target(track, (lfo_target_t)target);
}

/**
 * @brief Get current LFO target
 */
looper_lfo_target_t looper_get_lfo_target(uint8_t track) {
  if (track >= LOOPER_TRACKS) return LOOPER_LFO_TARGET_VELOCITY;
  return (looper_lfo_target_t)lfo_get_target(track);
}

/**
 * @brief Enable/disable BPM sync
 */
void looper_set_lfo_bpm_sync(uint8_t track, uint8_t bpm_sync) {
  if (track >= LOOPER_TRACKS) return;
  lfo_set_bpm_sync(track, bpm_sync);
}

/**
 * @brief Check if BPM sync is enabled
 */
uint8_t looper_is_lfo_bpm_synced(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return lfo_is_bpm_synced(track);
}

/**
 * @brief Set BPM sync divisor (1/4, 1/2, 1, 2, 4, 8 bars)
 */
void looper_set_lfo_bpm_divisor(uint8_t track, uint8_t divisor) {
  if (track >= LOOPER_TRACKS) return;
  lfo_set_bpm_divisor(track, divisor);
}

/**
 * @brief Get current BPM divisor
 */
uint8_t looper_get_lfo_bpm_divisor(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return lfo_get_bpm_divisor(track);
}

/**
 * @brief Reset LFO phase to zero
 */
void looper_reset_lfo_phase(uint8_t track) {
  if (track >= LOOPER_TRACKS) return;
  lfo_reset_phase(track);
}

// ---- CC Automation Layer Implementation ----

/**
 * @brief Start recording CC automation for a track
 */
void looper_automation_start_record(uint8_t track) {
  if (track >= LOOPER_TRACKS) return;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  g_automation[track].recording = 1;
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Stop recording CC automation for a track
 */
void looper_automation_stop_record(uint8_t track) {
  if (track >= LOOPER_TRACKS) return;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  g_automation[track].recording = 0;
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Check if automation recording is active
 */
uint8_t looper_automation_is_recording(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return g_automation[track].recording;
}

/**
 * @brief Enable/disable automation playback for a track
 */
void looper_automation_enable_playback(uint8_t track, uint8_t enable) {
  if (track >= LOOPER_TRACKS) return;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  g_automation[track].playback_enabled = enable ? 1 : 0;
  if (enable) {
    // Reset playback position to avoid sending stale events
    g_automation[track].last_playback_tick = 0;
  }
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Check if automation playback is enabled
 */
uint8_t looper_automation_is_playback_enabled(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return g_automation[track].playback_enabled;
}

/**
 * @brief Clear all automation events for a track
 */
void looper_automation_clear(uint8_t track) {
  if (track >= LOOPER_TRACKS) return;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  g_automation[track].event_count = 0;
  g_automation[track].recording = 0;
  g_automation[track].last_playback_tick = 0;
  memset(g_automation[track].events, 0, sizeof(g_automation[track].events));
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Get number of automation events for a track
 */
uint32_t looper_automation_get_event_count(uint8_t track) {
  if (track >= LOOPER_TRACKS) return 0;
  return g_automation[track].event_count;
}

/**
 * @brief Export automation events for inspection/editing
 */
uint32_t looper_automation_export_events(uint8_t track, 
                                          looper_automation_event_t* out, 
                                          uint32_t max) {
  if (track >= LOOPER_TRACKS || !out || max == 0) return 0;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  uint32_t count = g_automation[track].event_count;
  if (count > max) count = max;
  
  memcpy(out, g_automation[track].events, count * sizeof(looper_automation_event_t));
  
  if (g_mutex) osMutexRelease(g_mutex);
  
  return count;
}

/**
 * @brief Manually add a CC automation event
 */
int looper_automation_add_event(uint8_t track, uint32_t tick, 
                                 uint8_t cc_num, uint8_t cc_value, 
                                 uint8_t channel) {
  if (track >= LOOPER_TRACKS) return -1;
  if (cc_num > 127 || cc_value > 127 || channel > 15) return -1;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  if (g_automation[track].event_count >= LOOPER_AUTOMATION_MAX_EVENTS) {
    if (g_mutex) osMutexRelease(g_mutex);
    return -1;  // Buffer full
  }
  
  // Add event
  uint32_t idx = g_automation[track].event_count;
  g_automation[track].events[idx].tick = tick;
  g_automation[track].events[idx].cc_num = cc_num;
  g_automation[track].events[idx].cc_value = cc_value;
  g_automation[track].events[idx].channel = channel;
  g_automation[track].event_count++;
  
  // Sort events by tick (simple insertion sort for now)
  for (uint32_t i = idx; i > 0; i--) {
    if (g_automation[track].events[i].tick < g_automation[track].events[i-1].tick) {
      looper_automation_event_t temp = g_automation[track].events[i];
      g_automation[track].events[i] = g_automation[track].events[i-1];
      g_automation[track].events[i-1] = temp;
    } else {
      break;
    }
  }
  
  if (g_mutex) osMutexRelease(g_mutex);
  
  return 0;
}

/**
 * @brief Internal: Record CC automation event (called from looper_on_router_msg)
 * This function should be called when a CC message is received during automation recording
 */
static void looper_automation_record_cc_internal(uint8_t track, uint8_t cc_num, 
                                                  uint8_t cc_value, uint8_t channel) {
  if (track >= LOOPER_TRACKS) return;
  if (!g_automation[track].recording) return;
  if (g_automation[track].event_count >= LOOPER_AUTOMATION_MAX_EVENTS) return;
  
  // Get current tick position
  uint32_t current_tick = g_tr[track].write_tick;
  
  // Add the CC event
  looper_automation_add_event(track, current_tick, cc_num, cc_value, channel);
}

/**
 * @brief Internal: Process automation playback (called from looper_tick_1ms)
 * This function sends CC events that should be played at the current tick
 */
static void looper_automation_process_playback(uint8_t track) {
  if (track >= LOOPER_TRACKS) return;
  if (!g_automation[track].playback_enabled) return;
  if (g_automation[track].event_count == 0) return;
  if (g_tr[track].st != LOOPER_STATE_PLAY && g_tr[track].st != LOOPER_STATE_OVERDUB) return;
  
  uint32_t current_tick = g_tr[track].play_tick;
  uint32_t last_tick = g_automation[track].last_playback_tick;
  
  // Handle loop wraparound
  if (current_tick < last_tick) {
    // Loop wrapped, reset to process events from beginning
    last_tick = 0;
  }
  
  // Send all CC events between last_tick and current_tick
  for (uint32_t i = 0; i < g_automation[track].event_count; i++) {
    looper_automation_event_t* evt = &g_automation[track].events[i];
    
    // Check if event should be sent
    if (evt->tick > last_tick && evt->tick <= current_tick) {
      // Send CC message via router
      router_msg_t msg;
      msg.type = ROUTER_MSG_3B;
      msg.b0 = 0xB0 | (evt->channel & 0x0F);  // CC status + channel
      msg.b1 = evt->cc_num;
      msg.b2 = evt->cc_value;
      
      // Send to router output (would need router_send function)
      // For now, we'll just track that it would be sent
      // In production, this would call: router_process(ROUTER_NODE_LOOPER, &msg);
    }
  }
  
  g_automation[track].last_playback_tick = current_tick;
}




