/**
 * @file assist_hold.h
 * @brief Assist Hold - Automatically holds notes for people with motor disabilities
 * 
 * Designed for people who have difficulty maintaining pressure on keys.
 * Automatically sustains notes for a configurable duration or until
 * another note is played.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ASSIST_HOLD_MAX_TRACKS 4
#define ASSIST_HOLD_MAX_NOTES 16

/**
 * @brief Hold modes
 */
typedef enum {
    HOLD_MODE_DISABLED = 0,     // Normal operation
    HOLD_MODE_LATCH,            // Notes sustain until same note pressed again
    HOLD_MODE_TIMED,            // Notes sustain for fixed duration
    HOLD_MODE_NEXT_NOTE,        // Hold until next note is played
    HOLD_MODE_INFINITE,         // Hold forever until manually released
    HOLD_MODE_COUNT
} hold_mode_t;

/**
 * @brief Initialize assist hold module
 */
void assist_hold_init(void);

/**
 * @brief Set hold mode for a track
 * @param track Track index (0-3)
 * @param mode Hold mode
 */
void assist_hold_set_mode(uint8_t track, hold_mode_t mode);

/**
 * @brief Get hold mode
 * @param track Track index (0-3)
 * @return Current hold mode
 */
hold_mode_t assist_hold_get_mode(uint8_t track);

/**
 * @brief Set hold duration (for timed mode)
 * @param track Track index (0-3)
 * @param ms Duration in milliseconds (100-10000ms)
 */
void assist_hold_set_duration_ms(uint8_t track, uint16_t ms);

/**
 * @brief Get hold duration
 * @param track Track index (0-3)
 * @return Duration in milliseconds
 */
uint16_t assist_hold_get_duration_ms(uint8_t track);

/**
 * @brief Set minimum note velocity to activate hold
 * @param track Track index (0-3)
 * @param threshold Minimum velocity (1-127, default 1)
 */
void assist_hold_set_velocity_threshold(uint8_t track, uint8_t threshold);

/**
 * @brief Get velocity threshold
 * @param track Track index (0-3)
 * @return Velocity threshold
 */
uint8_t assist_hold_get_velocity_threshold(uint8_t track);

/**
 * @brief Enable/disable mono mode (only one note held at a time)
 * @param track Track index (0-3)
 * @param enabled 1 for mono, 0 for poly
 */
void assist_hold_set_mono_mode(uint8_t track, uint8_t enabled);

/**
 * @brief Check if mono mode is enabled
 * @param track Track index (0-3)
 * @return 1 if mono, 0 if poly
 */
uint8_t assist_hold_is_mono_mode(uint8_t track);

/**
 * @brief Process incoming MIDI note
 * @param track Track index (0-3)
 * @param note MIDI note number
 * @param velocity Note velocity (>0 = note on, 0 = note off)
 * @param channel MIDI channel
 * @param timestamp_ms Current time in milliseconds
 */
void assist_hold_process_note(uint8_t track, uint8_t note, uint8_t velocity, 
                              uint8_t channel, uint32_t timestamp_ms);

/**
 * @brief Called every 1ms to process timed releases
 */
void assist_hold_tick_1ms(void);

/**
 * @brief Release all held notes on a track
 * @param track Track index (0-3)
 */
void assist_hold_release_all(uint8_t track);

/**
 * @brief Get number of currently held notes
 * @param track Track index (0-3)
 * @return Number of held notes
 */
uint8_t assist_hold_get_held_count(uint8_t track);

/**
 * @brief Get mode name
 * @param mode Hold mode
 * @return Mode name string
 */
const char* assist_hold_get_mode_name(hold_mode_t mode);

/**
 * @brief Callback for outputting note on/off
 * @param track Track index
 * @param note MIDI note
 * @param velocity Velocity (0 = note off)
 * @param channel MIDI channel
 */
typedef void (*assist_hold_output_cb_t)(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel);

/**
 * @brief Set output callback
 * @param callback Callback function
 */
void assist_hold_set_output_callback(assist_hold_output_cb_t callback);

#ifdef __cplusplus
}
#endif
