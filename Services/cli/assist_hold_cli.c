/**
 * @file assist_hold_cli.c
 * @brief CLI integration for assist_hold module
 * 
 * Auto-hold for motor disabilities
 */

#include "Services/assist_hold/assist_hold.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int assist_hold_param_get_mode(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = assist_hold_get_mode();
  return 0;
}

static int assist_hold_param_set_mode(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val >= 3) return -1;
  assist_hold_set_mode((uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_INT(assist_hold, duration_ms, assist_hold_get_duration_ms, assist_hold_set_duration_ms)

DEFINE_PARAM_INT(assist_hold, velocity_threshold, assist_hold_get_velocity_threshold, assist_hold_set_velocity_threshold)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int assist_hold_cli_enable(uint8_t track) {
  (void)track;
  return 0;
}

static int assist_hold_cli_disable(uint8_t track) {
  (void)track;
  return 0;
}

static int assist_hold_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_mode_names[] = {
  "OFF",
  "PERMANENT",
  "TIMED",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_assist_hold_descriptor = {
  .name = "assist_hold",
  .description = "Auto-hold for motor disabilities",
  .category = MODULE_CATEGORY_ACCESSIBILITY,
  .init = assist_hold_init,
  .enable = assist_hold_cli_enable,
  .disable = assist_hold_cli_disable,
  .get_status = assist_hold_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_assist_hold_parameters(void) {
  module_param_t params[] = {
    {
      .name = "mode",
      .description = "Hold mode",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 2,
      .enum_values = s_mode_names,
      .enum_count = 3,
      .read_only = 0,
      .get_value = assist_hold_param_get_mode,
      .set_value = assist_hold_param_set_mode
    },
    PARAM_INT(assist_hold, duration_ms, "Hold duration (ms, timed mode)", 0, 10000),
    PARAM_INT(assist_hold, velocity_threshold, "Min velocity to hold (1-127)", 1, 127),
  };
  
  s_assist_hold_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_assist_hold_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int assist_hold_register_cli(void) {
  setup_assist_hold_parameters();
  return module_registry_register(&s_assist_hold_descriptor);
}
