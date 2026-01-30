/**
 * @file velocity_compressor_cli.c
 * @brief CLI integration for velocity_compressor module
 * 
 * Velocity dynamics compression
 */

#include "Services/velocity_compressor/velocity_compressor.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(velocity_compressor, enabled, velocity_compressor_get_enabled, velocity_compressor_set_enabled)

DEFINE_PARAM_INT_TRACK(velocity_compressor, threshold, velocity_compressor_get_threshold, velocity_compressor_set_threshold)

static int velocity_compressor_param_get_ratio(uint8_t track, param_value_t* out) {
  
  out->int_val = velocity_compressor_get_ratio(track);
  return 0;
}

static int velocity_compressor_param_set_ratio(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 6) return -1;
  velocity_compressor_set_ratio(track, (uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_INT_TRACK(velocity_compressor, makeup_gain, velocity_compressor_get_makeup_gain, velocity_compressor_set_makeup_gain)

static int velocity_compressor_param_get_knee(uint8_t track, param_value_t* out) {
  
  out->int_val = velocity_compressor_get_knee(track);
  return 0;
}

static int velocity_compressor_param_set_knee(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 2) return -1;
  velocity_compressor_set_knee(track, (uint8_t)val->int_val);
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(velocity_compressor, velocity_compressor_set_enabled, velocity_compressor_is_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_ratio_names[] = {
  "1_1",
  "2_1",
  "3_1",
  "4_1",
  "8_1",
  "INF_1",
};

static const char* s_knee_names[] = {
  "HARD",
  "SOFT",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_velocity_compressor_descriptor = {
  .name = "velocity_compressor",
  .description = "Velocity dynamics compression",
  .category = MODULE_CATEGORY_EFFECT,
  .init = velocity_compressor_init,
  .enable = velocity_compressor_cli_enable,
  .disable = velocity_compressor_cli_disable,
  .get_status = velocity_compressor_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_velocity_compressor_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(velocity_compressor, enabled, "Enable compressor"),
    PARAM_INT(velocity_compressor, threshold, "Compression threshold (1-127)", 1, 127),
    {
      .name = "ratio",
      .description = "Compression ratio",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 5,
      .enum_values = s_ratio_names,
      .enum_count = 6,
      .read_only = 0,
      .get_value = velocity_compressor_param_get_ratio,
      .set_value = velocity_compressor_param_set_ratio
    },
    PARAM_INT(velocity_compressor, makeup_gain, "Output gain (0-127)", 0, 127),
    {
      .name = "knee",
      .description = "Knee type",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 1,
      .enum_values = s_knee_names,
      .enum_count = 2,
      .read_only = 0,
      .get_value = velocity_compressor_param_get_knee,
      .set_value = velocity_compressor_param_set_knee
    },
  };
  
  s_velocity_compressor_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_velocity_compressor_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int velocity_compressor_register_cli(void) {
  setup_velocity_compressor_parameters();
  return module_registry_register(&s_velocity_compressor_descriptor);
}
