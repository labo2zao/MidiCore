/**
 * @file note_repeat_cli.c
 * @brief CLI integration for note_repeat module
 * 
 * Note repeat/ratchet/stutter (MPC-style)
 */

#include "Services/note_repeat/note_repeat.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(note_repeat, enabled, note_repeat_get_enabled, note_repeat_set_enabled)

static int note_repeat_param_get_rate(uint8_t track, param_value_t* out) {
  
  out->int_val = note_repeat_get_rate(track);
  return 0;
}

static int note_repeat_param_set_rate(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 7) return -1;
  note_repeat_set_rate(track, (uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_INT_TRACK(note_repeat, gate, note_repeat_get_gate, note_repeat_set_gate)

DEFINE_PARAM_INT_TRACK(note_repeat, velocity_decay, note_repeat_get_velocity_decay, note_repeat_set_velocity_decay)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(note_repeat, note_repeat_set_enabled, note_repeat_is_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_rate_names[] = {
  "1_4",
  "1_8",
  "1_16",
  "1_32",
  "1_8T",
  "1_16T",
  "1_32T",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_note_repeat_descriptor = {
  .name = "note_repeat",
  .description = "Note repeat/ratchet/stutter (MPC-style)",
  .category = MODULE_CATEGORY_EFFECT,
  .init = note_repeat_init,
  .enable = note_repeat_cli_enable,
  .disable = note_repeat_cli_disable,
  .get_status = note_repeat_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_note_repeat_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(note_repeat, enabled, "Enable repeat"),
    {
      .name = "rate",
      .description = "Repeat rate",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 6,
      .enum_values = s_rate_names,
      .enum_count = 7,
      .read_only = 0,
      .get_value = note_repeat_param_get_rate,
      .set_value = note_repeat_param_set_rate
    },
    PARAM_INT(note_repeat, gate, "Gate length (1-100%)", 1, 100),
    PARAM_INT(note_repeat, velocity_decay, "Velocity decay (0-100%)", 0, 100),
  };
  
  s_note_repeat_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_note_repeat_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int note_repeat_register_cli(void) {
  setup_note_repeat_parameters();
  return module_registry_register(&s_note_repeat_descriptor);
}
