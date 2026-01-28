/**
 * @file lfo_cli.c
 * @brief CLI integration for lfo module
 * 
 * Low Frequency Oscillator for modulation
 */

#include "Services/lfo/lfo.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(lfo, enabled, lfo_get_enabled, lfo_set_enabled)

static int lfo_param_get_waveform(uint8_t track, param_value_t* out) {
  
  out->int_val = lfo_get_waveform(track);
  return 0;
}

static int lfo_param_set_waveform(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 6) return -1;
  lfo_set_waveform(track, (uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_INT_TRACK(lfo, rate_hz, lfo_get_rate_hz, lfo_set_rate_hz)

DEFINE_PARAM_INT_TRACK(lfo, depth, lfo_get_depth, lfo_set_depth)

static int lfo_param_get_target(uint8_t track, param_value_t* out) {
  
  out->int_val = lfo_get_target(track);
  return 0;
}

static int lfo_param_set_target(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 4) return -1;
  lfo_set_target(track, (uint8_t)val->int_val);
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(lfo, lfo_set_enabled, lfo_is_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_waveform_names[] = {
  "SINE",
  "TRIANGLE",
  "SQUARE",
  "SAW_UP",
  "SAW_DOWN",
  "RANDOM",
};

static const char* s_target_names[] = {
  "CC",
  "PITCH",
  "VELOCITY",
  "TIMING",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_lfo_descriptor = {
  .name = "lfo",
  .description = "Low Frequency Oscillator for modulation",
  .category = MODULE_CATEGORY_EFFECT,
  .init = lfo_init,
  .enable = lfo_cli_enable,
  .disable = lfo_cli_disable,
  .get_status = lfo_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_lfo_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(lfo, enabled, "Enable LFO"),
    {
      .name = "waveform",
      .description = "Waveform",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 5,
      .enum_values = s_waveform_names,
      .enum_count = 6,
      .read_only = 0,
      .get_value = lfo_param_get_waveform,
      .set_value = lfo_param_set_waveform
    },
    PARAM_INT(lfo, rate_hz, "LFO rate (0.01-10Hz * 100)", 1, 1000),
    PARAM_INT(lfo, depth, "Modulation depth (0-127)", 0, 127),
    {
      .name = "target",
      .description = "Modulation target",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 3,
      .enum_values = s_target_names,
      .enum_count = 4,
      .read_only = 0,
      .get_value = lfo_param_get_target,
      .set_value = lfo_param_set_target
    },
  };
  
  s_lfo_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_lfo_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int lfo_register_cli(void) {
  setup_lfo_parameters();
  return module_registry_register(&s_lfo_descriptor);
}
