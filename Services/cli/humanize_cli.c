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

// Real getters/setters backed by the humanize module configuration
static int humanize_get_time_amount(uint8_t track) {
  return humanize_get_time_variation(track);
}

static void humanize_set_time_amount(uint8_t track, int value) {
  humanize_set_time_variation(track, value);
}

static int humanize_get_velocity_amount(uint8_t track) {
  return humanize_get_velocity_variation(track);
}

static void humanize_set_velocity_amount(uint8_t track, int value) {
  humanize_set_velocity_variation(track, value);
}

DEFINE_PARAM_INT_TRACK(humanize, time_amount, humanize_get_time_amount, humanize_set_time_amount)
DEFINE_PARAM_INT_TRACK(humanize, velocity_amount, humanize_get_velocity_amount, humanize_set_velocity_amount)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int humanize_cli_enable(uint8_t track) {
  humanize_set_enabled(track, 1);
  return 0;
}

static int humanize_cli_disable(uint8_t track) {
  humanize_set_enabled(track, 0);
  return 0;
}

static int humanize_cli_get_status(uint8_t track) {
  return humanize_is_enabled(track) ? MODULE_STATUS_ENABLED : MODULE_STATUS_DISABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static int humanize_cli_init(void) {
  humanize_init(0);
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
