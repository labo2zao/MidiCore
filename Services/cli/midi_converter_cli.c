/**
 * @file midi_converter_cli.c
 * @brief CLI integration for midi_converter module
 * 
 * Convert between MIDI message types
 */

#include "Services/midi_converter/midi_converter.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(midi_converter, enabled, midi_converter_get_enabled, midi_converter_set_enabled)

static int midi_converter_param_get_mode(uint8_t track, param_value_t* out) {
  
  out->int_val = midi_converter_get_mode(track);
  return 0;
}

static int midi_converter_param_set_mode(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 8) return -1;
  midi_converter_set_mode(track, (uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_INT_TRACK(midi_converter, source_cc, midi_converter_get_source_cc, midi_converter_set_source_cc)

DEFINE_PARAM_INT_TRACK(midi_converter, dest_cc, midi_converter_get_dest_cc, midi_converter_set_dest_cc)

DEFINE_PARAM_INT_TRACK(midi_converter, scale, midi_converter_get_scale, midi_converter_set_scale)

DEFINE_PARAM_INT_TRACK(midi_converter, offset, midi_converter_get_offset, midi_converter_set_offset)

DEFINE_PARAM_BOOL_TRACK(midi_converter, invert, midi_converter_get_invert, midi_converter_set_invert)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(midi_converter, midi_converter_set_enabled, midi_converter_is_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_mode_names[] = {
  "CC_TO_AT",
  "AT_TO_CC",
  "PB_TO_CC",
  "CC_TO_PB",
  "VEL_TO_CC",
  "CC_TO_CC",
  "NOTE_TO_CC",
  "CC_TO_NOTE",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_midi_converter_descriptor = {
  .name = "midi_converter",
  .description = "Convert between MIDI message types",
  .category = MODULE_CATEGORY_EFFECT,
  .init = midi_converter_init,
  .enable = midi_converter_cli_enable,
  .disable = midi_converter_cli_disable,
  .get_status = midi_converter_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_midi_converter_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(midi_converter, enabled, "Enable converter"),
    {
      .name = "mode",
      .description = "Conversion mode",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 7,
      .enum_values = s_mode_names,
      .enum_count = 8,
      .read_only = 0,
      .get_value = midi_converter_param_get_mode,
      .set_value = midi_converter_param_set_mode
    },
    PARAM_INT(midi_converter, source_cc, "Source CC number (0-127)", 0, 127),
    PARAM_INT(midi_converter, dest_cc, "Destination CC number (0-127)", 0, 127),
    PARAM_INT(midi_converter, scale, "Scale factor (0-200%)", 0, 200),
    PARAM_INT(midi_converter, offset, "Offset value (-127 to 127)", -127, 127),
    PARAM_BOOL(midi_converter, invert, "Invert values"),
  };
  
  s_midi_converter_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_midi_converter_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int midi_converter_register_cli(void) {
  setup_midi_converter_parameters();
  return module_registry_register(&s_midi_converter_descriptor);
}
