#include "Services/dout/dout_map.h"
#include <string.h>

static config_t g_cfg;

static inline void bit_set(uint8_t* buf, uint16_t bit, uint8_t v) {
  uint16_t b = bit >> 3;
  uint8_t m = (uint8_t)(1u << (bit & 7));
  if (v) buf[b] |= m;
  else   buf[b] &= (uint8_t)~m;
}

void dout_map_init(const config_t* cfg) {
  if (cfg) g_cfg = *cfg;
  else config_set_defaults(&g_cfg);
}

void dout_map_apply(const uint8_t* logical, uint8_t* physical, uint16_t bytes) {
  if (!logical || !physical || !bytes) return;
  memcpy(physical, logical, bytes);

  if (g_cfg.dout_invert_default) {
    for (uint16_t i=0;i<bytes;i++) physical[i] ^= 0xFF;
  }

  uint16_t max_bits = bytes * 8u;
  if (max_bits > 64) max_bits = 64;
  for (uint16_t bit=0; bit<max_bits; bit++) {
    if (g_cfg.bit_inv[bit]) {
      physical[bit>>3] ^= (uint8_t)(1u << (bit & 7));
    }
  }
}

void dout_set_rgb(uint8_t* logical, uint8_t led, uint8_t r, uint8_t g, uint8_t b) {
  if (!logical || led >= 16) return;

  if (g_cfg.rgb_r_invert) r = (uint8_t)!r;
  if (g_cfg.rgb_g_invert) g = (uint8_t)!g;
  if (g_cfg.rgb_b_invert) b = (uint8_t)!b;

  uint8_t rb = g_cfg.rgb_map_r[led];
  uint8_t gb = g_cfg.rgb_map_g[led];
  uint8_t bb = g_cfg.rgb_map_b[led];

  if (rb != 0xFF) bit_set(logical, rb, r);
  if (gb != 0xFF) bit_set(logical, gb, g);
  if (bb != 0xFF) bit_set(logical, bb, b);
}
