/**
 * @file midicore_query.h
 * @brief MidiCore Device Query Protocol Handler
 * 
 * Implements the MidiCore query protocol (device ID 0x32) for MIOS Studio compatibility.
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

/** @brief MidiCore Query Protocol Constants */
#define MIDICORE_QUERY_DEVICE_ID          0x32  // Device ID for query protocol
#define MIDICORE_QUERY_HEADER_LEN         7     // F0 00 00 7E 32 00/01 <type>

/** @brief Query Types */
#define MIDICORE_QUERY_TYPE_DEVICE_INFO   0x01  // Request device information

/** @brief Query/Response Direction */
#define MIDICORE_QUERY_DIRECTION_QUERY    0x00  // From host to device
#define MIDICORE_QUERY_DIRECTION_RESPONSE 0x01  // From device to host

/** @brief MidiCore Command Types */
#define MIDICORE_CMD_DEBUG_MESSAGE        0x0D  // Debug/terminal message to MIOS Studio

/**
 * @brief Check if SysEx message is a MidiCore query
 * @param data SysEx data (including F0 and F7)
 * @param len Length of data
 * @return true if this is a MidiCore query message
 */
bool midicore_query_is_query_message(const uint8_t* data, uint32_t len);

/**
 * @brief Process MidiCore query message and send response
 * @param data SysEx data (including F0 and F7)
 * @param len Length of data
 * @param cable USB MIDI cable number (0-3) to send response on
 * @return true if query was processed and response sent, false otherwise
 */
bool midicore_query_process(const uint8_t* data, uint32_t len, uint8_t cable);

/**
 * @brief Send MidiCore query response based on query type
 * @param query_type Query type (0x01-0x09, see MidiCore protocol)
 * @param device_id Device instance ID to echo back in the response
 * @param cable USB MIDI cable number (0-3) to send response on
 */
void midicore_query_send_response(uint8_t query_type, uint8_t device_id, uint8_t cable);

/**
 * @brief Send device info response to MIOS Studio (legacy, wraps midicore_query_send_response)
 * @param device_name Device name (e.g., "MidiCore")
 * @param version Version string (e.g., "1.0.0")
 * @param device_id Device instance ID to echo back in the response
 * @param cable USB MIDI cable number (0-3) to send response on
 */
void midicore_query_send_device_info(const char* device_name, const char* version, uint8_t device_id, uint8_t cable);

/**
 * @brief Send debug/terminal message to MIOS Studio via MIDI SysEx
 * 
 * Sends ASCII text to MIOS Studio terminal using MidiCore debug message protocol.
 * Format: F0 00 00 7E 32 00 0D <ascii_text> F7
 * 
 * @param text ASCII text to send (max ~240 chars to fit in SysEx)
 * @param cable USB MIDI cable number (0-3) to send on
 * @return true if message sent successfully, false otherwise
 */
bool midicore_debug_send_message(const char* text, uint8_t cable);

/**
 * @brief Queue a MidiCore query for deferred processing
 * 
 * This function is ISR-safe and should be called when a query is received in interrupt context.
 * The query will be processed later from task context by calling midicore_query_process_queued().
 * 
 * @param data SysEx data (including F0 and F7)
 * @param len Length of data (max 32 bytes supported)
 * @param cable USB MIDI cable number (0-3) where query was received
 * @return true if query was queued successfully, false if queue full
 */
bool midicore_query_queue(const uint8_t* data, uint32_t len, uint8_t cable);

/**
 * @brief Process any queued MidiCore queries from task context
 * 
 * This function must be called periodically from task context (NOT from ISR).
 * It processes all queued queries and sends responses safely.
 */
void midicore_query_process_queued(void);

#ifdef __cplusplus
}
#endif
