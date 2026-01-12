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

#ifdef __cplusplus
}
#endif
