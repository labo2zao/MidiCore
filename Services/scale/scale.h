/**
 * @file scale.h
 * @brief Musical scale definitions and note quantization
 * 
 * Provides scale types and functions to quantize notes to scales.
 * Used by LiveFX force-to-scale feature.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Scale types
 */
typedef enum {
  SCALE_CHROMATIC = 0,     // All notes (no quantization)
  SCALE_MAJOR,             // Major (Ionian)
  SCALE_MINOR_NAT,         // Natural Minor (Aeolian)
  SCALE_MINOR_HARM,        // Harmonic Minor
  SCALE_MINOR_MEL,         // Melodic Minor (ascending)
  SCALE_DORIAN,            // Dorian
  SCALE_PHRYGIAN,          // Phrygian
  SCALE_LYDIAN,            // Lydian
  SCALE_MIXOLYDIAN,        // Mixolydian
  SCALE_LOCRIAN,           // Locrian
  SCALE_PENTATONIC_MAJ,    // Major Pentatonic
  SCALE_PENTATONIC_MIN,    // Minor Pentatonic
  SCALE_BLUES,             // Blues scale
  SCALE_WHOLE_TONE,        // Whole tone
  SCALE_DIMINISHED,        // Diminished (octatonic)
  SCALE_COUNT              // Number of scales
} scale_type_t;

/**
 * @brief Initialize scale system
 */
void scale_init(void);

/**
 * @brief Quantize a MIDI note to the nearest note in a scale
 * @param note Input MIDI note (0-127)
 * @param scale Scale type
 * @param root Root note of the scale (0=C, 1=C#, ..., 11=B)
 * @return Quantized MIDI note
 */
uint8_t scale_quantize_note(uint8_t note, uint8_t scale, uint8_t root);

/**
 * @brief Get the name of a scale
 * @param scale Scale type
 * @return Scale name string
 */
const char* scale_get_name(uint8_t scale);

/**
 * @brief Get the name of a note
 * @param note Note number (0-11 for C-B)
 * @return Note name string (e.g., "C", "C#", "D", ...)
 */
const char* scale_get_note_name(uint8_t note);

#ifdef __cplusplus
}
#endif
