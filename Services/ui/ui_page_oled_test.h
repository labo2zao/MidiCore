#pragma once
#include <stdint.h>

void ui_page_oled_test_render(uint32_t ms);
void ui_page_oled_test_on_button(uint8_t id, uint8_t pressed);
void ui_page_oled_test_on_encoder(int8_t delta);
