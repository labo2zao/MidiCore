#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file ui_page_livefx.h
 * @brief LiveFX UI Page - Real-time MIDI effects control
 * 
 * Provides controls for transpose, velocity scaling, and force-to-scale
 * effects that can be applied per track.
 */

void ui_page_livefx_render(uint32_t now_ms);
void ui_page_livefx_on_button(uint8_t id, uint8_t pressed);
void ui_page_livefx_on_encoder(int8_t delta);

#ifdef __cplusplus
}
#endif
