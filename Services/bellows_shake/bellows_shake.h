/**
 * @file bellows_shake.h
 * @brief Bellows Shake - Detects and generates tremolo from bellows shaking
 * 
 * Detects rapid bellows movements (shake/vibrato) and converts to
 * musical tremolo effects. Common technique in accordion playing.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BELLOWS_SHAKE_MAX_TRACKS 4

/**
 * @brief Tremolo target parameters
 */
typedef enum {
    SHAKE_TARGET_VOLUME = 0,    // Modulate volume/expression
    SHAKE_TARGET_PITCH,         // Modulate pitch (vibrato)
    SHAKE_TARGET_FILTER,        // Modulate filter cutoff
    SHAKE_TARGET_BOTH,          // Volume + pitch
    SHAKE_TARGET_COUNT
} shake_target_t;

/**
 * @brief Initialize bellows shake module
 */
void bellows_shake_init(void);

/**
 * @brief Enable/disable shake detection
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void bellows_shake_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if shake detection is enabled
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t bellows_shake_is_enabled(uint8_t track);

/**
 * @brief Set shake sensitivity (detection threshold)
 * @param track Track index (0-3)
 * @param sensitivity 0-100% (0=least sensitive, 100=most sensitive)
 */
void bellows_shake_set_sensitivity(uint8_t track, uint8_t sensitivity);

/**
 * @brief Get shake sensitivity
 * @param track Track index (0-3)
 * @return Sensitivity percentage
 */
uint8_t bellows_shake_get_sensitivity(uint8_t track);

/**
 * @brief Set tremolo depth
 * @param track Track index (0-3)
 * @param depth Depth 0-100% (amount of modulation)
 */
void bellows_shake_set_depth(uint8_t track, uint8_t depth);

/**
 * @brief Get tremolo depth
 * @param track Track index (0-3)
 * @return Depth percentage
 */
uint8_t bellows_shake_get_depth(uint8_t track);

/**
 * @brief Set tremolo target
 * @param track Track index (0-3)
 * @param target What to modulate
 */
void bellows_shake_set_target(uint8_t track, shake_target_t target);

/**
 * @brief Get tremolo target
 * @param track Track index (0-3)
 * @return Current target
 */
shake_target_t bellows_shake_get_target(uint8_t track);

/**
 * @brief Set frequency range for detection
 * @param track Track index (0-3)
 * @param min_hz Minimum shake frequency (2-10 Hz)
 * @param max_hz Maximum shake frequency (5-20 Hz)
 */
void bellows_shake_set_freq_range(uint8_t track, uint8_t min_hz, uint8_t max_hz);

/**
 * @brief Get frequency range
 * @param track Track index (0-3)
 * @param min_hz Output: minimum frequency
 * @param max_hz Output: maximum frequency
 */
void bellows_shake_get_freq_range(uint8_t track, uint8_t* min_hz, uint8_t* max_hz);

/**
 * @brief Process bellows pressure reading for shake detection
 * @param track Track index (0-3)
 * @param pressure_pa Pressure in Pascals
 * @param timestamp_ms Current time in milliseconds
 * @param channel MIDI channel
 */
void bellows_shake_process_pressure(uint8_t track, int32_t pressure_pa, 
                                   uint32_t timestamp_ms, uint8_t channel);

/**
 * @brief Get current shake detection state
 * @param track Track index (0-3)
 * @return 1 if shake detected, 0 if not
 */
uint8_t bellows_shake_is_detected(uint8_t track);

/**
 * @brief Get detected shake frequency
 * @param track Track index (0-3)
 * @return Frequency in Hz (0 if not detected)
 */
uint8_t bellows_shake_get_frequency(uint8_t track);

/**
 * @brief Get current tremolo modulation value
 * @param track Track index (0-3)
 * @return Modulation value 0-127
 */
uint8_t bellows_shake_get_modulation(uint8_t track);

/**
 * @brief Called every 1ms for processing
 */
void bellows_shake_tick_1ms(void);

/**
 * @brief Callback for outputting tremolo CC
 * @param track Track index
 * @param cc_num CC number (11 for expression, 74 for filter, etc.)
 * @param value CC value
 * @param channel MIDI channel
 */
typedef void (*bellows_shake_cc_output_cb_t)(uint8_t track, uint8_t cc_num, 
                                             uint8_t value, uint8_t channel);

/**
 * @brief Callback for outputting pitchbend (for vibrato)
 * @param track Track index
 * @param pitchbend Pitchbend value (-8192 to +8191)
 * @param channel MIDI channel
 */
typedef void (*bellows_shake_pb_output_cb_t)(uint8_t track, int16_t pitchbend, uint8_t channel);

/**
 * @brief Set CC output callback
 * @param callback Callback function
 */
void bellows_shake_set_cc_callback(bellows_shake_cc_output_cb_t callback);

/**
 * @brief Set pitchbend output callback
 * @param callback Callback function
 */
void bellows_shake_set_pb_callback(bellows_shake_pb_output_cb_t callback);

#ifdef __cplusplus
}
#endif
