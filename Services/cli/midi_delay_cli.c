/**
 * @file midi_delay_cli.c
 * @brief CLI integration for MIDI delay/echo module
 */

#include "Services/midi_delay/midi_delay.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(midi_delay, enabled, midi_delay_is_enabled, midi_delay_set_enabled)

static int midi_delay_param_get_division(uint8_t track, param_value_t* out) {
  out->int_val = midi_delay_get_division(track);
  return 0;
}

static int midi_delay_param_set_division(uint8_t track, const param_value_t* val) {
  if (val->int_val < 0 || val->int_val >= 13) return -1;
  midi_delay_set_division(track, (uint8_t)val->int_val);
  return 0;
}

static int midi_delay_param_get_feedback(uint8_t track, param_value_t* out) {
  out->int_val = midi_delay_get_feedback(track);
  return 0;
}

static int midi_delay_param_set_feedback(uint8_t track, const param_value_t* val) {
  if (val->int_val < 0 || val->int_val > 100) return -1;
  midi_delay_set_feedback(track, (uint8_t)val->int_val);
  return 0;
}

static int midi_delay_param_get_mix(uint8_t track, param_value_t* out) {
  out->int_val = midi_delay_get_mix(track);
  return 0;
}

static int midi_delay_param_set_mix(uint8_t track, const param_value_t* val) {
  if (val->int_val < 0 || val->int_val > 100) return -1;
  midi_delay_set_mix(track, (uint8_t)val->int_val);
  return 0;
}

static int midi_delay_param_get_velocity_decay(uint8_t track, param_value_t* out) {
  out->int_val = midi_delay_get_velocity_decay(track);
  return 0;
}

static int midi_delay_param_set_velocity_decay(uint8_t track, const param_value_t* val) {
  if (val->int_val < 0 || val->int_val > 100) return -1;
  midi_delay_set_velocity_decay(track, (uint8_t)val->int_val);
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(midi_delay, midi_delay_set_enabled, midi_delay_is_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_division_names[] = {
  "1_64",
  "1_32",
  "1_16",
  "1_8",
  "1_4",
  "1_2",
  "1_1",
  "1_32T",
  "1_16T",
  "1_8T",
  "1_4T",
  "1_16_DOT",
  "1_8_DOT"
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_midi_delay_descriptor = {
  .name = "midi_delay",
  .description = "MIDI delay/echo with tempo sync",
  .category = MODULE_CATEGORY_EFFECT,
  .init = midi_delay_init,
  .enable = midi_delay_cli_enable,
  .disable = midi_delay_cli_disable,
  .get_status = midi_delay_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_midi_delay_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(midi_delay, enabled, "Enable delay"),
    {
      .name = "division",
      .description = "Time division",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 12,
      .enum_values = s_division_names,
      .enum_count = 13,
      .read_only = 0,
      .get_value = midi_delay_param_get_division,
      .set_value = midi_delay_param_set_division
    },
    PARAM_INT(midi_delay, feedback, "Feedback amount (0-100%)", 0, 100),
    PARAM_INT(midi_delay, mix, "Wet/dry mix (0-100%)", 0, 100),
    PARAM_INT(midi_delay, velocity_decay, "Velocity decay per repeat (0-100%)", 0, 100)
  };
  
  s_midi_delay_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_midi_delay_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int midi_delay_register_cli(void) {
  setup_midi_delay_parameters();
  return module_registry_register(&s_midi_delay_descriptor);
}

/*
 * CLI Examples:
 * module enable midi_delay 0
 * module set midi_delay division 1_8 0
 * module set midi_delay feedback 60 0
 * module set midi_delay mix 30 0
 * module set midi_delay velocity_decay 20 0
 */
