/**
 * @file bellows_expression.h
 * @brief Bellows Expression - Advanced bellows pressure to MIDI expression
 * 
 * Designed specifically for accordion players. Maps bellows pressure to
 * multiple MIDI parameters with sophisticated curves, bidirectional support,
 * and musette-aware dynamics.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BELLOWS_MAX_TRACKS 4

/**
 * @brief Bellows direction
 */
typedef enum {
    BELLOWS_DIR_NEUTRAL = 0,
    BELLOWS_DIR_PUSH,           // Closing bellows
    BELLOWS_DIR_PULL,           // Opening bellows
} bellows_direction_t;

/**
 * @brief Expression curves
 */
typedef enum {
    BELLOWS_CURVE_LINEAR = 0,
    BELLOWS_CURVE_EXPONENTIAL,  // More sensitive at low pressure
    BELLOWS_CURVE_LOGARITHMIC,  // More sensitive at high pressure
    BELLOWS_CURVE_S_CURVE,      // Smooth at extremes
    BELLOWS_CURVE_COUNT
} bellows_curve_t;

/**
 * @brief Initialize bellows expression module
 */
void bellows_init(void);

/**
 * @brief Set expression curve
 * @param track Track index (0-3)
 * @param curve Curve type
 */
void bellows_set_curve(uint8_t track, bellows_curve_t curve);

/**
 * @brief Get expression curve
 * @param track Track index (0-3)
 * @return Current curve type
 */
bellows_curve_t bellows_get_curve(uint8_t track);

/**
 * @brief Set pressure range (calibration)
 * @param track Track index (0-3)
 * @param min_pa Minimum pressure in Pascals
 * @param max_pa Maximum pressure in Pascals
 */
void bellows_set_pressure_range(uint8_t track, int32_t min_pa, int32_t max_pa);

/**
 * @brief Get pressure range
 * @param track Track index (0-3)
 * @param min_pa Output: minimum pressure
 * @param max_pa Output: maximum pressure
 */
void bellows_get_pressure_range(uint8_t track, int32_t* min_pa, int32_t* max_pa);

/**
 * @brief Enable/disable bidirectional mode
 * @param track Track index (0-3)
 * @param enabled 1 for bidirectional (push/pull different), 0 for unidirectional
 */
void bellows_set_bidirectional(uint8_t track, uint8_t enabled);

/**
 * @brief Check if bidirectional mode is enabled
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t bellows_is_bidirectional(uint8_t track);

/**
 * @brief Set expression CC number for velocity control
 * @param track Track index (0-3)
 * @param cc_num CC number (typically 11 for expression)
 */
void bellows_set_expression_cc(uint8_t track, uint8_t cc_num);

/**
 * @brief Get expression CC number
 * @param track Track index (0-3)
 * @return CC number
 */
uint8_t bellows_get_expression_cc(uint8_t track);

/**
 * @brief Set breath CC number
 * @param track Track index (0-3)
 * @param cc_num CC number (typically 2 for breath)
 */
void bellows_set_breath_cc(uint8_t track, uint8_t cc_num);

/**
 * @brief Get breath CC number
 * @param track Track index (0-3)
 * @return CC number
 */
uint8_t bellows_get_breath_cc(uint8_t track);

/**
 * @brief Set smoothing amount (anti-jitter)
 * @param track Track index (0-3)
 * @param amount Smoothing 0-100% (0=none, 100=heavy)
 */
void bellows_set_smoothing(uint8_t track, uint8_t amount);

/**
 * @brief Get smoothing amount
 * @param track Track index (0-3)
 * @return Smoothing percentage
 */
uint8_t bellows_get_smoothing(uint8_t track);

/**
 * @brief Set attack/release time for bellows changes
 * @param track Track index (0-3)
 * @param attack_ms Attack time in milliseconds
 * @param release_ms Release time in milliseconds
 */
void bellows_set_attack_release(uint8_t track, uint16_t attack_ms, uint16_t release_ms);

/**
 * @brief Process bellows pressure reading
 * @param track Track index (0-3)
 * @param pressure_pa Pressure in Pascals (positive=push, negative=pull)
 * @param channel MIDI channel
 */
void bellows_process_pressure(uint8_t track, int32_t pressure_pa, uint8_t channel);

/**
 * @brief Get current bellows direction
 * @param track Track index (0-3)
 * @return Current direction
 */
bellows_direction_t bellows_get_direction(uint8_t track);

/**
 * @brief Get current expression value
 * @param track Track index (0-3)
 * @return Expression value (0-127)
 */
uint8_t bellows_get_expression_value(uint8_t track);

/**
 * @brief Called every 1ms for smoothing
 */
void bellows_tick_1ms(void);

/**
 * @brief Callback for outputting CC messages
 * @param track Track index
 * @param cc_num CC number
 * @param value CC value
 * @param channel MIDI channel
 */
typedef void (*bellows_cc_output_cb_t)(uint8_t track, uint8_t cc_num, uint8_t value, uint8_t channel);

/**
 * @brief Set output callback
 * @param callback Callback function
 */
void bellows_set_output_callback(bellows_cc_output_cb_t callback);

#ifdef __cplusplus
}
#endif
