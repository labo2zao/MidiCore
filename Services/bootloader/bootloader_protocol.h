/**
 * @file bootloader_protocol.h
 * @brief MIOS32-compatible SysEx protocol for firmware updates
 * 
 * Protocol Structure (MIOS Studio Compatible):
 * F0 00 00 7E 40 <command> <data...> <checksum> F7
 * 
 * Commands:
 * - 0x01: Query (get bootloader info)
 * - 0x02: Write Block (send firmware data)
 * - 0x03: Read Block (verify firmware data)
 * - 0x04: Erase Application
 * - 0x05: Jump to Application
 * 
 * Responses:
 * - 0x0F: Acknowledge
 * - 0x0E: Error
 * 
 * Note: Device ID 0x40 is standard MidiCore for MIOS Studio compatibility.
 * Legacy ID 0x4E is also accepted for backward compatibility.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief MidiCore SysEx Header (Universal Non-Realtime, MIDIbox) */
#define SYSEX_MANUFACTURER_ID_1     0x00
#define SYSEX_MANUFACTURER_ID_2     0x00
#define SYSEX_MANUFACTURER_ID_3     0x7E
#define SYSEX_DEVICE_ID             0x40  // Standard MidiCore Device ID for MIOS Studio compatibility
#define SYSEX_DEVICE_ID_LEGACY      0x4E  // Legacy 'N' for backward compatibility

/** @brief Bootloader SysEx Commands */
#define SYSEX_CMD_QUERY             0x01
#define SYSEX_CMD_WRITE_BLOCK       0x02
#define SYSEX_CMD_READ_BLOCK        0x03
#define SYSEX_CMD_ERASE_APP         0x04
#define SYSEX_CMD_JUMP_APP          0x05

/** @brief Bootloader SysEx Responses */
#define SYSEX_RESP_ACK              0x0F
#define SYSEX_RESP_ERROR            0x0E

/** @brief Error codes */
#define SYSEX_ERROR_NONE            0x00
#define SYSEX_ERROR_INVALID_CMD     0x01
#define SYSEX_ERROR_INVALID_LEN     0x02
#define SYSEX_ERROR_INVALID_ADDR    0x03
#define SYSEX_ERROR_WRITE_FAILED    0x04
#define SYSEX_ERROR_ERASE_FAILED    0x05
#define SYSEX_ERROR_VERIFY_FAILED   0x06
#define SYSEX_ERROR_CHECKSUM        0x07

/** @brief Maximum data payload size per SysEx message */
#define SYSEX_MAX_DATA_SIZE         256

/**
 * @brief Parse and process bootloader SysEx message
 * @param data Complete SysEx message (including F0 and F7)
 * @param len Length of message
 * @return true if processed successfully, false otherwise
 */
bool bootloader_protocol_process(const uint8_t* data, uint32_t len);

/**
 * @brief Send SysEx ACK response
 * @param command Original command being acknowledged
 * @param address Address that was processed
 */
void bootloader_protocol_send_ack(uint8_t command, uint32_t address);

/**
 * @brief Send SysEx ERROR response
 * @param command Original command that caused error
 * @param error_code Error code
 */
void bootloader_protocol_send_error(uint8_t command, uint8_t error_code);

/**
 * @brief Send bootloader info response
 * @param version_major Major version
 * @param version_minor Minor version
 * @param version_patch Patch version
 * @param flash_size Flash size in KB
 * @param app_address Application start address
 */
void bootloader_protocol_send_info(uint8_t version_major, uint8_t version_minor,
                                     uint8_t version_patch, uint32_t flash_size,
                                     uint32_t app_address);

/**
 * @brief Encode 32-bit value to 7-bit MIDI format (5 bytes)
 * @param value 32-bit value to encode
 * @param output Output buffer (must be at least 5 bytes)
 */
void bootloader_protocol_encode_u32(uint32_t value, uint8_t* output);

/**
 * @brief Decode 32-bit value from 7-bit MIDI format (5 bytes)
 * @param input Input buffer (must be at least 5 bytes)
 * @return Decoded 32-bit value
 */
uint32_t bootloader_protocol_decode_u32(const uint8_t* input);

/**
 * @brief Calculate checksum for SysEx data
 * @param data Data buffer
 * @param len Length of data
 * @return 7-bit checksum
 */
uint8_t bootloader_protocol_checksum(const uint8_t* data, uint32_t len);

#ifdef __cplusplus
}
#endif
