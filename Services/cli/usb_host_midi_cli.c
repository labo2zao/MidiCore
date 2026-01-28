/**
 * @file usb_host_midi_cli.c
 * @brief CLI integration for USB Host MIDI
 * 
 * USB Host MIDI for connecting external USB MIDI devices
 */

#include "Services/usb_host_midi/usb_host_midi.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static uint8_t s_device_connected = 0;

static int usb_host_midi_param_get_connected(uint8_t track, param_value_t* out) {
  (void)track;
  out->bool_val = s_device_connected;
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int usb_host_midi_cli_enable(uint8_t track) {
  (void)track;
  return 0; // Always enabled when device connected
}

static int usb_host_midi_cli_disable(uint8_t track) {
  (void)track;
  return -1; // Cannot disable USB Host MIDI
}

static int usb_host_midi_cli_get_status(uint8_t track) {
  (void)track;
  return s_device_connected ? MODULE_STATUS_ENABLED : MODULE_STATUS_DISABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_usb_host_midi_descriptor = {
  .name = "usb_host_midi",
  .description = "USB Host MIDI for external devices",
  .category = MODULE_CATEGORY_MIDI,
  .init = usb_host_midi_init,
  .enable = usb_host_midi_cli_enable,
  .disable = usb_host_midi_cli_disable,
  .get_status = usb_host_midi_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_usb_host_midi_parameters(void) {
  module_param_t params[] = {
    {
      .name = "device_connected",
      .description = "USB MIDI device connected",
      .type = PARAM_TYPE_BOOL,
      .read_only = 1,
      .get_value = usb_host_midi_param_get_connected,
      .set_value = NULL
    }
  };
  
  s_usb_host_midi_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_usb_host_midi_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int usb_host_midi_register_cli(void) {
  setup_usb_host_midi_parameters();
  return module_registry_register(&s_usb_host_midi_descriptor);
}
