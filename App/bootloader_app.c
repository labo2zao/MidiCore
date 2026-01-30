/**
 * @file bootloader_app.c
 * @brief Example application integration with bootloader
 * 
 * This file demonstrates how to integrate the bootloader with
 * your main application to support firmware updates via USB MIDI.
 */

#include "Services/bootloader/bootloader.h"
#include "Config/module_config.h"

#if MODULE_ENABLE_BOOTLOADER

#if __has_include("Services/usb_midi/usb_midi_sysex.h")
#include "Services/usb_midi/usb_midi_sysex.h"
#endif

#if __has_include("Services/router/router.h")
#include "Services/router/router.h"
#endif

/**
 * @brief Check if SysEx message is a bootloader command
 * @param data SysEx data including F0 and F7
 * @param len Length of data
 * @return true if this is a bootloader message
 */
bool bootloader_app_is_bootloader_sysex(const uint8_t* data, uint32_t len) {
  // Minimum bootloader message: F0 00 00 7E 40 <cmd> <checksum> F7 = 8 bytes
  if (data == NULL || len < 8) {
    return false;
  }
  
  // Check for bootloader SysEx header: F0 00 00 7E 40 (or legacy 0x4E)
  if (data[0] == 0xF0 &&
      data[1] == 0x00 &&
      data[2] == 0x00 &&
      data[3] == 0x7E &&
      (data[4] == 0x40 || data[4] == 0x4E)) {  // Accept both MidiCore standard and legacy
    return true;
  }
  
  return false;
}

/**
 * @brief Handle received SysEx message in application
 * @param data SysEx data including F0 and F7
 * @param len Length of data
 * 
 * Call this from your SysEx receive handler in the application.
 * If the message is a bootloader command, the device will reset
 * into bootloader mode.
 */
void bootloader_app_handle_sysex(const uint8_t* data, uint32_t len) {
  if (bootloader_app_is_bootloader_sysex(data, len)) {
    // This is a bootloader command
    // Request bootloader entry and reset
    bootloader_request_entry();
    // System will reset into bootloader mode
  }
}

/**
 * @brief Example: Handle button press to enter bootloader
 * 
 * Call this when a specific button combination is detected
 * (e.g., hold shift + press a designated button)
 */
void bootloader_app_enter_via_button(void) {
  // Request bootloader mode
  bootloader_request_entry();
  // System will reset into bootloader mode
}

/**
 * @brief Example: Integration with USB MIDI receive callback
 * 
 * This shows how to hook the bootloader into your USB MIDI
 * receive callback.
 */
#ifdef ENABLE_USBD_MIDI
void usb_midi_rx_packet_callback(const uint8_t packet4[4]) {
  // Standard USB MIDI packet processing
  // ...
  
  // If this is a SysEx packet, accumulate and check for bootloader
  static uint8_t sysex_buffer[256];
  static uint32_t sysex_pos = 0;
  
  uint8_t cin = packet4[0] & 0x0F;
  
  // SysEx start or continue
  if (cin == 0x04) {
    // Three bytes of SysEx data
    for (int i = 1; i <= 3; i++) {
      if (sysex_pos < sizeof(sysex_buffer)) {
        sysex_buffer[sysex_pos++] = packet4[i];
      }
    }
  }
  // SysEx end
  else if (cin >= 0x05 && cin <= 0x07) {
    // Copy remaining bytes
    int bytes = (cin == 0x05) ? 1 : (cin == 0x06) ? 2 : 3;
    for (int i = 1; i <= bytes; i++) {
      if (sysex_pos < sizeof(sysex_buffer)) {
        sysex_buffer[sysex_pos++] = packet4[i];
      }
    }
    
    // Complete SysEx message received
    // Check if it's a bootloader message
    bootloader_app_handle_sysex(sysex_buffer, sysex_pos);
    
    // Reset buffer for next message
    sysex_pos = 0;
  }
}
#endif

/**
 * @brief Example: Add bootloader info to system status display
 * 
 * This can be called to display bootloader availability on OLED/LCD
 */
const char* bootloader_app_get_info_string(void) {
  return "Bootloader v1.0.0 available";
}

/**
 * @brief Example: MIDI router integration
 * 
 * If using MIDI router, you may want to intercept bootloader
 * SysEx messages before they are routed.
 */
#if MODULE_ENABLE_ROUTER
bool bootloader_app_router_filter(const uint8_t* data, uint32_t len) {
  // If this is a bootloader SysEx, don't route it
  if (bootloader_app_is_bootloader_sysex(data, len)) {
    bootloader_app_handle_sysex(data, len);
    return false; // Don't route further
  }
  return true; // Route normally
}
#endif

#endif // MODULE_ENABLE_BOOTLOADER
