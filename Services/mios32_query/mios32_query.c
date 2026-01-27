/**
 * @file mios32_query.c
 * @brief MIOS32 Device Query Protocol Implementation
 */

#include "Services/mios32_query/mios32_query.h"

#if __has_include("Services/usb_midi/usb_midi_sysex.h")
#include "Services/usb_midi/usb_midi_sysex.h"
#define HAS_USB_MIDI 1
#else
#define HAS_USB_MIDI 0
#endif

#include <string.h>

// Default device information
#ifndef MIOS32_DEVICE_NAME
#define MIOS32_DEVICE_NAME "MidiCore"
#endif

#ifndef MIOS32_DEVICE_VERSION
#define MIOS32_DEVICE_VERSION "1.0.0"
#endif

// Buffer for building SysEx responses (max 256 bytes)
static uint8_t sysex_response_buffer[256];

bool mios32_query_is_query_message(const uint8_t* data, uint32_t len) {
  // Minimum query: F0 00 00 7E 32 <dev_id> <cmd> F7 = 8 bytes
  if (data == NULL || len < 8) {
    return false;
  }
  
  // Check for MIOS32 query header: F0 00 00 7E 32
  if (data[0] == 0xF0 &&
      data[1] == 0x00 &&
      data[2] == 0x00 &&
      data[3] == 0x7E &&
      data[4] == MIOS32_QUERY_DEVICE_ID) {
    // data[5] is device instance ID (00 for query)
    // data[6] is command
    // For query messages, device_id should be 0x00 and command should be 0x00 or 0x01
    if (data[5] == 0x00 && (data[6] == 0x00 || data[6] == 0x01)) {
      return true;
    }
  }
  
  return false;
}

bool mios32_query_process(const uint8_t* data, uint32_t len) {
  if (!mios32_query_is_query_message(data, len)) {
    return false;
  }
  
  // Extract command (byte 6 for MIOS32 protocol: F0 00 00 7E 32 <dev> <cmd>)
  uint8_t command = data[6];
  
  // Command 0x00 or 0x01: Device Info Request
  if (command == 0x00 || command == 0x01) {
    // Respond with device information
    mios32_query_send_device_info(MIOS32_DEVICE_NAME, MIOS32_DEVICE_VERSION);
    return true;
  }
  
  // Unknown command - ignore
  return false;
}

void mios32_query_send_device_info(const char* device_name, const char* version) {
  if (!device_name || !version) {
    return;
  }
  
  uint8_t* p = sysex_response_buffer;
  
  // Build response: F0 00 00 7E 32 <dev_id> 0x0F <device_name> 00 <version> F7
  // Following MIOS32 pattern: response uses 0x0F (ACK/info response)
  *p++ = 0xF0;  // SysEx start
  *p++ = 0x00;  // Manufacturer ID 1
  *p++ = 0x00;  // Manufacturer ID 2
  *p++ = 0x7E;  // Manufacturer ID 3 (MIOS32)
  *p++ = MIOS32_QUERY_DEVICE_ID;  // Device ID (0x32)
  *p++ = 0x00;  // Device instance ID
  *p++ = 0x01;  // Command: Layout/Info Response
  
  // Copy device name (ASCII string, space-padded or null-terminated)
  size_t name_len = strlen(device_name);
  if (name_len > 32) name_len = 32;  // Limit to 32 chars
  memcpy(p, device_name, name_len);
  p += name_len;
  *p++ = 0x00;  // Null terminator
  
  // Copy version string
  size_t ver_len = strlen(version);
  if (ver_len > 16) ver_len = 16;  // Limit to 16 chars
  memcpy(p, version, ver_len);
  p += ver_len;
  
  *p++ = 0xF7;  // SysEx end
  
  // Send via USB MIDI
#if HAS_USB_MIDI
  usb_midi_send_sysex(sysex_response_buffer, p - sysex_response_buffer, 0);
#else
  // USB MIDI not available - response not sent
  (void)p;
#endif
}
