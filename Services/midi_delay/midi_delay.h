/**
 * @file midi_delay.h
 * @brief MIDI delay/echo effect with tempo sync and feedback
 * 
 * Repeats MIDI notes with tempo-synced delay time and adjustable feedback.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MIDI_DELAY_MAX_TRACKS 4

// Maximum delayed events per track (configurable for memory optimization)
// Each event: ~12 bytes
// Default: 128 events = 6KB total (128 * 12 * 4 tracks ≈ 6KB)
// Reduced: 64 events = 3KB total (saves 3KB RAM)
#ifndef MIDI_DELAY_MAX_EVENTS
  #define MIDI_DELAY_MAX_EVENTS 64  // Reduced from 128 to save 3KB RAM
#endif

/**
 * @brief Delay time divisions
 */
typedef enum {
    DELAY_DIV_1_64 = 0,   // 1/64 note
    DELAY_DIV_1_32,       // 1/32 note
    DELAY_DIV_1_16,       // 1/16 note
    DELAY_DIV_1_8,        // 1/8 note
    DELAY_DIV_1_4,        // 1/4 note
    DELAY_DIV_1_2,        // 1/2 note
    DELAY_DIV_1_1,        // Whole note
    DELAY_DIV_1_16T,      // 1/16 triplet
    DELAY_DIV_1_8T,       // 1/8 triplet
    DELAY_DIV_1_4T,       // 1/4 triplet
    DELAY_DIV_1_16D,      // 1/16 dotted
    DELAY_DIV_1_8D,       // 1/8 dotted
    DELAY_DIV_1_4D,       // 1/4 dotted
    DELAY_DIV_COUNT
} midi_delay_division_t;

/**
 * @brief Initialize MIDI delay module
 * @param tempo Initial tempo in BPM
 */
void midi_delay_init(uint16_t tempo);

/**
 * @brief Update tempo (for tempo-synced delays)
 * @param tempo Tempo in BPM (20-300)
 */
void midi_delay_set_tempo(uint16_t tempo);

/**
 * @brief Called every 1ms to process delayed events
 */
void midi_delay_tick_1ms(void);

/**
 * @brief Enable/disable delay for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void midi_delay_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if delay is enabled for a track
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t midi_delay_is_enabled(uint8_t track);

/**
 * @brief Set delay time division
 * @param track Track index (0-3)
 * @param division Delay time division
 */
void midi_delay_set_division(uint8_t track, midi_delay_division_t division);

/**
 * @brief Get delay time division
 * @param track Track index (0-3)
 * @return Current division
 */
midi_delay_division_t midi_delay_get_division(uint8_t track);

/**
 * @brief Set feedback amount (0-100%)
 * @param track Track index (0-3)
 * @param feedback Feedback percentage (0-100, higher = more repeats)
 */
void midi_delay_set_feedback(uint8_t track, uint8_t feedback);

/**
 * @brief Get feedback amount
 * @param track Track index (0-3)
 * @return Feedback percentage
 */
uint8_t midi_delay_get_feedback(uint8_t track);

/**
 * @brief Set mix (wet/dry balance)
 * @param track Track index (0-3)
 * @param mix Mix percentage (0=dry only, 100=wet only, 50=equal mix)
 */
void midi_delay_set_mix(uint8_t track, uint8_t mix);

/**
 * @brief Get mix
 * @param track Track index (0-3)
 * @return Mix percentage
 */
uint8_t midi_delay_get_mix(uint8_t track);

/**
 * @brief Set velocity decay per repeat
 * @param track Track index (0-3)
 * @param decay Velocity decay percentage (0-100, 0=no decay, 100=full decay)
 */
void midi_delay_set_velocity_decay(uint8_t track, uint8_t decay);

/**
 * @brief Get velocity decay
 * @param track Track index (0-3)
 * @return Velocity decay percentage
 */
uint8_t midi_delay_get_velocity_decay(uint8_t track);

/**
 * @brief Process input MIDI note (adds to delay buffer)
 * @param track Track index (0-3)
 * @param note MIDI note number
 * @param velocity Note velocity
 * @param channel MIDI channel
 */
void midi_delay_process_note(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel);

/**
 * @brief Clear all delayed events for a track
 * @param track Track index (0-3)
 */
void midi_delay_clear(uint8_t track);

/**
 * @brief Clear all delayed events for all tracks
 */
void midi_delay_clear_all(void);

/**
 * @brief Get division name
 * @param division Division type
 * @return Division name string
 */
const char* midi_delay_get_division_name(midi_delay_division_t division);

/**
 * @brief Callback for outputting delayed notes (set by user)
 * @param track Track index
 * @param note MIDI note
 * @param velocity Velocity
 * @param channel MIDI channel
 * @param is_note_on 1 for note on, 0 for note off
 */
typedef void (*midi_delay_output_cb_t)(uint8_t track, uint8_t note, uint8_t velocity, 
                                       uint8_t channel, uint8_t is_note_on);

/**
 * @brief Set output callback for delayed notes
 * @param callback Callback function
 */
void midi_delay_set_output_callback(midi_delay_output_cb_t callback);

#ifdef __cplusplus
}
#endif
