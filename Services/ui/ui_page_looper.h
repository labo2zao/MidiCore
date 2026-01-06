#pragma once
#include <stdint.h>
void ui_page_looper_render(uint32_t now_ms);
void ui_page_looper_on_button(uint8_t id, uint8_t pressed);
void ui_page_looper_on_encoder(int8_t delta);

uint8_t ui_page_looper_get_track(void);
