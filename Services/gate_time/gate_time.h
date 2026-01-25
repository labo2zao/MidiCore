/**
 * @file gate_time.h
 * @brief Note Length/Gate Time Control - adjust note durations
 * 
 * Controls MIDI note lengths (gate time) with multiple modes including
 * percentage-based, fixed millisecond, and fixed tick-based timing.
 * Provides per-track configuration with min/max length limits.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GATE_TIME_MAX_TRACKS 4
#define GATE_TIME_MAX_NOTES_PER_TRACK 32

/**
 * @brief Gate time mode
 */
typedef enum {
    GATE_TIME_MODE_PERCENT = 0,    // Percentage of original length (10-200%)
    GATE_TIME_MODE_FIXED_MS,       // Fixed milliseconds
    GATE_TIME_MODE_FIXED_TICKS,    // Fixed MIDI ticks
    GATE_TIME_MODE_COUNT
} gate_time_mode_t;

/**
 * @brief Active note structure
 */
typedef struct {
    uint8_t note;                  // MIDI note number
    uint8_t channel;               // MIDI channel
    uint32_t note_on_time_ms;      // When note was triggered
    uint32_t note_off_time_ms;     // When note should end
    uint8_t active;                // 1 if slot is active
} gate_time_note_t;

/**
 * @brief Note event callback function type
 * @param track Track index
 * @param note MIDI note number
 * @param velocity Velocity (0 for note off)
 * @param channel MIDI channel
 */
typedef void (*gate_time_note_callback_t)(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel);

/**
 * @brief Initialize gate time module
 */
void gate_time_init(void);

/**
 * @brief Set note output callback
 * @param callback Function to call for note on/off events
 */
void gate_time_set_callback(gate_time_note_callback_t callback);

/**
 * @brief Enable/disable gate time for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void gate_time_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if gate time is enabled for a track
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t gate_time_is_enabled(uint8_t track);

/**
 * @brief Set gate time mode
 * @param track Track index (0-3)
 * @param mode Gate time mode
 */
void gate_time_set_mode(uint8_t track, gate_time_mode_t mode);

/**
 * @brief Get gate time mode
 * @param track Track index (0-3)
 * @return Current gate time mode
 */
gate_time_mode_t gate_time_get_mode(uint8_t track);

/**
 * @brief Set gate time value (meaning depends on mode)
 * @param track Track index (0-3)
 * @param value For PERCENT: 10-200 (%), FIXED_MS: milliseconds, FIXED_TICKS: ticks
 */
void gate_time_set_value(uint8_t track, uint16_t value);

/**
 * @brief Get gate time value
 * @param track Track index (0-3)
 * @return Current gate time value
 */
uint16_t gate_time_get_value(uint8_t track);

/**
 * @brief Set minimum gate time in milliseconds
 * @param track Track index (0-3)
 * @param min_ms Minimum length in milliseconds (0=no limit)
 */
void gate_time_set_min_length(uint8_t track, uint16_t min_ms);

/**
 * @brief Get minimum gate time
 * @param track Track index (0-3)
 * @return Minimum length in milliseconds
 */
uint16_t gate_time_get_min_length(uint8_t track);

/**
 * @brief Set maximum gate time in milliseconds
 * @param track Track index (0-3)
 * @param max_ms Maximum length in milliseconds (0=no limit)
 */
void gate_time_set_max_length(uint8_t track, uint16_t max_ms);

/**
 * @brief Get maximum gate time
 * @param track Track index (0-3)
 * @return Maximum length in milliseconds
 */
uint16_t gate_time_get_max_length(uint8_t track);

/**
 * @brief Process note on event
 * @param track Track index (0-3)
 * @param note MIDI note number
 * @param velocity Note velocity
 * @param channel MIDI channel
 * @param time_ms Current time in milliseconds
 * @return 1 if note was processed, 0 if buffer full
 */
uint8_t gate_time_process_note_on(uint8_t track, uint8_t note, uint8_t velocity, 
                                   uint8_t channel, uint32_t time_ms);

/**
 * @brief Process note off event (stops tracking the note)
 * @param track Track index (0-3)
 * @param note MIDI note number
 * @param channel MIDI channel
 */
void gate_time_process_note_off(uint8_t track, uint8_t note, uint8_t channel);

/**
 * @brief Tick function - call every 1ms to process note offs
 * @param time_ms Current time in milliseconds
 */
void gate_time_tick(uint32_t time_ms);

/**
 * @brief Calculate gate time for a note
 * @param track Track index (0-3)
 * @param original_length_ms Original note length in milliseconds
 * @return Adjusted note length in milliseconds
 */
uint32_t gate_time_calculate_length(uint8_t track, uint32_t original_length_ms);

/**
 * @brief Reset gate time state for a track (stop all notes)
 * @param track Track index (0-3)
 */
void gate_time_reset(uint8_t track);

/**
 * @brief Reset gate time state for all tracks
 */
void gate_time_reset_all(void);

/**
 * @brief Get mode name string
 * @param mode Gate time mode
 * @return Mode name string
 */
const char* gate_time_get_mode_name(gate_time_mode_t mode);

/**
 * @brief Get statistics for a track
 * @param track Track index (0-3)
 * @param active_notes Output: number of currently active notes
 * @param total_notes Output: total notes processed since init/reset
 */
void gate_time_get_stats(uint8_t track, uint8_t* active_notes, uint32_t* total_notes);

#ifdef __cplusplus
}
#endif
