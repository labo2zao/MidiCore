/**
 * @file bootloader_protocol.c
 * @brief MIOS32-compatible SysEx protocol implementation
 */

#include "Services/bootloader/bootloader_protocol.h"
#include "Services/bootloader/bootloader.h"

#if __has_include("Services/usb_midi/usb_midi_sysex.h")
#include "Services/usb_midi/usb_midi_sysex.h"
#define BOOTLOADER_HAS_USB_MIDI 1
#else
#define BOOTLOADER_HAS_USB_MIDI 0
#endif

#include <string.h>

// Temporary buffer for building SysEx responses
static uint8_t sysex_tx_buffer[SYSEX_MAX_DATA_SIZE + 16];

void bootloader_protocol_encode_u32(uint32_t value, uint8_t* output) {
  // Encode 32-bit value into 5 bytes of 7-bit data
  output[0] = (value >> 28) & 0x0F;  // Top 4 bits
  output[1] = (value >> 21) & 0x7F;  // Bits 21-27
  output[2] = (value >> 14) & 0x7F;  // Bits 14-20
  output[3] = (value >> 7) & 0x7F;   // Bits 7-13
  output[4] = value & 0x7F;          // Bits 0-6
}

uint32_t bootloader_protocol_decode_u32(const uint8_t* input) {
  // Decode 5 bytes of 7-bit data into 32-bit value
  uint32_t value = 0;
  value |= ((uint32_t)(input[0] & 0x0F) << 28);
  value |= ((uint32_t)(input[1] & 0x7F) << 21);
  value |= ((uint32_t)(input[2] & 0x7F) << 14);
  value |= ((uint32_t)(input[3] & 0x7F) << 7);
  value |= (uint32_t)(input[4] & 0x7F);
  return value;
}

uint8_t bootloader_protocol_checksum(const uint8_t* data, uint32_t len) {
  uint8_t sum = 0;
  for (uint32_t i = 0; i < len; i++) {
    sum += data[i];
  }
  return (-sum) & 0x7F;  // Two's complement, 7-bit
}

static void send_sysex_message(const uint8_t* data, uint32_t len) {
#if BOOTLOADER_HAS_USB_MIDI
  usb_midi_send_sysex(data, len, 0);
#else
  // If USB MIDI not available, messages are dropped
  (void)data;
  (void)len;
#endif
}

void bootloader_protocol_send_ack(uint8_t command, uint32_t address) {
  uint8_t* p = sysex_tx_buffer;
  
  *p++ = 0xF0;  // SysEx start
  *p++ = SYSEX_MANUFACTURER_ID_1;
  *p++ = SYSEX_MANUFACTURER_ID_2;
  *p++ = SYSEX_MANUFACTURER_ID_3;
  *p++ = SYSEX_DEVICE_ID;
  *p++ = SYSEX_RESP_ACK;
  *p++ = command;  // Echo the command
  
  // Encode address
  bootloader_protocol_encode_u32(address, p);
  p += 5;
  
  // Checksum (over data after device ID)
  uint8_t checksum = bootloader_protocol_checksum(
    &sysex_tx_buffer[5], (p - sysex_tx_buffer - 5));
  *p++ = checksum;
  
  *p++ = 0xF7;  // SysEx end
  
  send_sysex_message(sysex_tx_buffer, p - sysex_tx_buffer);
}

void bootloader_protocol_send_error(uint8_t command, uint8_t error_code) {
  uint8_t* p = sysex_tx_buffer;
  
  *p++ = 0xF0;  // SysEx start
  *p++ = SYSEX_MANUFACTURER_ID_1;
  *p++ = SYSEX_MANUFACTURER_ID_2;
  *p++ = SYSEX_MANUFACTURER_ID_3;
  *p++ = SYSEX_DEVICE_ID;
  *p++ = SYSEX_RESP_ERROR;
  *p++ = command;  // Echo the command
  *p++ = error_code & 0x7F;
  
  // Checksum
  uint8_t checksum = bootloader_protocol_checksum(
    &sysex_tx_buffer[5], (p - sysex_tx_buffer - 5));
  *p++ = checksum;
  
  *p++ = 0xF7;  // SysEx end
  
  send_sysex_message(sysex_tx_buffer, p - sysex_tx_buffer);
}

void bootloader_protocol_send_info(uint8_t version_major, uint8_t version_minor,
                                     uint8_t version_patch, uint32_t flash_size,
                                     uint32_t app_address) {
  uint8_t* p = sysex_tx_buffer;
  
  *p++ = 0xF0;  // SysEx start
  *p++ = SYSEX_MANUFACTURER_ID_1;
  *p++ = SYSEX_MANUFACTURER_ID_2;
  *p++ = SYSEX_MANUFACTURER_ID_3;
  *p++ = SYSEX_DEVICE_ID;
  *p++ = SYSEX_RESP_ACK;
  *p++ = SYSEX_CMD_QUERY;
  
  // Version info
  *p++ = version_major & 0x7F;
  *p++ = version_minor & 0x7F;
  *p++ = version_patch & 0x7F;
  
  // Flash size (KB)
  bootloader_protocol_encode_u32(flash_size, p);
  p += 5;
  
  // Application address
  bootloader_protocol_encode_u32(app_address, p);
  p += 5;
  
  // Checksum
  uint8_t checksum = bootloader_protocol_checksum(
    &sysex_tx_buffer[5], (p - sysex_tx_buffer - 5));
  *p++ = checksum;
  
  *p++ = 0xF7;  // SysEx end
  
  send_sysex_message(sysex_tx_buffer, p - sysex_tx_buffer);
}

bool bootloader_protocol_process(const uint8_t* data, uint32_t len) {
  // Minimum valid message: F0 00 00 7E 40 <cmd> <checksum> F7 = 8 bytes
  if (data == NULL || len < 8) {
    return false;
  }
  
  // Verify SysEx framing
  if (data[0] != 0xF0 || data[len - 1] != 0xF7) {
    return false;
  }
  
  // Verify manufacturer ID and device ID (accept both 0x40 and 0x4E for compatibility)
  if (data[1] != SYSEX_MANUFACTURER_ID_1 ||
      data[2] != SYSEX_MANUFACTURER_ID_2 ||
      data[3] != SYSEX_MANUFACTURER_ID_3 ||
      (data[4] != SYSEX_DEVICE_ID && data[4] != SYSEX_DEVICE_ID_LEGACY)) {
    return false; // Not for us
  }
  
  uint8_t command = data[5];
  
  // Verify checksum (last byte before F7)
  uint8_t recv_checksum = data[len - 2];
  uint8_t calc_checksum = bootloader_protocol_checksum(&data[5], len - 7);
  
  if (recv_checksum != calc_checksum) {
    bootloader_protocol_send_error(command, SYSEX_ERROR_CHECKSUM);
    return false;
  }
  
  // Process command
  switch (command) {
    case SYSEX_CMD_QUERY: {
      // Query bootloader information
      bootloader_protocol_send_info(
        BOOTLOADER_VERSION_MAJOR,
        BOOTLOADER_VERSION_MINOR,
        BOOTLOADER_VERSION_PATCH,
        1024,  // 1MB flash
        APPLICATION_START_ADDRESS
      );
      return true;
    }
    
    case SYSEX_CMD_ERASE_APP: {
      // Erase application flash
      bool success = bootloader_erase_application();
      if (success) {
        bootloader_protocol_send_ack(command, 0);
      } else {
        bootloader_protocol_send_error(command, SYSEX_ERROR_ERASE_FAILED);
      }
      return success;
    }
    
    case SYSEX_CMD_WRITE_BLOCK: {
      // Write block: F0 ... <cmd> <addr:5> <len:2> <data...> <checksum> F7
      // Minimum: 8 header + 5 addr + 2 len + 1 data + 1 checksum + 1 F7 = 18 bytes
      if (len < 18) {
        bootloader_protocol_send_error(command, SYSEX_ERROR_INVALID_LEN);
        return false;
      }
      
      // Decode address (offset from application start)
      uint32_t offset = bootloader_protocol_decode_u32(&data[6]);
      
      // Decode length (7-bit encoded, max 127 bytes per message)
      uint16_t data_len = ((uint16_t)(data[11] & 0x7F) << 7) | (data[12] & 0x7F);
      
      // Validate length
      if (data_len == 0 || data_len > SYSEX_MAX_DATA_SIZE) {
        bootloader_protocol_send_error(command, SYSEX_ERROR_INVALID_LEN);
        return false;
      }
      
      // Expected message length
      uint32_t expected_len = 6 + 5 + 2 + data_len + 1 + 1;  // header + addr + len + data + checksum + F7
      if (len != expected_len) {
        bootloader_protocol_send_error(command, SYSEX_ERROR_INVALID_LEN);
        return false;
      }
      
      // Write data to flash
      // Note: Current implementation assumes 7-bit safe data (MSB=0 for each byte)
      // For production use with arbitrary binary data, implement proper 7-to-8 bit
      // decoding here. MidiCore typically uses nibble encoding or base64-like schemes.
      // Example: 7 bytes of 7-bit data -> 6 bytes of 8-bit data
      bool success = bootloader_write_flash(offset, &data[13], data_len);
      
      if (success) {
        // Verify written data
        if (bootloader_verify_flash(offset, &data[13], data_len)) {
          bootloader_protocol_send_ack(command, offset);
        } else {
          bootloader_protocol_send_error(command, SYSEX_ERROR_VERIFY_FAILED);
          return false;
        }
      } else {
        bootloader_protocol_send_error(command, SYSEX_ERROR_WRITE_FAILED);
        return false;
      }
      
      return true;
    }
    
    case SYSEX_CMD_JUMP_APP: {
      // Jump to application
      bootloader_protocol_send_ack(command, APPLICATION_START_ADDRESS);
      
      // Note: In production, implement proper delay using HAL_Delay()
      // or a system timer. This is a simple busy-wait for minimal bootloader.
      #ifdef HAL_DELAY_AVAILABLE
      HAL_Delay(100);  // 100ms delay to allow ACK transmission
      #else
      // Simple busy-wait (approx 100ms at 168MHz)
      for (volatile uint32_t i = 0; i < 4200000; i++);
      #endif
      
      bootloader_jump_to_application();
      return true;
    }
    
    default:
      bootloader_protocol_send_error(command, SYSEX_ERROR_INVALID_CMD);
      return false;
  }
}
