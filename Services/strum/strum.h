/**
 * @file strum.h
 * @brief MIDI Strum Effect Module
 * 
 * Staggers chord notes to simulate guitar/harp strumming.
 * Notes are delayed progressively based on strum direction and timing.
 * 
 * Features:
 * - Per-track configuration (up to 4 tracks)
 * - Configurable strum duration (0-200ms)
 * - Multiple strum directions (Up, Down, Up-Down, Random)
 * - Optional velocity ramping across strum
 * - Compatible with MidiCore architecture
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define STRUM_MAX_TRACKS 4
#define STRUM_MAX_TIME_MS 200
#define STRUM_MAX_CHORD_NOTES 8

/**
 * @brief Strum direction modes
 */
typedef enum {
    STRUM_DIR_UP = 0,        // Low to high notes (guitar upstroke)
    STRUM_DIR_DOWN,          // High to low notes (guitar downstroke)
    STRUM_DIR_UP_DOWN,       // Alternates between up and down
    STRUM_DIR_RANDOM,        // Random note order
    STRUM_DIR_COUNT
} strum_direction_t;

/**
 * @brief Velocity ramp modes
 */
typedef enum {
    STRUM_RAMP_NONE = 0,     // No velocity change
    STRUM_RAMP_INCREASE,     // Velocity increases across strum
    STRUM_RAMP_DECREASE,     // Velocity decreases across strum
    STRUM_RAMP_COUNT
} strum_ramp_t;

/**
 * @brief Initialize strum module
 */
void strum_init(void);

/**
 * @brief Enable/disable strum effect for a track
 * @param track Track number (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void strum_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Get enabled state for a track
 * @param track Track number (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t strum_is_enabled(uint8_t track);

/**
 * @brief Set strum duration (total time to strum all notes)
 * @param track Track number (0-3)
 * @param time_ms Duration in milliseconds (0-200ms)
 */
void strum_set_time(uint8_t track, uint8_t time_ms);

/**
 * @brief Get strum duration for a track
 * @param track Track number (0-3)
 * @return Duration in milliseconds
 */
uint8_t strum_get_time(uint8_t track);

/**
 * @brief Set strum direction
 * @param track Track number (0-3)
 * @param direction Strum direction mode
 */
void strum_set_direction(uint8_t track, strum_direction_t direction);

/**
 * @brief Get strum direction for a track
 * @param track Track number (0-3)
 * @return Strum direction mode
 */
strum_direction_t strum_get_direction(uint8_t track);

/**
 * @brief Set velocity ramping mode
 * @param track Track number (0-3)
 * @param ramp Velocity ramp mode
 */
void strum_set_velocity_ramp(uint8_t track, strum_ramp_t ramp);

/**
 * @brief Get velocity ramp mode for a track
 * @param track Track number (0-3)
 * @return Velocity ramp mode
 */
strum_ramp_t strum_get_velocity_ramp(uint8_t track);

/**
 * @brief Set velocity ramp amount
 * @param track Track number (0-3)
 * @param amount Ramp amount (0-100, percentage of velocity change)
 */
void strum_set_ramp_amount(uint8_t track, uint8_t amount);

/**
 * @brief Get velocity ramp amount for a track
 * @param track Track number (0-3)
 * @return Ramp amount (0-100)
 */
uint8_t strum_get_ramp_amount(uint8_t track);

/**
 * @brief Process note-on event through strum effect
 * 
 * Call this for each incoming MIDI note-on. The function determines
 * the delay offset for the note based on strum configuration.
 * 
 * @param track Track number (0-3)
 * @param note MIDI note number (0-127)
 * @param velocity Original velocity (1-127)
 * @param chord_notes Array of all notes in the chord (sorted low to high)
 * @param chord_size Number of notes in the chord (1-8)
 * @param delay_ms Output: calculated delay in milliseconds
 * @param new_velocity Output: modified velocity after ramping
 */
void strum_process_note(uint8_t track, uint8_t note, uint8_t velocity,
                        const uint8_t* chord_notes, uint8_t chord_size,
                        uint8_t* delay_ms, uint8_t* new_velocity);

/**
 * @brief Reset strum state for a track
 * 
 * Useful when switching patches or changing strum parameters
 * 
 * @param track Track number (0-3)
 */
void strum_reset(uint8_t track);

/**
 * @brief Get human-readable direction name
 * @param direction Strum direction mode
 * @return String name of direction
 */
const char* strum_get_direction_name(strum_direction_t direction);

/**
 * @brief Get human-readable ramp name
 * @param ramp Velocity ramp mode
 * @return String name of ramp mode
 */
const char* strum_get_ramp_name(strum_ramp_t ramp);

#ifdef __cplusplus
}
#endif
