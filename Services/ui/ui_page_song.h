#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file ui_page_song.h
 * @brief Song Mode UI Page - Scene arrangement and clip matrix
 * 
 * Displays a grid of scenes (A-H) with 4 tracks showing which clips
 * are recorded/active in each scene. Allows scene playback and arrangement.
 */

void ui_page_song_render(uint32_t now_ms);
void ui_page_song_on_button(uint8_t id, uint8_t pressed);
void ui_page_song_on_encoder(int8_t delta);

#ifdef __cplusplus
}
#endif
