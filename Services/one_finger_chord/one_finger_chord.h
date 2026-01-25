/**
 * @file one_finger_chord.h
 * @brief One-Finger Chord - Accessibility feature for playing full chords with single notes
 * 
 * Designed for people with limited mobility or motor disabilities who can only
 * press one key at a time. Automatically generates full chord accompaniment
 * from single melody notes.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ONE_FINGER_MAX_TRACKS 4

/**
 * @brief Chord recognition modes
 */
typedef enum {
    OFC_MODE_DISABLED = 0,      // Pass through unchanged
    OFC_MODE_AUTO,              // Auto-detect chord from melody
    OFC_MODE_SPLIT_KEYBOARD,    // Left hand = chord, right hand = melody
    OFC_MODE_SINGLE_NOTE_CHORD, // Each note triggers full chord
    OFC_MODE_COUNT
} ofc_mode_t;

/**
 * @brief Chord voicing styles for accessibility
 */
typedef enum {
    OFC_VOICING_SIMPLE = 0,     // Root + 5th (easiest to hear)
    OFC_VOICING_TRIAD,          // Root + 3rd + 5th
    OFC_VOICING_SEVENTH,        // Root + 3rd + 5th + 7th
    OFC_VOICING_FULL,           // All available chord tones
    OFC_VOICING_COUNT
} ofc_voicing_t;

/**
 * @brief Initialize one-finger chord module
 */
void ofc_init(void);

/**
 * @brief Set mode for a track
 * @param track Track index (0-3)
 * @param mode Operating mode
 */
void ofc_set_mode(uint8_t track, ofc_mode_t mode);

/**
 * @brief Get current mode
 * @param track Track index (0-3)
 * @return Current mode
 */
ofc_mode_t ofc_get_mode(uint8_t track);

/**
 * @brief Set chord voicing style
 * @param track Track index (0-3)
 * @param voicing Voicing style
 */
void ofc_set_voicing(uint8_t track, ofc_voicing_t voicing);

/**
 * @brief Get chord voicing style
 * @param track Track index (0-3)
 * @return Current voicing
 */
ofc_voicing_t ofc_get_voicing(uint8_t track);

/**
 * @brief Set keyboard split point (for split mode)
 * @param track Track index (0-3)
 * @param split_note MIDI note number (0-127, default 60/C4)
 */
void ofc_set_split_point(uint8_t track, uint8_t split_note);

/**
 * @brief Get split point
 * @param track Track index (0-3)
 * @return Split point note number
 */
uint8_t ofc_get_split_point(uint8_t track);

/**
 * @brief Set chord velocity relative to melody
 * @param track Track index (0-3)
 * @param percent Percentage (0-100%, default 70%)
 */
void ofc_set_chord_velocity(uint8_t track, uint8_t percent);

/**
 * @brief Get chord velocity percentage
 * @param track Track index (0-3)
 * @return Velocity percentage
 */
uint8_t ofc_get_chord_velocity(uint8_t track);

/**
 * @brief Enable/disable bass note generation
 * @param track Track index (0-3)
 * @param enabled 1 to add bass note, 0 to disable
 */
void ofc_set_bass_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if bass is enabled
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t ofc_is_bass_enabled(uint8_t track);

/**
 * @brief Process incoming MIDI note
 * @param track Track index (0-3)
 * @param note MIDI note number
 * @param velocity Note velocity (0 = note off)
 * @param channel MIDI channel
 */
void ofc_process_note(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel);

/**
 * @brief Manually set the current chord
 * @param track Track index (0-3)
 * @param root_note Root note of chord (0-11, C=0)
 * @param is_minor 1 for minor, 0 for major
 */
void ofc_set_chord(uint8_t track, uint8_t root_note, uint8_t is_minor);

/**
 * @brief Get mode name
 * @param mode Mode type
 * @return Mode name string
 */
const char* ofc_get_mode_name(ofc_mode_t mode);

/**
 * @brief Callback for outputting generated notes
 * @param track Track index
 * @param note MIDI note
 * @param velocity Velocity (0 = note off)
 * @param channel MIDI channel
 */
typedef void (*ofc_output_cb_t)(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel);

/**
 * @brief Set output callback
 * @param callback Callback function
 */
void ofc_set_output_callback(ofc_output_cb_t callback);

#ifdef __cplusplus
}
#endif
