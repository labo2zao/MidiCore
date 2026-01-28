/**
 * @file one_finger_chord_cli.c
 * @brief CLI integration for one-finger chord generator
 * 
 * Accessibility feature for playing full chords with single notes
 */

#include "Services/one_finger_chord/one_finger_chord.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_ENUM_TRACK(ofc, mode, ofc_get_mode, ofc_set_mode, ofc_mode_t)
DEFINE_PARAM_ENUM_TRACK(ofc, voicing, ofc_get_voicing, ofc_set_voicing, ofc_voicing_t)
DEFINE_PARAM_INT_TRACK(ofc, split_point, ofc_get_split_point, ofc_set_split_point)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int ofc_cli_enable(uint8_t track) {
  if (track >= ONE_FINGER_MAX_TRACKS) return -1;
  ofc_set_mode(track, OFC_MODE_SINGLE_NOTE_CHORD);
  return 0;
}

static int ofc_cli_disable(uint8_t track) {
  if (track >= ONE_FINGER_MAX_TRACKS) return -1;
  ofc_set_mode(track, OFC_MODE_DISABLED);
  return 0;
}

static int ofc_cli_get_status(uint8_t track) {
  if (track >= ONE_FINGER_MAX_TRACKS) return MODULE_STATUS_ERROR;
  return (ofc_get_mode(track) == OFC_MODE_DISABLED) ? 
         MODULE_STATUS_DISABLED : MODULE_STATUS_ENABLED;
}

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_mode_names[] = {
  "DISABLED",
  "AUTO",
  "SPLIT_KEYBOARD",
  "SINGLE_NOTE_CHORD",
};

static const char* s_voicing_names[] = {
  "SIMPLE",    // Root + 5th
  "TRIAD",     // Root + 3rd + 5th
  "SEVENTH",   // Root + 3rd + 5th + 7th
  "FULL",      // All chord tones
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_ofc_descriptor = {
  .name = "one_finger_chord",
  .description = "Accessibility: one-finger chord generation",
  .category = MODULE_CATEGORY_ACCORDION,
  .init = ofc_init,
  .enable = ofc_cli_enable,
  .disable = ofc_cli_disable,
  .get_status = ofc_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0,
  .max_tracks = ONE_FINGER_MAX_TRACKS
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_ofc_parameters(void) {
  module_param_t params[] = {
    {
      .name = "mode",
      .description = "Chord recognition mode",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = OFC_MODE_COUNT - 1,
      .enum_values = s_mode_names,
      .enum_count = OFC_MODE_COUNT,
      .read_only = 0,
      .get_value = ofc_param_get_mode,
      .set_value = ofc_param_set_mode
    },
    {
      .name = "voicing",
      .description = "Chord voicing style",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = OFC_VOICING_COUNT - 1,
      .enum_values = s_voicing_names,
      .enum_count = OFC_VOICING_COUNT,
      .read_only = 0,
      .get_value = ofc_param_get_voicing,
      .set_value = ofc_param_set_voicing
    },
    {
      .name = "split_point",
      .description = "Keyboard split note (0-127, for SPLIT mode)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 127,
      .read_only = 0,
      .get_value = ofc_param_get_split_point,
      .set_value = ofc_param_set_split_point
    }
  };
  
  s_ofc_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_ofc_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int one_finger_chord_register_cli(void) {
  setup_ofc_parameters();
  return module_registry_register(&s_ofc_descriptor);
}
