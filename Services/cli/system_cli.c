/**
 * @file system_cli.c
 * @brief CLI integration for system status and control
 * 
 * System status, reset, and fatal error reporting
 */

#include "Services/system/system_status.h"
#include "Services/system/safe_mode.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int system_param_get_sd_required(uint8_t track, param_value_t* out) {
  (void)track;
  out->bool_val = system_is_sd_required();
  return 0;
}

static int system_param_get_sd_ok(uint8_t track, param_value_t* out) {
  (void)track;
  out->bool_val = system_is_sd_ok();
  return 0;
}

static int system_param_get_fatal(uint8_t track, param_value_t* out) {
  (void)track;
  out->bool_val = system_is_fatal();
  return 0;
}

static int system_param_get_safe_mode(uint8_t track, param_value_t* out) {
  (void)track;
  out->bool_val = safe_mode_is_active();
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int system_cli_enable(uint8_t track) {
  (void)track;
  return 0; // Always enabled
}

static int system_cli_disable(uint8_t track) {
  (void)track;
  return -1; // Cannot disable system
}

static int system_cli_get_status(uint8_t track) {
  (void)track;
  return system_is_fatal() ? MODULE_STATUS_ERROR : MODULE_STATUS_ENABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_system_descriptor = {
  .name = "system",
  .description = "System status and control",
  .category = MODULE_CATEGORY_SYSTEM,
  .init = NULL, // System always initialized
  .enable = system_cli_enable,
  .disable = system_cli_disable,
  .get_status = system_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_system_parameters(void) {
  module_param_t params[] = {
    {
      .name = "sd_required",
      .description = "SD card required for operation",
      .type = PARAM_TYPE_BOOL,
      .read_only = 1,
      .get_value = system_param_get_sd_required,
      .set_value = NULL
    },
    {
      .name = "sd_ok",
      .description = "SD card mounted and ready",
      .type = PARAM_TYPE_BOOL,
      .read_only = 1,
      .get_value = system_param_get_sd_ok,
      .set_value = NULL
    },
    {
      .name = "fatal",
      .description = "Fatal error occurred",
      .type = PARAM_TYPE_BOOL,
      .read_only = 1,
      .get_value = system_param_get_fatal,
      .set_value = NULL
    },
    {
      .name = "safe_mode",
      .description = "Safe mode active",
      .type = PARAM_TYPE_BOOL,
      .read_only = 1,
      .get_value = system_param_get_safe_mode,
      .set_value = NULL
    }
  };
  
  s_system_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_system_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int system_register_cli(void) {
  setup_system_parameters();
  return module_registry_register(&s_system_descriptor);
}
