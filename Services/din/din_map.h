#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  DIN_MAP_TYPE_NONE = 0,
  DIN_MAP_TYPE_NOTE = 1,
  DIN_MAP_TYPE_CC   = 2
} DIN_MapType;

typedef struct {
  uint8_t     enabled;    // 0 = ignore
  uint8_t     invert;     // 0 = active-low, 1 = active-high
  uint8_t     type;       // DIN_MAP_TYPE_*
  uint8_t     channel;    // 0..15 (0 = MIDI ch1)
  uint8_t     number;     // note or CC number
  uint8_t     vel_on;     // velocity for Note On
  uint8_t     vel_off;    // velocity for Note Off (0 = note-off with vel 0)
  uint8_t     reserved;
} DIN_MapEntry;

// Callback prototype: called on logical DIN events interpreted as MIDI.
typedef void (*DIN_MapOutputFn)(DIN_MapType type,
                                uint8_t channel,
                                uint8_t number,
                                uint8_t value); // velocity or CC value

void din_map_init_defaults(uint8_t base_note);
DIN_MapEntry *din_map_get_table(void);
void din_map_set_output_cb(DIN_MapOutputFn cb);

// Process a DIN logical channel change.
// pressed = 1 for "pressed", 0 for "released" after inversion.
void din_map_process_event(uint8_t index, uint8_t pressed);

// Load overrides from SD config file (0:/cfg/din_map.ngc).
// Returns 0 on success, negative on error.
int din_map_load_sd(const char* path);

#ifdef __cplusplus
}
#endif