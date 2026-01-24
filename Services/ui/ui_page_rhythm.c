#include "Services/ui/ui_page_rhythm.h"
#include "Services/ui/ui.h"
#include "Services/rhythm_trainer/rhythm_trainer.h"
#include "Services/ui/ui_gfx.h"
#include <stdio.h>
#include <string.h>

// UI state
static uint8_t s_edit_mode = 0;          // 0=VIEW, 1=EDIT
static uint8_t s_selected_param = 0;     // Parameter selection

#define PARAM_DIFFICULTY   0
#define PARAM_SUBDIVISION  1
#define PARAM_PERFECT      2
#define PARAM_GOOD         3
#define PARAM_ADAPTIVE     4
#define PARAM_FEEDBACK     5
#define PARAM_COUNT        6

// Difficulty levels with preset thresholds
typedef enum {
  DIFFICULTY_EASY = 0,      // ±24 ticks perfect, ±48 good
  DIFFICULTY_MEDIUM,        // ±12 ticks perfect, ±24 good
  DIFFICULTY_HARD,          // ±6 ticks perfect, ±16 good
  DIFFICULTY_EXPERT         // ±4 ticks perfect, ±12 good
} difficulty_level_t;

static difficulty_level_t s_difficulty = DIFFICULTY_MEDIUM;

// Difficulty preset thresholds (perfect, good, off)
static const uint16_t difficulty_presets[][3] = {
  {24, 48, 96},   // EASY: ±24 (60ms), ±48 (120ms) @ 120bpm
  {12, 24, 72},   // MEDIUM: ±12 (30ms), ±24 (60ms)
  {6, 16, 48},    // HARD: ±6 (15ms), ±16 (40ms)
  {4, 12, 48}     // EXPERT: ±4 (10ms), ±12 (30ms)
};

/**
 * @brief Initialize rhythm trainer UI page
 */
void ui_page_rhythm_init(void) {
  s_edit_mode = 0;
  s_selected_param = 0;
  s_difficulty = DIFFICULTY_MEDIUM;
  
  // Apply medium difficulty presets
  rhythm_trainer_set_thresholds(
    difficulty_presets[DIFFICULTY_MEDIUM][0],
    difficulty_presets[DIFFICULTY_MEDIUM][1],
    difficulty_presets[DIFFICULTY_MEDIUM][2]
  );
}

/**
 * @brief Draw measure bar with beat subdivisions and threshold zones
 */
static void draw_measure_bar(uint8_t x, uint8_t y, uint8_t width, uint8_t height) {
  rhythm_config_t config;
  rhythm_trainer_get_config(&config);
  
  // Get current playback position and last hit
  rhythm_eval_t last_eval = rhythm_trainer_get_last_eval();
  int32_t last_error = rhythm_trainer_get_last_error();
  
  // Draw background bar
  ui_gfx_rect(x, y, width, height, 8);
  
  // Calculate subdivisions based on setting
  uint8_t num_subdivs = 4;  // Quarter notes (1/4)
  switch (config.subdivision) {
    case 0: num_subdivs = 4; break;    // 1/4 notes
    case 1: num_subdivs = 8; break;    // 1/8 notes
    case 2: num_subdivs = 16; break;   // 1/16 notes
    case 3: num_subdivs = 32; break;   // 1/32 notes
  }
  
  // Draw subdivision markers
  for (uint8_t i = 0; i <= num_subdivs; i++) {
    uint8_t marker_x = x + (i * width) / num_subdivs;
    
    // Every 4th subdivision gets a taller marker (quarter note)
    uint8_t marker_height = (i % 4 == 0) ? height : height / 2;
    uint8_t marker_y = (i % 4 == 0) ? y : y + height / 4;
    
    ui_gfx_vline(marker_x, marker_y, marker_height, 12);
  }
  
  // Draw threshold zones around beat markers
  // Show acceptance zones: perfect (green), good (yellow), outside (red)
  for (uint8_t i = 0; i <= num_subdivs; i++) {
    uint8_t beat_x = x + (i * width) / num_subdivs;
    
    // Calculate pixel width of thresholds
    uint8_t perfect_px = (config.perfect_window * width) / (96 * 4);  // Scale to bar
    uint8_t good_px = (config.good_window * width) / (96 * 4);
    
    // Clamp to reasonable sizes
    if (perfect_px < 1) perfect_px = 1;
    if (good_px < 2) good_px = 2;
    if (good_px > width / num_subdivs / 2) good_px = width / num_subdivs / 2;
    
    // Draw good zone (dim)
    if (beat_x >= good_px) {
      ui_gfx_fill_rect(beat_x - good_px, y + height + 2, good_px * 2, 3, 6);
    }
    
    // Draw perfect zone (bright) on top
    if (beat_x >= perfect_px) {
      ui_gfx_fill_rect(beat_x - perfect_px, y + height + 2, perfect_px * 2, 3, 12);
    }
  }
  
  // Draw last hit indicator
  if (last_error != 0 && last_eval != RHYTHM_EVAL_OFF) {
    // Calculate position based on error
    // Map error ticks to pixel position within measure
    int16_t error_px = (last_error * width) / (96 * 4);
    
    // Find nearest beat position
    uint8_t nearest_beat = 0;  // Simplified: assume beat 0
    uint8_t beat_x = x + (nearest_beat * width) / num_subdivs;
    
    int16_t hit_x = beat_x + error_px;
    
    // Clamp to bar bounds
    if (hit_x < x) hit_x = x;
    if (hit_x > x + width) hit_x = x + width;
    
    // Draw hit marker with color based on evaluation
    uint8_t brightness = 15;
    if (last_eval == RHYTHM_EVAL_PERFECT) brightness = 15;
    else if (last_eval == RHYTHM_EVAL_GOOD) brightness = 10;
    else brightness = 6;
    
    // Triangle marker pointing down
    ui_gfx_fill_rect(hit_x - 1, y - 3, 3, 3, brightness);
  }
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
  ui_gfx_text(x, y, buf, GFX_FONT_LARGE);
  
  // Note counts
  uint8_t row_y = y + 20;
  snprintf(buf, sizeof(buf), "P:%lu G:%lu", stats.perfect_count, stats.good_count);
  ui_gfx_text(x, row_y, buf, GFX_FONT_SMALL);
  
  row_y += 10;
  snprintf(buf, sizeof(buf), "E:%lu L:%lu", stats.early_count, stats.late_count);
  ui_gfx_text(x, row_y, buf, GFX_FONT_SMALL);
  
  row_y += 10;
  snprintf(buf, sizeof(buf), "Total: %lu", stats.total_notes);
  ui_gfx_text(x, row_y, buf, GFX_FONT_SMALL);
}

/**
 * @brief Draw parameter editor with difficulty levels
 */
static void draw_parameters(uint8_t x, uint8_t y) {
  rhythm_config_t config;
  rhythm_trainer_get_config(&config);
  
  char buf[32];
  uint8_t row_y = y;
  
  // Difficulty level
  const char* diff_names[] = {"EASY", "MEDIUM", "HARD", "EXPERT"};
  uint8_t highlight = (s_edit_mode && s_selected_param == PARAM_DIFFICULTY) ? 1 : 0;
  snprintf(buf, sizeof(buf), "%sDifficulty: %s", highlight ? ">" : " ", diff_names[s_difficulty]);
  ui_gfx_text(x, row_y, buf, GFX_FONT_SMALL);
  row_y += 10;
  
  // Subdivision
  const char* subdiv_names[] = {"1/4", "1/8", "1/16", "1/32"};
  highlight = (s_edit_mode && s_selected_param == PARAM_SUBDIVISION) ? 1 : 0;
  snprintf(buf, sizeof(buf), "%sSubdiv: %s", highlight ? ">" : " ", subdiv_names[config.subdivision]);
  ui_gfx_text(x, row_y, buf, GFX_FONT_SMALL);
  row_y += 10;
  
  // Perfect window (only show in custom mode)
  highlight = (s_edit_mode && s_selected_param == PARAM_PERFECT) ? 1 : 0;
  snprintf(buf, sizeof(buf), "%sPerfect: %dtk", highlight ? ">" : " ", config.perfect_window);
  ui_gfx_text(x, row_y, buf, GFX_FONT_SMALL);
  row_y += 10;
  
  // Good window (only show in custom mode)
  highlight = (s_edit_mode && s_selected_param == PARAM_GOOD) ? 1 : 0;
  snprintf(buf, sizeof(buf), "%sGood: %dtk", highlight ? ">" : " ", config.good_window);
  ui_gfx_text(x, row_y, buf, GFX_FONT_SMALL);
  row_y += 10;
  
  // Adaptive mode
  highlight = (s_edit_mode && s_selected_param == PARAM_ADAPTIVE) ? 1 : 0;
  snprintf(buf, sizeof(buf), "%sAdaptive: %s", highlight ? ">" : " ", config.adaptive ? "ON" : "OFF");
  ui_gfx_text(x, row_y, buf, GFX_FONT_SMALL);
  row_y += 10;
  
  // Feedback mode
  highlight = (s_edit_mode && s_selected_param == PARAM_FEEDBACK) ? 1 : 0;
  const char* feedback_names[] = {"NONE", "MUTE", "WARN"};
  uint8_t feedback_mode = rhythm_trainer_get_feedback_mode();
  if (feedback_mode > RHYTHM_FEEDBACK_WARNING) feedback_mode = RHYTHM_FEEDBACK_NONE;
  snprintf(buf, sizeof(buf), "%sFeedback: %s", highlight ? ">" : " ", feedback_names[feedback_mode]);
  ui_gfx_text(x, row_y, buf, GFX_FONT_SMALL);
}

/**
 * @brief Update rhythm trainer UI page
 */
void ui_page_rhythm_update(uint8_t force_redraw) {
  // Always clear screen to prevent text overlay/garbage
  ui_gfx_clear(0);
  
  // Header with 8x8 font
  ui_gfx_set_font(UI_FONT_8X8);
  ui_gfx_text(0, 0, "RHYTHM TRAINER", 15);
  
  uint8_t enabled = rhythm_trainer_get_enabled();
  ui_gfx_text(150, 0, enabled ? "[ON]" : "[OFF]", 12);
  
  ui_gfx_hline(0, 11, 256, 8);
  
  // Main content area
  if (enabled) {
    // Top: Measure bar with subdivisions and threshold zones (LoopA style)
    // Make it taller (14px instead of 10px) for better visibility
    draw_measure_bar(10, 15, 236, 14);
    
    // Middle: Current timing feedback and stats
    rhythm_eval_t last_eval = rhythm_trainer_get_last_eval();
    int32_t last_error = rhythm_trainer_get_last_error();
    
    // Left: Evaluation display with larger font
    const char* eval_label = rhythm_trainer_eval_name(last_eval);
    ui_gfx_text(10, 33, eval_label, 15);
    
    // Show timing error in ms
    char buf[32];
    float error_ms = (float)last_error * 1000.0f / (96.0f * 2.0f);  // Approximate @ 120bpm
    snprintf(buf, sizeof(buf), "%+.1fms", error_ms);
    ui_gfx_text(10, 44, buf, 12);
    
    // Right: Statistics (compact)
    rhythm_stats_t stats;
    rhythm_trainer_get_stats(&stats);
    
    snprintf(buf, sizeof(buf), "Accuracy: %d%%", stats.accuracy_percent);
    ui_gfx_text(130, 33, buf, 13);
    
    snprintf(buf, sizeof(buf), "P:%lu G:%lu", stats.perfect_count, stats.good_count);
    ui_gfx_text(130, 42, buf, 11);
    
    snprintf(buf, sizeof(buf), "E:%lu L:%lu O:%lu", stats.early_count, stats.late_count, stats.off_count);
    ui_gfx_text(130, 51, buf, 11);
  } else {
    ui_gfx_text(10, 25, "Trainer disabled", 12);
    ui_gfx_text(10, 38, "Press BTN3 to enable", 10);
  }
  
  // Footer with smaller font
  ui_gfx_hline(0, 54, 256, 6);
  ui_gfx_set_font(UI_FONT_5X7);
  ui_gfx_text(0, 56, s_edit_mode ? "VIEW" : "EDIT", 10);
  ui_gfx_text(40, 56, "RESET", 10);
  ui_gfx_text(80, 56, enabled ? "OFF" : "ON", 10);
  ui_gfx_text(220, 56, "PAGE", 10);
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
      case PARAM_DIFFICULTY:
        {
          int8_t diff = (int8_t)s_difficulty + delta;
          if (diff < 0) diff = 0;
          if (diff > 3) diff = 3;
          s_difficulty = (difficulty_level_t)diff;
          
          // Apply difficulty preset thresholds
          rhythm_trainer_set_thresholds(
            difficulty_presets[s_difficulty][0],
            difficulty_presets[s_difficulty][1],
            difficulty_presets[s_difficulty][2]
          );
        }
        break;
        
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
          // Manual adjustment (for custom difficulty)
          int16_t val = (int16_t)config.perfect_window + delta;
          if (val < 1) val = 1;
          if (val > 50) val = 50;
          config.perfect_window = (uint16_t)val;
          rhythm_trainer_set_config(&config);
        }
        break;
        
      case PARAM_GOOD:
        {
          // Manual adjustment (for custom difficulty)
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
        
      case PARAM_FEEDBACK:
        {
          uint8_t mode = rhythm_trainer_get_feedback_mode();
          int8_t new_mode = (int8_t)mode + delta;
          if (new_mode < 0) new_mode = RHYTHM_FEEDBACK_WARNING;  // Wrap around
          if (new_mode > RHYTHM_FEEDBACK_WARNING) new_mode = RHYTHM_FEEDBACK_NONE;
          rhythm_trainer_set_feedback_mode((uint8_t)new_mode);
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
