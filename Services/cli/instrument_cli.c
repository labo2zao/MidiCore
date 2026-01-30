/**
 * @file instrument_cli.c
 * @brief CLI integration for instrument configuration
 * 
 * Instrument humanization, velocity curves, and chord settings
 */

#include "Services/instrument/instrument_cfg.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int instrument_param_get_human_enable(uint8_t track, param_value_t* out) {
  (void)track;
  const instrument_cfg_t* cfg = instrument_cfg_get();
  out->bool_val = cfg->human_enable;
  return 0;
}

static int instrument_param_set_human_enable(uint8_t track, const param_value_t* val) {
  (void)track;
  instrument_cfg_t cfg = *instrument_cfg_get();
  cfg.human_enable = val->bool_val;
  instrument_cfg_set(&cfg);
  return 0;
}

static int instrument_param_get_human_time(uint8_t track, param_value_t* out) {
  (void)track;
  const instrument_cfg_t* cfg = instrument_cfg_get();
  out->int_val = cfg->human_time_ms;
  return 0;
}

static int instrument_param_set_human_time(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val > 255) return -1;
  instrument_cfg_t cfg = *instrument_cfg_get();
  cfg.human_time_ms = (uint8_t)val->int_val;
  instrument_cfg_set(&cfg);
  return 0;
}

static int instrument_param_get_human_vel(uint8_t track, param_value_t* out) {
  (void)track;
  const instrument_cfg_t* cfg = instrument_cfg_get();
  out->int_val = cfg->human_vel;
  return 0;
}

static int instrument_param_set_human_vel(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val > 127) return -1;
  instrument_cfg_t cfg = *instrument_cfg_get();
  cfg.human_vel = (uint8_t)val->int_val;
  instrument_cfg_set(&cfg);
  return 0;
}

static int instrument_param_get_vel_curve(uint8_t track, param_value_t* out) {
  (void)track;
  const instrument_cfg_t* cfg = instrument_cfg_get();
  out->int_val = cfg->vel_curve;
  return 0;
}

static int instrument_param_set_vel_curve(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val >= 4) return -1;
  instrument_cfg_t cfg = *instrument_cfg_get();
  cfg.vel_curve = (uint8_t)val->int_val;
  instrument_cfg_set(&cfg);
  return 0;
}

static int instrument_param_get_strum_enable(uint8_t track, param_value_t* out) {
  (void)track;
  const instrument_cfg_t* cfg = instrument_cfg_get();
  out->bool_val = cfg->strum_enable;
  return 0;
}

static int instrument_param_set_strum_enable(uint8_t track, const param_value_t* val) {
  (void)track;
  instrument_cfg_t cfg = *instrument_cfg_get();
  cfg.strum_enable = val->bool_val;
  instrument_cfg_set(&cfg);
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int instrument_cli_enable(uint8_t track) {
  (void)track;
  return 0; // Always enabled
}

static int instrument_cli_disable(uint8_t track) {
  (void)track;
  return 0;
}

static int instrument_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_vel_curve_names[] = {
  "LINEAR",
  "SOFT",
  "HARD",
  "CUSTOM",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_instrument_descriptor = {
  .name = "instrument",
  .description = "Instrument humanization and velocity curves",
  .category = MODULE_CATEGORY_EFFECT,
  .init = NULL, // Initialized via config system
  .enable = instrument_cli_enable,
  .disable = instrument_cli_disable,
  .get_status = instrument_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_instrument_parameters(void) {
  module_param_t params[] = {
    {
      .name = "human_enable",
      .description = "Enable humanization",
      .type = PARAM_TYPE_BOOL,
      .read_only = 0,
      .get_value = instrument_param_get_human_enable,
      .set_value = instrument_param_set_human_enable
    },
    {
      .name = "human_time_ms",
      .description = "Humanize timing (±ms)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 255,
      .read_only = 0,
      .get_value = instrument_param_get_human_time,
      .set_value = instrument_param_set_human_time
    },
    {
      .name = "human_vel",
      .description = "Humanize velocity (±)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 127,
      .read_only = 0,
      .get_value = instrument_param_get_human_vel,
      .set_value = instrument_param_set_human_vel
    },
    {
      .name = "vel_curve",
      .description = "Velocity curve",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 3,
      .enum_values = s_vel_curve_names,
      .enum_count = 4,
      .read_only = 0,
      .get_value = instrument_param_get_vel_curve,
      .set_value = instrument_param_set_vel_curve
    },
    {
      .name = "strum_enable",
      .description = "Enable chord strumming",
      .type = PARAM_TYPE_BOOL,
      .read_only = 0,
      .get_value = instrument_param_get_strum_enable,
      .set_value = instrument_param_set_strum_enable
    }
  };
  
  s_instrument_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_instrument_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int instrument_register_cli(void) {
  setup_instrument_parameters();
  return module_registry_register(&s_instrument_descriptor);
}
