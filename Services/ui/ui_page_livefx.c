/**
 * @file ui_page_livefx.c
 * @brief LiveFX UI Page - Real-time MIDI effects control
 * 
 * Allows configuring transpose, velocity scaling, and force-to-scale
 * effects per track during performance.
 */

#include "Services/ui/ui_page_livefx.h"
#include "Services/ui/ui_gfx.h"
#include "Services/livefx/livefx.h"
#include "Services/scale/scale.h"
#include <stdio.h>

// Current state
static uint8_t selected_track = 0;
static uint8_t selected_param = 0;  // 0=transpose, 1=velocity, 2=scale
static uint8_t edit_mode = 0;

/**
 * @brief Render the LiveFX page
 */
void ui_page_livefx_render(uint32_t now_ms) {
  (void)now_ms;
  
  ui_gfx_clear(0);
  
  // Header with 8x8 font
  ui_gfx_set_font(UI_FONT_8X8);
  char header[64];
  snprintf(header, sizeof(header), "LIVEFX T%u %s", 
           selected_track + 1, edit_mode ? "[EDIT]" : "[VIEW]");
  ui_gfx_text(0, 0, header, 15);
  ui_gfx_hline(0, 11, 256, 8);
  
  // Get current configuration
  const livefx_config_t* cfg = livefx_get_config(selected_track);
  if (!cfg) return;
  
  // Status
  char status[64];
  snprintf(status, sizeof(status), "Status: %s", cfg->enabled ? "ENABLED" : "BYPASSED");
  ui_gfx_text(0, 15, status, cfg->enabled ? 13 : 10);
  
  // Parameters with better spacing
  int y = 26;
  
  // Transpose
  char transpose_line[64];
  snprintf(transpose_line, sizeof(transpose_line), "Transpose:  %+d semitones", cfg->transpose);
  uint8_t gray_transpose = (selected_param == 0) ? 15 : 11;
  if (selected_param == 0 && edit_mode) {
    ui_gfx_text(0, y, ">", 15);
  }
  ui_gfx_text(12, y, transpose_line, gray_transpose);
  y += 10;
  
  // Velocity Scale
  char velocity_line[64];
  uint16_t vel_percent = (cfg->vel_scale * 100) / 128;
  snprintf(velocity_line, sizeof(velocity_line), "Velocity:   %u%%", vel_percent);
  uint8_t gray_velocity = (selected_param == 1) ? 15 : 11;
  if (selected_param == 1 && edit_mode) {
    ui_gfx_text(0, y, ">", 15);
  }
  ui_gfx_text(12, y, velocity_line, gray_velocity);
  y += 10;
  
  // Force-to-Scale
  char scale_line[64];
  if (cfg->force_scale) {
    const char* scale_name = scale_get_name(cfg->scale_type);
    const char* root_name = scale_get_note_name(cfg->scale_root);
    snprintf(scale_line, sizeof(scale_line), "Scale:      %s %s [ON]", root_name, scale_name);
  } else {
    snprintf(scale_line, sizeof(scale_line), "Scale:      [OFF]");
  }
  uint8_t gray_scale = (selected_param == 2) ? 15 : 11;
  if (selected_param == 2 && edit_mode) {
    ui_gfx_text(0, y, ">", 15);
  }
  ui_gfx_text(12, y, scale_line, gray_scale);
  
  // Highlight current parameter
  if (selected_param < 3) {
    ui_gfx_rect(0, 26 + selected_param * 10, 256, 10, 2);
  }
  
  // Footer with smaller font
  ui_gfx_hline(0, 54, 256, 6);
  ui_gfx_set_font(UI_FONT_5X7);
  ui_gfx_text(0, 56, "B1:EN/DIS B2:RESET B3:EDIT B4:TRACK ENC:adj", 10);
}

/**
 * @brief Handle button press in LiveFX page
 */
void ui_page_livefx_on_button(uint8_t id, uint8_t pressed) {
  if (!pressed) return;
  
  switch (id) {
    case 1:  // Enable/Disable LiveFX for track
      {
        uint8_t enabled = livefx_get_enabled(selected_track);
        livefx_set_enabled(selected_track, !enabled);
      }
      break;
      
    case 2:  // Reset to defaults
      livefx_set_transpose(selected_track, 0);
      livefx_set_velocity_scale(selected_track, 128);  // 100%
      livefx_set_force_scale(selected_track, 0, 0, 0);  // Off
      break;
      
    case 3:  // Toggle edit mode
      edit_mode = !edit_mode;
      break;
      
    case 4:  // Cycle tracks
      selected_track = (selected_track + 1) % 4;
      edit_mode = 0;
      break;
      
    default:
      break;
  }
}

/**
 * @brief Handle encoder rotation in LiveFX page
 */
void ui_page_livefx_on_encoder(int8_t delta) {
  const livefx_config_t* cfg = livefx_get_config(selected_track);
  if (!cfg) return;
  
  if (edit_mode) {
    // Edit parameter value
    switch (selected_param) {
      case 0:  // Transpose
        {
          int8_t transpose = cfg->transpose + delta;
          if (transpose < -12) transpose = -12;
          if (transpose > 12) transpose = 12;
          livefx_set_transpose(selected_track, transpose);
        }
        break;
        
      case 1:  // Velocity scale
        {
          int16_t vel = (int16_t)cfg->vel_scale + delta * 8;  // Change by ~6% per step
          if (vel < 0) vel = 0;
          if (vel > 255) vel = 255;
          livefx_set_velocity_scale(selected_track, (uint8_t)vel);
        }
        break;
        
      case 2:  // Force-to-scale
        {
          // For now, just toggle on/off with encoder
          // Future: could cycle through scales and roots
          uint8_t scale_type, root, enable;
          livefx_get_force_scale(selected_track, &scale_type, &root, &enable);
          
          if (delta != 0) {
            // Toggle enable or cycle scales
            if (!enable) {
              // Enable with Major scale, C root
              livefx_set_force_scale(selected_track, SCALE_MAJOR, 0, 1);
            } else {
              // Cycle through common scales
              scale_type++;
              if (scale_type >= SCALE_COUNT) scale_type = 0;
              livefx_set_force_scale(selected_track, scale_type, root, 1);
            }
          }
        }
        break;
    }
  } else {
    // Navigate parameters
    if (delta > 0) {
      selected_param = (selected_param + 1) % 3;
    } else if (delta < 0) {
      selected_param = (selected_param + 2) % 3;  // Wrap around
    }
  }
}
