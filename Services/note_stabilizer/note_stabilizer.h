/**
 * @file note_stabilizer.h
 * @brief Note Stabilizer - Filters unintended notes for people with tremors
 * 
 * Designed for people with tremors, spasms, or unintended movements.
 * Filters out rapid repeated notes, very short notes, and accidental
 * neighboring key presses.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NOTE_STAB_MAX_TRACKS 4

/**
 * @brief Initialize note stabilizer module
 */
void note_stab_init(void);

/**
 * @brief Enable/disable stabilizer for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void note_stab_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if stabilizer is enabled
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t note_stab_is_enabled(uint8_t track);

/**
 * @brief Set minimum note duration
 * @param track Track index (0-3)
 * @param ms Minimum duration in milliseconds (10-500ms)
 */
void note_stab_set_min_duration_ms(uint8_t track, uint16_t ms);

/**
 * @brief Get minimum note duration
 * @param track Track index (0-3)
 * @return Minimum duration in milliseconds
 */
uint16_t note_stab_get_min_duration_ms(uint8_t track);

/**
 * @brief Set retrigger delay (prevents rapid repeated notes)
 * @param track Track index (0-3)
 * @param ms Delay in milliseconds (10-1000ms)
 */
void note_stab_set_retrigger_delay_ms(uint8_t track, uint16_t ms);

/**
 * @brief Get retrigger delay
 * @param track Track index (0-3)
 * @return Retrigger delay in milliseconds
 */
uint16_t note_stab_get_retrigger_delay_ms(uint8_t track);

/**
 * @brief Set neighboring key filter range
 * @param track Track index (0-3)
 * @param semitones Range in semitones (0-12, 0=disabled)
 */
void note_stab_set_neighbor_range(uint8_t track, uint8_t semitones);

/**
 * @brief Get neighboring key filter range
 * @param track Track index (0-3)
 * @return Range in semitones
 */
uint8_t note_stab_get_neighbor_range(uint8_t track);

/**
 * @brief Set velocity stability threshold
 * @param track Track index (0-3)
 * @param threshold Minimum velocity change to register (0-127)
 */
void note_stab_set_velocity_threshold(uint8_t track, uint8_t threshold);

/**
 * @brief Get velocity threshold
 * @param track Track index (0-3)
 * @return Velocity threshold
 */
uint8_t note_stab_get_velocity_threshold(uint8_t track);

/**
 * @brief Enable/disable note averaging (smooths velocity)
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void note_stab_set_averaging_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if averaging is enabled
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t note_stab_is_averaging_enabled(uint8_t track);

/**
 * @brief Process incoming MIDI note
 * @param track Track index (0-3)
 * @param note MIDI note number
 * @param velocity Note velocity (>0 = note on, 0 = note off)
 * @param channel MIDI channel
 * @param timestamp_ms Current time in milliseconds
 */
void note_stab_process_note(uint8_t track, uint8_t note, uint8_t velocity,
                           uint8_t channel, uint32_t timestamp_ms);

/**
 * @brief Called every 1ms to process delayed notes
 */
void note_stab_tick_1ms(void);

/**
 * @brief Get statistics for monitoring
 * @param track Track index (0-3)
 * @param filtered_count Output: number of filtered notes
 * @param passed_count Output: number of passed notes
 */
void note_stab_get_stats(uint8_t track, uint32_t* filtered_count, uint32_t* passed_count);

/**
 * @brief Reset statistics
 * @param track Track index (0-3)
 */
void note_stab_reset_stats(uint8_t track);

/**
 * @brief Callback for outputting stabilized notes
 * @param track Track index
 * @param note MIDI note
 * @param velocity Velocity (0 = note off)
 * @param channel MIDI channel
 */
typedef void (*note_stab_output_cb_t)(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel);

/**
 * @brief Set output callback
 * @param callback Callback function
 */
void note_stab_set_output_callback(note_stab_output_cb_t callback);

#ifdef __cplusplus
}
#endif
