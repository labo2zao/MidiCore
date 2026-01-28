/**
 * @file harmonizer_cli.c
 * @brief CLI integration for harmonizer module
 * 
 * MIDI harmonizer - adds harmony notes
 */

#include "Services/harmonizer/harmonizer.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(harmonizer, enabled, harmonizer_is_enabled, harmonizer_set_enabled)

static int harmonizer_param_get_voice1_interval(uint8_t track, param_value_t* out) {
  out->int_val = harmonizer_get_voice_interval(track, 0);
  return 0;
}

static int harmonizer_param_set_voice1_interval(uint8_t track, const param_value_t* val) {
  if (val->int_val < 0 || val->int_val >= HARM_INTERVAL_COUNT) return -1;
  harmonizer_set_voice_interval(track, 0, (harmonizer_interval_t)val->int_val);
  return 0;
}

static int harmonizer_param_get_voice1_enabled(uint8_t track, param_value_t* out) {
  out->bool_val = harmonizer_is_voice_enabled(track, 0);
  return 0;
}

static int harmonizer_param_set_voice1_enabled(uint8_t track, const param_value_t* val) {
  harmonizer_set_voice_enabled(track, 0, val->bool_val);
  return 0;
}

static int harmonizer_param_get_voice2_interval(uint8_t track, param_value_t* out) {
  out->int_val = harmonizer_get_voice_interval(track, 1);
  return 0;
}

static int harmonizer_param_set_voice2_interval(uint8_t track, const param_value_t* val) {
  if (val->int_val < 0 || val->int_val >= HARM_INTERVAL_COUNT) return -1;
  harmonizer_set_voice_interval(track, 1, (harmonizer_interval_t)val->int_val);
  return 0;
}

static int harmonizer_param_get_voice2_enabled(uint8_t track, param_value_t* out) {
  out->bool_val = harmonizer_is_voice_enabled(track, 1);
  return 0;
}

static int harmonizer_param_set_voice2_enabled(uint8_t track, const param_value_t* val) {
  harmonizer_set_voice_enabled(track, 1, val->bool_val);
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(harmonizer, harmonizer_set_enabled, harmonizer_is_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_interval_names[] = {
  "UNISON",
  "THIRD_UP",
  "THIRD_DOWN",
  "FIFTH_UP",
  "FIFTH_DOWN",
  "OCTAVE_UP",
  "OCTAVE_DOWN",
  "FOURTH_UP",
  "FOURTH_DOWN",
  "SIXTH_UP",
  "SIXTH_DOWN",
};

// =============================================================================
// INIT WRAPPER
// =============================================================================

static int harmonizer_cli_init(void) {
  harmonizer_init();
  return 0;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_harmonizer_descriptor = {
  .name = "harmonizer",
  .description = "MIDI harmonizer - adds harmony notes",
  .category = MODULE_CATEGORY_EFFECT,
  .init = harmonizer_cli_init,
  .enable = harmonizer_cli_enable,
  .disable = harmonizer_cli_disable,
  .get_status = harmonizer_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_harmonizer_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(harmonizer, enabled, "Enable harmonizer"),
    {
      .name = "voice1_interval",
      .description = "Voice 1 interval",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 10,
      .enum_values = s_interval_names,
      .enum_count = 11,
      .read_only = 0,
      .get_value = harmonizer_param_get_voice1_interval,
      .set_value = harmonizer_param_set_voice1_interval
    },
    {
      .name = "voice1_enabled",
      .description = "Enable voice 1",
      .type = PARAM_TYPE_BOOL,
      .read_only = 0,
      .get_value = harmonizer_param_get_voice1_enabled,
      .set_value = harmonizer_param_set_voice1_enabled
    },
    {
      .name = "voice2_interval",
      .description = "Voice 2 interval",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 10,
      .enum_values = s_interval_names,
      .enum_count = 11,
      .read_only = 0,
      .get_value = harmonizer_param_get_voice2_interval,
      .set_value = harmonizer_param_set_voice2_interval
    },
    {
      .name = "voice2_enabled",
      .description = "Enable voice 2",
      .type = PARAM_TYPE_BOOL,
      .read_only = 0,
      .get_value = harmonizer_param_get_voice2_enabled,
      .set_value = harmonizer_param_set_voice2_enabled
    },
  };
  
  s_harmonizer_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_harmonizer_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int harmonizer_register_cli(void) {
  setup_harmonizer_parameters();
  return module_registry_register(&s_harmonizer_descriptor);
}
