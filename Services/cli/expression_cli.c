/**
 * @file expression_cli.c
 * @brief CLI integration for expression/breath controller
 * 
 * Breath/expression pressure control with filtering and curves
 */

#include "Services/expression/expression.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int expression_param_get_curve(uint8_t track, param_value_t* out) {
  (void)track;
  const expr_cfg_t* cfg = expression_get_cfg();
  out->int_val = cfg->curve;
  return 0;
}

static int expression_param_set_curve(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val >= 3) return -1;
  const expr_cfg_t* current = expression_get_cfg();
  expr_cfg_t cfg = *current;
  cfg.curve = (uint8_t)val->int_val;
  expression_set_cfg(&cfg);
  return 0;
}

static int expression_param_get_cc(uint8_t track, param_value_t* out) {
  (void)track;
  const expr_cfg_t* cfg = expression_get_cfg();
  out->int_val = cfg->cc_num;
  return 0;
}

static int expression_param_set_cc(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val > 127) return -1;
  const expr_cfg_t* current = expression_get_cfg();
  expr_cfg_t cfg = *current;
  cfg.cc_num = (uint8_t)val->int_val;
  expression_set_cfg(&cfg);
  return 0;
}

static int expression_param_get_bidir(uint8_t track, param_value_t* out) {
  (void)track;
  const expr_cfg_t* cfg = expression_get_cfg();
  out->bool_val = cfg->bidir;
  return 0;
}

static int expression_param_set_bidir(uint8_t track, const param_value_t* val) {
  (void)track;
  const expr_cfg_t* current = expression_get_cfg();
  expr_cfg_t cfg = *current;
  cfg.bidir = val->bool_val;
  expression_set_cfg(&cfg);
  return 0;
}

static int expression_param_get_deadband(uint8_t track, param_value_t* out) {
  (void)track;
  const expr_cfg_t* cfg = expression_get_cfg();
  out->int_val = cfg->deadband_cc;
  return 0;
}

static int expression_param_set_deadband(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val > 255) return -1;
  const expr_cfg_t* current = expression_get_cfg();
  expr_cfg_t cfg = *current;
  cfg.deadband_cc = (uint8_t)val->int_val;
  expression_set_cfg(&cfg);
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int expression_cli_enable(uint8_t track) {
  (void)track;
  return 0;
}

static int expression_cli_disable(uint8_t track) {
  (void)track;
  return 0;
}

static int expression_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_curve_names[] = {
  "LINEAR",
  "EXPONENTIAL",
  "S_CURVE",
};

// =============================================================================
// INIT WRAPPER
// =============================================================================

static int expression_cli_init(void) {
  expression_init();
  return 0;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_expression_descriptor = {
  .name = "expression",
  .description = "Expression/breath controller with filtering",
  .category = MODULE_CATEGORY_INPUT,
  .init = expression_cli_init,
  .enable = expression_cli_enable,
  .disable = expression_cli_disable,
  .get_status = expression_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_expression_parameters(void) {
  module_param_t params[] = {
    {
      .name = "curve",
      .description = "Response curve",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 2,
      .enum_values = s_curve_names,
      .enum_count = 3,
      .read_only = 0,
      .get_value = expression_param_get_curve,
      .set_value = expression_param_set_curve
    },
    {
      .name = "cc",
      .description = "Expression CC number (0-127)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 127,
      .read_only = 0,
      .get_value = expression_param_get_cc,
      .set_value = expression_param_set_cc
    },
    {
      .name = "bidir",
      .description = "Bidirectional mode (push/pull)",
      .type = PARAM_TYPE_BOOL,
      .read_only = 0,
      .get_value = expression_param_get_bidir,
      .set_value = expression_param_set_bidir
    },
    {
      .name = "deadband",
      .description = "Noise deadband (0-255)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 255,
      .read_only = 0,
      .get_value = expression_param_get_deadband,
      .set_value = expression_param_set_deadband
    }
  };
  
  s_expression_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_expression_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int expression_register_cli(void) {
  setup_expression_parameters();
  return module_registry_register(&s_expression_descriptor);
}
