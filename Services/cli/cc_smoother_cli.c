/**
 * @file cc_smoother_cli.c
 * @brief CLI integration for cc_smoother module
 * 
 * MIDI CC smoother - eliminate zipper noise
 */

#include "Services/cc_smoother/cc_smoother.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(cc_smoother, enabled, cc_smoother_get_enabled, cc_smoother_set_enabled)

static int cc_smoother_param_get_mode(uint8_t track, param_value_t* out) {
  
  out->int_val = cc_smoother_get_mode(track);
  return 0;
}

static int cc_smoother_param_set_mode(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 5) return -1;
  cc_smoother_set_mode(track, (uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_INT_TRACK(cc_smoother, amount, cc_smoother_get_amount, cc_smoother_set_amount)

DEFINE_PARAM_INT_TRACK(cc_smoother, attack, cc_smoother_get_attack, cc_smoother_set_attack)

DEFINE_PARAM_INT_TRACK(cc_smoother, release, cc_smoother_get_release, cc_smoother_set_release)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(cc_smoother, cc_smoother_set_enabled, cc_smoother_is_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_mode_names[] = {
  "OFF",
  "LIGHT",
  "MEDIUM",
  "HEAVY",
  "CUSTOM",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_cc_smoother_descriptor = {
  .name = "cc_smoother",
  .description = "MIDI CC smoother - eliminate zipper noise",
  .category = MODULE_CATEGORY_EFFECT,
  .init = cc_smoother_init,
  .enable = cc_smoother_cli_enable,
  .disable = cc_smoother_cli_disable,
  .get_status = cc_smoother_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_cc_smoother_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(cc_smoother, enabled, "Enable smoothing"),
    {
      .name = "mode",
      .description = "Smoothing mode",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 4,
      .enum_values = s_mode_names,
      .enum_count = 5,
      .read_only = 0,
      .get_value = cc_smoother_param_get_mode,
      .set_value = cc_smoother_param_set_mode
    },
    PARAM_INT(cc_smoother, amount, "Smoothing amount (0-255)", 0, 255),
    PARAM_INT(cc_smoother, attack, "Attack time (ms)", 0, 1000),
    PARAM_INT(cc_smoother, release, "Release time (ms)", 0, 1000),
  };
  
  s_cc_smoother_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_cc_smoother_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int cc_smoother_register_cli(void) {
  setup_cc_smoother_parameters();
  return module_registry_register(&s_cc_smoother_descriptor);
}
