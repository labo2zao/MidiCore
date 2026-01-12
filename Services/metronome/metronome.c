#include "metronome.h"
#include "Services/looper/looper.h"
#include "Services/router/router.h"
#include "cmsis_os2.h"
#include <string.h>

#define LOOPER_PPQN 96  // Pulses per quarter note

// Metronome state
static metronome_config_t g_config = {
  .enabled = 0,
  .mode = METRONOME_MODE_MIDI,
  .midi_channel = 9,  // Typically drum channel
  .accent_note = 76,  // High wood block (or cowbell)
  .regular_note = 77, // Low wood block (or hi-hat)
  .accent_velocity = 100,
  .regular_velocity = 80,
  .output_port = 0,
  .count_in_bars = 0
};

static uint16_t g_bpm = 120;
static uint8_t g_ts_num = 4;
static uint8_t g_ts_den = 4;
static uint32_t g_last_click_tick = 0xFFFFFFFF;
static uint8_t g_count_in_active = 0;
static uint32_t g_count_in_start_tick = 0;

static osMutexId_t g_mutex;

void metronome_init(void) {
  const osMutexAttr_t attr = { .attr_bits = osMutexRecursive };
  g_mutex = osMutexNew(&attr);
  
  // Sync with looper initial tempo
  g_bpm = looper_get_tempo();
  looper_transport_t tp;
  looper_get_transport(&tp);
  g_ts_num = tp.ts_num;
  g_ts_den = tp.ts_den;
}

void metronome_set_config(const metronome_config_t* config) {
  if (!config) return;
  
  osMutexAcquire(g_mutex, osWaitForever);
  memcpy(&g_config, config, sizeof(metronome_config_t));
  osMutexRelease(g_mutex);
}

void metronome_get_config(metronome_config_t* out) {
  if (!out) return;
  
  osMutexAcquire(g_mutex, osWaitForever);
  memcpy(out, &g_config, sizeof(metronome_config_t));
  osMutexRelease(g_mutex);
}

void metronome_set_enabled(uint8_t enable) {
  osMutexAcquire(g_mutex, osWaitForever);
  g_config.enabled = enable ? 1 : 0;
  osMutexRelease(g_mutex);
}

uint8_t metronome_get_enabled(void) {
  osMutexAcquire(g_mutex, osWaitForever);
  uint8_t enabled = g_config.enabled;
  osMutexRelease(g_mutex);
  return enabled;
}

void metronome_sync_tempo(uint16_t bpm, uint8_t ts_num, uint8_t ts_den) {
  osMutexAcquire(g_mutex, osWaitForever);
  g_bpm = bpm;
  g_ts_num = ts_num;
  g_ts_den = ts_den;
  osMutexRelease(g_mutex);
}

void metronome_start_count_in(void) {
  osMutexAcquire(g_mutex, osWaitForever);
  if (g_config.count_in_bars == 0) {
    osMutexRelease(g_mutex);
    return;
  }
  
  g_count_in_active = 1;
  g_count_in_start_tick = 0;
  g_last_click_tick = 0xFFFFFFFF;
  osMutexRelease(g_mutex);
}

uint8_t metronome_is_count_in_active(void) {
  osMutexAcquire(g_mutex, osWaitForever);
  uint8_t active = g_count_in_active;
  osMutexRelease(g_mutex);
  return active;
}

void metronome_cancel_count_in(void) {
  osMutexAcquire(g_mutex, osWaitForever);
  g_count_in_active = 0;
  osMutexRelease(g_mutex);
}

static void send_metronome_click(uint8_t is_accent) {
  if (g_config.mode != METRONOME_MODE_MIDI) return;
  
  // Build MIDI note on message
  router_msg_t msg;
  msg.type = ROUTER_MSG_3B;
  msg.b0 = 0x90 | (g_config.midi_channel & 0x0F);  // Note On
  msg.b1 = is_accent ? g_config.accent_note : g_config.regular_note;
  msg.b2 = is_accent ? g_config.accent_velocity : g_config.regular_velocity;
  msg.len = 3;
  msg.data = NULL;
  
  // Send note on through router (from metronome virtual node to configured output)
  // Use router_process with metronome as source node
  // Note: In actual integration, metronome would have its own node ID
  // For now, sending from a virtual node (e.g., 0xFF for internal)
  router_process(0xFF, &msg);
  
  // Note off is typically handled by the receiving synthesizer after a short duration
  // or can be sent explicitly after a delay using MIDI delay queue if needed
}

void metronome_tick_1ms(uint32_t current_tick, uint8_t is_playing) {
  if (!g_config.enabled) return;
  if (!is_playing && !g_count_in_active) return;
  
  osMutexAcquire(g_mutex, osWaitForever);
  
  // Calculate ticks per beat (quarter note = LOOPER_PPQN ticks)
  // For time signature denominator other than 4, adjust
  uint32_t ticks_per_beat = LOOPER_PPQN;
  if (g_ts_den == 8) {
    ticks_per_beat = LOOPER_PPQN / 2;
  } else if (g_ts_den == 2) {
    ticks_per_beat = LOOPER_PPQN * 2;
  }
  
  // Calculate which beat we're on
  uint32_t tick_in_bar = current_tick % (ticks_per_beat * g_ts_num);
  uint32_t current_beat = tick_in_bar / ticks_per_beat;
  uint32_t beat_tick = current_beat * ticks_per_beat;
  
  // Check if we've crossed a beat boundary since last click
  if (beat_tick != g_last_click_tick) {
    g_last_click_tick = beat_tick;
    
    // First beat of bar is accented
    uint8_t is_accent = (current_beat == 0);
    
    send_metronome_click(is_accent);
    
    // Check count-in completion
    if (g_count_in_active && g_config.count_in_bars > 0) {
      uint32_t count_in_ticks = g_config.count_in_bars * g_ts_num * ticks_per_beat;
      if (current_tick >= count_in_ticks) {
        g_count_in_active = 0;
      }
    }
  }
  
  osMutexRelease(g_mutex);
}
