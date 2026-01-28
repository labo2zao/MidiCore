/**
 * @file note_stabilizer_cli.c
 * @brief CLI integration for note_stabilizer module
 * 
 * Stabilize note timing and velocity
 */

#include "Services/note_stabilizer/note_stabilizer.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(note_stabilizer, enabled, note_stabilizer_get_enabled, note_stabilizer_set_enabled)

DEFINE_PARAM_INT_TRACK(note_stabilizer, min_duration_ms, note_stabilizer_get_min_duration_ms, note_stabilizer_set_min_duration_ms)

DEFINE_PARAM_INT_TRACK(note_stabilizer, retrigger_delay_ms, note_stabilizer_get_retrigger_delay_ms, note_stabilizer_set_retrigger_delay_ms)

DEFINE_PARAM_INT_TRACK(note_stabilizer, neighbor_range, note_stabilizer_get_neighbor_range, note_stabilizer_set_neighbor_range)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(note_stabilizer, note_stabilizer_set_enabled, note_stabilizer_is_enabled)

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_note_stabilizer_descriptor = {
  .name = "note_stabilizer",
  .description = "Stabilize note timing and velocity",
  .category = MODULE_CATEGORY_EFFECT,
  .init = note_stabilizer_init,
  .enable = note_stabilizer_cli_enable,
  .disable = note_stabilizer_cli_disable,
  .get_status = note_stabilizer_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_note_stabilizer_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(note_stabilizer, enabled, "Enable stabilizer"),
    PARAM_INT(note_stabilizer, min_duration_ms, "Min note duration (10-500ms)", 10, 500),
    PARAM_INT(note_stabilizer, retrigger_delay_ms, "Retrigger delay (10-1000ms)", 10, 1000),
    PARAM_INT(note_stabilizer, neighbor_range, "Neighbor semitones (0-12)", 0, 12),
  };
  
  s_note_stabilizer_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_note_stabilizer_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int note_stabilizer_register_cli(void) {
  setup_note_stabilizer_parameters();
  return module_registry_register(&s_note_stabilizer_descriptor);
}
