/**
 * @file humanize_cli.c
 * @brief CLI integration for humanize module
 * 
 * Humanize timing and velocity
 */

#include "Services/humanize/humanize.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

// Note: humanize module doesn't have get/set functions, it works via instrument_cfg_t
// For CLI, we need to provide stub wrappers that always return 0
// In a real implementation, these would interface with a configuration system

static int humanize_stub_get_time_amount(uint8_t track) {
  (void)track;
  // TODO: Get from configuration system
  return 0;
}

static void humanize_stub_set_time_amount(uint8_t track, int value) {
  (void)track;
  (void)value;
  // TODO: Set in configuration system
}

static int humanize_stub_get_velocity_amount(uint8_t track) {
  (void)track;
  // TODO: Get from configuration system
  return 0;
}

static void humanize_stub_set_velocity_amount(uint8_t track, int value) {
  (void)track;
  (void)value;
  // TODO: Set in configuration system
}

DEFINE_PARAM_INT_TRACK(humanize, time_amount, humanize_stub_get_time_amount, humanize_stub_set_time_amount)
DEFINE_PARAM_INT_TRACK(humanize, velocity_amount, humanize_stub_get_velocity_amount, humanize_stub_set_velocity_amount)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int humanize_cli_enable(uint8_t track) {
  (void)track;
  return 0;
}

static int humanize_cli_disable(uint8_t track) {
  (void)track;
  return 0;
}

static int humanize_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static int humanize_cli_init(void) {
  humanize_init(0);  // TODO: Pass actual instrument config
  return 0;
}

static module_descriptor_t s_humanize_descriptor = {
  .name = "humanize",
  .description = "Humanize timing and velocity",
  .category = MODULE_CATEGORY_EFFECT,
  .init = humanize_cli_init,
  .enable = humanize_cli_enable,
  .disable = humanize_cli_disable,
  .get_status = humanize_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_humanize_parameters(void) {
  module_param_t params[] = {
    PARAM_INT(humanize, time_amount, "Timing variation (0-100%)", 0, 100),
    PARAM_INT(humanize, velocity_amount, "Velocity variation (0-100%)", 0, 100),
  };
  
  s_humanize_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_humanize_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int humanize_register_cli(void) {
  setup_humanize_parameters();
  return module_registry_register(&s_humanize_descriptor);
}
