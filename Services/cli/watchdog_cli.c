/**
 * @file watchdog_cli.c
 * @brief CLI integration for watchdog module
 * 
 * System watchdog and health monitoring
 */

#include "Services/watchdog/watchdog.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// MODULE DESCRIPTOR (No configurable parameters)
// =============================================================================

static int watchdog_cli_enable(uint8_t track) {
  (void)track;
  // Module has no enable/disable function
  return 0;
}

static int watchdog_cli_disable(uint8_t track) {
  (void)track;
  return 0;
}

static int watchdog_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

static module_descriptor_t s_watchdog_descriptor = {
  .name = "watchdog",
  .description = "System watchdog and health monitoring",
  .category = MODULE_CATEGORY_SYSTEM,
  .init = watchdog_init,
  .enable = watchdog_cli_enable,
  .disable = watchdog_cli_disable,
  .get_status = watchdog_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

int watchdog_register_cli(void) {
  return module_registry_register(&s_watchdog_descriptor);
}
