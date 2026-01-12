#include "Services/ui/ui_page_rhythm.h"
#include "Services/ui/ui.h"
#include "Services/rhythm_trainer/rhythm_trainer.h"
#include "Services/gfx/gfx.h"
#include <stdio.h>
#include <string.h>

// UI state
static uint8_t s_edit_mode = 0;          // 0=VIEW, 1=EDIT
static uint8_t s_selected_param = 0;     // 0=Subdiv, 1=Perfect, 2=Good, 3=Adaptive

#define PARAM_SUBDIVISION 0
#define PARAM_PERFECT     1
#define PARAM_GOOD        2
#define PARAM_ADAPTIVE    3
#define PARAM_COUNT       4

/**
 * @brief Initialize rhythm trainer UI page
 */
void ui_page_rhythm_init(void) {
  s_edit_mode = 0;
  s_selected_param = 0;
}

/**
 * @brief Draw timing indicator with color coding
 */
static void draw_timing_indicator(uint8_t x, uint8_t y, rhythm_eval_t eval) {
  const char* label = rhythm_trainer_eval_name(eval);
  
  // Draw label
  gfx_text(x, y, label, GFX_FONT_SMALL);
  
  // Draw color bar based on evaluation
  uint8_t bar_y = y + 12;
  uint8_t bar_width = 60;
  
  switch (eval) {
    case RHYTHM_EVAL_PERFECT:
      // Green bar (bright)
      gfx_fill_rect(x, bar_y, bar_width, 4, 15);
      break;
    case RHYTHM_EVAL_GOOD:
      // Medium brightness
      gfx_fill_rect(x, bar_y, bar_width, 4, 10);
      break;
    case RHYTHM_EVAL_EARLY:
      // Left-aligned bar (early = left)
      gfx_fill_rect(x, bar_y, bar_width / 3, 4, 8);
      break;
    case RHYTHM_EVAL_LATE:
      // Right-aligned bar (late = right)
      gfx_fill_rect(x + bar_width * 2 / 3, bar_y, bar_width / 3, 4, 8);
      break;
    case RHYTHM_EVAL_OFF:
      // Dim bar
      gfx_fill_rect(x, bar_y, bar_width, 4, 3);
      break;
  }
  
  // Draw center line
  gfx_draw_vline(x + bar_width / 2, bar_y, 4, 15);
}

/**
 * @brief Draw statistics panel
 */
static void draw_statistics(uint8_t x, uint8_t y) {
  rhythm_stats_t stats;
  rhythm_trainer_get_stats(&stats);
  
  char buf[32];
  
  // Accuracy percentage (large)
  snprintf(buf, sizeof(buf), "%d%%", stats.accuracy_percent);
  gfx_text(x, y, buf, GFX_FONT_LARGE);
  
  // Note counts
  uint8_t row_y = y + 20;
  snprintf(buf, sizeof(buf), "P:%lu G:%lu", stats.perfect_count, stats.good_count);
  gfx_text(x, row_y, buf, GFX_FONT_SMALL);
  
  row_y += 10;
  snprintf(buf, sizeof(buf), "E:%lu L:%lu", stats.early_count, stats.late_count);
  gfx_text(x, row_y, buf, GFX_FONT_SMALL);
  
  row_y += 10;
  snprintf(buf, sizeof(buf), "Total: %lu", stats.total_notes);
  gfx_text(x, row_y, buf, GFX_FONT_SMALL);
}

/**
 * @brief Draw parameter editor
 */
static void draw_parameters(uint8_t x, uint8_t y) {
  rhythm_config_t config;
  rhythm_trainer_get_config(&config);
  
  char buf[32];
  uint8_t row_y = y;
  
  // Subdivision
  const char* subdiv_names[] = {"1/4", "1/8", "1/16", "1/32"};
  uint8_t highlight = (s_edit_mode && s_selected_param == PARAM_SUBDIVISION) ? 1 : 0;
  snprintf(buf, sizeof(buf), "%sSubdiv: %s", highlight ? ">" : " ", subdiv_names[config.subdivision]);
  gfx_text(x, row_y, buf, GFX_FONT_SMALL);
  row_y += 10;
  
  // Perfect window
  highlight = (s_edit_mode && s_selected_param == PARAM_PERFECT) ? 1 : 0;
  snprintf(buf, sizeof(buf), "%sPerfect: %d", highlight ? ">" : " ", config.perfect_window);
  gfx_text(x, row_y, buf, GFX_FONT_SMALL);
  row_y += 10;
  
  // Good window
  highlight = (s_edit_mode && s_selected_param == PARAM_GOOD) ? 1 : 0;
  snprintf(buf, sizeof(buf), "%sGood: %d", highlight ? ">" : " ", config.good_window);
  gfx_text(x, row_y, buf, GFX_FONT_SMALL);
  row_y += 10;
  
  // Adaptive mode
  highlight = (s_edit_mode && s_selected_param == PARAM_ADAPTIVE) ? 1 : 0;
  snprintf(buf, sizeof(buf), "%sAdaptive: %s", highlight ? ">" : " ", config.adaptive ? "ON" : "OFF");
  gfx_text(x, row_y, buf, GFX_FONT_SMALL);
}

/**
 * @brief Update rhythm trainer UI page
 */
void ui_page_rhythm_update(uint8_t force_redraw) {
  if (force_redraw) {
    gfx_clear();
  }
  
  // Header
  gfx_text(0, 0, "RHYTHM TRAINER", GFX_FONT_NORMAL);
  
  uint8_t enabled = rhythm_trainer_get_enabled();
  gfx_text(150, 0, enabled ? "[ON]" : "[OFF]", GFX_FONT_SMALL);
  
  // Draw page indicator
  gfx_text(240, 0, "RHYT", GFX_FONT_SMALL);
  
  // Main content area
  if (enabled) {
    // Left: Current timing feedback
    rhythm_eval_t last_eval = rhythm_trainer_get_last_eval();
    int32_t last_error = rhythm_trainer_get_last_error();
    
    draw_timing_indicator(10, 15, last_eval);
    
    // Show timing error in ms
    char buf[16];
    float error_ms = (float)last_error * 1000.0f / (96.0f * 2.0f);  // Approximate @ 120bpm
    snprintf(buf, sizeof(buf), "%+.1fms", error_ms);
    gfx_text(10, 35, buf, GFX_FONT_SMALL);
    
    // Right: Statistics
    draw_statistics(120, 15);
  } else {
    gfx_text(10, 25, "Trainer disabled", GFX_FONT_NORMAL);
    gfx_text(10, 40, "Press BTN1 to enable", GFX_FONT_SMALL);
  }
  
  // Bottom: Parameters (always visible)
  gfx_draw_hline(0, 50, 256, 8);
  draw_parameters(5, 52);
  
  // Footer: Button labels
  gfx_text(0, 56, s_edit_mode ? "VIEW" : "EDIT", GFX_FONT_SMALL);
  gfx_text(40, 56, "RESET", GFX_FONT_SMALL);
  gfx_text(80, 56, enabled ? "OFF" : "ON", GFX_FONT_SMALL);
  gfx_text(220, 56, "PAGE", GFX_FONT_SMALL);
}

/**
 * @brief Handle button press on rhythm trainer page
 */
void ui_page_rhythm_button(uint8_t button) {
  switch (button) {
    case 0:  // BTN1 - Toggle VIEW/EDIT mode
      s_edit_mode = !s_edit_mode;
      break;
      
    case 1:  // BTN2 - Reset statistics
      rhythm_trainer_reset_stats();
      break;
      
    case 2:  // BTN3 - Toggle enabled
      {
        uint8_t enabled = rhythm_trainer_get_enabled();
        rhythm_trainer_set_enabled(!enabled);
      }
      break;
      
    case 3:  // BTN4 - Reserved
      break;
      
    case 4:  // BTN5 - Page navigation (handled by main UI)
      break;
  }
}

/**
 * @brief Handle encoder change on rhythm trainer page
 */
void ui_page_rhythm_encoder(int8_t delta) {
  rhythm_config_t config;
  rhythm_trainer_get_config(&config);
  
  if (s_edit_mode) {
    // Edit selected parameter
    switch (s_selected_param) {
      case PARAM_SUBDIVISION:
        {
          int8_t subdiv = (int8_t)config.subdivision + delta;
          if (subdiv < 0) subdiv = 0;
          if (subdiv > 3) subdiv = 3;
          rhythm_trainer_set_subdivision((uint8_t)subdiv);
        }
        break;
        
      case PARAM_PERFECT:
        {
          int16_t val = (int16_t)config.perfect_window + delta;
          if (val < 1) val = 1;
          if (val > 50) val = 50;
          config.perfect_window = (uint16_t)val;
          rhythm_trainer_set_config(&config);
        }
        break;
        
      case PARAM_GOOD:
        {
          int16_t val = (int16_t)config.good_window + delta;
          if (val < 2) val = 2;
          if (val > 100) val = 100;
          config.good_window = (uint16_t)val;
          rhythm_trainer_set_config(&config);
        }
        break;
        
      case PARAM_ADAPTIVE:
        if (delta != 0) {
          config.adaptive = !config.adaptive;
          rhythm_trainer_set_config(&config);
        }
        break;
    }
  } else {
    // Navigate parameters
    int8_t param = (int8_t)s_selected_param + delta;
    if (param < 0) param = PARAM_COUNT - 1;
    if (param >= PARAM_COUNT) param = 0;
    s_selected_param = (uint8_t)param;
  }
}
