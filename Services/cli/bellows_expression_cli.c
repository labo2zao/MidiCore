/**
 * @file bellows_expression_cli.c
 * @brief CLI integration for bellows_expression module
 * 
 * Bellows pressure sensor
 */

#include "Services/bellows_expression/bellows_expression.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int bellows_expression_param_get_curve(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = bellows_expression_get_curve();
  return 0;
}

static int bellows_expression_param_set_curve(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val >= 4) return -1;
  bellows_expression_set_curve((uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_INT(bellows_expression, min_pa, bellows_expression_get_min_pa, bellows_expression_set_min_pa)

DEFINE_PARAM_INT(bellows_expression, max_pa, bellows_expression_get_max_pa, bellows_expression_set_max_pa)

DEFINE_PARAM_BOOL(bellows_expression, bidirectional, bellows_expression_get_bidirectional, bellows_expression_set_bidirectional)

DEFINE_PARAM_INT(bellows_expression, expression_cc, bellows_expression_get_expression_cc, bellows_expression_set_expression_cc)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int bellows_expression_cli_enable(uint8_t track) {
  (void)track;
  return 0;
}

static int bellows_expression_cli_disable(uint8_t track) {
  (void)track;
  return 0;
}

static int bellows_expression_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_curve_names[] = {
  "LINEAR",
  "EXPONENTIAL",
  "LOGARITHMIC",
  "S_CURVE",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_bellows_expression_descriptor = {
  .name = "bellows_expression",
  .description = "Bellows pressure sensor",
  .category = MODULE_CATEGORY_ACCORDION,
  .init = bellows_expression_init,
  .enable = bellows_expression_cli_enable,
  .disable = bellows_expression_cli_disable,
  .get_status = bellows_expression_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_bellows_expression_parameters(void) {
  module_param_t params[] = {
    {
      .name = "curve",
      .description = "Expression curve",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 3,
      .enum_values = s_curve_names,
      .enum_count = 4,
      .read_only = 0,
      .get_value = bellows_expression_param_get_curve,
      .set_value = bellows_expression_param_set_curve
    },
    PARAM_INT(bellows_expression, min_pa, "Minimum pressure (Pa)", 0, 5000),
    PARAM_INT(bellows_expression, max_pa, "Maximum pressure (Pa)", 0, 5000),
    PARAM_BOOL(bellows_expression, bidirectional, "Push/pull detection"),
    PARAM_INT(bellows_expression, expression_cc, "Expression CC (0-127)", 0, 127),
  };
  
  s_bellows_expression_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_bellows_expression_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int bellows_expression_register_cli(void) {
  setup_bellows_expression_parameters();
  return module_registry_register(&s_bellows_expression_descriptor);
}
