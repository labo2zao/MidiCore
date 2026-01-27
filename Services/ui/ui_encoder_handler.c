/**
 * @file ui_encoder_handler.c
 * @brief Rotary Encoder Handler Implementation
 */

#include "ui_encoder_handler.h"
#include <string.h>
#include <stdlib.h>

// =============================================================================
// ENCODER STATE
// =============================================================================

#define MAX_ENCODERS 2

typedef struct {
  int16_t accumulated;      // Accumulated delta (for dead zone)
  uint32_t last_time_ms;    // Last movement time (for acceleration)
  int8_t last_direction;    // Last movement direction
  uint8_t velocity;         // Movement velocity (for acceleration)
} encoder_state_t;

static encoder_config_t s_config;
static encoder_state_t s_state[MAX_ENCODERS];
static uint8_t s_initialized = 0;

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

static uint32_t get_time_ms(void) {
  extern uint32_t HAL_GetTick(void);
  return HAL_GetTick();
}

static int16_t apply_acceleration(int16_t delta, uint8_t velocity) {
  if (!s_config.acceleration_enabled) {
    return delta;
  }
  
  // Acceleration curve: multiply by factor based on velocity
  // velocity 0-10 maps to multiplier 1-10
  uint8_t multiplier = 1 + (velocity * s_config.acceleration_factor) / 10;
  if (multiplier > 10) multiplier = 10;
  
  return delta * multiplier;
}

// =============================================================================
// PUBLIC API
// =============================================================================

void ui_encoder_handler_init(const encoder_config_t* config) {
  if (config) {
    memcpy(&s_config, config, sizeof(encoder_config_t));
  } else {
    ui_encoder_handler_get_defaults(&s_config);
  }
  
  memset(s_state, 0, sizeof(s_state));
  s_initialized = 1;
}

int16_t ui_encoder_handler_process(uint8_t enc_id, int8_t delta, encoder_mode_t mode) {
  if (enc_id >= MAX_ENCODERS) return 0;
  if (delta == 0) return 0;
  
  encoder_state_t* state = &s_state[enc_id];
  uint32_t now = get_time_ms();
  uint32_t dt = now - state->last_time_ms;
  
  // Update velocity based on time since last movement
  if (dt < 50) {
    // Fast movement - increase velocity
    if (state->velocity < 10) state->velocity++;
  } else if (dt > 200) {
    // Slow movement - reset velocity
    state->velocity = 0;
  } else {
    // Medium speed - slowly decrease velocity
    if (state->velocity > 0) state->velocity--;
  }
  
  // Check direction change (resets velocity)
  int8_t direction = (delta > 0) ? 1 : -1;
  if (direction != state->last_direction && state->last_direction != 0) {
    state->velocity = 0;
  }
  state->last_direction = direction;
  state->last_time_ms = now;
  
  // Accumulate delta for dead zone handling
  state->accumulated += delta;
  
  // Check dead zone
  if (abs(state->accumulated) < s_config.dead_zone) {
    return 0;  // Within dead zone
  }
  
  // Process accumulated delta
  int16_t processed_delta = state->accumulated;
  state->accumulated = 0;  // Reset accumulator
  
  // Apply detent steps
  if (s_config.detent_steps > 1) {
    processed_delta = processed_delta / s_config.detent_steps;
    if (processed_delta == 0 && delta != 0) {
      processed_delta = (delta > 0) ? 1 : -1;  // Ensure at least 1 step
    }
  }
  
  // Apply acceleration based on mode
  switch (mode) {
    case ENC_MODE_NAVIGATION:
      // No acceleration for navigation (always 1 step)
      processed_delta = (processed_delta > 0) ? 1 : -1;
      break;
      
    case ENC_MODE_PARAM_EDIT:
    case ENC_MODE_VALUE_ADJUST:
      // Apply acceleration for value adjustment
      processed_delta = apply_acceleration(processed_delta, state->velocity);
      break;
      
    case ENC_MODE_LIST_SELECT:
      // Moderate acceleration for list navigation
      if (state->velocity > 5) {
        processed_delta = apply_acceleration(processed_delta, state->velocity / 2);
      }
      break;
  }
  
  return processed_delta;
}

void ui_encoder_handler_reset(uint8_t enc_id) {
  if (enc_id >= MAX_ENCODERS) return;
  
  memset(&s_state[enc_id], 0, sizeof(encoder_state_t));
}

void ui_encoder_handler_get_defaults(encoder_config_t* config) {
  if (!config) return;
  
  config->acceleration_enabled = 1;
  config->acceleration_factor = 5;  // Moderate acceleration
  config->dead_zone = 1;            // Small dead zone
  config->detent_steps = 1;         // 1 step per detent (adjust for your hardware)
}
