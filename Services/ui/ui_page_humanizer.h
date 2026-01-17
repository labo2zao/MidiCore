#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file ui_page_humanizer.h
 * @brief Humanizer + LFO UI Page - Musical humanization and cyclic modulation
 * 
 * Provides controls for humanizer parameters (velocity/timing humanization)
 * and LFO parameters (waveform, rate, depth, target) for "dream" effects.
 */

void ui_page_humanizer_render(uint32_t now_ms);
void ui_page_humanizer_on_button(uint8_t id, uint8_t pressed);
void ui_page_humanizer_on_encoder(int8_t delta);

#ifdef __cplusplus
}
#endif
