/**
 * @file velocity_compressor.h
 * @brief MIDI Velocity Compressor/Limiter for dynamic range control
 * 
 * Compresses MIDI velocity dynamics by reducing the difference between soft and loud notes.
 * Useful for taming overly dynamic performances, protecting sound modules from hot signals,
 * or creating more consistent velocity levels.
 * 
 * Features:
 * - Configurable threshold (where compression starts)
 * - Multiple compression ratios (2:1, 4:1, 8:1, ∞:1 for limiting)
 * - Makeup gain to compensate for reduced peaks
 * - Soft/hard knee compression curves
 * - Min/max velocity caps
 * - Per-track configuration (4 tracks)
 * - Bypass option
 * 
 * Compression Basics:
 * - Threshold: Velocity level above which compression is applied
 * - Ratio: Amount of compression (e.g., 4:1 = for every 4dB over threshold, output 1dB)
 * - Makeup Gain: Adds gain after compression to restore overall level
 * - Knee: How gradually compression engages at threshold (soft = gradual, hard = immediate)
 * 
 * Example Usage:
 * @code
 *   velocity_compressor_init();
 *   velocity_compressor_set_enabled(0, 1);
 *   velocity_compressor_set_threshold(0, 64);
 *   velocity_compressor_set_ratio(0, COMP_RATIO_4_1);
 *   velocity_compressor_set_makeup_gain(0, 10);
 *   
 *   uint8_t compressed_vel = velocity_compressor_process(0, input_velocity);
 * @endcode
 * 
 * @note This module operates on MIDI velocity values (1-127), not audio signals.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define VELOCITY_COMP_MAX_TRACKS 4

/**
 * @brief Compression ratio presets
 */
typedef enum {
    COMP_RATIO_1_1 = 0,     // No compression (1:1)
    COMP_RATIO_2_1,         // Gentle compression (2:1)
    COMP_RATIO_3_1,         // Mild compression (3:1)
    COMP_RATIO_4_1,         // Medium compression (4:1)
    COMP_RATIO_6_1,         // Strong compression (6:1)
    COMP_RATIO_8_1,         // Heavy compression (8:1)
    COMP_RATIO_10_1,        // Very heavy compression (10:1)
    COMP_RATIO_INF,         // Limiter (∞:1 - hard limiting at threshold)
    COMP_RATIO_COUNT
} velocity_comp_ratio_t;

/**
 * @brief Compression knee type
 */
typedef enum {
    COMP_KNEE_HARD = 0,     // Hard knee (immediate compression at threshold)
    COMP_KNEE_SOFT,         // Soft knee (gradual compression around threshold)
    COMP_KNEE_COUNT
} velocity_comp_knee_t;

/**
 * @brief Initialize velocity compressor module
 * 
 * Initializes all tracks with default settings:
 * - Compression disabled
 * - Threshold: 80
 * - Ratio: 4:1
 * - Makeup gain: 0
 * - Hard knee
 * - Min velocity: 1
 * - Max velocity: 127
 */
void velocity_compressor_init(void);

/**
 * @brief Enable/disable velocity compression for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable (bypass)
 */
void velocity_compressor_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if velocity compression is enabled for a track
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t velocity_compressor_is_enabled(uint8_t track);

/**
 * @brief Set compression threshold
 * @param track Track index (0-3)
 * @param threshold Velocity threshold (1-127, typically 60-100)
 * 
 * @note Velocities below threshold pass through unchanged.
 *       Velocities above threshold are compressed according to ratio.
 */
void velocity_compressor_set_threshold(uint8_t track, uint8_t threshold);

/**
 * @brief Get compression threshold
 * @param track Track index (0-3)
 * @return Current threshold (1-127)
 */
uint8_t velocity_compressor_get_threshold(uint8_t track);

/**
 * @brief Set compression ratio
 * @param track Track index (0-3)
 * @param ratio Compression ratio preset
 */
void velocity_compressor_set_ratio(uint8_t track, velocity_comp_ratio_t ratio);

/**
 * @brief Get compression ratio
 * @param track Track index (0-3)
 * @return Current compression ratio
 */
velocity_comp_ratio_t velocity_compressor_get_ratio(uint8_t track);

/**
 * @brief Set makeup gain (post-compression boost)
 * @param track Track index (0-3)
 * @param gain Makeup gain in velocity units (-20 to +40)
 * 
 * @note Positive values boost the signal after compression.
 *       Use to compensate for level loss from compression.
 *       Typical values: +5 to +15 for 4:1 compression.
 */
void velocity_compressor_set_makeup_gain(uint8_t track, int8_t gain);

/**
 * @brief Get makeup gain
 * @param track Track index (0-3)
 * @return Current makeup gain (-20 to +40)
 */
int8_t velocity_compressor_get_makeup_gain(uint8_t track);

/**
 * @brief Set compression knee type
 * @param track Track index (0-3)
 * @param knee Knee type (hard or soft)
 * 
 * @note Hard knee: Compression engages immediately at threshold
 *       Soft knee: Compression engages gradually around threshold
 */
void velocity_compressor_set_knee(uint8_t track, velocity_comp_knee_t knee);

/**
 * @brief Get compression knee type
 * @param track Track index (0-3)
 * @return Current knee type
 */
velocity_comp_knee_t velocity_compressor_get_knee(uint8_t track);

/**
 * @brief Set minimum velocity cap
 * @param track Track index (0-3)
 * @param min_vel Minimum output velocity (1-127)
 * 
 * @note Output velocities below this value are clamped to min_vel.
 *       Use to ensure notes are never too soft.
 */
void velocity_compressor_set_min_velocity(uint8_t track, uint8_t min_vel);

/**
 * @brief Get minimum velocity cap
 * @param track Track index (0-3)
 * @return Current minimum velocity
 */
uint8_t velocity_compressor_get_min_velocity(uint8_t track);

/**
 * @brief Set maximum velocity cap
 * @param track Track index (0-3)
 * @param max_vel Maximum output velocity (1-127)
 * 
 * @note Output velocities above this value are clamped to max_vel.
 *       Use to prevent excessively loud notes.
 */
void velocity_compressor_set_max_velocity(uint8_t track, uint8_t max_vel);

/**
 * @brief Get maximum velocity cap
 * @param track Track index (0-3)
 * @return Current maximum velocity
 */
uint8_t velocity_compressor_get_max_velocity(uint8_t track);

/**
 * @brief Process a velocity value through the compressor
 * @param track Track index (0-3)
 * @param velocity Input velocity (1-127)
 * @return Compressed velocity (1-127)
 * 
 * @note If compression is disabled, returns input velocity unchanged.
 *       This is the main function to call for each MIDI note.
 */
uint8_t velocity_compressor_process(uint8_t track, uint8_t velocity);

/**
 * @brief Reset compression settings to defaults for a track
 * @param track Track index (0-3)
 */
void velocity_compressor_reset_track(uint8_t track);

/**
 * @brief Reset all tracks to default settings
 */
void velocity_compressor_reset_all(void);

/**
 * @brief Get compression ratio name
 * @param ratio Compression ratio
 * @return Ratio name string (e.g., "4:1", "∞:1")
 */
const char* velocity_compressor_get_ratio_name(velocity_comp_ratio_t ratio);

/**
 * @brief Get knee type name
 * @param knee Knee type
 * @return Knee name string ("Hard", "Soft")
 */
const char* velocity_compressor_get_knee_name(velocity_comp_knee_t knee);

/**
 * @brief Calculate gain reduction for a given input velocity
 * @param track Track index (0-3)
 * @param velocity Input velocity (1-127)
 * @return Gain reduction in velocity units (0 = no reduction)
 * 
 * @note Useful for metering/visualization. Shows how much compression is applied.
 */
uint8_t velocity_compressor_get_gain_reduction(uint8_t track, uint8_t velocity);

#ifdef __cplusplus
}
#endif
