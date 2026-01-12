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
  
  // Header with current scene
  uint8_t current_scene = looper_get_current_scene();
  char header[64];
  snprintf(header, sizeof(header), "SONG MODE  BPM:%3u  Scene:%c", 
           tp.bpm, 'A' + current_scene);
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
      
      // Get clip info from looper
      looper_scene_clip_t clip = looper_get_scene_clip(s, t);
      
      uint8_t has_clip = clip.has_clip;
      uint8_t is_selected = (t == selected_track && s == selected_scene) ? 1 : 0;
      uint8_t is_current = (s == current_scene) ? 1 : 0;
      
      if (has_clip) {
        // Filled box for clips
        uint8_t brightness = is_current ? 15 : (is_selected ? 12 : 10);
        ui_gfx_rect(x, y, 6, 6, brightness);
      } else {
        // Empty box outline
        uint8_t brightness = is_current ? 12 : (is_selected ? 10 : 6);
        ui_gfx_rect(x, y, 6, 1, brightness);
        ui_gfx_rect(x, y + 5, 6, 1, brightness);
        ui_gfx_rect(x, y, 1, 6, brightness);
        ui_gfx_rect(x + 5, y, 1, 6, brightness);
      }
    }
  }
  
  // Footer
  ui_gfx_rect(0, 62, 256, 1, 4);
  ui_gfx_text(0, 54, "B1 TRIG  B2 SAVE  B3 EDIT  B4 LOAD  ENC nav", 8);
}

/**
 * @brief Handle button press in song mode
 */
void ui_page_song_on_button(uint8_t id, uint8_t pressed) {
  if (!pressed) return;
  
  switch (id) {
    case 1:  // TRIGGER scene - load and play selected scene
      looper_trigger_scene(selected_scene);
      break;
      
    case 2:  // SAVE current track state to selected scene
      looper_save_to_scene(selected_scene, selected_track);
      break;
      
    case 3:  // EDIT - cycle through tracks
      selected_track = (selected_track + 1) % LOOPER_TRACKS;
      break;
      
    case 4:  // LOAD - load selected scene's track to current
      looper_load_from_scene(selected_scene, selected_track);
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
