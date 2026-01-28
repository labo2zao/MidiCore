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

static int ain_cli_init(void) { 
  ain_init(); 
  return 0; 
}

static module_descriptor_t s_ain_descriptor = {
  .name = "ain",
  .description = "Analog input (Hall sensor keyboard)",
  .category = MODULE_CATEGORY_INPUT,
  .init = ain_cli_init,
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
  // AIN module has no runtime parameters, all config is compile-time
  s_ain_descriptor.param_count = 0;
}

// =============================================================================
// REGISTRATION
// =============================================================================

int ain_register_cli(void) {
  setup_ain_parameters();
  return module_registry_register(&s_ain_descriptor);
}
