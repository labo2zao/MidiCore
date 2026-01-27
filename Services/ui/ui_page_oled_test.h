#pragma once
#include <stdint.h>

// ============================================================================
// OLED Test UI Page - Only available when MODULE_TEST_OLED=1
// Comprehensive OLED hardware testing and visual verification
// NOT NEEDED FOR PRODUCTION
// ============================================================================
#ifdef MODULE_TEST_OLED

void ui_page_oled_test_render(uint32_t ms);
void ui_page_oled_test_on_button(uint8_t id, uint8_t pressed);
void ui_page_oled_test_on_encoder(int8_t delta);

#endif // MODULE_TEST_OLED
