#include "Services/input/input.h"
#include "Services/ui/ui.h"
#include <string.h>

#ifndef INPUT_MAX_BUTTONS
#define INPUT_MAX_BUTTONS 128
#endif
#ifndef INPUT_MAX_ENCODERS
#define INPUT_MAX_ENCODERS 16
#endif

typedef struct {
  uint8_t stable;
  uint8_t last_raw;
  uint16_t cnt;
} db_t;

static input_config_t g_cfg;
static uint32_t g_now = 0;

static db_t g_btn[INPUT_MAX_BUTTONS];

static uint8_t g_shift = 0;
static uint16_t g_shift_phys = 0xFFFF;
static uint32_t g_shift_press_ms = 0;

static inline uint8_t map_button(uint16_t phys, uint8_t shift) {
  // ---- Default mapping (edit later) ----
  // phys 0..3 -> UI buttons 1..4
  if (!shift) {
    if (phys < 9u) return (uint8_t)(phys + 1u); // 0->1 ... 8->9
    if (phys == 9u) return 5u;                 // page cycle
    if (phys == 10u) return 0u;                // reserved shift
  } else {
    // SHIFT layer: reuse phys 0..3 for tools: dup/transpose/humanize etc
    // Here we map to 6..9 as used by Piano-roll tools
    if (phys == 0u) return 6u; // dup
    if (phys == 1u) return 7u; // transpose +
    if (phys == 2u) return 8u; // transpose -
    if (phys == 3u) return 9u; // humanize
  }
  return 0;
}

static uint8_t map_encoder(uint16_t phys) {
  (void)phys;
  // single encoder -> UI encoder
  return 1;
}

void input_init(const input_config_t* cfg) {
  g_cfg.debounce_ms = 20;
  g_cfg.shift_hold_ms = 500;
  g_cfg.shift_button_id = 10;

  if (cfg) g_cfg = *cfg;

  // Optimize: only clear button states, not entire array
  for (uint16_t i = 0; i < INPUT_MAX_BUTTONS; i++) {
    g_btn[i].stable = 0;
    g_btn[i].last_raw = 0;
    g_btn[i].cnt = 0;
  }
  g_shift = 0;
  g_shift_phys = 0xFFFF;
  g_shift_press_ms = 0;
}

uint8_t input_shift_active(void) { return g_shift; }

void input_tick(uint32_t now_ms) {
  g_now = now_ms;

  // long-press detect for shift
  if (g_shift_phys != 0xFFFF) {
    if (!g_shift && (g_now - g_shift_press_ms) >= g_cfg.shift_hold_ms) {
      g_shift = 1;
    }
  }
}

void input_feed_button(uint16_t phys_id, uint8_t pressed) {
  if (phys_id >= INPUT_MAX_BUTTONS) return;

  db_t* b = &g_btn[phys_id];
  if (pressed != b->last_raw) {
    b->last_raw = pressed;
    b->cnt = 0;
    return; // wait for stable
  }

  if (b->cnt < g_cfg.debounce_ms) {
    b->cnt++;
    return;
  }

  if (pressed == b->stable) return;
  b->stable = pressed;

  // SHIFT physical button (default phys 10)
  if (phys_id == 10) {
    if (pressed) { g_shift_phys = phys_id; g_shift_press_ms = g_now; }
    else { g_shift_phys = 0xFFFF; g_shift = 0; }
    return;
  }

  uint8_t logical = map_button(phys_id, g_shift);
  if (logical) ui_on_button(logical, pressed);
}

void input_feed_encoder(uint16_t phys_id, int8_t delta) {
  (void)map_encoder(phys_id);
  ui_on_encoder(delta);
}


uint8_t input_get_phys_state(uint16_t phys_id) {
  if (phys_id >= INPUT_MAX_BUTTONS) return 0;
  return g_btn[phys_id].stable ? 1u : 0u;
}
