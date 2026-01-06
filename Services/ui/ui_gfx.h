#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void ui_gfx_set_fb(uint8_t* fb, uint16_t w, uint16_t h);
void ui_gfx_clear(uint8_t gray);
void ui_gfx_pixel(int x, int y, uint8_t gray);
void ui_gfx_rect(int x, int y, int w, int h, uint8_t gray);
void ui_gfx_text(int x, int y, const char* s, uint8_t gray);

#ifdef __cplusplus
}
#endif
