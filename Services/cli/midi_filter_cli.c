/**
 * @file midi_filter_cli.c
 * @brief CLI integration for MIDI filter module
 * 
 * Provides CLI access to MIDI message filtering and routing control.
 */

#include "Services/midi_filter/midi_filter.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(midi_filter, enabled, midi_filter_is_enabled, midi_filter_set_enabled)

static int midi_filter_param_get_channel_mode(uint8_t track, param_value_t* out) {
  out->int_val = midi_filter_get_channel_mode(track);
  return 0;
}

static int midi_filter_param_set_channel_mode(uint8_t track, const param_value_t* val) {
  if (val->int_val < 0 || val->int_val > 2) return -1;
  midi_filter_set_channel_mode(track, (uint8_t)val->int_val);
  return 0;
}

static int midi_filter_param_get_min_note(uint8_t track, param_value_t* out) {
  uint8_t min, max;
  midi_filter_get_note_range(track, &min, &max);
  out->int_val = min;
  return 0;
}

static int midi_filter_param_set_min_note(uint8_t track, const param_value_t* val) {
  if (val->int_val < 0 || val->int_val > 127) return -1;
  uint8_t min, max;
  midi_filter_get_note_range(track, &min, &max);
  midi_filter_set_note_range(track, (uint8_t)val->int_val, max);
  return 0;
}

static int midi_filter_param_get_max_note(uint8_t track, param_value_t* out) {
  uint8_t min, max;
  midi_filter_get_note_range(track, &min, &max);
  out->int_val = max;
  return 0;
}

static int midi_filter_param_set_max_note(uint8_t track, const param_value_t* val) {
  if (val->int_val < 0 || val->int_val > 127) return -1;
  uint8_t min, max;
  midi_filter_get_note_range(track, &min, &max);
  midi_filter_set_note_range(track, min, (uint8_t)val->int_val);
  return 0;
}

static int midi_filter_param_get_min_velocity(uint8_t track, param_value_t* out) {
  uint8_t min, max;
  midi_filter_get_velocity_range(track, &min, &max);
  out->int_val = min;
  return 0;
}

static int midi_filter_param_set_min_velocity(uint8_t track, const param_value_t* val) {
  if (val->int_val < 1 || val->int_val > 127) return -1;
  uint8_t min, max;
  midi_filter_get_velocity_range(track, &min, &max);
  midi_filter_set_velocity_range(track, (uint8_t)val->int_val, max);
  return 0;
}

static int midi_filter_param_get_max_velocity(uint8_t track, param_value_t* out) {
  uint8_t min, max;
  midi_filter_get_velocity_range(track, &min, &max);
  out->int_val = max;
  return 0;
}

static int midi_filter_param_set_max_velocity(uint8_t track, const param_value_t* val) {
  if (val->int_val < 1 || val->int_val > 127) return -1;
  uint8_t min, max;
  midi_filter_get_velocity_range(track, &min, &max);
  midi_filter_set_velocity_range(track, min, (uint8_t)val->int_val);
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(midi_filter, midi_filter_set_enabled, midi_filter_is_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_channel_mode_names[] = {
  "ALL",
  "ALLOW",
  "BLOCK"
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_midi_filter_descriptor = {
  .name = "midi_filter",
  .description = "MIDI message filtering and routing control",
  .category = MODULE_CATEGORY_EFFECT,
  .init = midi_filter_init,
  .enable = midi_filter_cli_enable,
  .disable = midi_filter_cli_disable,
  .get_status = midi_filter_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_midi_filter_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(midi_filter, enabled, "Enable filter"),
    {
      .name = "channel_mode",
      .description = "Channel filter mode (ALL/ALLOW/BLOCK)",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 2,
      .enum_values = s_channel_mode_names,
      .enum_count = 3,
      .read_only = 0,
      .get_value = midi_filter_param_get_channel_mode,
      .set_value = midi_filter_param_set_channel_mode
    },
    PARAM_INT(midi_filter, min_note, "Minimum note to pass (0-127)", 0, 127),
    PARAM_INT(midi_filter, max_note, "Maximum note to pass (0-127)", 0, 127),
    PARAM_INT(midi_filter, min_velocity, "Minimum velocity to pass (1-127)", 1, 127),
    PARAM_INT(midi_filter, max_velocity, "Maximum velocity to pass (1-127)", 1, 127)
  };
  
  s_midi_filter_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_midi_filter_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int midi_filter_register_cli(void) {
  setup_midi_filter_parameters();
  return module_registry_register(&s_midi_filter_descriptor);
}

// =============================================================================
// CLI USAGE EXAMPLES
// =============================================================================

/*
 * module enable midi_filter 0
 * module disable midi_filter 1
 * module set midi_filter enabled true 0
 * module set midi_filter channel_mode ALLOW 0
 * module set midi_filter min_note 36 0
 * module set midi_filter max_note 96 0
 * module set midi_filter min_velocity 10 0
 * module set midi_filter max_velocity 120 0
 */
