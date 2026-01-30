/**
 * @file usb_cdc_cli.c
 * @brief CLI integration for USB CDC (Virtual COM Port)
 * 
 * USB CDC terminal and debug communication
 */

#include "Services/usb_cdc/usb_cdc.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int usb_cdc_param_get_connected(uint8_t track, param_value_t* out) {
  (void)track;
  out->bool_val = usb_cdc_is_connected();
  return 0;
}

static int usb_cdc_param_get_tx_ready(uint8_t track, param_value_t* out) {
  (void)track;
  out->bool_val = usb_cdc_is_tx_ready();
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int usb_cdc_cli_enable(uint8_t track) {
  (void)track;
  return 0; // Always enabled
}

static int usb_cdc_cli_disable(uint8_t track) {
  (void)track;
  return -1; // Cannot disable USB CDC
}

static int usb_cdc_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_usb_cdc_descriptor = {
  .name = "usb_cdc",
  .description = "USB CDC (Virtual COM Port)",
  .category = MODULE_CATEGORY_SYSTEM,
  .init = usb_cdc_init,
  .enable = usb_cdc_cli_enable,
  .disable = usb_cdc_cli_disable,
  .get_status = usb_cdc_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_usb_cdc_parameters(void) {
  module_param_t params[] = {
    {
      .name = "connected",
      .description = "USB CDC connected",
      .type = PARAM_TYPE_BOOL,
      .read_only = 1,
      .get_value = usb_cdc_param_get_connected,
      .set_value = NULL
    },
    {
      .name = "tx_ready",
      .description = "Transmit buffer ready",
      .type = PARAM_TYPE_BOOL,
      .read_only = 1,
      .get_value = usb_cdc_param_get_tx_ready,
      .set_value = NULL
    }
  };
  
  s_usb_cdc_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_usb_cdc_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int usb_cdc_register_cli(void) {
  setup_usb_cdc_parameters();
  return module_registry_register(&s_usb_cdc_descriptor);
}
