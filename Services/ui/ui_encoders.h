#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef UI_MAX_ENCODERS
#define UI_MAX_ENCODERS 2
#endif

typedef enum {
  UI_ENC_MODE_NAV = 0,
  UI_ENC_MODE_UI  = 1
} ui_enc_mode_t;

typedef struct {
  uint16_t shift_din;     // DIN bit for SHIFT (65535 disables)
  uint16_t shift_long_ms; // long-press threshold to toggle latch (0 disables latch)
  uint8_t  shift_latch;   // 0=momentary only, 1=allow latch on long press
     // DIN bit for SHIFT (65535 disables)
  uint16_t enc_a[UI_MAX_ENCODERS];
  uint16_t enc_b[UI_MAX_ENCODERS];
  uint16_t enc_btn[UI_MAX_ENCODERS];   // optional, 65535 disables
  ui_enc_mode_t enc_mode[UI_MAX_ENCODERS];
} ui_encoders_cfg_t;

void ui_encoders_defaults(ui_encoders_cfg_t* c);
int ui_encoders_load(ui_encoders_cfg_t* c, const char* path);

#ifdef __cplusplus
}
#endif
