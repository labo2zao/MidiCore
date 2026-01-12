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
 * timestamp, port, and decoded message information.
 */

void ui_page_midi_monitor_render(uint32_t now_ms);
void ui_page_midi_monitor_on_button(uint8_t id, uint8_t pressed);
void ui_page_midi_monitor_on_encoder(int8_t delta);

/**
 * @brief Capture a MIDI message for display (called by router hooks)
 * @param port Port/node index
 * @param data MIDI message data
 * @param len Message length (1-3 bytes)
 * @param timestamp_ms Timestamp in milliseconds
 */
void ui_midi_monitor_capture(uint8_t port, const uint8_t* data, uint8_t len, uint32_t timestamp_ms);

#ifdef __cplusplus
}
#endif
