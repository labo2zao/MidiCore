#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Font selection constants
#define UI_FONT_5X7  0  // Original small font (5x7, 6px spacing)
#define UI_FONT_8X8  1  // New larger font (8x8, 9px spacing)

// Font/brightness constants for compatibility
#define GFX_FONT_NORMAL 15
#define GFX_FONT_SMALL  12
#define GFX_FONT_LARGE  15

void ui_gfx_set_fb(uint8_t* fb, uint16_t w, uint16_t h);
void ui_gfx_set_font(uint8_t font_id);  // Set current font (UI_FONT_5X7 or UI_FONT_8X8)
void ui_gfx_clear(uint8_t gray);
void ui_gfx_pixel(int x, int y, uint8_t gray);
void ui_gfx_rect(int x, int y, int w, int h, uint8_t gray);
void ui_gfx_text(int x, int y, const char* s, uint8_t gray);

// Additional drawing functions for rhythm page
void ui_gfx_fill_rect(int x, int y, int w, int h, uint8_t gray);
void ui_gfx_hline(int x, int y, int w, uint8_t gray);
void ui_gfx_vline(int x, int y, int h, uint8_t gray);

// New drawing functions for enhanced OLED tests
void ui_gfx_circle(int cx, int cy, int radius, uint8_t gray);
void ui_gfx_line(int x0, int y0, int x1, int y1, uint8_t gray);

// Advanced graphics primitives for UI elements
void ui_gfx_filled_circle(int cx, int cy, int radius, uint8_t gray);
void ui_gfx_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint8_t gray);
void ui_gfx_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint8_t gray);
void ui_gfx_arc(int cx, int cy, int radius, int start_angle, int end_angle, uint8_t gray);

#ifdef __cplusplus
}
#endif
