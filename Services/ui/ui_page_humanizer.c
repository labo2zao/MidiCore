/**
 * @file ui_page_humanizer.c
 * @brief Humanizer + LFO UI Page - Musical humanization and cyclic modulation
 * 
 * Allows configuring both the humanizer (groove-aware micro-variations) and
 * LFO (cyclic modulation) for creating evolving, "dream" textures.
 */

#include "Services/ui/ui_page_humanizer.h"
#include "Services/ui/ui_gfx.h"
#include "Services/looper/looper.h"
#include <stdio.h>

// Current state
static uint8_t selected_track = 0;
static uint8_t selected_param = 0;  // 0-6 for different parameters
static uint8_t edit_mode = 0;
static uint8_t view_mode = 0;  // 0=Humanizer, 1=LFO

// Waveform names for display
static const char* waveform_names[] = {
  "Sine",
  "Triangle",
  "Saw",
  "Square",
  "Random",
  "S&H"
};

// Target names for display
static const char* target_names[] = {
  "Velocity",
  "Timing",
  "Pitch"
};

/**
 * @brief Render the Humanizer page
 */
void ui_page_humanizer_render(uint32_t now_ms) {
  (void)now_ms;
  
  ui_gfx_clear(0);
  
  // Header with 8x8 font
  ui_gfx_set_font(UI_FONT_8X8);
  char header[64];
  const char* mode_name = view_mode ? "LFO" : "HUMANIZER";
  snprintf(header, sizeof(header), "%s T%u %s", 
           mode_name, selected_track + 1, edit_mode ? "[EDIT]" : "[VIEW]");
  ui_gfx_text(0, 0, header, 15);
  ui_gfx_hline(0, 11, 256, 8);
  
  int y = 15;
  
  if (view_mode == 0) {
    // HUMANIZER VIEW
    
    // Get humanizer status via looper
    uint8_t humanizer_enabled = looper_is_humanizer_enabled(selected_track);
    
    // Status
    char status[64];
    snprintf(status, sizeof(status), "Status: %s", humanizer_enabled ? "ENABLED" : "BYPASSED");
    ui_gfx_text(0, y, status, humanizer_enabled ? 13 : 10);
    y += 12;
    
    // Velocity humanization
    uint8_t vel_amount = looper_get_humanizer_velocity(selected_track);
    char vel_line[64];
    snprintf(vel_line, sizeof(vel_line), "Velocity:   %u/32", vel_amount);
    uint8_t gray_vel = (selected_param == 0) ? 15 : 11;
    if (selected_param == 0 && edit_mode) {
      ui_gfx_text(0, y, ">", 15);
    }
    ui_gfx_text(12, y, vel_line, gray_vel);
    y += 10;
    
    // Timing humanization
    uint8_t timing_amount = looper_get_humanizer_timing(selected_track);
    char timing_line[64];
    snprintf(timing_line, sizeof(timing_line), "Timing:     %u/6 ticks", timing_amount);
    uint8_t gray_timing = (selected_param == 1) ? 15 : 11;
    if (selected_param == 1 && edit_mode) {
      ui_gfx_text(0, y, ">", 15);
    }
    ui_gfx_text(12, y, timing_line, gray_timing);
    y += 10;
    
    // Intensity
    uint8_t intensity = looper_get_humanizer_intensity(selected_track);
    char intensity_line[64];
    snprintf(intensity_line, sizeof(intensity_line), "Intensity:  %u%%", intensity);
    uint8_t gray_intensity = (selected_param == 2) ? 15 : 11;
    if (selected_param == 2 && edit_mode) {
      ui_gfx_text(0, y, ">", 15);
    }
    ui_gfx_text(12, y, intensity_line, gray_intensity);
    
    // Highlight current parameter
    if (selected_param < 3) {
      ui_gfx_rect(0, 26 + selected_param * 10, 256, 10, 2);
    }
    
  } else {
    // LFO VIEW
    
    uint8_t lfo_enabled = looper_is_lfo_enabled(selected_track);
    
    // Status
    char status[64];
    snprintf(status, sizeof(status), "Status: %s", lfo_enabled ? "ENABLED" : "BYPASSED");
    ui_gfx_text(0, y, status, lfo_enabled ? 12 : 8);
    y += 12;
    
    // Waveform
    looper_lfo_waveform_t waveform = looper_get_lfo_waveform(selected_track);
    char wave_line[64];
    snprintf(wave_line, sizeof(wave_line), "Waveform:   %s", waveform_names[waveform]);
    uint8_t gray_wave = (selected_param == 0) ? 15 : 10;
    if (selected_param == 0 && edit_mode) {
      ui_gfx_text(0, y, ">", 15);
    }
    ui_gfx_text(12, y, wave_line, gray_wave);
    y += 10;
    
    // Rate
    uint16_t rate = looper_get_lfo_rate(selected_track);
    char rate_line[64];
    if (looper_is_lfo_bpm_synced(selected_track)) {
      uint8_t divisor = looper_get_lfo_bpm_divisor(selected_track);
      snprintf(rate_line, sizeof(rate_line), "Rate:       %u bar%s [SYNC]", divisor, divisor > 1 ? "s" : "");
    } else {
      // Rate is in 0.01Hz units
      snprintf(rate_line, sizeof(rate_line), "Rate:       %u.%02u Hz", rate / 100, rate % 100);
    }
    uint8_t gray_rate = (selected_param == 1) ? 15 : 10;
    if (selected_param == 1 && edit_mode) {
      ui_gfx_text(0, y, ">", 15);
    }
    ui_gfx_text(12, y, rate_line, gray_rate);
    y += 10;
    
    // Depth
    uint8_t depth = looper_get_lfo_depth(selected_track);
    char depth_line[64];
    snprintf(depth_line, sizeof(depth_line), "Depth:      %u%%", depth);
    uint8_t gray_depth = (selected_param == 2) ? 15 : 10;
    if (selected_param == 2 && edit_mode) {
      ui_gfx_text(0, y, ">", 15);
    }
    ui_gfx_text(12, y, depth_line, gray_depth);
    y += 10;
    
    // Target
    looper_lfo_target_t target = looper_get_lfo_target(selected_track);
    char target_line[64];
    snprintf(target_line, sizeof(target_line), "Target:     %s", target_names[target]);
    uint8_t gray_target = (selected_param == 3) ? 15 : 10;
    if (selected_param == 3 && edit_mode) {
      ui_gfx_text(0, y, ">", 15);
    }
    ui_gfx_text(12, y, target_line, gray_target);
    
    // Highlight current parameter
    if (selected_param < 4) {
      ui_gfx_rect(0, 26 + selected_param * 10, 256, 10, 2);
    }
  }
  
  // Footer with smaller font
  ui_gfx_hline(0, 54, 256, 6);
  ui_gfx_set_font(UI_FONT_5X7);
  const char* footer = view_mode ? 
    "B1:EN/DIS B2:SYNC B3:EDIT B4:HUM ENC:adj" :
    "B1:EN/DIS B2:RESET B3:EDIT B4:LFO ENC:adj";
  ui_gfx_text(0, 56, footer, 10);
}

/**
 * @brief Handle button press in Humanizer page
 */
void ui_page_humanizer_on_button(uint8_t id, uint8_t pressed) {
  if (!pressed) return;
  
  switch (id) {
    case 1:  // Enable/Disable current module
      if (view_mode == 0) {
        // Toggle humanizer
        uint8_t enabled = looper_is_humanizer_enabled(selected_track);
        looper_set_humanizer_enabled(selected_track, !enabled);
      } else {
        // Toggle LFO
        uint8_t enabled = looper_is_lfo_enabled(selected_track);
        looper_set_lfo_enabled(selected_track, !enabled);
      }
      break;
      
    case 2:  // Reset/Sync toggle
      if (view_mode == 0) {
        // Reset humanizer to defaults
        looper_set_humanizer_velocity(selected_track, 16);
        looper_set_humanizer_timing(selected_track, 3);
        looper_set_humanizer_intensity(selected_track, 50);
      } else {
        // Toggle BPM sync for LFO
        uint8_t synced = looper_is_lfo_bpm_synced(selected_track);
        looper_set_lfo_bpm_sync(selected_track, !synced);
        // Reset phase when changing sync mode
        looper_reset_lfo_phase(selected_track);
      }
      break;
      
    case 3:  // Toggle edit mode
      edit_mode = !edit_mode;
      break;
      
    case 4:  // Switch between Humanizer and LFO view
      view_mode = !view_mode;
      selected_param = 0;
      edit_mode = 0;
      break;
      
    default:
      break;
  }
}

/**
 * @brief Handle encoder rotation in Humanizer page
 */
void ui_page_humanizer_on_encoder(int8_t delta) {
  if (edit_mode) {
    // Edit parameter value
    if (view_mode == 0) {
      // HUMANIZER parameters
      switch (selected_param) {
        case 0:  // Velocity humanization
          {
            int16_t vel = (int16_t)looper_get_humanizer_velocity(selected_track) + delta;
            if (vel < 0) vel = 0;
            if (vel > 32) vel = 32;
            looper_set_humanizer_velocity(selected_track, (uint8_t)vel);
          }
          break;
          
        case 1:  // Timing humanization
          {
            int16_t timing = (int16_t)looper_get_humanizer_timing(selected_track) + delta;
            if (timing < 0) timing = 0;
            if (timing > 6) timing = 6;
            looper_set_humanizer_timing(selected_track, (uint8_t)timing);
          }
          break;
          
        case 2:  // Intensity
          {
            int16_t intensity = (int16_t)looper_get_humanizer_intensity(selected_track) + delta * 5;
            if (intensity < 0) intensity = 0;
            if (intensity > 100) intensity = 100;
            looper_set_humanizer_intensity(selected_track, (uint8_t)intensity);
          }
          break;
      }
    } else {
      // LFO parameters
      switch (selected_param) {
        case 0:  // Waveform
          {
            int16_t waveform = (int16_t)looper_get_lfo_waveform(selected_track) + delta;
            if (waveform < 0) waveform = 5;
            if (waveform > 5) waveform = 0;
            looper_set_lfo_waveform(selected_track, (looper_lfo_waveform_t)waveform);
          }
          break;
          
        case 1:  // Rate
          {
            if (looper_is_lfo_bpm_synced(selected_track)) {
              // Adjust BPM divisor
              int16_t divisor = (int16_t)looper_get_lfo_bpm_divisor(selected_track);
              if (delta > 0) {
                divisor *= 2;
                if (divisor > 32) divisor = 32;
              } else {
                divisor /= 2;
                if (divisor < 1) divisor = 1;
              }
              looper_set_lfo_bpm_divisor(selected_track, (uint8_t)divisor);
            } else {
              // Adjust free-running rate
              uint16_t rate = looper_get_lfo_rate(selected_track);
              int32_t new_rate = (int32_t)rate + delta * 5;  // 0.05 Hz steps
              if (new_rate < 1) new_rate = 1;      // Min 0.01 Hz
              if (new_rate > 1000) new_rate = 1000;  // Max 10 Hz
              looper_set_lfo_rate(selected_track, (uint16_t)new_rate);
            }
          }
          break;
          
        case 2:  // Depth
          {
            int16_t depth = (int16_t)looper_get_lfo_depth(selected_track) + delta * 5;
            if (depth < 0) depth = 0;
            if (depth > 100) depth = 100;
            looper_set_lfo_depth(selected_track, (uint8_t)depth);
          }
          break;
          
        case 3:  // Target
          {
            int16_t target = (int16_t)looper_get_lfo_target(selected_track) + delta;
            if (target < 0) target = 2;
            if (target > 2) target = 0;
            looper_set_lfo_target(selected_track, (looper_lfo_target_t)target);
          }
          break;
      }
    }
  } else {
    // Navigate between parameters
    int16_t new_param = (int16_t)selected_param + delta;
    int16_t max_param = view_mode ? 3 : 2;  // LFO has 4 params (0-3), Humanizer has 3 (0-2)
    if (new_param < 0) new_param = max_param;
    if (new_param > max_param) new_param = 0;
    selected_param = (uint8_t)new_param;
  }
}
