/**
 * @file usb_msc_cli.c
 * @brief CLI integration for USB Mass Storage Class
 * 
 * USB MSC exposes SD card as USB Mass Storage device
 */

#include "Services/usb_msc/usb_msc.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int usb_msc_param_get_mounted(uint8_t track, param_value_t* out) {
  (void)track;
  out->bool_val = usb_msc_is_mounted();
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int usb_msc_cli_enable(uint8_t track) {
  (void)track;
  return 0; // Always enabled
}

static int usb_msc_cli_disable(uint8_t track) {
  (void)track;
  return -1; // Cannot disable USB MSC
}

static int usb_msc_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_usb_msc_descriptor = {
  .name = "usb_msc",
  .description = "USB Mass Storage (SD card)",
  .category = MODULE_CATEGORY_SYSTEM,
  .init = usb_msc_init,
  .enable = usb_msc_cli_enable,
  .disable = usb_msc_cli_disable,
  .get_status = usb_msc_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_usb_msc_parameters(void) {
  module_param_t params[] = {
    {
      .name = "mounted",
      .description = "Host has mounted SD card",
      .type = PARAM_TYPE_BOOL,
      .read_only = 1,
      .get_value = usb_msc_param_get_mounted,
      .set_value = NULL
    }
  };
  
  s_usb_msc_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_usb_msc_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int usb_msc_register_cli(void) {
  setup_usb_msc_parameters();
  return module_registry_register(&s_usb_msc_descriptor);
}
