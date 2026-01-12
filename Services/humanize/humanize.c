#include "Services/humanize/humanize.h"

static uint32_t g_rng = 0x12345678u;

void humanize_init(uint32_t seed) {
  if (seed) g_rng = seed;
}

static inline uint32_t rng32(void) {
  g_rng = g_rng * 1664525u + 1013904223u;
  return g_rng;
}

static inline int8_t rand_sym(uint8_t mag) {
  if (!mag) return 0;
  uint32_t r = rng32();
  int32_t v = (int32_t)(r % (uint32_t)(mag << 1u | 1u)) - (int32_t)mag;
  if (v < -127) v = -127;
  if (v > 127) v = 127;
  return (int8_t)v;
}

int8_t humanize_time_ms(const instrument_cfg_t* cfg, uint8_t apply_flag) {
  if (!cfg || !cfg->human_enable) return 0;
  if ((cfg->human_apply_mask & apply_flag) == 0) return 0;
  return rand_sym(cfg->human_time_ms);
}

int8_t humanize_vel_delta(const instrument_cfg_t* cfg, uint8_t apply_flag) {
  if (!cfg || !cfg->human_enable) return 0;
  if ((cfg->human_apply_mask & apply_flag) == 0) return 0;
  return rand_sym(cfg->human_vel);
}
