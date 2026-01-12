#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Generic AINSER mapping layer.
//
// Goal:
//  - decouple raw 12-bit ADC readings (0..4095) from MIDI/event generation
//  - provide per-channel mapping with:
//      * CC number
//      * MIDI channel
//      * min/max range (12-bit)
//      * curve (linear / expo / log-ish)
//      * inversion
//      * enable flag
//      * per-channel threshold (minimal delta before emitting)
//  - optional output callback to integrate either with:
//      * simple UART/USB MIDI sender (selftest)
//      * higher level router in the main application
//
// This module is intentionally agnostic of the AINSER backend:
//  - it only assumes channel indices 0..63
//  - you call ainser_map_process_channel(idx, raw12) from your scan loop.

#define AINSER_NUM_CHANNELS 64u
#define AINSER_ADC_MAX      4095u

// Curves for mapping the 0..127 domain.
// They are intentionally cheap to compute (no floats required).
typedef enum {
    AINSER_CURVE_LINEAR = 0u,
    AINSER_CURVE_EXPO   = 1u, // more resolution near 0
    AINSER_CURVE_LOG    = 2u  // more resolution near 127
} AINSER_Curve;

// One entry per AINSER logical channel (0..63).
typedef struct {
    uint8_t  cc;        // MIDI CC number (0..127)
    uint8_t  channel;   // MIDI channel (0..15)
    uint8_t  curve;     // AINSER_Curve
    uint8_t  invert;    // 0=normal, 1=inverted
    uint8_t  enabled;   // 0=ignore, 1=active
    uint8_t  reserved;  // padding / future use
    uint16_t min;       // 12-bit ADC min  (0..4095)
    uint16_t max;       // 12-bit ADC max  (0..4095), must be > min
    uint16_t threshold; // minimal delta (12-bit) to trigger an update
} AINSER_MapEntry;

// Output callback type:
//  - channel: MIDI channel (0..15)
//  - cc:      MIDI CC number (0..127)
//  - value:   MIDI value (0..127)
typedef void (*AINSER_MapOutputFn)(uint8_t channel, uint8_t cc, uint8_t value);

// Returns the internal mapping table so that higher level code can tweak it
// at runtime before starting the scan loop.
AINSER_MapEntry *ainser_map_get_table(void);

// Initialise mapping table, smoothing and caches to reasonable defaults.
// This does NOT touch any hardware, only the software mapping state.
void ainser_map_init_defaults(void);

// Optional: set the output callback.
// If NULL, events are computed but discarded.
void ainser_map_set_output_cb(AINSER_MapOutputFn cb);

// Process a single AINSER logical channel (0..63) with a 12-bit raw value.
// This function:
//  - applies per-channel threshold
//  - applies smoothing
//  - clamps to min/max and applies inversion
//  - applies curve (linear/expo/log)
//  - quantises to 0..127
//  - only emits a new value when the 7-bit result actually changes
void ainser_map_process_channel(uint8_t index, uint16_t raw12);
// Load mapping overrides from SD card config file (0:/cfg/ainser_map.ngc by default).
// Returns 0 on success, negative on error. Defaults remain in place for channels not mentioned in the file.
int ainser_map_load_sd(const char* path);

#ifdef __cplusplus
}
#endif
