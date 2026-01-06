#include <stdint.h>

// Weak stubs so ui_actions can call these safely even if not implemented yet.
// Provide real implementations in your UI module later.

__attribute__((weak)) void ui_prev_page(void) {}
__attribute__((weak)) void ui_next_page(void) {}
__attribute__((weak)) void ui_cursor_move(int8_t delta) { (void)delta; }
__attribute__((weak)) void ui_zoom(int8_t delta) { (void)delta; }
__attribute__((weak)) void ui_quantize(void) {}
__attribute__((weak)) void ui_delete(void) {}
__attribute__((weak)) void ui_toggle_chord_mode(void) {}
__attribute__((weak)) void ui_toggle_auto_loop(void) {}
