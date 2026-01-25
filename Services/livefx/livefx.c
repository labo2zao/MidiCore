/**
 * @file livefx.c
 * @brief Live FX system for real-time MIDI manipulation
 */

#include "Services/livefx/livefx.h"
#include "Services/scale/scale.h"
#include <string.h>

// Per-track LiveFX configurations
static livefx_config_t g_livefx[LIVEFX_MAX_TRACKS];

/**
 * @brief Initialize LiveFX system
 */
void livefx_init(void) {
  memset(g_livefx, 0, sizeof(g_livefx));
  
  // Initialize defaults
  for (uint8_t i = 0; i < LIVEFX_MAX_TRACKS; i++) {
    g_livefx[i].transpose = 0;
    g_livefx[i].vel_scale = 128;  // 100%
    g_livefx[i].force_scale = 0;
    g_livefx[i].scale_type = 0;   // Chromatic (no change)
    g_livefx[i].scale_root = 0;   // C
    g_livefx[i].enabled = 0;      // Bypass by default
  }
}

/**
 * @brief Set transpose amount for a track
 */
void livefx_set_transpose(uint8_t track, int8_t semitones) {
  if (track >= LIVEFX_MAX_TRACKS) return;
  
  // Clamp to -12..+12
  if (semitones < -12) semitones = -12;
  if (semitones > 12) semitones = 12;
  
  g_livefx[track].transpose = semitones;
}

/**
 * @brief Get transpose amount for a track
 */
int8_t livefx_get_transpose(uint8_t track) {
  if (track >= LIVEFX_MAX_TRACKS) return 0;
  return g_livefx[track].transpose;
}

/**
 * @brief Set velocity scale for a track
 */
void livefx_set_velocity_scale(uint8_t track, uint8_t scale) {
  if (track >= LIVEFX_MAX_TRACKS) return;
  g_livefx[track].vel_scale = scale;
}

/**
 * @brief Get velocity scale for a track
 */
uint8_t livefx_get_velocity_scale(uint8_t track) {
  if (track >= LIVEFX_MAX_TRACKS) return 128;
  return g_livefx[track].vel_scale;
}

/**
 * @brief Set force-to-scale for a track
 */
void livefx_set_force_scale(uint8_t track, uint8_t scale_type, uint8_t root, uint8_t enable) {
  if (track >= LIVEFX_MAX_TRACKS) return;
  
  g_livefx[track].scale_type = scale_type;
  g_livefx[track].scale_root = root % 12;
  g_livefx[track].force_scale = enable ? 1 : 0;
}

/**
 * @brief Get force-to-scale configuration
 */
void livefx_get_force_scale(uint8_t track, uint8_t* scale_type, uint8_t* root, uint8_t* enable) {
  if (track >= LIVEFX_MAX_TRACKS) return;
  
  if (scale_type) *scale_type = g_livefx[track].scale_type;
  if (root) *root = g_livefx[track].scale_root;
  if (enable) *enable = g_livefx[track].force_scale;
}

/**
 * @brief Enable/disable LiveFX for a track
 */
void livefx_set_enabled(uint8_t track, uint8_t enable) {
  if (track >= LIVEFX_MAX_TRACKS) return;
  g_livefx[track].enabled = enable ? 1 : 0;
}

/**
 * @brief Check if LiveFX is enabled for a track
 */
uint8_t livefx_get_enabled(uint8_t track) {
  if (track >= LIVEFX_MAX_TRACKS) return 0;
  return g_livefx[track].enabled;
}

/**
 * @brief Apply transpose to a note
 */
static uint8_t apply_transpose(uint8_t note, int8_t transpose) {
  int16_t result = (int16_t)note + transpose;
  if (result < 0) result = 0;
  if (result > 127) result = 127;
  return (uint8_t)result;
}

/**
 * @brief Apply velocity scaling
 */
static uint8_t apply_velocity_scale(uint8_t velocity, uint8_t scale) {
  if (scale == 128) return velocity;  // No change at 100%
  
  uint16_t result = ((uint16_t)velocity * scale) / 128;
  if (result > 127) result = 127;
  return (uint8_t)result;
}

/**
 * @brief Apply LiveFX to a MIDI message
 */
int livefx_apply(uint8_t track, router_msg_t* msg) {
  if (track >= LIVEFX_MAX_TRACKS) return 0;
  if (!msg) return -1;
  
  livefx_config_t* fx = &g_livefx[track];
  
  // Bypass if not enabled
  if (!fx->enabled) return 0;
  
  // Only process channel messages (not system messages)
  if (msg->type == ROUTER_MSG_SYSEX) return 0;
  if ((msg->b0 & 0xF0) >= 0xF0) return 0;  // System messages
  
  uint8_t status = msg->b0 & 0xF0;
  
  // Process Note On/Off messages
  if (status == 0x90 || status == 0x80) {
    uint8_t note = msg->b1;
    uint8_t velocity = msg->b2;
    
    // Apply transpose
    if (fx->transpose != 0) {
      note = apply_transpose(note, fx->transpose);
    }
    
    // Apply force-to-scale
    if (fx->force_scale) {
      note = scale_quantize_note(note, fx->scale_type, fx->scale_root);
    }
    
    // Apply velocity scaling (Note On only)
    if (status == 0x90 && fx->vel_scale != 128) {
      velocity = apply_velocity_scale(velocity, fx->vel_scale);
    }
    
    msg->b1 = note;
    msg->b2 = velocity;
  }
  
  // Process Polyphonic Aftertouch (transpose note)
  else if (status == 0xA0) {
    uint8_t note = msg->b1;
    
    if (fx->transpose != 0) {
      note = apply_transpose(note, fx->transpose);
    }
    
    if (fx->force_scale) {
      note = scale_quantize_note(note, fx->scale_type, fx->scale_root);
    }
    
    msg->b1 = note;
  }
  
  return 0;
}

/**
 * @brief Get configuration for a track
 */
const livefx_config_t* livefx_get_config(uint8_t track) {
  if (track >= LIVEFX_MAX_TRACKS) return NULL;
  return &g_livefx[track];
}
