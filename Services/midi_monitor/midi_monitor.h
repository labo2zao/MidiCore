/**
 * @file midi_monitor.h
 * @brief MIDI Monitor Service - Real-time MIDI message inspection
 * 
 * Inspired by MIOS32 MIDI Monitor functionality.
 * Captures and displays MIDI messages from all sources (DIN, USB, etc.)
 * with comprehensive decoding and filtering capabilities.
 * 
 * Features:
 * - Circular buffer for last N messages
 * - Human-readable message decoding
 * - Port/node identification
 * - Timestamp tracking
 * - Filter by message type, channel, port
 * - UART debug output support
 * - UI integration support
 * 
 * @author MidiCore Team
 */

#pragma once

#include <stdint.h>
#include <stddef.h>  // For size_t

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Configuration
// ============================================================================

#ifndef MIDI_MONITOR_BUFFER_SIZE
#define MIDI_MONITOR_BUFFER_SIZE 64  // Circular buffer size (must be power of 2)
#endif

#ifndef MIDI_MONITOR_ENABLE_UART_OUTPUT
#define MIDI_MONITOR_ENABLE_UART_OUTPUT 1  // Enable debug UART output
#endif

// ============================================================================
// Data Types
// ============================================================================

/**
 * @brief MIDI Monitor message types (for filtering)
 */
typedef enum {
  MIDI_MON_MSG_NOTE_OFF       = 0x80,
  MIDI_MON_MSG_NOTE_ON        = 0x90,
  MIDI_MON_MSG_POLY_PRESSURE  = 0xA0,
  MIDI_MON_MSG_CC             = 0xB0,
  MIDI_MON_MSG_PROGRAM_CHANGE = 0xC0,
  MIDI_MON_MSG_CHANNEL_PRESSURE = 0xD0,
  MIDI_MON_MSG_PITCH_BEND     = 0xE0,
  MIDI_MON_MSG_SYSEX          = 0xF0,
  MIDI_MON_MSG_REALTIME       = 0xF8,
  MIDI_MON_MSG_SYSTEM_COMMON  = 0xF1,
  MIDI_MON_MSG_ALL            = 0xFF
} midi_monitor_msg_type_t;

/**
 * @brief MIDI Monitor event structure
 */
typedef struct {
  uint32_t timestamp_ms;     // Timestamp in milliseconds
  uint8_t  node;             // Source/destination node (ROUTER_NODE_xxx)
  uint8_t  len;              // Message length (1-3 for short messages, >3 for SysEx)
  uint8_t  data[16];         // MIDI bytes (up to 16 for partial SysEx display)
  uint8_t  is_sysex;         // 1 if this is a SysEx message
  uint8_t  is_routed;        // 1 if message was routed, 0 if filtered/blocked
  uint16_t sysex_total_len;  // Total SysEx length if is_sysex=1
} midi_monitor_event_t;

/**
 * @brief MIDI Monitor statistics
 */
typedef struct {
  uint32_t total_messages;    // Total messages captured
  uint32_t dropped_messages;  // Messages dropped due to buffer full
  uint32_t note_on_count;
  uint32_t note_off_count;
  uint32_t cc_count;
  uint32_t sysex_count;
  uint32_t realtime_count;
} midi_monitor_stats_t;

/**
 * @brief MIDI Monitor filter configuration
 */
typedef struct {
  uint8_t enabled;            // 1 = monitor enabled, 0 = disabled
  uint8_t filter_node;        // 0xFF = all nodes, else specific node
  uint8_t filter_channel;     // 0xFF = all channels, 0-15 = specific channel
  uint8_t filter_msg_type;    // MIDI_MON_MSG_xxx or ALL
  uint8_t show_sysex;         // 1 = show SysEx messages
  uint8_t show_realtime;      // 1 = show realtime messages (0xF8-0xFF)
  uint8_t uart_output;        // 1 = enable UART debug output
} midi_monitor_config_t;

// ============================================================================
// Public API
// ============================================================================

/**
 * @brief Initialize MIDI monitor service
 */
void midi_monitor_init(void);

/**
 * @brief Capture a short MIDI message (1-3 bytes)
 * @param node Source/destination router node
 * @param data MIDI message bytes
 * @param len Message length (1-3)
 * @param timestamp_ms Timestamp in milliseconds
 * @param is_routed 1 if message was routed, 0 if filtered/blocked
 */
void midi_monitor_capture_short(uint8_t node, const uint8_t* data, uint8_t len, uint32_t timestamp_ms, uint8_t is_routed);

/**
 * @brief Capture a SysEx message
 * @param node Source/destination router node
 * @param data SysEx data (including F0 and F7)
 * @param len Total SysEx length
 * @param timestamp_ms Timestamp in milliseconds
 * @param is_routed 1 if message was routed, 0 if filtered/blocked
 */
void midi_monitor_capture_sysex(uint8_t node, const uint8_t* data, uint16_t len, uint32_t timestamp_ms, uint8_t is_routed);

/**
 * @brief Get number of events in buffer
 * @return Number of events available for reading
 */
uint16_t midi_monitor_get_count(void);

/**
 * @brief Get event at index (0 = oldest, count-1 = newest)
 * @param index Event index
 * @param event Pointer to event structure to fill
 * @return 1 if successful, 0 if index out of range
 */
uint8_t midi_monitor_get_event(uint16_t index, midi_monitor_event_t* event);

/**
 * @brief Clear all captured events
 */
void midi_monitor_clear(void);

/**
 * @brief Get monitor statistics
 * @param stats Pointer to stats structure to fill
 */
void midi_monitor_get_stats(midi_monitor_stats_t* stats);

/**
 * @brief Reset statistics counters
 */
void midi_monitor_reset_stats(void);

/**
 * @brief Get current filter configuration
 * @param config Pointer to config structure to fill
 */
void midi_monitor_get_config(midi_monitor_config_t* config);

/**
 * @brief Set filter configuration
 * @param config Pointer to config structure with new settings
 */
void midi_monitor_set_config(const midi_monitor_config_t* config);

/**
 * @brief Enable/disable monitor
 * @param enabled 1 = enabled, 0 = disabled
 */
void midi_monitor_set_enabled(uint8_t enabled);

/**
 * @brief Enable/disable UART debug output
 * @param enabled 1 = enabled, 0 = disabled
 */
void midi_monitor_set_uart_output(uint8_t enabled);

/**
 * @brief Enable/disable OLED mirroring (if OLED module is active)
 * @param enabled 1 = enabled, 0 = disabled
 * @note In test mode, uses test_oled_mirror. In production mode, uses UI labels.
 * @note Only available when MODULE_ENABLE_OLED && MODULE_ENABLE_UI
 */
void midi_monitor_set_oled_output(uint8_t enabled);

/**
 * @brief Check if OLED mirroring is enabled
 * @return 1 if enabled, 0 if disabled
 * @note Only available when MODULE_ENABLE_OLED && MODULE_ENABLE_UI
 */
uint8_t midi_monitor_get_oled_output(void);

/**
 * @brief Update OLED display (call periodically in production mode)
 * @note In test mode, this is handled automatically
 * @note In production mode, call every 100ms or so to refresh UI
 * @note Only available when MODULE_ENABLE_OLED && MODULE_ENABLE_UI
 */
void midi_monitor_update_oled(void);

/**
 * @brief Decode MIDI message to human-readable string
 * @param data MIDI message bytes
 * @param len Message length
 * @param out Output string buffer
 * @param out_size Output buffer size
 * @return Number of characters written (excluding null terminator)
 */
int midi_monitor_decode_message(const uint8_t* data, uint8_t len, char* out, size_t out_size);

/**
 * @brief Get node name string
 * @param node Router node index
 * @param out Output string buffer
 * @param out_size Output buffer size
 * @return Number of characters written (excluding null terminator)
 */
int midi_monitor_get_node_name(uint8_t node, char* out, size_t out_size);

/**
 * @brief Convert note number to note name (e.g., 60 -> "C4")
 * @param note MIDI note number (0-127)
 * @param out Output string buffer (minimum 4 bytes)
 */
void midi_monitor_note_to_name(uint8_t note, char* out);

#ifdef __cplusplus
}
#endif
