#pragma once
#include <stdint.h>

#define AIN_NUM_KEYS 64

typedef enum {
  AIN_EV_NONE = 0,
  AIN_EV_NOTE_ON,
  AIN_EV_NOTE_OFF,
  AIN_EV_MOVE
} ain_ev_type_t;

typedef struct {
  uint8_t key;
  ain_ev_type_t type;
  uint16_t pos;      // 0..16383
  uint8_t velocity;  // valid for NOTE_ON
} ain_event_t;

void ain_init(void);
void ain_tick_5ms(void);
uint8_t ain_pop_event(ain_event_t* ev);

// Debug helpers --------------------------------------------------------------
// Copies the latest raw (ADC counts, typically 0..4095) values for each key.
// dst must point to an array of at least AIN_NUM_KEYS uint16_t.
void ain_debug_get_raw(uint16_t* dst, uint16_t dst_len);

// Copies the latest filtered raw values (same unit as raw, before 14-bit scaling).
void ain_debug_get_filt(uint16_t* dst, uint16_t dst_len);

// Copies the latest scaled position values (0..16383).
void ain_debug_get_pos(uint16_t* dst, uint16_t dst_len);
