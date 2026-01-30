/**
 * @file strum_cli.c
 * @brief CLI integration for strum module
 * 
 * Guitar-style strum effect
 */

#include "Services/strum/strum.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(strum, enabled, strum_get_enabled, strum_set_enabled)

DEFINE_PARAM_INT_TRACK(strum, time, strum_get_time, strum_set_time)

static int strum_param_get_direction(uint8_t track, param_value_t* out) {
  
  out->int_val = strum_get_direction(track);
  return 0;
}

static int strum_param_set_direction(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 4) return -1;
  strum_set_direction(track, (uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_BOOL_TRACK(strum, velocity_ramp, strum_get_velocity_ramp, strum_set_velocity_ramp)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(strum, strum_set_enabled, strum_is_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_direction_names[] = {
  "UP",
  "DOWN",
  "UP_DOWN",
  "RANDOM",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_strum_descriptor = {
  .name = "strum",
  .description = "Guitar-style strum effect",
  .category = MODULE_CATEGORY_EFFECT,
  .init = strum_init,
  .enable = strum_cli_enable,
  .disable = strum_cli_disable,
  .get_status = strum_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_strum_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(strum, enabled, "Enable strum"),
    PARAM_INT(strum, time, "Strum time (0-200ms)", 0, 200),
    {
      .name = "direction",
      .description = "Direction",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 3,
      .enum_values = s_direction_names,
      .enum_count = 4,
      .read_only = 0,
      .get_value = strum_param_get_direction,
      .set_value = strum_param_set_direction
    },
    PARAM_BOOL(strum, velocity_ramp, "Velocity ramp"),
  };
  
  s_strum_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_strum_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int strum_register_cli(void) {
  setup_strum_parameters();
  return module_registry_register(&s_strum_descriptor);
}
