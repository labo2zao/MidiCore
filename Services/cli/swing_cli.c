/**
 * @file swing_cli.c
 * @brief CLI integration for swing module
 * 
 * Swing/groove timing
 */

#include "Services/swing/swing.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(swing, enabled, swing_get_enabled, swing_set_enabled)

DEFINE_PARAM_INT_TRACK(swing, amount, swing_get_amount, swing_set_amount)

static int swing_param_get_resolution(uint8_t track, param_value_t* out) {
  
  out->int_val = swing_get_resolution(track);
  return 0;
}

static int swing_param_set_resolution(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 3) return -1;
  swing_set_resolution(track, (uint8_t)val->int_val);
  return 0;
}

static int swing_param_get_groove(uint8_t track, param_value_t* out) {
  
  out->int_val = swing_get_groove(track);
  return 0;
}

static int swing_param_set_groove(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 5) return -1;
  swing_set_groove(track, (uint8_t)val->int_val);
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(swing, swing_set_enabled, swing_is_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_resolution_names[] = {
  "8TH",
  "16TH",
  "32ND",
};

static const char* s_groove_names[] = {
  "STRAIGHT",
  "SWING",
  "SHUFFLE",
  "HALF_TIME",
  "DOUBLE_TIME",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_swing_descriptor = {
  .name = "swing",
  .description = "Swing/groove timing",
  .category = MODULE_CATEGORY_EFFECT,
  .init = swing_init,
  .enable = swing_cli_enable,
  .disable = swing_cli_disable,
  .get_status = swing_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_swing_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(swing, enabled, "Enable swing"),
    PARAM_INT(swing, amount, "Swing amount (0-100%, 50=straight)", 0, 100),
    {
      .name = "resolution",
      .description = "Resolution",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 2,
      .enum_values = s_resolution_names,
      .enum_count = 3,
      .read_only = 0,
      .get_value = swing_param_get_resolution,
      .set_value = swing_param_set_resolution
    },
    {
      .name = "groove",
      .description = "Groove preset",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 4,
      .enum_values = s_groove_names,
      .enum_count = 5,
      .read_only = 0,
      .get_value = swing_param_get_groove,
      .set_value = swing_param_set_groove
    },
  };
  
  s_swing_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_swing_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int swing_register_cli(void) {
  setup_swing_parameters();
  return module_registry_register(&s_swing_descriptor);
}
