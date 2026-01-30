/**
 * @file dout_map_cli.c
 * @brief CLI integration for DOUT (digital output) mapping
 * 
 * Digital output (LED) control and RGB LED pattern management
 */

#include "Services/dout/dout_map.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int dout_map_param_get_led_count(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = 256; // Maximum DOUT outputs
  return 0;
}

static int dout_map_param_get_rgb_count(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = 85; // 256/3 = ~85 RGB LEDs
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int dout_map_cli_enable(uint8_t track) {
  (void)track;
  return 0; // Always enabled
}

static int dout_map_cli_disable(uint8_t track) {
  (void)track;
  return -1; // Cannot disable hardware output
}

static int dout_map_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_dout_map_descriptor = {
  .name = "dout",
  .description = "Digital output (LED) control",
  .category = MODULE_CATEGORY_OUTPUT,
  .init = dout_map_init,
  .enable = dout_map_cli_enable,
  .disable = dout_map_cli_disable,
  .get_status = dout_map_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_dout_map_parameters(void) {
  module_param_t params[] = {
    {
      .name = "led_count",
      .description = "Total number of DOUT outputs",
      .type = PARAM_TYPE_INT,
      .min = 256,
      .max = 256,
      .read_only = 1,
      .get_value = dout_map_param_get_led_count,
      .set_value = NULL
    },
    {
      .name = "rgb_count",
      .description = "Number of RGB LEDs (3 outputs each)",
      .type = PARAM_TYPE_INT,
      .min = 85,
      .max = 85,
      .read_only = 1,
      .get_value = dout_map_param_get_rgb_count,
      .set_value = NULL
    }
  };
  
  s_dout_map_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_dout_map_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int dout_map_register_cli(void) {
  setup_dout_map_parameters();
  return module_registry_register(&s_dout_map_descriptor);
}
