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
    case 0: ticks_per_subdiv = ticks_per_quarter; break;       // Quarter notes
    case 1: ticks_per_subdiv = ticks_per_quarter / 2; break;   // Eighth notes
    case 2: ticks_per_subdiv = ticks_per_quarter / 4; break;   // Sixteenth notes
    case 3: ticks_per_subdiv = ticks_per_quarter / 8; break;   // Thirty-second notes
    default: ticks_per_subdiv = ticks_per_quarter; break;
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
  if (subdiv > 3) subdiv = 3;
  
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
