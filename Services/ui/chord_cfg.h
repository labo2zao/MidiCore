#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CHORD_MAX_PRESETS 8

typedef struct {
  int8_t intervals[4];    // semitone offsets from root (0 must be first)
  uint8_t count;          // number of notes (1..4)
  int8_t transpose;       // global transpose semitones (-24..+24)
  uint8_t vel_scale[4];   // per-note velocity scale percent (0..200)
} chord_preset_t;

typedef struct {
  chord_preset_t preset[CHORD_MAX_PRESETS];
  uint8_t preset_count;         // how many are defined (1..CHORD_MAX_PRESETS)
  uint8_t map_noteclass[12];    // for root%12 -> preset index (0..preset_count-1)
} chord_bank_t;

void chord_bank_defaults(chord_bank_t* b);

/** Load from SD: /cfg/chord_bank.ngc. Sections:
  [CHORD0]..[CHORD7] and [MAP] (NOTECLASS0..11 or C,C#,D,...)
*/
int chord_bank_load(chord_bank_t* b, const char* path);

/** Expand root note using mapped preset (root%12). Returns count. */
uint8_t chord_bank_expand(const chord_bank_t* b, uint8_t root, uint8_t notes_out[4], uint8_t* preset_used);

/** Scale a velocity for note index using a preset. */
uint8_t chord_preset_scale_vel(const chord_preset_t* c, uint8_t idx, uint8_t vel);


#ifdef __cplusplus
}
#endif
