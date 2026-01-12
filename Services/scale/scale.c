/**
 * @file scale.c
 * @brief Musical scale definitions and note quantization
 */

#include "Services/scale/scale.h"
#include <stdlib.h>

// Scale intervals (in semitones from root)
// Each array contains the notes in the scale relative to root (0-11)
static const uint8_t scale_intervals[][12] = {
  // SCALE_CHROMATIC - All notes
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11},
  
  // SCALE_MAJOR - W-W-H-W-W-W-H
  {0, 2, 4, 5, 7, 9, 11, 0, 0, 0, 0, 0},
  
  // SCALE_MINOR_NAT - W-H-W-W-H-W-W
  {0, 2, 3, 5, 7, 8, 10, 0, 0, 0, 0, 0},
  
  // SCALE_MINOR_HARM - W-H-W-W-H-WH-H
  {0, 2, 3, 5, 7, 8, 11, 0, 0, 0, 0, 0},
  
  // SCALE_MINOR_MEL - W-H-W-W-W-W-H (ascending)
  {0, 2, 3, 5, 7, 9, 11, 0, 0, 0, 0, 0},
  
  // SCALE_DORIAN - W-H-W-W-W-H-W
  {0, 2, 3, 5, 7, 9, 10, 0, 0, 0, 0, 0},
  
  // SCALE_PHRYGIAN - H-W-W-W-H-W-W
  {0, 1, 3, 5, 7, 8, 10, 0, 0, 0, 0, 0},
  
  // SCALE_LYDIAN - W-W-W-H-W-W-H
  {0, 2, 4, 6, 7, 9, 11, 0, 0, 0, 0, 0},
  
  // SCALE_MIXOLYDIAN - W-W-H-W-W-H-W
  {0, 2, 4, 5, 7, 9, 10, 0, 0, 0, 0, 0},
  
  // SCALE_LOCRIAN - H-W-W-H-W-W-W
  {0, 1, 3, 5, 6, 8, 10, 0, 0, 0, 0, 0},
  
  // SCALE_PENTATONIC_MAJ - W-W-WH-W-WH
  {0, 2, 4, 7, 9, 0, 0, 0, 0, 0, 0, 0},
  
  // SCALE_PENTATONIC_MIN - WH-W-W-WH-W
  {0, 3, 5, 7, 10, 0, 0, 0, 0, 0, 0, 0},
  
  // SCALE_BLUES - WH-W-H-H-WH-W
  {0, 3, 5, 6, 7, 10, 0, 0, 0, 0, 0, 0},
  
  // SCALE_WHOLE_TONE - W-W-W-W-W-W
  {0, 2, 4, 6, 8, 10, 0, 0, 0, 0, 0, 0},
  
  // SCALE_DIMINISHED - H-W-H-W-H-W-H-W
  {0, 1, 3, 4, 6, 7, 9, 10, 0, 0, 0, 0}
};

// Number of notes in each scale
static const uint8_t scale_note_counts[] = {
  12,  // CHROMATIC
  7,   // MAJOR
  7,   // MINOR_NAT
  7,   // MINOR_HARM
  7,   // MINOR_MEL
  7,   // DORIAN
  7,   // PHRYGIAN
  7,   // LYDIAN
  7,   // MIXOLYDIAN
  7,   // LOCRIAN
  5,   // PENTATONIC_MAJ
  5,   // PENTATONIC_MIN
  6,   // BLUES
  6,   // WHOLE_TONE
  8    // DIMINISHED
};

// Scale names
static const char* scale_names[] = {
  "Chromatic",
  "Major",
  "Minor (Natural)",
  "Minor (Harmonic)",
  "Minor (Melodic)",
  "Dorian",
  "Phrygian",
  "Lydian",
  "Mixolydian",
  "Locrian",
  "Pentatonic Major",
  "Pentatonic Minor",
  "Blues",
  "Whole Tone",
  "Diminished"
};

// Note names
static const char* note_names[] = {
  "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

/**
 * @brief Initialize scale system
 */
void scale_init(void) {
  // Nothing to initialize
}

/**
 * @brief Quantize a MIDI note to the nearest note in a scale
 */
uint8_t scale_quantize_note(uint8_t note, uint8_t scale, uint8_t root) {
  if (note > 127) note = 127;
  if (scale >= SCALE_COUNT) scale = SCALE_CHROMATIC;
  root = root % 12;
  
  // Chromatic scale - no quantization
  if (scale == SCALE_CHROMATIC) return note;
  
  // Get note within octave (0-11)
  uint8_t octave = note / 12;
  uint8_t note_in_octave = note % 12;
  
  // Shift by root
  int16_t shifted = note_in_octave - root;
  if (shifted < 0) shifted += 12;
  
  // Find nearest note in scale
  const uint8_t* intervals = scale_intervals[scale];
  uint8_t count = scale_note_counts[scale];
  
  int16_t min_distance = 12;
  uint8_t nearest = 0;
  
  for (uint8_t i = 0; i < count; i++) {
    int16_t interval = intervals[i];
    int16_t distance = shifted - interval;
    
    // Check both directions (wrap around octave)
    if (distance < 0) distance = -distance;
    if (distance > 6) distance = 12 - distance;
    
    if (distance < min_distance) {
      min_distance = distance;
      nearest = interval;
    }
  }
  
  // Reconstruct note with quantized value
  uint8_t quantized_in_octave = (nearest + root) % 12;
  uint8_t result = octave * 12 + quantized_in_octave;
  
  // Make sure we don't exceed MIDI range
  if (result > 127) result = 127;
  
  return result;
}

/**
 * @brief Get the name of a scale
 */
const char* scale_get_name(uint8_t scale) {
  if (scale >= SCALE_COUNT) return "Unknown";
  return scale_names[scale];
}

/**
 * @brief Get the name of a note
 */
const char* scale_get_note_name(uint8_t note) {
  note = note % 12;
  return note_names[note];
}
