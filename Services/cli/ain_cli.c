/**
 * @file ain_cli.c
 * @brief CLI integration for ain module
 * 
 * Analog input (Hall sensor keyboard)
 */

#include "Services/ain/ain.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL(ain, enable, ain_get_enable, ain_set_enable)

DEFINE_PARAM_BOOL(ain, velocity_enable, ain_get_velocity_enable, ain_set_velocity_enable)

DEFINE_PARAM_INT(ain, scan_ms, ain_get_scan_ms, ain_set_scan_ms)

DEFINE_PARAM_INT(ain, deadband, ain_get_deadband, ain_set_deadband)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int ain_cli_enable(uint8_t track) {
  (void)track;
  return 0;
}

static int ain_cli_disable(uint8_t track) {
  (void)track;
  return 0;
}

static int ain_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_ain_descriptor = {
  .name = "ain",
  .description = "Analog input (Hall sensor keyboard)",
  .category = MODULE_CATEGORY_INPUT,
  .init = ain_init,
  .enable = ain_cli_enable,
  .disable = ain_cli_disable,
  .get_status = ain_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_ain_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(ain, enable, "Enable AIN scanning"),
    PARAM_BOOL(ain, velocity_enable, "Enable velocity sensing"),
    PARAM_INT(ain, scan_ms, "Scan interval (ms)", 1, 50),
    PARAM_INT(ain, deadband, "ADC deadband", 0, 100),
  };
  
  s_ain_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_ain_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int ain_register_cli(void) {
  setup_ain_parameters();
  return module_registry_register(&s_ain_descriptor);
}
