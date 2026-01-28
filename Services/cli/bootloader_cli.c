/**
 * @file bootloader_cli.c
 * @brief CLI integration for bootloader control
 * 
 * Bootloader version info and firmware update control
 */

#include "Services/bootloader/bootloader.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static char s_version_string[32];

static int bootloader_param_get_version(uint8_t track, param_value_t* out) {
  (void)track;
  snprintf(s_version_string, sizeof(s_version_string), "%d.%d.%d",
           BOOTLOADER_VERSION_MAJOR,
           BOOTLOADER_VERSION_MINOR,
           BOOTLOADER_VERSION_PATCH);
  out->str_val = s_version_string;
  return 0;
}

static int bootloader_param_get_app_valid(uint8_t track, param_value_t* out) {
  (void)track;
  out->bool_val = bootloader_check_application();
  return 0;
}

static int bootloader_param_get_app_address(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = APPLICATION_START_ADDRESS;
  return 0;
}

static int bootloader_param_get_app_size(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = APPLICATION_MAX_SIZE;
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int bootloader_cli_enable(uint8_t track) {
  (void)track;
  return 0; // Always enabled
}

static int bootloader_cli_disable(uint8_t track) {
  (void)track;
  return -1; // Cannot disable bootloader
}

static int bootloader_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_bootloader_descriptor = {
  .name = "bootloader",
  .description = "Bootloader control and firmware update",
  .category = MODULE_CATEGORY_SYSTEM,
  .init = NULL, // Bootloader initialized separately
  .enable = bootloader_cli_enable,
  .disable = bootloader_cli_disable,
  .get_status = bootloader_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_bootloader_parameters(void) {
  module_param_t params[] = {
    {
      .name = "version",
      .description = "Bootloader version",
      .type = PARAM_TYPE_STRING,
      .read_only = 1,
      .get_value = bootloader_param_get_version,
      .set_value = NULL
    },
    {
      .name = "app_valid",
      .description = "Valid application exists",
      .type = PARAM_TYPE_BOOL,
      .read_only = 1,
      .get_value = bootloader_param_get_app_valid,
      .set_value = NULL
    },
    {
      .name = "app_address",
      .description = "Application start address",
      .type = PARAM_TYPE_INT,
      .min = APPLICATION_START_ADDRESS,
      .max = APPLICATION_START_ADDRESS,
      .read_only = 1,
      .get_value = bootloader_param_get_app_address,
      .set_value = NULL
    },
    {
      .name = "app_max_size",
      .description = "Maximum application size (bytes)",
      .type = PARAM_TYPE_INT,
      .min = APPLICATION_MAX_SIZE,
      .max = APPLICATION_MAX_SIZE,
      .read_only = 1,
      .get_value = bootloader_param_get_app_size,
      .set_value = NULL
    }
  };
  
  s_bootloader_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_bootloader_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int bootloader_register_cli(void) {
  setup_bootloader_parameters();
  return module_registry_register(&s_bootloader_descriptor);
}
