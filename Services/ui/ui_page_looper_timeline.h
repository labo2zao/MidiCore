#pragma once
#include <stdint.h>
void ui_page_looper_timeline_render(uint32_t now_ms);
void ui_page_looper_timeline_on_button(uint8_t id, uint8_t pressed);
void ui_page_looper_timeline_on_encoder(int8_t delta);

uint8_t ui_page_looper_timeline_get_track(void);

void ui_page_looper_timeline_zoom_in(void);
void ui_page_looper_timeline_zoom_out(void);
