/**
 * @file midi_filter.h
 * @brief MIDI Filter - comprehensive message filtering and routing control
 * 
 * Provides extensive filtering capabilities for MIDI messages including:
 * - Message type filtering (Note, CC, Program Change, Pitch Bend, etc.)
 * - Per-track and per-channel filtering (4 tracks, 16 channels)
 * - Note range filtering (min/max note numbers)
 * - CC number filtering (block specific CCs)
 * - Velocity filtering (min/max thresholds)
 * - SysEx and realtime message filtering
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MIDI_FILTER_MAX_TRACKS 4
#define MIDI_FILTER_MAX_CHANNELS 16
#define MIDI_FILTER_MAX_CC 128
#define MIDI_FILTER_MAX_NOTE 128

/**
 * @brief MIDI message type flags (can be combined with bitwise OR)
 */
typedef enum {
    MIDI_FILTER_MSG_NOTE_ON        = (1 << 0),   // Note On messages (0x90)
    MIDI_FILTER_MSG_NOTE_OFF       = (1 << 1),   // Note Off messages (0x80)
    MIDI_FILTER_MSG_POLY_AFTERTOUCH = (1 << 2),  // Polyphonic Aftertouch (0xA0)
    MIDI_FILTER_MSG_CONTROL_CHANGE = (1 << 3),   // Control Change (0xB0)
    MIDI_FILTER_MSG_PROGRAM_CHANGE = (1 << 4),   // Program Change (0xC0)
    MIDI_FILTER_MSG_CHAN_AFTERTOUCH = (1 << 5),  // Channel Aftertouch (0xD0)
    MIDI_FILTER_MSG_PITCH_BEND     = (1 << 6),   // Pitch Bend (0xE0)
    MIDI_FILTER_MSG_SYSEX          = (1 << 7),   // System Exclusive (0xF0)
    MIDI_FILTER_MSG_CLOCK          = (1 << 8),   // MIDI Clock (0xF8)
    MIDI_FILTER_MSG_START          = (1 << 9),   // Start (0xFA)
    MIDI_FILTER_MSG_CONTINUE       = (1 << 10),  // Continue (0xFB)
    MIDI_FILTER_MSG_STOP           = (1 << 11),  // Stop (0xFC)
    MIDI_FILTER_MSG_ACTIVE_SENSING = (1 << 12),  // Active Sensing (0xFE)
    MIDI_FILTER_MSG_SYSTEM_RESET   = (1 << 13),  // System Reset (0xFF)
    MIDI_FILTER_MSG_ALL            = 0xFFFF      // All message types
} midi_filter_msg_type_t;

/**
 * @brief Channel filter mode
 */
typedef enum {
    MIDI_FILTER_CHANNEL_MODE_ALL = 0,  // Pass all channels
    MIDI_FILTER_CHANNEL_MODE_ALLOW,    // Only allow specified channels
    MIDI_FILTER_CHANNEL_MODE_BLOCK     // Block specified channels
} midi_filter_channel_mode_t;

/**
 * @brief Filter result
 */
typedef enum {
    MIDI_FILTER_RESULT_PASS = 0,    // Message passes filter
    MIDI_FILTER_RESULT_BLOCK        // Message blocked by filter
} midi_filter_result_t;

/**
 * @brief Initialize MIDI filter module
 */
void midi_filter_init(void);

/**
 * @brief Enable/disable filter for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void midi_filter_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if filter is enabled for a track
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t midi_filter_is_enabled(uint8_t track);

/**
 * @brief Set which message types to allow (all others blocked)
 * @param track Track index (0-3)
 * @param msg_types Bitwise OR of MIDI_FILTER_MSG_* flags
 */
void midi_filter_set_allowed_messages(uint8_t track, uint16_t msg_types);

/**
 * @brief Get allowed message types mask
 * @param track Track index (0-3)
 * @return Bitwise OR of MIDI_FILTER_MSG_* flags
 */
uint16_t midi_filter_get_allowed_messages(uint8_t track);

/**
 * @brief Enable/disable specific message type
 * @param track Track index (0-3)
 * @param msg_type Message type flag
 * @param enabled 1 to allow, 0 to block
 */
void midi_filter_set_message_enabled(uint8_t track, midi_filter_msg_type_t msg_type, uint8_t enabled);

/**
 * @brief Check if specific message type is enabled
 * @param track Track index (0-3)
 * @param msg_type Message type flag
 * @return 1 if allowed, 0 if blocked
 */
uint8_t midi_filter_is_message_enabled(uint8_t track, midi_filter_msg_type_t msg_type);

/**
 * @brief Set channel filter mode
 * @param track Track index (0-3)
 * @param mode Channel filter mode
 */
void midi_filter_set_channel_mode(uint8_t track, midi_filter_channel_mode_t mode);

/**
 * @brief Get channel filter mode
 * @param track Track index (0-3)
 * @return Channel filter mode
 */
midi_filter_channel_mode_t midi_filter_get_channel_mode(uint8_t track);

/**
 * @brief Enable/disable specific MIDI channel
 * @param track Track index (0-3)
 * @param channel MIDI channel (0-15)
 * @param enabled 1 to enable, 0 to disable (meaning depends on channel mode)
 */
void midi_filter_set_channel_enabled(uint8_t track, uint8_t channel, uint8_t enabled);

/**
 * @brief Check if specific MIDI channel is enabled
 * @param track Track index (0-3)
 * @param channel MIDI channel (0-15)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t midi_filter_is_channel_enabled(uint8_t track, uint8_t channel);

/**
 * @brief Set all channels enabled/disabled at once
 * @param track Track index (0-3)
 * @param channel_mask 16-bit mask where each bit represents a channel
 */
void midi_filter_set_channel_mask(uint8_t track, uint16_t channel_mask);

/**
 * @brief Get channel enable mask
 * @param track Track index (0-3)
 * @return 16-bit mask where each bit represents a channel
 */
uint16_t midi_filter_get_channel_mask(uint8_t track);

/**
 * @brief Set note range filter
 * @param track Track index (0-3)
 * @param min_note Minimum note number (0-127)
 * @param max_note Maximum note number (0-127)
 */
void midi_filter_set_note_range(uint8_t track, uint8_t min_note, uint8_t max_note);

/**
 * @brief Get note range filter
 * @param track Track index (0-3)
 * @param min_note Output: minimum note number
 * @param max_note Output: maximum note number
 */
void midi_filter_get_note_range(uint8_t track, uint8_t* min_note, uint8_t* max_note);

/**
 * @brief Enable/disable note range filter
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void midi_filter_set_note_range_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if note range filter is enabled
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t midi_filter_is_note_range_enabled(uint8_t track);

/**
 * @brief Set velocity range filter
 * @param track Track index (0-3)
 * @param min_velocity Minimum velocity (0-127)
 * @param max_velocity Maximum velocity (0-127)
 */
void midi_filter_set_velocity_range(uint8_t track, uint8_t min_velocity, uint8_t max_velocity);

/**
 * @brief Get velocity range filter
 * @param track Track index (0-3)
 * @param min_velocity Output: minimum velocity
 * @param max_velocity Output: maximum velocity
 */
void midi_filter_get_velocity_range(uint8_t track, uint8_t* min_velocity, uint8_t* max_velocity);

/**
 * @brief Enable/disable velocity range filter
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void midi_filter_set_velocity_range_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if velocity range filter is enabled
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t midi_filter_is_velocity_range_enabled(uint8_t track);

/**
 * @brief Enable/disable specific CC number
 * @param track Track index (0-3)
 * @param cc_number CC number (0-127)
 * @param enabled 1 to allow, 0 to block
 */
void midi_filter_set_cc_enabled(uint8_t track, uint8_t cc_number, uint8_t enabled);

/**
 * @brief Check if specific CC number is enabled
 * @param track Track index (0-3)
 * @param cc_number CC number (0-127)
 * @return 1 if allowed, 0 if blocked
 */
uint8_t midi_filter_is_cc_enabled(uint8_t track, uint8_t cc_number);

/**
 * @brief Enable/disable CC filtering (when disabled, all CCs pass)
 * @param track Track index (0-3)
 * @param enabled 1 to enable CC filtering, 0 to allow all CCs
 */
void midi_filter_set_cc_filter_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if CC filtering is enabled
 * @param track Track index (0-3)
 * @return 1 if CC filtering enabled, 0 if all CCs pass
 */
uint8_t midi_filter_is_cc_filter_enabled(uint8_t track);

/**
 * @brief Test if a MIDI message passes the filter
 * @param track Track index (0-3)
 * @param status MIDI status byte
 * @param data1 First data byte
 * @param data2 Second data byte
 * @return MIDI_FILTER_RESULT_PASS or MIDI_FILTER_RESULT_BLOCK
 */
midi_filter_result_t midi_filter_test_message(uint8_t track, uint8_t status, uint8_t data1, uint8_t data2);

/**
 * @brief Reset filter configuration for a track to defaults
 * @param track Track index (0-3)
 */
void midi_filter_reset(uint8_t track);

/**
 * @brief Reset all tracks to default filter configuration
 */
void midi_filter_reset_all(void);

/**
 * @brief Get message type name
 * @param msg_type Message type flag
 * @return Message type name string
 */
const char* midi_filter_get_message_type_name(midi_filter_msg_type_t msg_type);

/**
 * @brief Get channel mode name
 * @param mode Channel mode
 * @return Channel mode name string
 */
const char* midi_filter_get_channel_mode_name(midi_filter_channel_mode_t mode);

#ifdef __cplusplus
}
#endif
