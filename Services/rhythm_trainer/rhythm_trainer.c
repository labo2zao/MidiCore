#include "Services/rhythm_trainer/rhythm_trainer.h"
#include "cmsis_os2.h"
#include <string.h>
#include <stdlib.h>

#ifndef RHYTHM_PPQN
#define RHYTHM_PPQN 96u  // Pulses per quarter note
#endif

// Global state
static rhythm_config_t g_config;
static rhythm_stats_t g_stats;
static rhythm_eval_t g_last_eval;
static int32_t g_last_error;
static osMutexId_t g_mutex;

/**
 * @brief Initialize rhythm trainer
 */
void rhythm_trainer_init(void) {
  memset(&g_config, 0, sizeof(g_config));
  memset(&g_stats, 0, sizeof(g_stats));
  
  // Default configuration
  g_config.enabled = 0;
  g_config.perfect_window = 4;    // ±4 ticks (~10ms @ 120bpm)
  g_config.good_window = 12;      // ±12 ticks (~30ms @ 120bpm)
  g_config.off_window = 48;       // ±48 ticks (1/8 note @ PPQN=96)
  g_config.subdivision = 0;       // Quarter notes
  g_config.bpm = 120;
  g_config.ts_num = 4;
  g_config.ts_den = 4;
  g_config.adaptive = 0;
  g_config.target_accuracy = 85;
  
  // Audio feedback defaults
  g_config.feedback_mode = RHYTHM_FEEDBACK_NONE;
  g_config.warning_note = 38;     // Snare drum
  g_config.warning_velocity = 90;
  g_config.warning_channel = 9;   // Drum channel
  g_config.warning_port = 0;
  
  g_last_eval = RHYTHM_EVAL_OFF;
  g_last_error = 0;
  
  // Create mutex for thread safety
  const osMutexAttr_t mutex_attr = {
    .name = "rhythm_trainer_mutex",
    .attr_bits = osMutexRecursive | osMutexPrioInherit
  };
  g_mutex = osMutexNew(&mutex_attr);
}

/**
 * @brief Set trainer configuration
 */
void rhythm_trainer_set_config(const rhythm_config_t* config) {
  if (!config) return;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  memcpy(&g_config, config, sizeof(rhythm_config_t));
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Get current configuration
 */
void rhythm_trainer_get_config(rhythm_config_t* out) {
  if (!out) return;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  memcpy(out, &g_config, sizeof(rhythm_config_t));
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Enable/disable rhythm trainer
 */
void rhythm_trainer_set_enabled(uint8_t enable) {
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  g_config.enabled = enable ? 1 : 0;
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Get enabled status
 */
uint8_t rhythm_trainer_get_enabled(void) {
  return g_config.enabled;
}

/**
 * @brief Calculate nearest beat/subdivision tick
 */
static uint32_t calculate_nearest_grid_tick(uint32_t note_tick, uint8_t subdivision) {
  // Calculate ticks per subdivision
  uint32_t ticks_per_quarter = RHYTHM_PPQN;
  uint32_t ticks_per_subdiv;
  
  switch (subdivision) {
    case 0: // RHYTHM_SUBDIV_1_4 - Quarter notes
      ticks_per_subdiv = ticks_per_quarter; 
      break;
    case 1: // RHYTHM_SUBDIV_1_8 - Eighth notes
      ticks_per_subdiv = ticks_per_quarter / 2; 
      break;
    case 2: // RHYTHM_SUBDIV_1_16 - Sixteenth notes
      ticks_per_subdiv = ticks_per_quarter / 4; 
      break;
    case 3: // RHYTHM_SUBDIV_1_32 - Thirty-second notes
      ticks_per_subdiv = ticks_per_quarter / 8; 
      break;
    case 4: // RHYTHM_SUBDIV_1_8T - Eighth note triplets
      ticks_per_subdiv = (ticks_per_quarter * 2) / 3;  // 2/3 of quarter = triplet eighth
      break;
    case 5: // RHYTHM_SUBDIV_1_16T - Sixteenth note triplets
      ticks_per_subdiv = ticks_per_quarter / 3;  // 1/3 of quarter = triplet sixteenth
      break;
    case 6: // RHYTHM_SUBDIV_1_4D - Dotted quarter notes
      ticks_per_subdiv = (ticks_per_quarter * 3) / 2;  // 1.5x quarter
      break;
    case 7: // RHYTHM_SUBDIV_1_8D - Dotted eighth notes
      ticks_per_subdiv = (ticks_per_quarter * 3) / 4;  // 3/4 of quarter
      break;
    case 8: // RHYTHM_SUBDIV_1_16D - Dotted sixteenth notes
      ticks_per_subdiv = (ticks_per_quarter * 3) / 8;  // 3/8 of quarter
      break;
    case 9: // RHYTHM_SUBDIV_QUINTUPLET - 5-tuplets (5 per quarter)
      ticks_per_subdiv = ticks_per_quarter / 5;  // Quarter divided by 5
      break;
    case 10: // RHYTHM_SUBDIV_SEPTUPLET - 7-tuplets (7 per quarter)
      ticks_per_subdiv = ticks_per_quarter / 7;  // Quarter divided by 7
      break;
    case 11: // RHYTHM_SUBDIV_OCTUPLET - 8-tuplets (8 per quarter)
      ticks_per_subdiv = ticks_per_quarter / 8;  // Quarter divided by 8 (double 16ths)
      break;
    case 12: // RHYTHM_SUBDIV_11TUPLET - 11-tuplets (11 per quarter)
      ticks_per_subdiv = ticks_per_quarter / 11;  // Quarter divided by 11
      break;
    case 13: // RHYTHM_SUBDIV_13TUPLET - 13-tuplets (13 per quarter)
      ticks_per_subdiv = ticks_per_quarter / 13;  // Quarter divided by 13
      break;
    default: 
      ticks_per_subdiv = ticks_per_quarter; 
      break;
  }
  
  if (ticks_per_subdiv == 0) ticks_per_subdiv = 1;
  
  // Find nearest grid point
  uint32_t grid_index = (note_tick + ticks_per_subdiv / 2) / ticks_per_subdiv;
  return grid_index * ticks_per_subdiv;
}

/**
 * @brief Evaluate a note timing relative to beat grid
 */
rhythm_eval_t rhythm_trainer_evaluate_note(uint32_t note_tick, uint8_t note_num, uint8_t velocity) {
  if (!g_config.enabled) return RHYTHM_EVAL_OFF;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  // Find nearest grid tick
  uint32_t nearest_tick = calculate_nearest_grid_tick(note_tick, g_config.subdivision);
  
  // Calculate error (signed: + = late, - = early)
  int32_t error = (int32_t)note_tick - (int32_t)nearest_tick;
  int32_t abs_error = abs(error);
  
  // Evaluate timing
  rhythm_eval_t eval;
  
  if (abs_error <= g_config.perfect_window) {
    eval = RHYTHM_EVAL_PERFECT;
    g_stats.perfect_count++;
  } else if (abs_error <= g_config.good_window) {
    eval = RHYTHM_EVAL_GOOD;
    g_stats.good_count++;
  } else if (abs_error <= g_config.off_window) {
    if (error < 0) {
      eval = RHYTHM_EVAL_EARLY;
      g_stats.early_count++;
    } else {
      eval = RHYTHM_EVAL_LATE;
      g_stats.late_count++;
    }
  } else {
    eval = RHYTHM_EVAL_OFF;
    g_stats.off_count++;
  }
  
  // Update statistics
  g_stats.total_notes++;
  
  // Update average error (weighted moving average)
  if (g_stats.total_notes == 1) {
    g_stats.avg_error_ticks = error;
  } else {
    // Weighted average: 90% old + 10% new
    g_stats.avg_error_ticks = (g_stats.avg_error_ticks * 9 + error) / 10;
  }
  
  // Update max errors
  if (error < g_stats.max_early_ticks) {
    g_stats.max_early_ticks = error;
  }
  if (error > g_stats.max_late_ticks) {
    g_stats.max_late_ticks = error;
  }
  
  // Calculate accuracy percentage
  uint32_t accurate = g_stats.perfect_count + g_stats.good_count;
  g_stats.accuracy_percent = (uint8_t)((accurate * 100) / g_stats.total_notes);
  
  // Store last result for UI
  g_last_eval = eval;
  g_last_error = error;
  
  // Adaptive threshold adjustment
  if (g_config.adaptive && g_stats.total_notes % 20 == 0) {
    // Every 20 notes, check if we should tighten thresholds
    if (g_stats.accuracy_percent >= g_config.target_accuracy) {
      // Tighten thresholds by 10%
      g_config.perfect_window = (g_config.perfect_window * 9) / 10;
      g_config.good_window = (g_config.good_window * 9) / 10;
      
      // Don't go below minimum thresholds
      if (g_config.perfect_window < 2) g_config.perfect_window = 2;
      if (g_config.good_window < 6) g_config.good_window = 6;
    }
  }
  
  if (g_mutex) osMutexRelease(g_mutex);
  
  return eval;
}

/**
 * @brief Get current statistics
 */
void rhythm_trainer_get_stats(rhythm_stats_t* out) {
  if (!out) return;
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  memcpy(out, &g_stats, sizeof(rhythm_stats_t));
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Reset statistics to zero
 */
void rhythm_trainer_reset_stats(void) {
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  memset(&g_stats, 0, sizeof(g_stats));
  g_last_eval = RHYTHM_EVAL_OFF;
  g_last_error = 0;
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Get last evaluation result
 */
rhythm_eval_t rhythm_trainer_get_last_eval(void) {
  return g_last_eval;
}

/**
 * @brief Get last timing error in ticks
 */
int32_t rhythm_trainer_get_last_error(void) {
  return g_last_error;
}

/**
 * @brief Update tempo from looper transport
 */
void rhythm_trainer_update_tempo(uint16_t bpm, uint8_t ts_num, uint8_t ts_den) {
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  g_config.bpm = bpm;
  g_config.ts_num = ts_num;
  g_config.ts_den = ts_den;
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Set target subdivision for practice
 */
void rhythm_trainer_set_subdivision(uint8_t subdiv) {
  if (subdiv >= 9) subdiv = 0;  // Clamp to valid range (0-8)
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  g_config.subdivision = subdiv;
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Get current subdivision setting
 */
uint8_t rhythm_trainer_get_subdivision(void) {
  return g_config.subdivision;
}

/**
 * @brief Get subdivision name as string
 */
const char* rhythm_trainer_subdivision_name(uint8_t subdiv) {
  switch (subdiv) {
    case 0: return "1/4";     // Quarter notes
    case 1: return "1/8";     // Eighth notes
    case 2: return "1/16";    // Sixteenth notes
    case 3: return "1/32";    // Thirty-second notes
    case 4: return "1/8T";    // Eighth note triplets
    case 5: return "1/16T";   // Sixteenth note triplets
    case 6: return "1/4.";    // Dotted quarter notes
    case 7: return "1/8.";    // Dotted eighth notes
    case 8: return "1/16.";   // Dotted sixteenth notes
    case 9: return "5-let";   // 5-tuplets (quintuplets)
    case 10: return "7-let";  // 7-tuplets (septuplets)
    case 11: return "8-let";  // 8-tuplets (octuplets)
    case 12: return "11-let"; // 11-tuplets
    case 13: return "13-let"; // 13-tuplets
    default: return "1/4";
  }
}

/**
 * @brief Set timing thresholds
 */
void rhythm_trainer_set_thresholds(uint16_t perfect_ticks, uint16_t good_ticks, uint16_t off_ticks) {
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  g_config.perfect_window = perfect_ticks;
  g_config.good_window = good_ticks;
  g_config.off_window = off_ticks;
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Get evaluation name as string
 */
const char* rhythm_trainer_eval_name(rhythm_eval_t eval) {
  switch (eval) {
    case RHYTHM_EVAL_PERFECT: return "PERFECT";
    case RHYTHM_EVAL_GOOD:    return "GOOD";
    case RHYTHM_EVAL_EARLY:   return "EARLY";
    case RHYTHM_EVAL_LATE:    return "LATE";
    case RHYTHM_EVAL_OFF:     return "OFF";
    default:                  return "UNKNOWN";
  }
}

/**
 * @brief Set audio feedback mode
 */
void rhythm_trainer_set_feedback_mode(uint8_t mode) {
  if (mode > RHYTHM_FEEDBACK_WARNING) mode = RHYTHM_FEEDBACK_NONE; // Clamp to valid range
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  g_config.feedback_mode = mode;
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Get audio feedback mode
 */
uint8_t rhythm_trainer_get_feedback_mode(void) {
  return g_config.feedback_mode;
}

/**
 * @brief Configure warning sound parameters
 */
void rhythm_trainer_set_warning_sound(uint8_t note, uint8_t velocity, 
                                      uint8_t channel, uint8_t port) {
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  
  g_config.warning_note = note;
  g_config.warning_velocity = velocity;
  g_config.warning_channel = channel;
  g_config.warning_port = port;
  
  if (g_mutex) osMutexRelease(g_mutex);
}

/**
 * @brief Process note with audio feedback
 */
int rhythm_trainer_process_note(uint32_t tick, uint8_t note_num, uint8_t velocity,
                                uint8_t* out_note_num, uint8_t* out_velocity, 
                                uint8_t* out_channel) {
  if (!g_config.enabled) {
    // Trainer disabled - pass through
    *out_note_num = note_num;
    *out_velocity = velocity;
    // out_channel left unchanged
    return 1;
  }
  
  // Evaluate the note timing
  rhythm_eval_t eval = rhythm_trainer_evaluate_note(tick, note_num, velocity);
  
  if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
  uint8_t feedback_mode = g_config.feedback_mode;
  if (g_mutex) osMutexRelease(g_mutex);
  
  // Apply audio feedback based on mode and evaluation
  if (feedback_mode == RHYTHM_FEEDBACK_NONE) {
    // NONE - pass through all notes
    *out_note_num = note_num;
    *out_velocity = velocity;
    return 1;
  }
  
  // Check if note is within acceptable timing (PERFECT or GOOD)
  int is_good_timing = (eval == RHYTHM_EVAL_PERFECT || eval == RHYTHM_EVAL_GOOD);
  
  if (is_good_timing) {
    // Good timing - allow note to play
    *out_note_num = note_num;
    *out_velocity = velocity;
    return 1;
  }
  
  // Bad timing - apply feedback
  if (feedback_mode == RHYTHM_FEEDBACK_MUTE) {
    // MUTE mode - block the note
    return 0;
  } else if (feedback_mode == RHYTHM_FEEDBACK_WARNING) {
    // WARNING mode - replace with warning sound
    if (g_mutex) osMutexAcquire(g_mutex, osWaitForever);
    *out_note_num = g_config.warning_note;
    *out_velocity = g_config.warning_velocity;
    *out_channel = g_config.warning_channel;
    if (g_mutex) osMutexRelease(g_mutex);
    return 1;
  }
  
  // Default - pass through
  *out_note_num = note_num;
  *out_velocity = velocity;
  return 1;
}
