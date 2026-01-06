#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void ui_prev_page(void);
void ui_next_page(void);
void ui_cursor_move(int8_t d);
void ui_zoom(int8_t d);
void ui_quantize(void);
void ui_delete(void);
void ui_toggle_chord_mode(void);
void ui_toggle_auto_loop(void);
#ifdef __cplusplus
}
#endif
