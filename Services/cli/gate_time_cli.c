/**
 * @file gate_time_cli.c
 * @brief CLI integration for gate_time module
 * 
 * Note length/gate time control
 */

#include "Services/gate_time/gate_time.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(gate_time, enabled, gate_time_get_enabled, gate_time_set_enabled)

static int gate_time_param_get_mode(uint8_t track, param_value_t* out) {
  
  out->int_val = gate_time_get_mode(track);
  return 0;
}

static int gate_time_param_set_mode(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 3) return -1;
  gate_time_set_mode(track, (uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_INT_TRACK(gate_time, value, gate_time_get_value, gate_time_set_value)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(gate_time, gate_time_set_enabled, gate_time_is_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_mode_names[] = {
  "FIXED",
  "PERCENT",
  "ADD_SUBTRACT",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_gate_time_descriptor = {
  .name = "gate_time",
  .description = "Note length/gate time control",
  .category = MODULE_CATEGORY_EFFECT,
  .init = gate_time_init,
  .enable = gate_time_cli_enable,
  .disable = gate_time_cli_disable,
  .get_status = gate_time_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_gate_time_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(gate_time, enabled, "Enable gate control"),
    {
      .name = "mode",
      .description = "Mode",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 2,
      .enum_values = s_mode_names,
      .enum_count = 3,
      .read_only = 0,
      .get_value = gate_time_param_get_mode,
      .set_value = gate_time_param_set_mode
    },
    PARAM_INT(gate_time, value, "Gate value (depends on mode)", 0, 1000),
  };
  
  s_gate_time_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_gate_time_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int gate_time_register_cli(void) {
  setup_gate_time_parameters();
  return module_registry_register(&s_gate_time_descriptor);
}
