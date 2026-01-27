/**
 * @file ui_page_modules.h
 * @brief Module Control UI Page
 * 
 * Provides a hierarchical menu system for accessing and configuring all
 * firmware modules via the OLED display and rotary encoders.
 * 
 * Features:
 * - Browse modules by category
 * - Enable/disable modules
 * - Edit module parameters
 * - Save/load configurations
 * - Status display
 * 
 * Navigation:
 * - Encoder 1: Navigate menu items
 * - Encoder 1 button: Enter/exit submenus, toggle edit mode
 * - Encoder 2: Edit parameter values (when in edit mode)
 * - Button 1: Enable/disable current module
 * - Button 2: Save configuration
 * - Button 3: Load configuration
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize modules UI page
 */
void ui_page_modules_init(void);

/**
 * @brief Render modules UI page
 * @param fb Framebuffer pointer
 * @param w Framebuffer width
 * @param h Framebuffer height
 */
void ui_page_modules_render(uint8_t* fb, int w, int h);

/**
 * @brief Handle encoder input
 * @param enc_id Encoder ID (0 or 1)
 * @param delta Encoder delta (-127 to 127)
 */
void ui_page_modules_on_encoder(uint8_t enc_id, int8_t delta);

/**
 * @brief Handle button input
 * @param btn_id Button ID
 * @param pressed 1 if pressed, 0 if released
 */
void ui_page_modules_on_button(uint8_t btn_id, uint8_t pressed);

/**
 * @brief Page tick (called every 20ms)
 */
void ui_page_modules_tick(void);

#ifdef __cplusplus
}
#endif
