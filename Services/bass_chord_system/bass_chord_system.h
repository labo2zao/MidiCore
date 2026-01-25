/**
 * @file bass_chord_system.h
 * @brief Bass Chord System - Stradella bass for accordion
 * 
 * Implements the standard Stradella bass system found on accordion left hand.
 * 120-bass, 96-bass, and 72-bass layouts. Automatically generates bass notes
 * and chords from single button presses.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BASS_CHORD_MAX_TRACKS 4

/**
 * @brief Bass system layouts
 */
typedef enum {
    BASS_LAYOUT_120 = 0,    // 120-bass standard (6 rows)
    BASS_LAYOUT_96,         // 96-bass (5 rows)
    BASS_LAYOUT_72,         // 72-bass (4 rows)
    BASS_LAYOUT_48,         // 48-bass (compact)
    BASS_LAYOUT_FREE,       // Free bass (chromatic)
    BASS_LAYOUT_COUNT
} bass_layout_t;

/**
 * @brief Chord types in Stradella system
 */
typedef enum {
    STRADELLA_COUNTER_BASS = 0,  // Counter bass (root + 5th above)
    STRADELLA_BASS,              // Fundamental bass (root + octave)
    STRADELLA_MAJOR,             // Major chord
    STRADELLA_MINOR,             // Minor chord
    STRADELLA_DOMINANT_7,        // Dominant 7th chord
    STRADELLA_DIMINISHED,        // Diminished 7th chord
    STRADELLA_COUNT
} stradella_type_t;

/**
 * @brief Initialize bass chord system
 */
void bass_chord_init(void);

/**
 * @brief Set bass layout
 * @param track Track index (0-3)
 * @param layout Bass layout type
 */
void bass_chord_set_layout(uint8_t track, bass_layout_t layout);

/**
 * @brief Get bass layout
 * @param track Track index (0-3)
 * @return Current layout
 */
bass_layout_t bass_chord_get_layout(uint8_t track);

/**
 * @brief Set bass note range
 * @param track Track index (0-3)
 * @param start_note Starting MIDI note for bass row (typically C2=36)
 */
void bass_chord_set_base_note(uint8_t track, uint8_t start_note);

/**
 * @brief Get bass note range
 * @param track Track index (0-3)
 * @return Base note
 */
uint8_t bass_chord_get_base_note(uint8_t track);

/**
 * @brief Enable/disable octave doubling in bass
 * @param track Track index (0-3)
 * @param enabled 1 to double bass notes, 0 for single note
 */
void bass_chord_set_octave_doubling(uint8_t track, uint8_t enabled);

/**
 * @brief Check if octave doubling is enabled
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t bass_chord_is_octave_doubling(uint8_t track);

/**
 * @brief Set chord voicing density
 * @param track Track index (0-3)
 * @param density 0=sparse (3 notes), 1=normal (4 notes), 2=dense (5+ notes)
 */
void bass_chord_set_voicing_density(uint8_t track, uint8_t density);

/**
 * @brief Get chord voicing density
 * @param track Track index (0-3)
 * @return Density level
 */
uint8_t bass_chord_get_voicing_density(uint8_t track);

/**
 * @brief Set bass velocity
 * @param track Track index (0-3)
 * @param percent Percentage of input velocity (0-150%)
 */
void bass_chord_set_bass_velocity(uint8_t track, uint8_t percent);

/**
 * @brief Get bass velocity percentage
 * @param track Track index (0-3)
 * @return Velocity percentage
 */
uint8_t bass_chord_get_bass_velocity(uint8_t track);

/**
 * @brief Set chord velocity
 * @param track Track index (0-3)
 * @param percent Percentage of input velocity (0-150%)
 */
void bass_chord_set_chord_velocity(uint8_t track, uint8_t percent);

/**
 * @brief Get chord velocity percentage
 * @param track Track index (0-3)
 * @return Velocity percentage
 */
uint8_t bass_chord_get_chord_velocity(uint8_t track);

/**
 * @brief Process incoming bass button press
 * @param track Track index (0-3)
 * @param button Button number (0-119 for 120-bass)
 * @param velocity Button velocity (0 = release)
 * @param channel MIDI channel
 */
void bass_chord_process_button(uint8_t track, uint8_t button, uint8_t velocity, uint8_t channel);

/**
 * @brief Map button to Stradella type and root note
 * @param track Track index (0-3)
 * @param button Button number
 * @param type Output: Stradella type
 * @param root Output: Root note (0-11, C=0)
 */
void bass_chord_button_to_stradella(uint8_t track, uint8_t button, 
                                   stradella_type_t* type, uint8_t* root);

/**
 * @brief Get layout name
 * @param layout Layout type
 * @return Layout name string
 */
const char* bass_chord_get_layout_name(bass_layout_t layout);

/**
 * @brief Callback for outputting bass/chord notes
 * @param track Track index
 * @param note MIDI note
 * @param velocity Velocity (0 = note off)
 * @param channel MIDI channel
 */
typedef void (*bass_chord_output_cb_t)(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel);

/**
 * @brief Set output callback
 * @param callback Callback function
 */
void bass_chord_set_output_callback(bass_chord_output_cb_t callback);

#ifdef __cplusplus
}
#endif
