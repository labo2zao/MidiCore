#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file rhythm_trainer.h
 * @brief Rhythm Trainer - Pedagogical tool for timing practice
 * 
 * Provides real-time feedback on note timing accuracy relative to beat grid.
 * Helps musicians improve their rhythmic precision through visual feedback
 * and statistics.
 */

// Timing evaluation results
typedef enum {
  RHYTHM_EVAL_PERFECT = 0,    // Within perfect threshold
  RHYTHM_EVAL_GOOD,           // Within good threshold
  RHYTHM_EVAL_EARLY,          // Before beat, outside good threshold
  RHYTHM_EVAL_LATE,           // After beat, outside good threshold
  RHYTHM_EVAL_OFF             // Way off beat
} rhythm_eval_t;

// Statistics for tracking progress
typedef struct {
  uint32_t total_notes;       // Total notes played
  uint32_t perfect_count;     // Notes within perfect threshold
  uint32_t good_count;        // Notes within good threshold
  uint32_t early_count;       // Notes early (outside good)
  uint32_t late_count;        // Notes late (outside good)
  uint32_t off_count;         // Notes way off beat
  
  int32_t avg_error_ticks;    // Average timing error in ticks (+ late, - early)
  int32_t max_early_ticks;    // Worst early error
  int32_t max_late_ticks;     // Worst late error
  
  uint8_t accuracy_percent;   // Overall accuracy (0-100%)
} rhythm_stats_t;

// Training configuration
typedef struct {
  uint8_t enabled;            // Trainer active flag
  
  // Thresholds in ticks (at PPQN=96)
  uint16_t perfect_window;    // ±ticks for "perfect" (default: 4 = ~10ms @ 120bpm)
  uint16_t good_window;       // ±ticks for "good" (default: 12 = ~30ms @ 120bpm)
  uint16_t off_window;        // ±ticks for "off" vs "way off" (default: 48 = 1/8 note)
  
  // Target subdivision for evaluation
  uint8_t subdivision;        // 0=1/4, 1=1/8, 2=1/16, 3=1/32
  
  // Metronome settings (linked to looper transport)
  uint16_t bpm;               // Beats per minute
  uint8_t ts_num;             // Time signature numerator
  uint8_t ts_den;             // Time signature denominator
  
  // Difficulty progression
  uint8_t adaptive;           // Auto-adjust thresholds based on performance
  uint8_t target_accuracy;    // Target accuracy for threshold tightening (80-100%)
} rhythm_config_t;

/**
 * @brief Initialize rhythm trainer
 */
void rhythm_trainer_init(void);

/**
 * @brief Set trainer configuration
 * @param config Configuration structure
 */
void rhythm_trainer_set_config(const rhythm_config_t* config);

/**
 * @brief Get current configuration
 * @param out Output configuration structure
 */
void rhythm_trainer_get_config(rhythm_config_t* out);

/**
 * @brief Enable/disable rhythm trainer
 * @param enable 1 to enable, 0 to disable
 */
void rhythm_trainer_set_enabled(uint8_t enable);

/**
 * @brief Get enabled status
 * @return 1 if enabled, 0 if disabled
 */
uint8_t rhythm_trainer_get_enabled(void);

/**
 * @brief Evaluate a note timing relative to beat grid
 * @param note_tick Tick position when note was played
 * @param note_num MIDI note number
 * @param velocity MIDI velocity
 * @return Timing evaluation result
 * 
 * Call this when a Note On event is received. The trainer will
 * determine how close the note was to the expected beat/subdivision
 * and return an evaluation result.
 */
rhythm_eval_t rhythm_trainer_evaluate_note(uint32_t note_tick, uint8_t note_num, uint8_t velocity);

/**
 * @brief Get current statistics
 * @param out Output statistics structure
 */
void rhythm_trainer_get_stats(rhythm_stats_t* out);

/**
 * @brief Reset statistics to zero
 */
void rhythm_trainer_reset_stats(void);

/**
 * @brief Get last evaluation result (for UI display)
 * @return Last evaluation result
 */
rhythm_eval_t rhythm_trainer_get_last_eval(void);

/**
 * @brief Get last timing error in ticks (for UI display)
 * @return Signed timing error (+ = late, - = early)
 */
int32_t rhythm_trainer_get_last_error(void);

/**
 * @brief Update tempo from looper transport
 * @param bpm Beats per minute
 * @param ts_num Time signature numerator
 * @param ts_den Time signature denominator
 * 
 * Should be called when looper tempo changes to keep trainer in sync
 */
void rhythm_trainer_update_tempo(uint16_t bpm, uint8_t ts_num, uint8_t ts_den);

/**
 * @brief Set target subdivision for practice
 * @param subdiv 0=quarter, 1=eighth, 2=sixteenth, 3=thirty-second
 */
void rhythm_trainer_set_subdivision(uint8_t subdiv);

/**
 * @brief Get current subdivision setting
 * @return Subdivision index
 */
uint8_t rhythm_trainer_get_subdivision(void);

/**
 * @brief Set timing thresholds
 * @param perfect_ticks ±ticks for perfect timing
 * @param good_ticks ±ticks for good timing
 * @param off_ticks ±ticks for off vs way off
 */
void rhythm_trainer_set_thresholds(uint16_t perfect_ticks, uint16_t good_ticks, uint16_t off_ticks);

/**
 * @brief Get evaluation name as string
 * @param eval Evaluation result
 * @return String name ("PERFECT", "GOOD", "EARLY", "LATE", "OFF")
 */
const char* rhythm_trainer_eval_name(rhythm_eval_t eval);

#ifdef __cplusplus
}
#endif
