/**
 * @file ui_page_song.c
 * @brief Song Mode UI Page - Scene arrangement and clip matrix
 * 
 * Shows a grid of scenes (4 tracks × 8 scenes). Each cell shows whether
 * a clip exists and its length. Allows scene playback and arrangement.
 */

#include "Services/ui/ui_page_song.h"
#include "Services/ui/ui_gfx.h"
#include "Services/looper/looper.h"
#include <stdio.h>

#define NUM_SCENES 8

// Current state
static uint8_t selected_scene = 0;
static uint8_t selected_track = 0;

/**
 * @brief Render the song mode page
 * 
 * Layout:
 * - Header: "SONG MODE  BPM:120  Scene: A"
 * - Grid: 4 tracks × 8 scenes (showing clip status)
 * - Footer: Button hints
 */
void ui_page_song_render(uint32_t now_ms) {
  (void)now_ms;
  
  looper_transport_t tp;
  looper_get_transport(&tp);
  
  ui_gfx_clear(0);
  
  // Header
  char header[64];
  snprintf(header, sizeof(header), "SONG MODE  BPM:%3u  Scene:%c", 
           tp.bpm, 'A' + selected_scene);
  ui_gfx_text(0, 0, header, 15);
  ui_gfx_rect(0, 9, 256, 1, 4);
  
  // Scene labels (A, B, C, D, E, F, G, H)
  for (uint8_t s = 0; s < NUM_SCENES; s++) {
    char label[4];
    snprintf(label, sizeof(label), "%c", 'A' + s);
    uint8_t gray = (s == selected_scene) ? 15 : 8;
    ui_gfx_text(30 + s * 28, 14, label, gray);
  }
  
  // Draw grid: 4 tracks × 8 scenes
  for (uint8_t t = 0; t < LOOPER_TRACKS; t++) {
    // Track label
    char track_label[8];
    snprintf(track_label, sizeof(track_label), "T%u", t + 1);
    uint8_t gray_label = (t == selected_track) ? 15 : 10;
    ui_gfx_text(0, 24 + t * 10, track_label, gray_label);
    
    // Scene cells for this track
    for (uint8_t s = 0; s < NUM_SCENES; s++) {
      int x = 24 + s * 28;
      int y = 24 + t * 10;
      
      // For now, show track state (will be per-scene in future)
      looper_state_t st = looper_get_state(t);
      uint16_t beats = looper_get_loop_beats(t);
      
      // Draw cell indicator
      uint8_t has_clip = (st != LOOPER_STATE_STOP || beats > 0) ? 1 : 0;
      uint8_t is_selected = (t == selected_track && s == selected_scene) ? 1 : 0;
      
      if (has_clip) {
        // Filled box for clips
        ui_gfx_rect(x, y, 6, 6, is_selected ? 15 : 10);
      } else {
        // Empty box outline
        ui_gfx_rect(x, y, 6, 1, is_selected ? 12 : 6);
        ui_gfx_rect(x, y + 5, 6, 1, is_selected ? 12 : 6);
        ui_gfx_rect(x, y, 1, 6, is_selected ? 12 : 6);
        ui_gfx_rect(x + 5, y, 1, 6, is_selected ? 12 : 6);
      }
    }
  }
  
  // Footer
  ui_gfx_rect(0, 62, 256, 1, 4);
  ui_gfx_text(0, 54, "B1 PLAY  B2 STOP  B3 EDIT  B4 CHAIN  ENC nav", 8);
}

/**
 * @brief Handle button press in song mode
 */
void ui_page_song_on_button(uint8_t id, uint8_t pressed) {
  if (!pressed) return;
  
  switch (id) {
    case 1:  // PLAY selected scene
      // For now, play all tracks that have clips
      for (uint8_t t = 0; t < LOOPER_TRACKS; t++) {
        if (looper_get_loop_beats(t) > 0) {
          looper_set_state(t, LOOPER_STATE_PLAY);
        }
      }
      break;
      
    case 2:  // STOP all tracks
      for (uint8_t t = 0; t < LOOPER_TRACKS; t++) {
        looper_set_state(t, LOOPER_STATE_STOP);
      }
      break;
      
    case 3:  // EDIT - cycle through tracks
      selected_track = (selected_track + 1) % LOOPER_TRACKS;
      break;
      
    case 4:  // CHAIN - cycle through scenes
      selected_scene = (selected_scene + 1) % NUM_SCENES;
      break;
      
    default:
      break;
  }
}

/**
 * @brief Handle encoder rotation in song mode
 */
void ui_page_song_on_encoder(int8_t delta) {
  if (delta > 0) {
    selected_scene = (selected_scene + 1) % NUM_SCENES;
  } else if (delta < 0) {
    selected_scene = (selected_scene + NUM_SCENES - 1) % NUM_SCENES;
  }
}
