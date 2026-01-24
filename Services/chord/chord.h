/**
 * @file chord.h
 * @brief Chord trigger - converts single notes to chords with voicings and inversions
 * 
 * Allows triggering full chords from single notes with configurable chord types,
 * voicings, and inversions.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CHORD_MAX_TRACKS 4
#define CHORD_MAX_NOTES 6  // Maximum notes in a chord

/**
 * @brief Chord types
 */
typedef enum {
    CHORD_TYPE_MAJOR = 0,        // Major triad (0, 4, 7)
    CHORD_TYPE_MINOR,            // Minor triad (0, 3, 7)
    CHORD_TYPE_DIMINISHED,       // Diminished triad (0, 3, 6)
    CHORD_TYPE_AUGMENTED,        // Augmented triad (0, 4, 8)
    CHORD_TYPE_SUS2,             // Suspended 2nd (0, 2, 7)
    CHORD_TYPE_SUS4,             // Suspended 4th (0, 5, 7)
    CHORD_TYPE_MAJ7,             // Major 7th (0, 4, 7, 11)
    CHORD_TYPE_MIN7,             // Minor 7th (0, 3, 7, 10)
    CHORD_TYPE_DOM7,             // Dominant 7th (0, 4, 7, 10)
    CHORD_TYPE_DIM7,             // Diminished 7th (0, 3, 6, 9)
    CHORD_TYPE_HALFIDIM7,        // Half-diminished 7th (0, 3, 6, 10)
    CHORD_TYPE_AUG7,             // Augmented 7th (0, 4, 8, 10)
    CHORD_TYPE_MAJ9,             // Major 9th (0, 4, 7, 11, 14)
    CHORD_TYPE_MIN9,             // Minor 9th (0, 3, 7, 10, 14)
    CHORD_TYPE_DOM9,             // Dominant 9th (0, 4, 7, 10, 14)
    CHORD_TYPE_POWER,            // Power chord (0, 7, 12)
    CHORD_TYPE_OCTAVE,           // Octaves (0, 12, 24)
    CHORD_TYPE_COUNT
} chord_type_t;

/**
 * @brief Chord voicing spread
 */
typedef enum {
    CHORD_VOICING_CLOSE = 0,     // Close voicing (within 1 octave)
    CHORD_VOICING_DROP2,         // Drop-2 voicing
    CHORD_VOICING_DROP3,         // Drop-3 voicing
    CHORD_VOICING_SPREAD,        // Spread voicing (wide intervals)
    CHORD_VOICING_COUNT
} chord_voicing_t;

/**
 * @brief Initialize chord module
 */
void chord_init(void);

/**
 * @brief Enable/disable chord trigger for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void chord_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if chord trigger is enabled for a track
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t chord_is_enabled(uint8_t track);

/**
 * @brief Set chord type for a track
 * @param track Track index (0-3)
 * @param type Chord type
 */
void chord_set_type(uint8_t track, chord_type_t type);

/**
 * @brief Get chord type for a track
 * @param track Track index (0-3)
 * @return Current chord type
 */
chord_type_t chord_get_type(uint8_t track);

/**
 * @brief Set chord inversion
 * @param track Track index (0-3)
 * @param inversion Inversion number (0=root position, 1=1st inversion, etc.)
 */
void chord_set_inversion(uint8_t track, uint8_t inversion);

/**
 * @brief Get chord inversion
 * @param track Track index (0-3)
 * @return Current inversion
 */
uint8_t chord_get_inversion(uint8_t track);

/**
 * @brief Set chord voicing
 * @param track Track index (0-3)
 * @param voicing Voicing type
 */
void chord_set_voicing(uint8_t track, chord_voicing_t voicing);

/**
 * @brief Get chord voicing
 * @param track Track index (0-3)
 * @return Current voicing type
 */
chord_voicing_t chord_get_voicing(uint8_t track);

/**
 * @brief Generate chord notes from a root note
 * @param track Track index (0-3)
 * @param root_note Root MIDI note
 * @param notes Output array for chord notes (must have space for CHORD_MAX_NOTES)
 * @return Number of notes in the chord
 */
uint8_t chord_generate(uint8_t track, uint8_t root_note, uint8_t* notes);

/**
 * @brief Get chord type name
 * @param type Chord type
 * @return Chord type name string
 */
const char* chord_get_type_name(chord_type_t type);

#ifdef __cplusplus
}
#endif
