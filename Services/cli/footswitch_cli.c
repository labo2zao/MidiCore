/**
 * @file footswitch_cli.c
 * @brief CLI integration for footswitch input handling
 * 
 * 8 footswitch inputs with configurable actions
 */

#include "Services/footswitch/footswitch.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int footswitch_param_get_count(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = 8; // 8 footswitch inputs
  return 0;
}

static int footswitch_param_get_pressed(uint8_t track, param_value_t* out) {
  if (track >= 8) return -1;
  out->bool_val = footswitch_is_pressed(track);
  return 0;
}

static int footswitch_param_get_raw(uint8_t track, param_value_t* out) {
  if (track >= 8) return -1;
  out->bool_val = footswitch_read_raw(track);
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int footswitch_cli_enable(uint8_t track) {
  (void)track;
  return 0; // Always enabled
}

static int footswitch_cli_disable(uint8_t track) {
  (void)track;
  return -1; // Cannot disable hardware input
}

static int footswitch_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_footswitch_descriptor = {
  .name = "footswitch",
  .description = "8 footswitch inputs with debouncing",
  .category = MODULE_CATEGORY_INPUT,
  .init = footswitch_init,
  .enable = footswitch_cli_enable,
  .disable = footswitch_cli_disable,
  .get_status = footswitch_cli_get_status,
  .has_per_track_state = 1,  // Per-footswitch state
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_footswitch_parameters(void) {
  module_param_t params[] = {
    {
      .name = "count",
      .description = "Total number of footswitches",
      .type = PARAM_TYPE_INT,
      .min = 8,
      .max = 8,
      .read_only = 1,
      .get_value = footswitch_param_get_count,
      .set_value = NULL
    },
    {
      .name = "pressed",
      .description = "Debounced press state",
      .type = PARAM_TYPE_BOOL,
      .read_only = 1,
      .get_value = footswitch_param_get_pressed,
      .set_value = NULL
    },
    {
      .name = "raw",
      .description = "Raw input state (no debounce)",
      .type = PARAM_TYPE_BOOL,
      .read_only = 1,
      .get_value = footswitch_param_get_raw,
      .set_value = NULL
    }
  };
  
  s_footswitch_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_footswitch_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int footswitch_register_cli(void) {
  setup_footswitch_parameters();
  return module_registry_register(&s_footswitch_descriptor);
}
