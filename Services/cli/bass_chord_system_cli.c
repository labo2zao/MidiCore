/**
 * @file bass_chord_system_cli.c
 * @brief CLI integration for bass_chord_system module
 * 
 * Stradella bass for accordion
 */

#include "Services/bass_chord_system/bass_chord_system.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int bass_chord_system_param_get_layout(uint8_t track, param_value_t* out) {
  
  out->int_val = bass_chord_system_get_layout(track);
  return 0;
}

static int bass_chord_system_param_set_layout(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 5) return -1;
  bass_chord_system_set_layout(track, (uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_INT_TRACK(bass_chord_system, base_note, bass_chord_system_get_base_note, bass_chord_system_set_base_note)

DEFINE_PARAM_BOOL_TRACK(bass_chord_system, octave_doubling, bass_chord_system_get_octave_doubling, bass_chord_system_set_octave_doubling)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int bass_chord_system_cli_enable(uint8_t track) {
  
  return 0;
}

static int bass_chord_system_cli_disable(uint8_t track) {
  
  return 0;
}

static int bass_chord_system_cli_get_status(uint8_t track) {
  
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_layout_names[] = {
  "STRADELLA_120",
  "STRADELLA_96",
  "STRADELLA_72",
  "STRADELLA_48",
  "FREE_BASS",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_bass_chord_system_descriptor = {
  .name = "bass_chord_system",
  .description = "Stradella bass for accordion",
  .category = MODULE_CATEGORY_ACCORDION,
  .init = bass_chord_system_init,
  .enable = bass_chord_system_cli_enable,
  .disable = bass_chord_system_cli_disable,
  .get_status = bass_chord_system_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_bass_chord_system_parameters(void) {
  module_param_t params[] = {
    {
      .name = "layout",
      .description = "Bass layout",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 4,
      .enum_values = s_layout_names,
      .enum_count = 5,
      .read_only = 0,
      .get_value = bass_chord_system_param_get_layout,
      .set_value = bass_chord_system_param_set_layout
    },
    PARAM_INT(bass_chord_system, base_note, "Starting note (0-127)", 0, 127),
    PARAM_BOOL(bass_chord_system, octave_doubling, "Enable octave doubling"),
  };
  
  s_bass_chord_system_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_bass_chord_system_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int bass_chord_system_register_cli(void) {
  setup_bass_chord_system_parameters();
  return module_registry_register(&s_bass_chord_system_descriptor);
}
