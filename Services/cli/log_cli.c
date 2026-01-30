/**
 * @file log_cli.c
 * @brief CLI integration for logging control
 * 
 * Logging system control with SD card output
 */

#include "Services/log/log.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static uint8_t s_enabled = 1;
static uint8_t s_sd_enabled = 1;

static int log_param_get_enabled(uint8_t track, param_value_t* out) {
  (void)track;
  out->bool_val = s_enabled;
  return 0;
}

static int log_param_set_enabled(uint8_t track, const param_value_t* val) {
  (void)track;
  s_enabled = val->bool_val;
  return 0;
}

static int log_param_get_sd_enabled(uint8_t track, param_value_t* out) {
  (void)track;
  out->bool_val = s_sd_enabled;
  return 0;
}

static int log_param_set_sd_enabled(uint8_t track, const param_value_t* val) {
  (void)track;
  s_sd_enabled = val->bool_val;
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int log_cli_enable(uint8_t track) {
  (void)track;
  s_enabled = 1;
  return 0;
}

static int log_cli_disable(uint8_t track) {
  (void)track;
  s_enabled = 0;
  return 0;
}

static int log_cli_get_status(uint8_t track) {
  (void)track;
  return s_enabled ? MODULE_STATUS_ENABLED : MODULE_STATUS_DISABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_log_descriptor = {
  .name = "log",
  .description = "Logging system with SD card output",
  .category = MODULE_CATEGORY_SYSTEM,
  .init = log_init,
  .enable = log_cli_enable,
  .disable = log_cli_disable,
  .get_status = log_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_log_parameters(void) {
  module_param_t params[] = {
    {
      .name = "enabled",
      .description = "Enable logging",
      .type = PARAM_TYPE_BOOL,
      .read_only = 0,
      .get_value = log_param_get_enabled,
      .set_value = log_param_set_enabled
    },
    {
      .name = "sd_enabled",
      .description = "Enable SD card logging",
      .type = PARAM_TYPE_BOOL,
      .read_only = 0,
      .get_value = log_param_get_sd_enabled,
      .set_value = log_param_set_sd_enabled
    }
  };
  
  s_log_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_log_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int log_register_cli(void) {
  setup_log_parameters();
  return module_registry_register(&s_log_descriptor);
}
