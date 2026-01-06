#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  HUMAN_APPLY_KEYS   = 1u<<0,
  HUMAN_APPLY_CHORD  = 1u<<1,
  HUMAN_APPLY_LOOPER = 1u<<2,
  HUMAN_APPLY_THRU   = 1u<<3
} human_apply_t;

typedef enum {
  STRUM_UP=0,
  STRUM_DOWN=1,
  STRUM_RANDOM=2
} strum_dir_t;

typedef enum {
  VCURVE_LINEAR=0,
  VCURVE_SOFT=1,
  VCURVE_HARD=2,
  VCURVE_CUSTOM=3
} vcurve_t;

typedef struct {
  // humanization
  uint8_t human_enable;
  uint8_t human_time_ms;   // +/- ms
  uint8_t human_vel;       // +/- velocity
  uint8_t human_apply_mask;

  // chord conditional
  uint8_t chord_cond_enable;
  uint8_t chord_vel_gt;    // 0 disables
  uint8_t chord_vel_lt;    // 0 disables
  uint8_t chord_need_hold; // 1 requires HOLD
  uint8_t chord_block_shift;//1 disables if SHIFT
  uint16_t hold_phys_id;   // button phys id used as HOLD (default 4)

  // chord strum/spread
  uint8_t strum_enable;
  uint8_t strum_spread_ms; // total spread
  uint8_t strum_dir;       // strum_dir_t

  // velocity mapping
  uint8_t vel_min;         // 1..127
  uint8_t vel_max;         // 1..127
  uint8_t vel_curve;       // vcurve_t
  float vel_gamma;         // for custom
} instrument_cfg_t;

void instrument_cfg_defaults(instrument_cfg_t* c);
int instrument_cfg_load_sd(instrument_cfg_t* c, const char* path);

const instrument_cfg_t* instrument_cfg_get(void);
void instrument_cfg_set(const instrument_cfg_t* c);

#ifdef __cplusplus
}
#endif
