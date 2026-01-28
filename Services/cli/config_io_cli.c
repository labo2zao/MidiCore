/**
 * @file config_io_cli.c
 * @brief CLI integration for configuration file I/O
 * 
 * NGC configuration file reading and writing
 */

#include "Services/config_io/config_io.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static char s_config_path[128] = CONFIG_FILE_PATH;

static int config_io_param_get_path(uint8_t track, param_value_t* out) {
  (void)track;
  out->str_val = s_config_path;
  return 0;
}

static int config_io_param_get_sd_available(uint8_t track, param_value_t* out) {
  (void)track;
  out->bool_val = config_io_sd_available();
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int config_io_cli_enable(uint8_t track) {
  (void)track;
  return 0; // Always enabled
}

static int config_io_cli_disable(uint8_t track) {
  (void)track;
  return 0;
}

static int config_io_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_config_io_descriptor = {
  .name = "config_io",
  .description = "NGC configuration file I/O",
  .category = MODULE_CATEGORY_SYSTEM,
  .init = config_io_init,
  .enable = config_io_cli_enable,
  .disable = config_io_cli_disable,
  .get_status = config_io_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_config_io_parameters(void) {
  module_param_t params[] = {
    {
      .name = "path",
      .description = "Configuration file path",
      .type = PARAM_TYPE_STRING,
      .read_only = 1,
      .get_value = config_io_param_get_path,
      .set_value = NULL
    },
    {
      .name = "sd_available",
      .description = "SD card available",
      .type = PARAM_TYPE_BOOL,
      .read_only = 1,
      .get_value = config_io_param_get_sd_available,
      .set_value = NULL
    }
  };
  
  s_config_io_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_config_io_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int config_io_register_cli(void) {
  setup_config_io_parameters();
  return module_registry_register(&s_config_io_descriptor);
}
