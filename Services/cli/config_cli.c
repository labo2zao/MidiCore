/**
 * @file config_cli.c
 * @brief CLI integration for config module
 * 
 * Global system configuration
 */

#include "Services/config/config.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL(config, srio_enable, config_get_srio_enable, config_set_srio_enable)

DEFINE_PARAM_BOOL(config, srio_din_enable, config_get_srio_din_enable, config_set_srio_din_enable)

DEFINE_PARAM_BOOL(config, srio_dout_enable, config_get_srio_dout_enable, config_set_srio_dout_enable)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int config_cli_enable(uint8_t track) {
  (void)track;
  return 0;
}

static int config_cli_disable(uint8_t track) {
  (void)track;
  return 0;
}

static int config_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_config_descriptor = {
  .name = "config",
  .description = "Global system configuration",
  .category = MODULE_CATEGORY_SYSTEM,
  .init = config_init,
  .enable = config_cli_enable,
  .disable = config_cli_disable,
  .get_status = config_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_config_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(config, srio_enable, "Enable SRIO subsystem"),
    PARAM_BOOL(config, srio_din_enable, "Enable DIN scanning"),
    PARAM_BOOL(config, srio_dout_enable, "Enable DOUT output"),
  };
  
  s_config_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_config_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int config_register_cli(void) {
  setup_config_parameters();
  return module_registry_register(&s_config_descriptor);
}
