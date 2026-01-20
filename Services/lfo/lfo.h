#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file lfo.h
 * @brief Low Frequency Oscillator module for MIDI parameter modulation
 * 
 * Provides cyclic modulation for velocity, timing, and pitch with multiple waveforms.
 * Can be synced to BPM or run freely for "dream" effects.
 */

#define LFO_MAX_TRACKS 4

typedef enum {
    LFO_WAVEFORM_SINE = 0,      ///< Smooth sine wave
    LFO_WAVEFORM_TRIANGLE,      ///< Linear triangle wave
    LFO_WAVEFORM_SAW,           ///< Ascending sawtooth
    LFO_WAVEFORM_SQUARE,        ///< Square wave (50% duty cycle)
    LFO_WAVEFORM_RANDOM,        ///< Smooth random (interpolated)
    LFO_WAVEFORM_SAMPLE_HOLD,  ///< Stepped random (sample & hold)
    LFO_WAVEFORM_COUNT
} lfo_waveform_t;

typedef enum {
    LFO_TARGET_VELOCITY = 0,    ///< Modulate note velocity
    LFO_TARGET_TIMING,          ///< Modulate note timing (ticks)
    LFO_TARGET_PITCH,           ///< Modulate note pitch (semitones)
    LFO_TARGET_COUNT
} lfo_target_t;

/**
 * @brief Initialize LFO module
 */
void lfo_init(void);

/**
 * @brief Called every 1ms to advance LFO phase
 */
void lfo_tick_1ms(void);

/**
 * @brief Enable/disable LFO for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void lfo_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if LFO is enabled for a track
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t lfo_is_enabled(uint8_t track);

/**
 * @brief Set LFO waveform
 * @param track Track index (0-3)
 * @param waveform Waveform type
 */
void lfo_set_waveform(uint8_t track, lfo_waveform_t waveform);

/**
 * @brief Get current LFO waveform
 * @param track Track index (0-3)
 * @return Current waveform type
 */
lfo_waveform_t lfo_get_waveform(uint8_t track);

/**
 * @brief Set LFO rate in Hz (0.01 - 10.0 Hz, stored as 0.01Hz units)
 * @param track Track index (0-3)
 * @param rate_hundredths Rate in 0.01Hz units (1 = 0.01Hz, 1000 = 10Hz)
 */
void lfo_set_rate(uint8_t track, uint16_t rate_hundredths);

/**
 * @brief Get current LFO rate
 * @param track Track index (0-3)
 * @return Rate in 0.01Hz units
 */
uint16_t lfo_get_rate(uint8_t track);

/**
 * @brief Set LFO depth (0-100%)
 * @param track Track index (0-3)
 * @param depth Modulation depth percentage (0-100)
 */
void lfo_set_depth(uint8_t track, uint8_t depth);

/**
 * @brief Get current LFO depth
 * @param track Track index (0-3)
 * @return Depth percentage (0-100)
 */
uint8_t lfo_get_depth(uint8_t track);

/**
 * @brief Set LFO target parameter
 * @param track Track index (0-3)
 * @param target Target parameter to modulate
 */
void lfo_set_target(uint8_t track, lfo_target_t target);

/**
 * @brief Get current LFO target
 * @param track Track index (0-3)
 * @return Target parameter
 */
lfo_target_t lfo_get_target(uint8_t track);

/**
 * @brief Enable/disable BPM sync
 * @param track Track index (0-3)
 * @param bpm_sync 1 for BPM sync, 0 for free-running
 */
void lfo_set_bpm_sync(uint8_t track, uint8_t bpm_sync);

/**
 * @brief Check if BPM sync is enabled
 * @param track Track index (0-3)
 * @return 1 if BPM synced, 0 if free-running
 */
uint8_t lfo_is_bpm_synced(uint8_t track);

/**
 * @brief Set BPM sync divisor (1/4, 1/2, 1, 2, 4, 8 bars)
 * @param track Track index (0-3)
 * @param divisor Bars divisor (1, 2, 4, 8, 16, 32)
 */
void lfo_set_bpm_divisor(uint8_t track, uint8_t divisor);

/**
 * @brief Get current BPM divisor
 * @param track Track index (0-3)
 * @return Bars divisor
 */
uint8_t lfo_get_bpm_divisor(uint8_t track);

/**
 * @brief Reset LFO phase to zero
 * @param track Track index (0-3)
 */
void lfo_reset_phase(uint8_t track);

/**
 * @brief Get current LFO value for velocity modulation
 * @param track Track index (0-3)
 * @param base_velocity Base velocity value (0-127)
 * @return Modulated velocity value (0-127)
 */
uint8_t lfo_get_velocity_value(uint8_t track, uint8_t base_velocity);

/**
 * @brief Get current LFO value for timing modulation
 * @param track Track index (0-3)
 * @return Timing offset in ticks (-12 to +12)
 */
int8_t lfo_get_timing_value(uint8_t track);

/**
 * @brief Get current LFO value for pitch modulation
 * @param track Track index (0-3)
 * @param base_note Base MIDI note (0-127)
 * @return Modulated note value (0-127)
 */
uint8_t lfo_get_pitch_value(uint8_t track, uint8_t base_note);

/**
 * @brief Set current tempo (for BPM sync calculations)
 * @param bpm Beats per minute (20-300)
 */
void lfo_set_tempo(uint16_t bpm);

#ifdef __cplusplus
}
#endif
