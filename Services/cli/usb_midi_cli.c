/**
 * @file usb_midi_cli.c
 * @brief CLI integration for USB MIDI Device (4 ports)
 * 
 * USB Device MIDI transport with 4 virtual ports
 */

#include "Services/usb_midi/usb_midi.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int usb_midi_param_get_port_count(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = 4; // 4 USB MIDI ports (cables)
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int usb_midi_cli_enable(uint8_t track) {
  (void)track;
  return 0; // Always enabled
}

static int usb_midi_cli_disable(uint8_t track) {
  (void)track;
  return -1; // Cannot disable USB MIDI
}

static int usb_midi_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_usb_midi_descriptor = {
  .name = "usb_midi",
  .description = "USB Device MIDI (4 ports/cables)",
  .category = MODULE_CATEGORY_MIDI,
  .init = usb_midi_init,
  .enable = usb_midi_cli_enable,
  .disable = usb_midi_cli_disable,
  .get_status = usb_midi_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_usb_midi_parameters(void) {
  module_param_t params[] = {
    {
      .name = "port_count",
      .description = "Number of USB MIDI ports",
      .type = PARAM_TYPE_INT,
      .min = 4,
      .max = 4,
      .read_only = 1,
      .get_value = usb_midi_param_get_port_count,
      .set_value = NULL
    }
  };
  
  s_usb_midi_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_usb_midi_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int usb_midi_register_cli(void) {
  setup_usb_midi_parameters();
  return module_registry_register(&s_usb_midi_descriptor);
}
