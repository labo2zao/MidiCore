#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  EXPR_CURVE_LINEAR = 0,
  EXPR_CURVE_EXPO   = 1,   // gamma curve, param = gamma*100 (e.g. 180 => 1.80)
  EXPR_CURVE_S      = 2    // smoothstep-ish
} expr_curve_t;

typedef enum {
  EXPR_BIDIR_OFF = 0,
  EXPR_BIDIR_PUSH_PULL = 1
} expr_bidir_t;

typedef struct {
  uint8_t enable;

  uint8_t midi_ch;     // 0..15
  uint8_t cc_num;      // used when BIDIR=0
  uint8_t cc_push;     // used when BIDIR=1
  uint8_t cc_pull;     // used when BIDIR=1
  uint8_t bidir;       // expr_bidir_t

  uint16_t raw_min;    // 0..4095
  uint16_t raw_max;

  // Neutral zone around 0 Pa to avoid push/pull flips
  uint16_t zero_deadband_pa; // +/- Pa

  uint8_t out_min;     // 0..127
  uint8_t out_max;

  uint8_t rate_ms;
  uint8_t smoothing;   // 0..255 (higher = smoother)

  // A) Deadband/hysteresis (CC steps)
  uint8_t deadband_cc;
  uint8_t hyst_cc;

  // B) Curve
  uint8_t curve;       // expr_curve_t
  uint16_t curve_param;// gamma*100 for EXPO

} expr_cfg_t;

void expression_init(void);
void expression_set_cfg(const expr_cfg_t* cfg);
const expr_cfg_t* expression_get_cfg(void);

void expression_set_raw(uint16_t raw);          // 0..4095
void expression_set_pressure_pa(int32_t pa);    // signed Pa (for BIDIR)

void expression_tick_1ms(void);

// resets filter/timers/last-sent without changing cfg
void expression_runtime_reset(void);

#ifdef __cplusplus
}
#endif
