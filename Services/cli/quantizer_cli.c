/**
 * @file quantizer_cli.c
 * @brief CLI integration for quantizer module
 * 
 * Timing quantizer for MIDI notes
 */

#include "Services/quantizer/quantizer.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(quantizer, enabled, quantizer_get_enabled, quantizer_set_enabled)

static int quantizer_param_get_resolution(uint8_t track, param_value_t* out) {
  
  out->int_val = quantizer_get_resolution(track);
  return 0;
}

static int quantizer_param_set_resolution(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 8) return -1;
  quantizer_set_resolution(track, (uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_INT_TRACK(quantizer, strength, quantizer_get_strength, quantizer_set_strength)

DEFINE_PARAM_INT_TRACK(quantizer, lookahead, quantizer_get_lookahead, quantizer_set_lookahead)

DEFINE_PARAM_INT_TRACK(quantizer, swing, quantizer_get_swing, quantizer_set_swing)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(quantizer, quantizer_set_enabled, quantizer_is_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_resolution_names[] = {
  "1_4",
  "1_8",
  "1_16",
  "1_32",
  "1_8T",
  "1_16T",
  "1_4_DOT",
  "1_8_DOT",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_quantizer_descriptor = {
  .name = "quantizer",
  .description = "Timing quantizer for MIDI notes",
  .category = MODULE_CATEGORY_EFFECT,
  .init = quantizer_init,
  .enable = quantizer_cli_enable,
  .disable = quantizer_cli_disable,
  .get_status = quantizer_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_quantizer_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(quantizer, enabled, "Enable quantization"),
    {
      .name = "resolution",
      .description = "Grid resolution",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 7,
      .enum_values = s_resolution_names,
      .enum_count = 8,
      .read_only = 0,
      .get_value = quantizer_param_get_resolution,
      .set_value = quantizer_param_set_resolution
    },
    PARAM_INT(quantizer, strength, "Quantization strength (0-100%)", 0, 100),
    PARAM_INT(quantizer, lookahead, "Lookahead window (ms)", 0, 500),
    PARAM_INT(quantizer, swing, "Swing amount (0-100%)", 0, 100),
  };
  
  s_quantizer_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_quantizer_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int quantizer_register_cli(void) {
  setup_quantizer_parameters();
  return module_registry_register(&s_quantizer_descriptor);
}
