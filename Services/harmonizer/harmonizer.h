/**
 * @file harmonizer.h
 * @brief MIDI harmonizer - adds harmony notes (thirds, fifths) according to scale
 * 
 * Intelligently adds harmony notes based on the current scale, creating
 * diatonic harmonies that sound musical.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HARMONIZER_MAX_TRACKS 4
#define HARMONIZER_MAX_VOICES 4  // Original + up to 3 harmony voices

/**
 * @brief Harmony interval types
 */
typedef enum {
    HARM_INTERVAL_UNISON = 0,    // No harmony (original only)
    HARM_INTERVAL_THIRD_UP,      // Third above (diatonic)
    HARM_INTERVAL_THIRD_DOWN,    // Third below (diatonic)
    HARM_INTERVAL_FIFTH_UP,      // Fifth above (diatonic)
    HARM_INTERVAL_FIFTH_DOWN,    // Fifth below (diatonic)
    HARM_INTERVAL_OCTAVE_UP,     // Octave above
    HARM_INTERVAL_OCTAVE_DOWN,   // Octave below
    HARM_INTERVAL_FOURTH_UP,     // Fourth above (diatonic)
    HARM_INTERVAL_FOURTH_DOWN,   // Fourth below (diatonic)
    HARM_INTERVAL_SIXTH_UP,      // Sixth above (diatonic)
    HARM_INTERVAL_SIXTH_DOWN,    // Sixth below (diatonic)
    HARM_INTERVAL_COUNT
} harmonizer_interval_t;

/**
 * @brief Initialize harmonizer module
 */
void harmonizer_init(void);

/**
 * @brief Enable/disable harmonizer for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void harmonizer_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if harmonizer is enabled for a track
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t harmonizer_is_enabled(uint8_t track);

/**
 * @brief Set harmony voice interval
 * @param track Track index (0-3)
 * @param voice Voice index (0-3, where 0 is original note)
 * @param interval Harmony interval type
 */
void harmonizer_set_voice_interval(uint8_t track, uint8_t voice, harmonizer_interval_t interval);

/**
 * @brief Get harmony voice interval
 * @param track Track index (0-3)
 * @param voice Voice index (0-3)
 * @return Harmony interval type
 */
harmonizer_interval_t harmonizer_get_voice_interval(uint8_t track, uint8_t voice);

/**
 * @brief Enable/disable a harmony voice
 * @param track Track index (0-3)
 * @param voice Voice index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void harmonizer_set_voice_enabled(uint8_t track, uint8_t voice, uint8_t enabled);

/**
 * @brief Check if a harmony voice is enabled
 * @param track Track index (0-3)
 * @param voice Voice index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t harmonizer_is_voice_enabled(uint8_t track, uint8_t voice);

/**
 * @brief Set voice velocity offset (for mixing)
 * @param track Track index (0-3)
 * @param voice Voice index (0-3)
 * @param offset Velocity offset (-64 to +63, 0 = no change)
 */
void harmonizer_set_voice_velocity(uint8_t track, uint8_t voice, int8_t offset);

/**
 * @brief Get voice velocity offset
 * @param track Track index (0-3)
 * @param voice Voice index (0-3)
 * @return Velocity offset
 */
int8_t harmonizer_get_voice_velocity(uint8_t track, uint8_t voice);

/**
 * @brief Set scale for harmonization
 * @param track Track index (0-3)
 * @param scale_type Scale type (from scale.h)
 * @param root Root note (0-11, C=0)
 */
void harmonizer_set_scale(uint8_t track, uint8_t scale_type, uint8_t root);

/**
 * @brief Get scale for harmonization
 * @param track Track index (0-3)
 * @param scale_type Output: scale type
 * @param root Output: root note
 */
void harmonizer_get_scale(uint8_t track, uint8_t* scale_type, uint8_t* root);

/**
 * @brief Generate harmony notes from an input note
 * @param track Track index (0-3)
 * @param input_note Input MIDI note
 * @param input_velocity Input velocity
 * @param output_notes Output array for harmony notes (must have space for HARMONIZER_MAX_VOICES)
 * @param output_velocities Output array for velocities (must have space for HARMONIZER_MAX_VOICES)
 * @return Number of output notes (including original)
 */
uint8_t harmonizer_generate(uint8_t track, uint8_t input_note, uint8_t input_velocity,
                            uint8_t* output_notes, uint8_t* output_velocities);

/**
 * @brief Get interval name
 * @param interval Interval type
 * @return Interval name string
 */
const char* harmonizer_get_interval_name(harmonizer_interval_t interval);

#ifdef __cplusplus
}
#endif
