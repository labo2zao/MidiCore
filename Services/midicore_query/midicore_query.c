/**
 * @file midicore_query.c
 * @brief MidiCore Device Query Protocol Implementation
 * 
 * MIOS32 PRINCIPLES:
 * - NO printf / snprintf / vsnprintf (causes stack overflow!)
 * - Fixed string outputs only via cli_puts/cli_print_u32
 * - Minimal stack usage
 */

#include "Services/midicore_query/midicore_query.h"
#include "Config/module_config.h"
#include "stm32f4xx.h"  // For __get_IPSR() CMSIS intrinsic
#include "stm32f4xx_hal.h"  // For HAL_Delay() in retry logic

/* Use proper module configuration macro instead of __has_include
 * to ensure correct behavior across all build configurations */
#if MODULE_ENABLE_USB_MIDI
#include "Services/usb_midi/usb_midi_sysex.h"
#endif

#include <string.h>

/* NO stdio.h - we don't use printf! */

// Default device information
#ifndef MIDICORE_DEVICE_NAME
#define MIDICORE_DEVICE_NAME "MidiCore"
#endif

#ifndef MIDICORE_DEVICE_VERSION
#define MIDICORE_DEVICE_VERSION "1.0.0"  // Fixed: Was MIOS32_DEVICE_VERSION (copyright compliance)
#endif

// Buffer for building SysEx responses (max 256 bytes)
#define SYSEX_BUFFER_SIZE 256
#define SYSEX_HEADER_SIZE 8  // F0 00 00 7E 32 device_id cmd F7 = 8 bytes overhead
#define MAX_RESPONSE_STRING (SYSEX_BUFFER_SIZE - SYSEX_HEADER_SIZE - 1)  // -1 for safety
static uint8_t sysex_response_buffer[SYSEX_BUFFER_SIZE];

// Query queue for deferred processing from task context
#define MIDICORE_QUERY_QUEUE_SIZE 4
#define MIDICORE_QUERY_MAX_LEN 32

typedef struct {
  uint8_t data[MIDICORE_QUERY_MAX_LEN];
  uint32_t len;
  uint8_t cable;
  uint8_t valid;
} midicore_query_queue_entry_t;

static midicore_query_queue_entry_t query_queue[MIDICORE_QUERY_QUEUE_SIZE];
static volatile uint8_t query_queue_write = 0;
static volatile uint8_t query_queue_read = 0;

bool midicore_query_is_query_message(const uint8_t* data, uint32_t len) {
  // Minimum query: F0 00 00 7E <dev_id> <target> <cmd> F7 = 8 bytes
  if (data == NULL || len < 8) {
    return false;
  }
  
  // Check for SysEx start and manufacturer ID: F0 00 00 7E
  if (data[0] != 0xF0 ||
      data[1] != 0x00 ||
      data[2] != 0x00 ||
      data[3] != 0x7E) {
    return false;  // Not a MIDIbox/MidiCore SysEx
  }
  
  // CRITICAL: Accept ALL MIDIbox protocol SysEx to prevent routing to MIDI router!
  // MIOS Studio uses multiple device IDs for different protocols:
  //   - 0x32 = MidiCore device query/response (our primary protocol)
  //   - 0x40 = MIOS32 bootloader commands (upload, read memory, etc.)
  //
  // ALL of these are device management protocols, NOT music data.
  // Routing them to MIDI router causes MIOS Studio crashes/freezes.
  //
  // Known device IDs from MIOS Studio MIDI monitor:
  //   f0 00 00 7e 32 00 00 01 f7         -> 0x32 MidiCore query
  //   f0 00 00 7e 40 00 0d 02 ...        -> 0x40 MIOS32 bootloader
  //   f0 00 00 7e 40 00 02 00 ...        -> 0x40 MIOS32 read memory
  
  uint8_t device_id = data[4];
  
  if (device_id == MIDICORE_QUERY_DEVICE_ID ||   // 0x32 = MidiCore protocol
      device_id == 0x40) {                       // 0x40 = MIOS32 bootloader protocol
    // Accept ALL commands for these device IDs, prevents routing
    return true;
  }
  
  return false;
}

bool midicore_query_process(const uint8_t* data, uint32_t len, uint8_t cable) {
  if (!midicore_query_is_query_message(data, len)) {
    return false;
  }
  
  // Check which protocol this message belongs to (data[4] = device ID)
  uint8_t protocol_id = data[4];
  
  /* MIOS32-STYLE: No debug output here - visible in debugger via breakpoint */
  (void)protocol_id; /* Used in conditionals below */
  
  // ============================================================
  // MIOS32 BOOTLOADER PROTOCOL (0x40)
  // ============================================================
  // MIOS Studio sends these to check for bootloader / upload firmware.
  // We don't have a MIOS32-compatible bootloader, so we:
  // 1. Accept the message (prevents routing to MIDI router = crash fix)
  // 2. Do NOT respond (device won't appear as having bootloader)
  // 3. Return true (message handled)
  //
  // Format: F0 00 00 7E 40 <target> <cmd> <data...> F7
  //   cmd 0x02 = Read memory block
  //   cmd 0x0D = Debug command
  //   etc.
  // ============================================================
  if (protocol_id == 0x40) {
    /* MIOS32 bootloader command - ignore but accept to prevent routing */
    /* Visible in debugger via breakpoint */
    return true;
  }
  
  // ============================================================
  // MIDICORE PROTOCOL (0x32)
  // ============================================================
  // Our device query/response protocol
  // Format: F0 00 00 7E 32 <target> <cmd> <type> <data...> F7
  // ============================================================
  if (protocol_id != MIDICORE_QUERY_DEVICE_ID) {
    // Unknown protocol - ignore but prevent routing
    return true;
  }
  
  // Extract command (byte 6 for MidiCore protocol: F0 00 00 7E 32 <dev> <cmd>)
  uint8_t device_id = data[5];
  uint8_t command = data[6];
  uint8_t query_type = (len > 7) ? data[7] : 0x01; // Default to 0x01 if not specified
  
  /* MIOS32-STYLE: No debug output - device_id/command/query_type visible in debugger */
  (void)device_id; /* Suppress unused warning when debugger not attached */
  
  // Command 0x00: Device Info Request (MIOS Studio uses data[7]=query_type)
  // Command 0x01: Device Info Request (alternate form)
  if (command == 0x01 || (command == 0x00 && len >= 8)) {
    // Respond based on query type, on the same cable the query came from
    midicore_query_send_response(query_type, device_id, cable);
    return true;
  }
  
  // Command 0x0D: Debug/Terminal message from MIOS Studio
  // Format: F0 00 00 7E 32 <dev_id> 0D <type> <text...> F7
  //   type 0x00 = input (command from user)
  //   type 0x40 = output (response to user) - we send these, not receive
  if (command == 0x0D && len > 8) {
    uint8_t msg_type = data[7];
    
    if (msg_type == 0x00) {
      // Input command from MIOS Studio terminal - pass to CLI
      // Extract text from data[8] to data[len-2] (before F7)
      uint32_t text_len = len - 9;  // Skip header (8 bytes) and F7 (1 byte)
      if (text_len > 0 && text_len < 200) {
        // Copy text to null-terminated buffer
        char cmd_buf[201];
        for (uint32_t i = 0; i < text_len; i++) {
          cmd_buf[i] = (char)(data[8 + i] & 0x7F);  // Mask to 7-bit ASCII
        }
        cmd_buf[text_len] = '\0';
        
        /* MIOS32-STYLE: No debug output - command visible in debugger via cmd_buf */
        
        // Feed command to CLI system
        // The CLI will process it and send response back via midicore_debug_send_message()
        extern void cli_process_mios_command(const char* cmd);
        cli_process_mios_command(cmd_buf);
      }
      return true;
    }
    // type 0x40 = output message (we don't expect to receive these)
    return true;  // Acknowledge but don't process
  }
  
  // Command 0x0F: Acknowledge - just ignore (used for bootloader)
  if (command == 0x0F) {
    return true;  // Acknowledged, nothing to do
  }
  
  /* MIOS32-STYLE: No debug output - command visible in debugger */
  (void)command; /* Suppress unused warning */
  
  // Unknown command - ignore but return true to prevent routing
  return true;
}

void midicore_query_send_response(uint8_t query_type, uint8_t device_id, uint8_t cable) {
  // CRITICAL: Check if we're in ISR context
  // NEVER send USB MIDI from ISR - causes reentrancy crash!
  uint32_t ipsr = __get_IPSR();
  if (ipsr != 0) {
    // In ISR context - cannot send USB MIDI response
    // MIOS32-STYLE: No debug output - set breakpoint here to debug
    return;
  }
  
  uint8_t* p = sysex_response_buffer;
  const char* response_str = NULL;
  
  // Determine response string based on query type
  switch (query_type) {
    case 0x01: // Operating system
      response_str = "MidiCore";  // Fixed: Was "MIOS32" (copyright compliance)
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
      response_str = MIDICORE_DEVICE_NAME;
      break;
    case 0x09: // Application name line 2
      response_str = MIDICORE_DEVICE_VERSION;  // Fixed: Was MIOS32_DEVICE_VERSION (copyright compliance)
      break;
    default:
      // Unknown query type - send application name as default
      response_str = MIDICORE_DEVICE_NAME;
      break;
  }
  
  /* MIOS32-STYLE: No debug output - response_str visible in debugger */
  
  // Build response: F0 00 00 7E 32 <device_id> 0x0F <string> F7
  // Following actual MidiCore implementation (mios32/common/mios32_midi.c)
  *p++ = 0xF0;  // SysEx start
  *p++ = 0x00;  // Manufacturer ID 1
  *p++ = 0x00;  // Manufacturer ID 2
  *p++ = 0x7E;  // Manufacturer ID 3 (MIOS)
  *p++ = MIDICORE_QUERY_DEVICE_ID;  // Device ID (0x32)
  *p++ = device_id;  // Device instance ID (echo query)
  *p++ = 0x0F;  // Command: 0x0F = ACK response
  
  // CRITICAL: Validate and safely copy response string with bounds checking
  // Buffer overflow here can crash MIOS Studio or corrupt firmware memory!
  size_t str_len = strlen(response_str);
  if (str_len > MAX_RESPONSE_STRING) {
    /* MIOS32-STYLE: No debug output - truncation visible in debugger */
    str_len = MAX_RESPONSE_STRING;  // Truncate to safe length
  }
  
  // Safe copy with validated length (NO null terminator in SysEx stream!)
  memcpy(p, response_str, str_len);
  p += str_len;
  
  *p++ = 0xF7;  // SysEx end
  
  // Send via USB MIDI on the same cable the query came from
#if MODULE_ENABLE_USB_MIDI
  // CRITICAL: Query response must succeed for MIOS Studio to detect device
  // Retry a few times if TX queue is full
  bool sent = false;
  for (int retry = 0; retry < 5 && !sent; retry++) {
    sent = usb_midi_send_sysex(sysex_response_buffer, p - sysex_response_buffer, cable);
    if (!sent && retry < 4) {
      // TX queue full - wait a bit for it to drain
      // Use HAL_Delay since queries are rare and detection is critical
      HAL_Delay(2);  // 2ms delay between retries
    }
  }
  
  /* MIOS32-STYLE: No debug output - sent status visible in debugger */
  (void)sent; /* Suppress unused warning when debugger not attached */
#else
  (void)cable;  // Suppress unused parameter warning
#endif
}

void midicore_query_send_device_info(const char* device_name, const char* version, uint8_t device_id, uint8_t cable) {
  // This function sends a simple device info response
  // For now, just send the application name (query type 0x08)
  // The device_name and version parameters are ignored in favor of configured defaults
  (void)device_name;  // Suppress unused parameter warning
  (void)version;      // Suppress unused parameter warning
  
  midicore_query_send_response(0x08, device_id, cable);
}

bool midicore_debug_send_message(const char* text, uint8_t cable) {
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
  
  // Build MidiCore debug message SysEx:
  // F0 00 00 7E 32 00 0D <ascii_text> F7
  uint8_t sysex[256];
  uint8_t* p = sysex;
  
  *p++ = 0xF0;                    // SysEx start
  *p++ = 0x00;                    // MidiCore manufacturer ID
  *p++ = 0x00;
  *p++ = 0x7E;
  *p++ = MIDICORE_QUERY_DEVICE_ID;  // 0x32
  *p++ = 0x00;                    // Device instance 0
  *p++ = MIDICORE_CMD_DEBUG_MESSAGE; // 0x0D - debug message command
  *p++ = 0x40;                    // Message type: 0x40 = received (terminal output)
                                   // CRITICAL: MIOS Studio requires this byte!
                                   // See: mios32/tools/mios_studio/src/gui/MiosTerminal.cpp line 113
  
  // Copy ASCII text
  memcpy(p, text, text_len);
  p += text_len;
  
  *p++ = 0xF7;                    // SysEx end
  
  uint32_t total_len = p - sysex;
  
  // Send via USB MIDI SysEx - returns bool indicating success
  // Debug messages are non-critical, so don't retry if TX queue full
  bool sent = usb_midi_send_sysex(sysex, total_len, cable);
  
  if (!sent) {
    // TX queue was full - some packets were dropped
    // This is reported separately by the caller (dbg_print) via CDC
    return false;
  }
  
  return true; // Message sent successfully
  
#endif
}

bool midicore_query_queue(const uint8_t* data, uint32_t len, uint8_t cable) {
  // ISR-safe: Queue query for later processing from task context
  if (!data || len == 0 || len > MIDICORE_QUERY_MAX_LEN) {
    return false;
  }
  
  // Check if queue has space
  if ((query_queue_write - query_queue_read) >= MIDICORE_QUERY_QUEUE_SIZE) {
    return false;  // Queue full
  }
  
  // Add to queue
  uint8_t idx = query_queue_write % MIDICORE_QUERY_QUEUE_SIZE;
  query_queue[idx].len = len;
  memcpy(query_queue[idx].data, data, len);
  query_queue[idx].cable = cable;
  query_queue[idx].valid = 1;
  
  // Increment write pointer (atomic on Cortex-M)
  query_queue_write++;
  
  return true;
}

void midicore_query_process_queued(void) {
  // Process all queued queries from task context (safe to send USB MIDI)
  while (query_queue_read != query_queue_write) {
    uint8_t idx = query_queue_read % MIDICORE_QUERY_QUEUE_SIZE;
    
    if (query_queue[idx].valid) {
      // Process query and send response (now safe - we're in task context)
      midicore_query_process(query_queue[idx].data,
                          query_queue[idx].len,
                          query_queue[idx].cable);
      query_queue[idx].valid = 0;
    }
    
    // Increment read pointer
    query_queue_read++;
  }
}
