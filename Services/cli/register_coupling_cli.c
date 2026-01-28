/**
 * @file register_coupling_cli.c
 * @brief CLI integration for register_coupling module
 * 
 * Accordion register switching
 */

#include "Services/register_coupling/register_coupling.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int register_coupling_param_get_register(uint8_t track, param_value_t* out) {
  
  out->int_val = register_coupling_get_register(track);
  return 0;
}

static int register_coupling_param_set_register(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 10) return -1;
  register_coupling_set_register(track, (uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_BOOL_TRACK(register_coupling, smooth_transition, register_coupling_get_smooth_transition, register_coupling_set_smooth_transition)

DEFINE_PARAM_INT_TRACK(register_coupling, transition_time, register_coupling_get_transition_time, register_coupling_set_transition_time)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int register_coupling_cli_enable(uint8_t track) {
  
  return 0;
}

static int register_coupling_cli_disable(uint8_t track) {
  
  return 0;
}

static int register_coupling_cli_get_status(uint8_t track) {
  
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_register_names[] = {
  "MASTER",
  "MUSETTE",
  "BANDONEON",
  "VIOLIN",
  "CLARINET",
  "BASSOON",
  "PICCOLO",
  "ORGAN",
  "OBOE",
  "FLUTE",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_register_coupling_descriptor = {
  .name = "register_coupling",
  .description = "Accordion register switching",
  .category = MODULE_CATEGORY_ACCORDION,
  .init = register_coupling_init,
  .enable = register_coupling_cli_enable,
  .disable = register_coupling_cli_disable,
  .get_status = register_coupling_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_register_coupling_parameters(void) {
  module_param_t params[] = {
    {
      .name = "register",
      .description = "Current register",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 9,
      .enum_values = s_register_names,
      .enum_count = 10,
      .read_only = 0,
      .get_value = register_coupling_param_get_register,
      .set_value = register_coupling_param_set_register
    },
    PARAM_BOOL(register_coupling, smooth_transition, "Smooth register transition"),
    PARAM_INT(register_coupling, transition_time, "Transition time (ms)", 0, 1000),
  };
  
  s_register_coupling_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_register_coupling_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int register_coupling_register_cli(void) {
  setup_register_coupling_parameters();
  return module_registry_register(&s_register_coupling_descriptor);
}
