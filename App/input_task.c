#include "cmsis_os2.h"
#include "Services/input/input.h"
#include "Services/watchdog/watchdog.h"
#include "Services/log/log.h"
#include "Services/srio/srio.h"
#include "Services/srio/srio_user_config.h"
#include "Services/config/config.h"
#include "Services/dout/dout_map.h"
#include "Services/patch/patch_system.h"
#include "Services/ui/ui_bindings.h"
#include "Services/ui/ui_encoders.h"
#include "Services/ui/ui_actions.h"
#include "Services/ui/ui.h"

// This task is compile-safe and demonstrates how to drive the input layer.
// Replace the demo section with your SRIO scanner/encoder decoder.

static uint32_t s_ms = 0;

static uint8_t get_din_bit(const uint8_t* din, uint16_t phys) {
  uint16_t byte = (uint16_t)(phys >> 3);
  uint8_t bit = (uint8_t)(phys & 7u);
  return (din[byte] & (1u<<bit)) ? 1u : 0u;
}

void InputTask(void *argument) {
  (void)argument;

  input_config_t cfg = {
    .debounce_ms = 20,
    .shift_hold_ms = 500,
    .shift_button_id = 10
  };
  input_init(&cfg);

  config_t cfg_sd;
  (void)config_load_from_sd(&cfg_sd, "/cfg/system.ngc");
  ui_bindings_t binds;
  (void)ui_bindings_load(&binds, "/cfg/ui_bindings.ngc");
  dout_map_init(&cfg_sd);

  if (cfg_sd.ui_shift_hold_ms) {
    cfg.shift_hold_ms = cfg_sd.ui_shift_hold_ms;
    input_init(&cfg);
  }

#ifdef SRIO_ENABLE
  srio_config_t scfg = {
    .hspi = SRIO_SPI_HANDLE,
    .din_pl_port = SRIO_DIN_PL_PORT,
    .din_pl_pin = SRIO_DIN_PL_PIN,
    .dout_rclk_port = SRIO_DOUT_RCLK_PORT,
    .dout_rclk_pin = SRIO_DOUT_RCLK_PIN,
    .dout_oe_port = NULL,
    .dout_oe_pin = 0,
    .dout_oe_active_low = 1,
    .din_bytes = (cfg_sd.srio_din_bytes ? cfg_sd.srio_din_bytes : SRIO_DIN_BYTES),
    .dout_bytes = (cfg_sd.srio_dout_bytes ? cfg_sd.srio_dout_bytes : SRIO_DOUT_BYTES),
  };
  srio_init(&scfg);

  static uint8_t din_prev[SRIO_DIN_BYTES];
  static uint8_t din_cur[SRIO_DIN_BYTES];
  memset(din_prev, 0xFF, sizeof(din_prev));

  static uint8_t dout_buf[SRIO_DOUT_BYTES];
  memset(dout_buf, 0, sizeof(dout_buf));
  srio_write_dout(dout_buf);
#endif

  static uint32_t s_log_ms = 0;
  for (;;) {
    osDelay(1);
    s_ms++;

    // Drive timing for debounce/shift
    input_tick(s_ms);

#ifdef SRIO_ENABLE
    uint16_t scan_ms = (cfg_sd.srio_scan_ms ? cfg_sd.srio_scan_ms : 5u);
    if (scan_ms == 0) scan_ms = 5u;
    if ((s_ms % scan_ms) == 0u) {
      if (cfg_sd.srio_enable && cfg_sd.srio_din_enable && srio_read_din(din_cur) == 0) {
        for (uint16_t b=0; b<SRIO_DIN_BYTES; b++) {
          uint8_t diff = (uint8_t)(din_cur[b] ^ din_prev[b]);
          if (!diff) continue;
          for (uint8_t bit=0; bit<8; bit++) {
            if (diff & (1u<<bit)) {
              uint16_t phys = (uint16_t)(b*8u + bit);
              uint8_t pressed = (din_cur[b] & (1u<<bit)) ? 1u : 0u;
              if (cfg_sd.din_invert_default) pressed = (uint8_t)!pressed;
              
// If your buttons are active-low, invert pressed here.
// Intercept UI bindings for bank/patch navigation (only on press)
uint8_t consumed = 0;
if (pressed) {
  if (phys == binds.din_patch_prev) { (void)patch_system_patch_prev(); consumed = 1; }
  else if (phys == binds.din_patch_next) { (void)patch_system_patch_next(); consumed = 1; }
  else if (phys == binds.din_load_apply) { (void)patch_system_apply(); consumed = 1; }
  else if (phys == binds.din_bank_prev) { (void)patch_system_bank_prev(); consumed = 1; }
  else if (phys == binds.din_bank_next) { (void)patch_system_bank_next(); consumed = 1; }
  if (consumed) {
    // refresh OLED header
    const patch_manager_t* pm = patch_system_get();
    const char* bank = pm->bank.bank_id[0] ? pm->bank.bank_id : pm->bank.bank_name;
    const char* patch = pm->bank.patches[pm->state.patch_index].label[0] ?
                        pm->bank.patches[pm->state.patch_index].label : "patch";
    ui_set_patch_status(bank, patch);
  }
}
if (!consumed) {
  input_feed_button(phys, pressed);
}
            }
          }
        }

// ---- Encoder decode (ENC0/ENC1) ----
// SHIFT supports long-press latch (configurable in /cfg/ui_encoders.ngc)
static uint8_t shift_latched = 0;
static uint32_t shift_down_ms = 0;
static uint8_t shift_prev_raw = 0;

uint8_t shift_raw = 0;
if (enc_cfg.shift_din != 0xFFFFu) {
  shift_raw = get_din_bit(din_cur, enc_cfg.shift_din);
  if (cfg_sd.din_invert_default) shift_raw = (uint8_t)!shift_raw;
}

if (shift_raw && !shift_prev_raw) {
  shift_down_ms = s_ms;
}
if (!shift_raw && shift_prev_raw) {
  // released
}
if (enc_cfg.shift_latch && enc_cfg.shift_long_ms) {
  if (shift_raw && !shift_latched) {
    if ((uint32_t)(s_ms - shift_down_ms) >= (uint32_t)enc_cfg.shift_long_ms) {
      shift_latched = 1;
    }
  } else if (shift_raw && shift_latched) {
    if ((uint32_t)(s_ms - shift_down_ms) >= (uint32_t)enc_cfg.shift_long_ms) {
      // long press while latched toggles off
      shift_latched = 0;
    }
  }
}

uint8_t shift = (uint8_t)(shift_raw | shift_latched);
shift_prev_raw = shift_raw;

// Gray code step table
static const int8_t step_lut[16] = {
  0, +1, -1, 0,
  -1, 0, 0, +1,
  +1, 0, 0, -1,
  0, -1, +1, 0
};

static uint8_t enc_prev_ab[UI_MAX_ENCODERS] = {0};
static uint8_t enc_btn_prev[UI_MAX_ENCODERS] = {0};

for (uint8_t e = 0; e < UI_MAX_ENCODERS; e++) {
  if (enc_cfg.enc_a[e] == 0xFFFFu || enc_cfg.enc_b[e] == 0xFFFFu) continue;

  uint8_t a = get_din_bit(din_cur, enc_cfg.enc_a[e]);
  uint8_t b = get_din_bit(din_cur, enc_cfg.enc_b[e]);
  if (cfg_sd.din_invert_default) { a = (uint8_t)!a; b = (uint8_t)!b; }
  uint8_t ab = (uint8_t)((a<<1) | b);

  uint8_t idx = (uint8_t)((enc_prev_ab[e]<<2) | ab);
  int8_t step = step_lut[idx];
  enc_prev_ab[e] = ab;

  if (step) {
    if (enc_cfg.enc_mode[e] == UI_ENC_MODE_UI) {
      input_feed_encoder(e, step);
    } else {
      // NAV mode: no shift => patch, shift => bank
      if (shift) { if (step > 0) (void)patch_system_bank_next(); else (void)patch_system_bank_prev(); }
      else { if (step > 0) (void)patch_system_patch_next(); else (void)patch_system_patch_prev(); }

      const patch_manager_t* pm = patch_system_get();
      const char* bank = pm->bank.bank_id[0] ? pm->bank.bank_id : pm->bank.bank_name;
      const char* patch = pm->bank.patches[pm->state.patch_index].label[0] ?
                          pm->bank.patches[pm->state.patch_index].label : "patch";
      ui_set_patch_status(bank, patch);
    }
  }

  // Encoder button -> LOAD/APPLY (optional)
  if (enc_cfg.enc_btn[e] != 0xFFFFu) {
    uint8_t btn = get_din_bit(din_cur, enc_cfg.enc_btn[e]);
    if (cfg_sd.din_invert_default) btn = (uint8_t)!btn;
    if (btn && !enc_btn_prev[e]) {
      if (enc_cfg.enc_mode[e] == UI_ENC_MODE_UI) {
        ui_actions_on_button(&act_cfg, e, shift);
      } else {
        (void)patch_system_apply();
        const patch_manager_t* pm = patch_system_get();
        const char* bank = pm->bank.bank_id[0] ? pm->bank.bank_id : pm->bank.bank_name;
        const char* patch = pm->bank.patches[pm->state.patch_index].label[0] ?
                            pm->bank.patches[pm->state.patch_index].label : "patch";
        ui_set_patch_status(bank, patch);
      }
    }
    enc_btn_prev[e] = btn;
  }
}

// Encoder button -> LOAD/APPLY (optional)
if (enc_cfg.enc_btn[0] != 0xFFFFu) {
  uint8_t btn = get_din_bit(din_cur, enc_cfg.enc_btn[0]);
  if (cfg_sd.din_invert_default) btn = (uint8_t)!btn;
  if (btn && !enc0_btn_prev) {
    (void)patch_system_apply();
    const patch_manager_t* pm = patch_system_get();
    const char* bank = pm->bank.bank_id[0] ? pm->bank.bank_id : pm->bank.bank_name;
    const char* patch = pm->bank.patches[pm->state.patch_index].label[0] ?
                        pm->bank.patches[pm->state.patch_index].label : "patch";
    ui_set_patch_status(bank, patch);
  }
  enc0_btn_prev = btn;
}

        memcpy(din_prev, din_cur, SRIO_DIN_BYTES);
      }
      if (cfg_sd.srio_enable && cfg_sd.srio_dout_enable) {
        static uint8_t logical_dout[SRIO_DOUT_BYTES];
        memset(logical_dout, 0, sizeof(logical_dout));
        // Example: LED0 red follows SHIFT state (requires RGB mapping in config)
        dout_set_rgb(logical_dout, 0, input_shift_active(), 0, 0);
        dout_map_apply(logical_dout, dout_buf, SRIO_DOUT_BYTES);
        (void)srio_write_dout(dout_buf);
      }
    }
#endif

    // ---- DEMO SECTION (disabled by default) ----
    // If you want a quick UI test without hardware:
    // - define INPUT_DEMO in compiler symbols
    // - it will press BTN5 every 2 seconds to cycle pages.
#ifdef INPUT_DEMO
    if ((s_ms % 2000u) == 10u) input_feed_button(9, 1); // phys 9 -> logical 5 (page cycle)
    if ((s_ms % 2000u) == 30u) input_feed_button(9, 0);
#endif
  }
}
