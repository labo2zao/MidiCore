#include "Services/humanize/humanize.h"
#include <string.h>

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

/* === Runtime control API for CLI (single global instrument cfg) === */
static instrument_cfg_t g_humanize_cfg;

int humanize_get_time_variation(uint8_t track) {
  (void)track;
  return g_humanize_cfg.human_time_ms;
}

int humanize_get_velocity_variation(uint8_t track) {
  (void)track;
  return g_humanize_cfg.human_vel;
}

void humanize_set_time_variation(uint8_t track, int value) {
  (void)track;
  if (value < 0) value = 0;
  if (value > 100) value = 100;
  g_humanize_cfg.human_time_ms = (uint8_t)value;
}

void humanize_set_velocity_variation(uint8_t track, int value) {
  (void)track;
  if (value < 0) value = 0;
  if (value > 100) value = 100;
  g_humanize_cfg.human_vel = (uint8_t)value;
}

void humanize_set_enabled(uint8_t track, uint8_t enable) {
  (void)track;
  g_humanize_cfg.human_enable = enable ? 1u : 0u;
}

uint8_t humanize_is_enabled(uint8_t track) {
  (void)track;
  return g_humanize_cfg.human_enable;
}

/* Initialize default cfg at startup */
__attribute__((constructor))
static void humanize_ctor(void) {
  memset(&g_humanize_cfg, 0, sizeof(g_humanize_cfg));
  g_humanize_cfg.human_apply_mask = HUMAN_APPLY_KEYS | HUMAN_APPLY_CHORD | HUMAN_APPLY_LOOPER | HUMAN_APPLY_THRU;
}
