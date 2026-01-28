/**
 * @file chord_cli.c
 * @brief CLI integration for chord module
 * 
 * Chord trigger - single note to chord
 */

#include "Services/chord/chord.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(chord, enabled, chord_get_enabled, chord_set_enabled)

static int chord_param_get_type(uint8_t track, param_value_t* out) {
  
  out->int_val = chord_get_type(track);
  return 0;
}

static int chord_param_set_type(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 9) return -1;
  chord_set_type(track, (uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_INT_TRACK(chord, inversion, chord_get_inversion, chord_set_inversion)

static int chord_param_get_voicing(uint8_t track, param_value_t* out) {
  
  out->int_val = chord_get_voicing(track);
  return 0;
}

static int chord_param_set_voicing(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 4) return -1;
  chord_set_voicing(track, (uint8_t)val->int_val);
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(chord, chord_set_enabled, chord_is_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_type_names[] = {
  "MAJOR",
  "MINOR",
  "DIM",
  "AUG",
  "MAJ7",
  "MIN7",
  "DOM7",
  "SUS2",
  "SUS4",
};

static const char* s_voicing_names[] = {
  "CLOSE",
  "SPREAD",
  "DROP2",
  "DROP3",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static int chord_cli_init(void) { 
  chord_init(); 
  return 0; 
}

static module_descriptor_t s_chord_descriptor = {
  .name = "chord",
  .description = "Chord trigger - single note to chord",
  .category = MODULE_CATEGORY_EFFECT,
  .init = chord_cli_init,
  .enable = chord_cli_enable,
  .disable = chord_cli_disable,
  .get_status = chord_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_chord_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(chord, enabled, "Enable chord trigger"),
    {
      .name = "type",
      .description = "Chord type",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 8,
      .enum_values = s_type_names,
      .enum_count = 9,
      .read_only = 0,
      .get_value = chord_param_get_type,
      .set_value = chord_param_set_type
    },
    PARAM_INT(chord, inversion, "Chord inversion (0-3)", 0, 3),
    {
      .name = "voicing",
      .description = "Voicing",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 3,
      .enum_values = s_voicing_names,
      .enum_count = 4,
      .read_only = 0,
      .get_value = chord_param_get_voicing,
      .set_value = chord_param_set_voicing
    },
  };
  
  s_chord_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_chord_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int chord_register_cli(void) {
  setup_chord_parameters();
  return module_registry_register(&s_chord_descriptor);
}
