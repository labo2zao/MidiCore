#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Font/brightness constants for compatibility
#define GFX_FONT_NORMAL 15
#define GFX_FONT_SMALL  12
#define GFX_FONT_LARGE  15

void ui_gfx_set_fb(uint8_t* fb, uint16_t w, uint16_t h);
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

#ifdef __cplusplus
}
#endif
