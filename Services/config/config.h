#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  // --- SRIO ---
  uint8_t  srio_enable;
  uint8_t  srio_din_enable;
  uint8_t  srio_dout_enable;
  uint16_t srio_din_bytes;   // 0 -> use compile-time default
  uint16_t srio_dout_bytes;  // 0 -> use compile-time default
  uint16_t srio_scan_ms;     // 0 -> default 5ms

  // --- Polarity / inversion ---
  uint8_t dout_invert_default;
  uint8_t din_invert_default;

  // per-bit inversion (up to 64 bits)
  uint8_t bit_inv[64];

  // --- RGB mapping/inversion (optional) ---
  uint8_t rgb_r_invert;
  uint8_t rgb_g_invert;
  uint8_t rgb_b_invert;
  uint8_t rgb_map_r[16];
  uint8_t rgb_map_g[16];
  uint8_t rgb_map_b[16];

  // --- UI ---
  uint16_t ui_shift_hold_ms;

  // --- Instrument options ---
  uint8_t instrument_auto_loop;

  // --- Global / Safety ---
  uint8_t global_safe_mode;
  uint8_t global_sd_required;
  uint8_t global_shift_active_low;

  // --- AINSER placeholders ---
  uint8_t  ainser_enable;
  uint16_t ainser_scan_ms;
} config_t;

void config_set_defaults(config_t* c);

/** Load config from SD.
    Supports:
    - flat KEY=VALUE
    - sections [UI], [SRIO], [DOUT], [AINSER], [INSTRUMENT]
      keys inside are auto-prefixed, e.g. [SRIO] ENABLE=1 -> SRIO_ENABLE=1
*/
int config_load_from_sd(config_t* c, const char* path);

#ifdef __cplusplus
}
#endif
