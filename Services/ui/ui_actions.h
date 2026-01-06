#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  UI_ACT_NONE = 0,

  // Navigation / patch
  UI_ACT_PATCH_PREV,
  UI_ACT_PATCH_NEXT,
  UI_ACT_BANK_PREV,
  UI_ACT_BANK_NEXT,
  UI_ACT_LOAD_APPLY,

  // Looper/UI editing (generic)
  UI_ACT_UI_PREV_PAGE,
  UI_ACT_UI_NEXT_PAGE,
  UI_ACT_CURSOR_LEFT,
  UI_ACT_CURSOR_RIGHT,
  UI_ACT_ZOOM_OUT,
  UI_ACT_ZOOM_IN,
  UI_ACT_QUANTIZE,
  UI_ACT_DELETE,
  UI_ACT_TOGGLE_CHORD_MODE,
  UI_ACT_TOGGLE_AUTO_LOOP,
} ui_action_t;

typedef struct {
  // Per encoder mapping (0..UI_MAX_ENCODERS-1)
  ui_action_t enc_cw[2];         // clockwise
  ui_action_t enc_ccw[2];
  ui_action_t enc_shift_cw[2];
  ui_action_t enc_shift_ccw[2];
  ui_action_t enc_btn[2];
  ui_action_t enc_shift_btn[2];
} ui_actions_cfg_t;

void ui_actions_defaults(ui_actions_cfg_t* c);

/** Load from SD: /cfg/ui_actions.ngc (keys ENC0_CW=..., ENC1_SHIFT_CCW=...). */
int ui_actions_load(ui_actions_cfg_t* c, const char* path);

/** Handle encoder step for encoder index. step: +1 or -1. shift: 0/1. */
void ui_actions_on_encoder(const ui_actions_cfg_t* c, uint8_t enc, int8_t step, uint8_t shift);

/** Handle encoder button press (rising edge). shift: 0/1. */
void ui_actions_on_button(const ui_actions_cfg_t* c, uint8_t enc, uint8_t shift);

#ifdef __cplusplus
}
#endif
