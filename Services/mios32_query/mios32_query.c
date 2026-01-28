/**
 * @file mios32_query.c
 * @brief MIOS32 Device Query Protocol Implementation
 */

#include "Services/mios32_query/mios32_query.h"
#include "Config/module_config.h"

/* Use proper module configuration macro instead of __has_include
 * to ensure correct behavior across all build configurations */
#if MODULE_ENABLE_USB_MIDI
#include "Services/usb_midi/usb_midi_sysex.h"
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
    // data[5] is device instance ID
    // data[6] is command (0x00 = query, 0x01 = response/info)
    // Accept device info query forms used by MIOS Studio.
    if (data[6] == 0x00 || data[6] == 0x01) {
      return true;
    }
  }
  
  return false;
}

bool mios32_query_process(const uint8_t* data, uint32_t len, uint8_t cable) {
  if (!mios32_query_is_query_message(data, len)) {
    return false;
  }
  
#ifdef MODULE_TEST_USB_DEVICE_MIDI
  // Debug: Show MIOS32 query reception
  extern void dbg_print(const char *str);
  char buf[80];
  snprintf(buf, sizeof(buf), "[MIOS32-Q] Received query len:%lu cable:%u\r\n",
           (unsigned long)len, (unsigned int)cable);
  dbg_print(buf);
#endif
  
  // Extract command (byte 6 for MIOS32 protocol: F0 00 00 7E 32 <dev> <cmd>)
  uint8_t device_id = data[5];
  uint8_t command = data[6];
  uint8_t query_type = (len > 7) ? data[7] : 0x01; // Default to 0x01 if not specified
  
#ifdef MODULE_TEST_USB_DEVICE_MIDI
  // Debug: Show query details
  snprintf(buf, sizeof(buf), "[MIOS32-Q] dev_id:%02X cmd:%02X type:%02X\r\n",
           device_id, command, query_type);
  dbg_print(buf);
#endif
  
  // Command 0x00: Device Info Request (MIOS Studio uses data[7]=query_type)
  // Command 0x01: Device Info Request (alternate form)
  if (command == 0x01 || (command == 0x00 && len >= 8)) {
    // Respond based on query type, on the same cable the query came from
    mios32_query_send_response(query_type, device_id, cable);
    return true;
  }
  
#ifdef MODULE_TEST_USB_DEVICE_MIDI
  // Debug: Unknown command
  snprintf(buf, sizeof(buf), "[MIOS32-Q] Unknown command ignored\r\n");
  dbg_print(buf);
#endif
  
  // Unknown command - ignore
  return false;
}

void mios32_query_send_response(uint8_t query_type, uint8_t device_id, uint8_t cable) {
  uint8_t* p = sysex_response_buffer;
  const char* response_str = NULL;
  
  // Determine response string based on query type
  switch (query_type) {
    case 0x01: // Operating system
      response_str = "MIOS32";
      break;
    case 0x02: // Board
      response_str = "STM32F407VGT6";
      break;
    case 0x03: // Core family
      response_str = "STM32F4";
      break;
    case 0x04: // Chip ID (would need actual chip ID reading)
      response_str = "00000000"; // Placeholder
      break;
    case 0x05: // Serial number
      response_str = "000001"; // Placeholder
      break;
    case 0x06: // Flash memory size
      response_str = "1048576"; // 1MB
      break;
    case 0x07: // RAM memory size
      response_str = "131072"; // 128KB
      break;
    case 0x08: // Application name line 1
      response_str = MIOS32_DEVICE_NAME;
      break;
    case 0x09: // Application name line 2
      response_str = MIOS32_DEVICE_VERSION;
      break;
    default:
      // Unknown query type - send application name as default
      response_str = MIOS32_DEVICE_NAME;
      break;
  }
  
#ifdef MODULE_TEST_USB_DEVICE_MIDI
  // Debug: Show response being sent
  extern void dbg_print(const char *str);
  char buf[100];
  snprintf(buf, sizeof(buf), "[MIOS32-R] Sending type:%02X \"%s\" cable:%u\r\n",
           query_type, response_str, (unsigned int)cable);
  dbg_print(buf);
#endif
  
  // Build response: F0 00 00 7E 32 <device_id> 0x0F <string> F7
  // Following actual MIOS32 implementation (mios32/common/mios32_midi.c)
  *p++ = 0xF0;  // SysEx start
  *p++ = 0x00;  // Manufacturer ID 1
  *p++ = 0x00;  // Manufacturer ID 2
  *p++ = 0x7E;  // Manufacturer ID 3 (MIOS)
  *p++ = MIOS32_QUERY_DEVICE_ID;  // Device ID (0x32)
  *p++ = device_id;  // Device instance ID (echo query)
  *p++ = 0x0F;  // Command: 0x0F = ACK response
  
  // Copy response string (NO null terminator in SysEx stream!)
  while (*response_str && (p < sysex_response_buffer + 250)) {
    *p++ = *response_str++;
  }
  
  *p++ = 0xF7;  // SysEx end
  
  // Send via USB MIDI on the same cable the query came from
#if MODULE_ENABLE_USB_MIDI
  usb_midi_send_sysex(sysex_response_buffer, p - sysex_response_buffer, cable);
#ifdef MODULE_TEST_USB_DEVICE_MIDI
  snprintf(buf, sizeof(buf), "[MIOS32-R] Sent %lu bytes\r\n",
           (unsigned long)(p - sysex_response_buffer));
  dbg_print(buf);
#endif
#else
  (void)cable;  // Suppress unused parameter warning
#endif
}

void mios32_query_send_device_info(const char* device_name, const char* version, uint8_t device_id, uint8_t cable) {
  // This function sends a simple device info response
  // For now, just send the application name (query type 0x08)
  // The device_name and version parameters are ignored in favor of configured defaults
  (void)device_name;  // Suppress unused parameter warning
  (void)version;      // Suppress unused parameter warning
  
  mios32_query_send_response(0x08, device_id, cable);
}

bool mios32_debug_send_message(const char* text, uint8_t cable) {
  if (!text) return false;
  
#if !MODULE_ENABLE_USB_MIDI
  // USB MIDI not enabled, cannot send
  return false;
#else
  
  size_t text_len = strlen(text);
  if (text_len == 0 || text_len > 240) {
    // Empty or too long (SysEx has ~256 byte limit, need room for header/footer)
    return false;
  }
  
  // Build MIOS32 debug message SysEx:
  // F0 00 00 7E 32 00 0D <ascii_text> F7
  uint8_t sysex[256];
  uint8_t* p = sysex;
  
  *p++ = 0xF0;                    // SysEx start
  *p++ = 0x00;                    // MIOS32 manufacturer ID
  *p++ = 0x00;
  *p++ = 0x7E;
  *p++ = MIOS32_QUERY_DEVICE_ID;  // 0x32
  *p++ = 0x00;                    // Device instance 0
  *p++ = MIOS32_CMD_DEBUG_MESSAGE; // 0x0D - debug message command
  
  // Copy ASCII text
  memcpy(p, text, text_len);
  p += text_len;
  
  *p++ = 0xF7;                    // SysEx end
  
  uint32_t total_len = p - sysex;
  
  // Send via USB MIDI SysEx
  return usb_midi_send_sysex(sysex, total_len, cable);
  
#endif
}
