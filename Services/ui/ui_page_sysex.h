#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file ui_page_sysex.h
 * @brief SysEx UI Page - System Exclusive message capture and display
 * 
 * Displays captured SysEx messages with hex view and basic decoding.
 * Allows send/receive/save operations.
 */

void ui_page_sysex_render(uint32_t now_ms);
void ui_page_sysex_on_button(uint8_t id, uint8_t pressed);
void ui_page_sysex_on_encoder(int8_t delta);

#ifdef __cplusplus
}
#endif
