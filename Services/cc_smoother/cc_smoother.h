/**
 * @file cc_smoother.h
 * @brief MIDI CC (Control Change) smoother to eliminate zipper noise and staircase effects
 * 
 * Smooths CC messages using exponential moving average (EMA) algorithm with configurable
 * attack/release times and slew rate limiting. Supports per-track configuration for all
 * 128 CC numbers independently.
 * 
 * Features:
 * - Exponential moving average smoothing
 * - Configurable smoothing amount (0-100%)
 * - Independent attack/release times
 * - Slew rate limiting option
 * - Per-track configuration (4 tracks)
 * - All 128 CC numbers supported independently
 * 
 * @note This module prevents zipper noise in filter sweeps, volume changes, and other
 *       CC modulations by smoothing rapid CC value changes.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CC_SMOOTHER_MAX_TRACKS 4
#define CC_SMOOTHER_MAX_CC_NUMBERS 128

/**
 * @brief CC smoother mode
 */
typedef enum {
    CC_SMOOTH_MODE_OFF = 0,        // No smoothing (pass-through)
    CC_SMOOTH_MODE_LIGHT,          // Light smoothing (fast response, minimal latency)
    CC_SMOOTH_MODE_MEDIUM,         // Medium smoothing (balanced)
    CC_SMOOTH_MODE_HEAVY,          // Heavy smoothing (slow response, very smooth)
    CC_SMOOTH_MODE_CUSTOM,         // Custom settings (use configured parameters)
    CC_SMOOTH_MODE_COUNT
} cc_smoother_mode_t;

/**
 * @brief Initialize CC smoother module
 * 
 * Initializes all tracks with default settings:
 * - Smoothing disabled
 * - Medium smoothing mode
 * - Attack time: 50ms
 * - Release time: 100ms
 * - Slew rate limit: 127 (no limiting)
 */
void cc_smoother_init(void);

/**
 * @brief Enable/disable CC smoothing for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void cc_smoother_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if CC smoothing is enabled for a track
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t cc_smoother_is_enabled(uint8_t track);

/**
 * @brief Set smoothing mode for a track
 * @param track Track index (0-3)
 * @param mode Smoothing mode preset
 */
void cc_smoother_set_mode(uint8_t track, cc_smoother_mode_t mode);

/**
 * @brief Get smoothing mode for a track
 * @param track Track index (0-3)
 * @return Current smoothing mode
 */
cc_smoother_mode_t cc_smoother_get_mode(uint8_t track);

/**
 * @brief Set smoothing amount (for custom mode)
 * @param track Track index (0-3)
 * @param amount Smoothing amount (0-100, where 0=no smoothing, 100=heavy smoothing)
 */
void cc_smoother_set_amount(uint8_t track, uint8_t amount);

/**
 * @brief Get smoothing amount
 * @param track Track index (0-3)
 * @return Smoothing amount (0-100)
 */
uint8_t cc_smoother_get_amount(uint8_t track);

/**
 * @brief Set attack time (how fast CC increases)
 * @param track Track index (0-3)
 * @param attack_ms Attack time in milliseconds (1-1000ms)
 */
void cc_smoother_set_attack(uint8_t track, uint16_t attack_ms);

/**
 * @brief Get attack time
 * @param track Track index (0-3)
 * @return Attack time in milliseconds
 */
uint16_t cc_smoother_get_attack(uint8_t track);

/**
 * @brief Set release time (how fast CC decreases)
 * @param track Track index (0-3)
 * @param release_ms Release time in milliseconds (1-1000ms)
 */
void cc_smoother_set_release(uint8_t track, uint16_t release_ms);

/**
 * @brief Get release time
 * @param track Track index (0-3)
 * @return Release time in milliseconds
 */
uint16_t cc_smoother_get_release(uint8_t track);

/**
 * @brief Set slew rate limit (maximum change per update)
 * @param track Track index (0-3)
 * @param slew_limit Maximum CC value change per millisecond (1-127, 127=no limit)
 */
void cc_smoother_set_slew_limit(uint8_t track, uint8_t slew_limit);

/**
 * @brief Get slew rate limit
 * @param track Track index (0-3)
 * @return Current slew rate limit
 */
uint8_t cc_smoother_get_slew_limit(uint8_t track);

/**
 * @brief Enable/disable smoothing for a specific CC number on a track
 * @param track Track index (0-3)
 * @param cc_number CC number (0-127)
 * @param enabled 1 to enable, 0 to disable
 * 
 * @note By default, all CC numbers are enabled for smoothing when track is enabled.
 *       Use this to exclude specific CC numbers (e.g., switches, buttons) from smoothing.
 */
void cc_smoother_set_cc_enabled(uint8_t track, uint8_t cc_number, uint8_t enabled);

/**
 * @brief Check if smoothing is enabled for a specific CC number
 * @param track Track index (0-3)
 * @param cc_number CC number (0-127)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t cc_smoother_is_cc_enabled(uint8_t track, uint8_t cc_number);

/**
 * @brief Process a CC message (apply smoothing)
 * @param track Track index (0-3)
 * @param cc_number CC number (0-127)
 * @param value CC value (0-127)
 * @return Smoothed CC value (0-127)
 * 
 * @note Call this function for each incoming CC message.
 *       If smoothing is disabled, returns input value unchanged.
 */
uint8_t cc_smoother_process(uint8_t track, uint8_t cc_number, uint8_t value);

/**
 * @brief Update smoothing (call periodically, e.g., every 1ms)
 * 
 * @note This function updates all active smoothing filters.
 *       Should be called from a timer interrupt or main loop.
 */
void cc_smoother_tick_1ms(void);

/**
 * @brief Reset all smoothing state for a track
 * @param track Track index (0-3)
 * 
 * @note Resets all CC values to current targets (no smoothing transition).
 */
void cc_smoother_reset_track(uint8_t track);

/**
 * @brief Reset smoothing state for a specific CC number
 * @param track Track index (0-3)
 * @param cc_number CC number (0-127)
 * 
 * @note Resets CC value to current target (no smoothing transition).
 */
void cc_smoother_reset_cc(uint8_t track, uint8_t cc_number);

/**
 * @brief Reset all smoothing state for all tracks
 */
void cc_smoother_reset_all(void);

/**
 * @brief Get current smoothed value for a CC (without processing new input)
 * @param track Track index (0-3)
 * @param cc_number CC number (0-127)
 * @return Current smoothed CC value (0-127)
 */
uint8_t cc_smoother_get_current_value(uint8_t track, uint8_t cc_number);

/**
 * @brief Get smoothing mode name
 * @param mode Smoothing mode
 * @return Mode name string
 */
const char* cc_smoother_get_mode_name(cc_smoother_mode_t mode);

/**
 * @brief Callback for outputting smoothed CC messages (set by user)
 * @param track Track index
 * @param cc_number CC number
 * @param value Smoothed CC value
 * @param channel MIDI channel
 */
typedef void (*cc_smoother_output_cb_t)(uint8_t track, uint8_t cc_number, 
                                        uint8_t value, uint8_t channel);

/**
 * @brief Set output callback for smoothed CC messages
 * @param callback Callback function
 * 
 * @note When set, smoothed CC values will be sent via callback when they change.
 *       If not set, use cc_smoother_get_current_value() to poll values.
 */
void cc_smoother_set_output_callback(cc_smoother_output_cb_t callback);

#ifdef __cplusplus
}
#endif
