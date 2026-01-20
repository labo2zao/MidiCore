#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file ui_page_config.h
 * @brief Config Editor UI Page - SD card configuration file editor
 * 
 * Provides a simple editor for .ngc configuration files on the SD card.
 * Allows browsing, viewing, and editing DIN, AINSER, and AIN module settings.
 */

void ui_page_config_render(uint32_t now_ms);
void ui_page_config_on_button(uint8_t id, uint8_t pressed);
void ui_page_config_on_encoder(int8_t delta);

#ifdef __cplusplus
}
#endif
