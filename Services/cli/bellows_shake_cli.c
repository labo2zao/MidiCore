/**
 * @file bellows_shake_cli.c
 * @brief CLI integration for bellows_shake module
 * 
 * Tremolo from bellows shaking
 */

#include "Services/bellows_shake/bellows_shake.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(bellows_shake, enabled, bellows_shake_is_enabled, bellows_shake_set_enabled)

DEFINE_PARAM_INT_TRACK(bellows_shake, sensitivity, bellows_shake_get_sensitivity, bellows_shake_set_sensitivity)

DEFINE_PARAM_INT_TRACK(bellows_shake, depth, bellows_shake_get_depth, bellows_shake_set_depth)

static int bellows_shake_param_get_target(uint8_t track, param_value_t* out) {
  out->int_val = bellows_shake_get_target(track);
  return 0;
}

static int bellows_shake_param_set_target(uint8_t track, const param_value_t* val) {
  if (val->int_val < 0 || val->int_val >= 4) return -1;
  bellows_shake_set_target(track, (uint8_t)val->int_val);
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(bellows_shake, bellows_shake_set_enabled, bellows_shake_is_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_target_names[] = {
  "MOD_WHEEL",
  "VOLUME",
  "FILTER",
  "BOTH",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static int bellows_shake_cli_init(void) { 
  bellows_shake_init(); 
  return 0; 
}

static module_descriptor_t s_bellows_shake_descriptor = {
  .name = "bellows_shake",
  .description = "Tremolo from bellows shaking",
  .category = MODULE_CATEGORY_ACCORDION,
  .init = bellows_shake_cli_init,
  .enable = bellows_shake_cli_enable,
  .disable = bellows_shake_cli_disable,
  .get_status = bellows_shake_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_bellows_shake_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(bellows_shake, enabled, "Enable shake detection"),
    PARAM_INT(bellows_shake, sensitivity, "Detection sensitivity (0-100)", 0, 100),
    PARAM_INT(bellows_shake, depth, "Tremolo depth (0-127)", 0, 127),
    {
      .name = "target",
      .description = "Target",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 3,
      .enum_values = s_target_names,
      .enum_count = 4,
      .read_only = 0,
      .get_value = bellows_shake_param_get_target,
      .set_value = bellows_shake_param_set_target
    },
  };
  
  s_bellows_shake_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_bellows_shake_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int bellows_shake_register_cli(void) {
  setup_bellows_shake_parameters();
  return module_registry_register(&s_bellows_shake_descriptor);
}
