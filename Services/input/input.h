#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Button logical IDs used by UI: 1..9 (see UI pages). You can map any physical.
typedef struct {
  uint16_t debounce_ms;        // default 20
  uint16_t shift_hold_ms;      // default 500 (long-press to enter shift)
  uint8_t  shift_button_id;    // logical button id used for SHIFT (default 10 internal)
} input_config_t;

void input_init(const input_config_t* cfg);

/** Call at 1ms or 5ms periodic rate (your choice; set debounce accordingly). */
void input_tick(uint32_t now_ms);

/** Feed a raw physical button state change. phys_id: 0..N-1. pressed: 1/0. */
void input_feed_button(uint16_t phys_id, uint8_t pressed);

/** Feed encoder delta (already decoded). phys_id: 0..N-1. delta: -127..127. */
void input_feed_encoder(uint16_t phys_id, int8_t delta);

/** Optional: query shift state. */
uint8_t input_shift_active(void);
uint8_t input_get_phys_state(uint16_t phys_id);

#ifdef __cplusplus
}
#endif
