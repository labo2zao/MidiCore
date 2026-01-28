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

DEFINE_PARAM_BOOL_TRACK(harmonizer, enabled, harmonizer_get_enabled, harmonizer_set_enabled)

static int harmonizer_param_get_voice1_interval(uint8_t track, param_value_t* out) {
  
  out->int_val = harmonizer_get_voice1_interval(track);
  return 0;
}

static int harmonizer_param_set_voice1_interval(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 7) return -1;
  harmonizer_set_voice1_interval(track, (uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_BOOL_TRACK(harmonizer, voice1_enabled, harmonizer_get_voice1_enabled, harmonizer_set_voice1_enabled)

static int harmonizer_param_get_voice2_interval(uint8_t track, param_value_t* out) {
  
  out->int_val = harmonizer_get_voice2_interval(track);
  return 0;
}

static int harmonizer_param_set_voice2_interval(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 7) return -1;
  harmonizer_set_voice2_interval(track, (uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_BOOL_TRACK(harmonizer, voice2_enabled, harmonizer_get_voice2_enabled, harmonizer_set_voice2_enabled)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(harmonizer, harmonizer_set_enabled, harmonizer_is_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_voice1_interval_names[] = {
  "UNISON",
  "THIRD_UP",
  "THIRD_DOWN",
  "FIFTH_UP",
  "FIFTH_DOWN",
  "OCTAVE_UP",
  "OCTAVE_DOWN",
};

static const char* s_voice2_interval_names[] = {
  "UNISON",
  "THIRD_UP",
  "THIRD_DOWN",
  "FIFTH_UP",
  "FIFTH_DOWN",
  "OCTAVE_UP",
  "OCTAVE_DOWN",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_harmonizer_descriptor = {
  .name = "harmonizer",
  .description = "MIDI harmonizer - adds harmony notes",
  .category = MODULE_CATEGORY_EFFECT,
  .init = harmonizer_init,
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
      .max = 6,
      .enum_values = s_voice1_interval_names,
      .enum_count = 7,
      .read_only = 0,
      .get_value = harmonizer_param_get_voice1_interval,
      .set_value = harmonizer_param_set_voice1_interval
    },
    PARAM_BOOL(harmonizer, voice1_enabled, "Enable voice 1"),
    {
      .name = "voice2_interval",
      .description = "Voice 2 interval",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 6,
      .enum_values = s_voice2_interval_names,
      .enum_count = 7,
      .read_only = 0,
      .get_value = harmonizer_param_get_voice2_interval,
      .set_value = harmonizer_param_set_voice2_interval
    },
    PARAM_BOOL(harmonizer, voice2_enabled, "Enable voice 2"),
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
