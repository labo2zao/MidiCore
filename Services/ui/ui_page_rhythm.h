#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize rhythm trainer UI page
 */
void ui_page_rhythm_init(void);

/**
 * @brief Update rhythm trainer UI page
 * @param force_redraw Force full screen redraw
 */
void ui_page_rhythm_update(uint8_t force_redraw);

/**
 * @brief Handle button press on rhythm trainer page
 * @param button Button number (0-4)
 */
void ui_page_rhythm_button(uint8_t button);

/**
 * @brief Handle encoder change on rhythm trainer page
 * @param delta Encoder delta value
 */
void ui_page_rhythm_encoder(int8_t delta);

#ifdef __cplusplus
}
#endif
