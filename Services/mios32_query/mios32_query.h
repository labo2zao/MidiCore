/**
 * @file mios32_query.h
 * @brief MIOS32 Device Query Protocol Handler
 * 
 * Implements the MIOS32 query protocol (device ID 0x32) for MIOS Studio compatibility.
 * This protocol allows MIOS Studio to query connected devices for identification.
 * 
 * Protocol Format:
 *   Query:    F0 00 00 7E 32 <dev_id> <query_type> <data...> F7
 *   Response: F0 00 00 7E 32 <dev_id> <query_type> <data...> F7
 * 
 * Query Types:
 *   0x00: Reserved
 *   0x01: Device Info Query - requests device name, version, etc.
 * 
 * Device Info Response Format:
 *   F0 00 00 7E 32 <dev_id> 01 <device_name> 00 <version> F7
 *   - device_name: ASCII string (null-terminated)
 *   - version: ASCII string (e.g., "1.0.0")
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @brief MIOS32 Query Protocol Constants */
#define MIOS32_QUERY_DEVICE_ID          0x32  // Device ID for query protocol
#define MIOS32_QUERY_HEADER_LEN         7     // F0 00 00 7E 32 00/01 <type>

/** @brief Query Types */
#define MIOS32_QUERY_TYPE_DEVICE_INFO   0x01  // Request device information

/** @brief Query/Response Direction */
#define MIOS32_QUERY_DIRECTION_QUERY    0x00  // From host to device
#define MIOS32_QUERY_DIRECTION_RESPONSE 0x01  // From device to host

/**
 * @brief Check if SysEx message is a MIOS32 query
 * @param data SysEx data (including F0 and F7)
 * @param len Length of data
 * @return true if this is a MIOS32 query message
 */
bool mios32_query_is_query_message(const uint8_t* data, uint32_t len);

/**
 * @brief Process MIOS32 query message and send response
 * @param data SysEx data (including F0 and F7)
 * @param len Length of data
 * @param cable USB MIDI cable number (0-3) to send response on
 * @return true if query was processed and response sent, false otherwise
 */
bool mios32_query_process(const uint8_t* data, uint32_t len, uint8_t cable);

/**
 * @brief Send device info response to MIOS Studio
 * @param device_name Device name (e.g., "MidiCore")
 * @param version Version string (e.g., "1.0.0")
 * @param device_id Device instance ID to echo back in the response
 * @param cable USB MIDI cable number (0-3) to send response on
 */
void mios32_query_send_device_info(const char* device_name, const char* version, uint8_t device_id, uint8_t cable);

#ifdef __cplusplus
}
#endif
