#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file ui_page_midi_monitor.h
 * @brief MIDI Monitor UI Page - Real-time MIDI message display
 * 
 * Displays incoming and outgoing MIDI messages in real-time with
 * timestamp, port, routing status, and decoded message information.
 * 
 * Features:
 * - Integrates with Services/midi_monitor for event capture
 * - NGC configuration file support
 * - Can be used as debug mirror in test mode
 * - Shows [R]outed or [F]iltered status for each message
 * - Configurable display options (timestamp, hex, auto-scroll)
 */

/**
 * @brief Render the MIDI monitor page
 * @param now_ms Current time in milliseconds
 */
void ui_page_midi_monitor_render(uint32_t now_ms);

/**
 * @brief Handle button press
 * @param id Button ID (1-4)
 * @param pressed 1=pressed, 0=released
 * 
 * Button functions:
 * - B1: Pause/Resume capture
 * - B2: Clear buffer
 * - B3: Toggle HEX display
 * - B4: Toggle TIMESTAMP display
 */
void ui_page_midi_monitor_on_button(uint8_t id, uint8_t pressed);

/**
 * @brief Handle encoder rotation
 * @param delta Rotation delta (-N to +N)
 */
void ui_page_midi_monitor_on_encoder(int8_t delta);

/**
 * @brief Capture a MIDI message for display (called by router hooks)
 * @param node Router node index (port)
 * @param data MIDI message data
 * @param len Message length (1-3 bytes)
 * @param timestamp_ms Timestamp in milliseconds
 * @param is_routed 1 if message was routed, 0 if filtered
 */
void ui_midi_monitor_capture(uint8_t node, const uint8_t* data, uint8_t len, 
                             uint32_t timestamp_ms, uint8_t is_routed);

/**
 * @brief Parse NGC configuration line
 * @param line NGC config line to parse
 * @return 0 if parsed successfully, -1 if not a MIDI_MONITOR command
 * 
 * Supported NGC commands:
 * - MIDI_MONITOR SHOW_TIMESTAMP ON|OFF
 * - MIDI_MONITOR SHOW_HEX ON|OFF
 * - MIDI_MONITOR SHOW_ROUTED_STATUS ON|OFF
 * - MIDI_MONITOR AUTO_SCROLL ON|OFF
 * - MIDI_MONITOR UPDATE_RATE <ms>  (50-1000)
 * 
 * Example NGC file:
 * ```
 * MIDI_MONITOR SHOW_TIMESTAMP ON
 * MIDI_MONITOR SHOW_HEX OFF
 * MIDI_MONITOR AUTO_SCROLL ON
 * MIDI_MONITOR UPDATE_RATE 100
 * ```
 */
int ui_page_midi_monitor_parse_ngc(const char* line);

/**
 * @brief Print text to MIDI monitor (debug mirror mode)
 * @param text Text to display
 * @note Useful in test mode for displaying debug messages on UI
 */
void ui_page_midi_monitor_print(const char* text);

/**
 * @brief Printf-style debug output to MIDI monitor
 * @param format Printf-style format string
 * @param ... Variable arguments
 * @note Useful in test mode for formatted debug output
 */
void ui_page_midi_monitor_printf(const char* format, ...);

#ifdef __cplusplus
}
#endif
