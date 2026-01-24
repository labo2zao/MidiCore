/**
 * @file note_repeat.h
 * @brief Note repeat/ratchet/stutter effect (MPC-style)
 * 
 * Generates fast repeated notes at configurable rates, perfect for
 * creating drum rolls, stutters, and rhythmic effects.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NOTE_REPEAT_MAX_TRACKS 4

/**
 * @brief Repeat rate divisions
 */
typedef enum {
    REPEAT_RATE_1_4 = 0,    // 1/4 notes
    REPEAT_RATE_1_8,        // 1/8 notes
    REPEAT_RATE_1_16,       // 1/16 notes
    REPEAT_RATE_1_32,       // 1/32 notes
    REPEAT_RATE_1_64,       // 1/64 notes
    REPEAT_RATE_1_8T,       // 1/8 triplets
    REPEAT_RATE_1_16T,      // 1/16 triplets
    REPEAT_RATE_1_32T,      // 1/32 triplets
    REPEAT_RATE_COUNT
} note_repeat_rate_t;

/**
 * @brief Initialize note repeat module
 * @param tempo Initial tempo in BPM
 */
void note_repeat_init(uint16_t tempo);

/**
 * @brief Update tempo
 * @param tempo Tempo in BPM (20-300)
 */
void note_repeat_set_tempo(uint16_t tempo);

/**
 * @brief Called every 1ms to generate repeats
 */
void note_repeat_tick_1ms(void);

/**
 * @brief Enable/disable note repeat for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void note_repeat_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if note repeat is enabled for a track
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t note_repeat_is_enabled(uint8_t track);

/**
 * @brief Set repeat rate
 * @param track Track index (0-3)
 * @param rate Repeat rate division
 */
void note_repeat_set_rate(uint8_t track, note_repeat_rate_t rate);

/**
 * @brief Get repeat rate
 * @param track Track index (0-3)
 * @return Current repeat rate
 */
note_repeat_rate_t note_repeat_get_rate(uint8_t track);

/**
 * @brief Set gate length (0-100%)
 * @param track Track index (0-3)
 * @param gate Gate length percentage (10-95, controls note duration)
 */
void note_repeat_set_gate(uint8_t track, uint8_t gate);

/**
 * @brief Get gate length
 * @param track Track index (0-3)
 * @return Gate length percentage
 */
uint8_t note_repeat_get_gate(uint8_t track);

/**
 * @brief Set velocity decay per repeat
 * @param track Track index (0-3)
 * @param decay Velocity decay percentage (0-50, 0=constant velocity)
 */
void note_repeat_set_velocity_decay(uint8_t track, uint8_t decay);

/**
 * @brief Get velocity decay
 * @param track Track index (0-3)
 * @return Velocity decay percentage
 */
uint8_t note_repeat_get_velocity_decay(uint8_t track);

/**
 * @brief Set accent pattern (bit mask for which repeats are accented)
 * @param track Track index (0-3)
 * @param pattern 8-bit pattern (bit 0 = first repeat, etc.)
 */
void note_repeat_set_accent_pattern(uint8_t track, uint8_t pattern);

/**
 * @brief Get accent pattern
 * @param track Track index (0-3)
 * @return Accent pattern
 */
uint8_t note_repeat_get_accent_pattern(uint8_t track);

/**
 * @brief Trigger note repeat (call when note is pressed)
 * @param track Track index (0-3)
 * @param note MIDI note number
 * @param velocity Note velocity
 * @param channel MIDI channel
 */
void note_repeat_trigger(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel);

/**
 * @brief Stop note repeat (call when note is released)
 * @param track Track index (0-3)
 * @param note MIDI note number
 * @param channel MIDI channel
 */
void note_repeat_stop(uint8_t track, uint8_t note, uint8_t channel);

/**
 * @brief Stop all repeats on a track
 * @param track Track index (0-3)
 */
void note_repeat_stop_all(uint8_t track);

/**
 * @brief Get rate name
 * @param rate Rate type
 * @return Rate name string
 */
const char* note_repeat_get_rate_name(note_repeat_rate_t rate);

/**
 * @brief Callback for outputting repeated notes (set by user)
 * @param track Track index
 * @param note MIDI note
 * @param velocity Velocity
 * @param channel MIDI channel
 * @param is_note_on 1 for note on, 0 for note off
 */
typedef void (*note_repeat_output_cb_t)(uint8_t track, uint8_t note, uint8_t velocity,
                                        uint8_t channel, uint8_t is_note_on);

/**
 * @brief Set output callback for repeated notes
 * @param callback Callback function
 */
void note_repeat_set_output_callback(note_repeat_output_cb_t callback);

#ifdef __cplusplus
}
#endif
