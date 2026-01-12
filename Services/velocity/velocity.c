#include "Services/velocity/velocity.h"
#include <math.h>

// Pre-computed constant for velocity normalization (1/127)
#define VELOCITY_NORM_FACTOR (1.0f / 127.0f)

static inline float norm(uint8_t v) { 
  return (float)v * VELOCITY_NORM_FACTOR; 
}

static inline uint8_t denorm(float x) {
  if (x < 0.0f) x = 0.0f;
  if (x > 1.0f) x = 1.0f;
  int iv = (int)lrintf(x * 127.0f);
  if (iv < 1) iv = 1;
  if (iv > 127) iv = 127;
  return (uint8_t)iv;
}

uint8_t velocity_apply_curve(uint8_t in_vel, const instrument_cfg_t* cfg) {
  if (!cfg) return in_vel;
  if (in_vel < 1) in_vel = 1;
  if (in_vel > 127) in_vel = 127;

  float x = norm(in_vel);
  float y = x;

  switch (cfg->vel_curve) {
    default:
    case VCURVE_LINEAR: y = x; break;
    case VCURVE_SOFT:   y = powf(x, 0.75f); break; // softer: higher out for low x
    case VCURVE_HARD:   y = powf(x, 1.45f); break; // harder: lower out for low x
    case VCURVE_CUSTOM: y = powf(x, (cfg->vel_gamma > 0.1f ? cfg->vel_gamma : 1.0f)); break;
  }

  uint8_t v = denorm(y);

  // clamp to min/max range
  if (v < cfg->vel_min) v = cfg->vel_min;
  if (v > cfg->vel_max) v = cfg->vel_max;
  return v;
}
