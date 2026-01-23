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
  
  // Header with 8x8 font
  ui_gfx_set_font(UI_FONT_8X8);
  uint8_t current_scene = looper_get_current_scene();
  char header[64];
  snprintf(header, sizeof(header), "SONG BPM:%3u Scene:%c", 
           tp.bpm, 'A' + current_scene);
  ui_gfx_text(0, 0, header, 15);
  ui_gfx_hline(0, 11, 256, 8);
  
  // Scene labels (A, B, C, D, E, F, G, H) - larger spacing
  for (uint8_t s = 0; s < NUM_SCENES; s++) {
    char label[4];
    snprintf(label, sizeof(label), "%c", 'A' + s);
    uint8_t gray = (s == selected_scene) ? 15 : 10;
    ui_gfx_text(32 + s * 28, 15, label, gray);
  }
  
  // Draw grid: 4 tracks × 8 scenes with larger cells (8x8 instead of 6x6)
  for (uint8_t t = 0; t < LOOPER_TRACKS; t++) {
    // Track label
    char track_label[8];
    snprintf(track_label, sizeof(track_label), "T%u", t + 1);
    uint8_t gray_label = (t == selected_track) ? 15 : 12;
    ui_gfx_text(0, 27 + t * 11, track_label, gray_label);
    
    // Scene cells for this track
    for (uint8_t s = 0; s < NUM_SCENES; s++) {
      int x = 26 + s * 28;
      int y = 27 + t * 11;
      
      // Get clip info from looper
      looper_scene_clip_t clip = looper_get_scene_clip(s, t);
      
      uint8_t has_clip = clip.has_clip;
      uint8_t is_selected = (t == selected_track && s == selected_scene) ? 1 : 0;
      uint8_t is_current = (s == current_scene) ? 1 : 0;
      
      if (has_clip) {
        // Filled box for clips - larger and more visible
        uint8_t brightness = is_current ? 15 : (is_selected ? 13 : 11);
        ui_gfx_fill_rect(x, y, 8, 8, brightness);
      } else {
        // Empty box outline - thicker borders
        uint8_t brightness = is_current ? 12 : (is_selected ? 10 : 7);
        ui_gfx_hline(x, y, 8, brightness);
        ui_gfx_hline(x, y + 7, 8, brightness);
        ui_gfx_vline(x, y, 8, brightness);
        ui_gfx_vline(x + 7, y, 8, brightness);
      }
    }
  }
  
  // Footer with smaller font
  ui_gfx_hline(0, 54, 256, 6);
  ui_gfx_set_font(UI_FONT_5X7);
  ui_gfx_text(0, 56, "B1:TRIG B2:SAVE B3:EDIT B4:LOAD ENC:nav", 10);
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
