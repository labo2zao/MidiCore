/**
 * @file test_oled_mirror.h
 * @brief Debug output mirroring to OLED display
 * 
 * Mirrors test debug output (dbg_print/dbg_printf) to OLED display
 * for visual debugging without needing UART connection.
 * Acts as a GDB-style debug mirror on OLED.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define OLED_MIRROR_LINES 8        // Number of text lines on OLED
#define OLED_MIRROR_LINE_LEN 32    // Characters per line

/**
 * @brief Initialize OLED debug mirror
 * Must be called after UI/OLED initialization
 */
void oled_mirror_init(void);

/**
 * @brief Enable/disable OLED mirroring
 * @param enabled 1 to enable, 0 to disable
 */
void oled_mirror_set_enabled(uint8_t enabled);

/**
 * @brief Check if OLED mirroring is enabled
 * @return 1 if enabled, 0 if disabled
 */
uint8_t oled_mirror_is_enabled(void);

/**
 * @brief Print string to OLED mirror
 * @param str String to print
 */
void oled_mirror_print(const char* str);

/**
 * @brief Print formatted string to OLED mirror
 * @param format Printf-style format string
 * @param ... Variable arguments
 */
void oled_mirror_printf(const char* format, ...);

/**
 * @brief Clear OLED mirror display
 */
void oled_mirror_clear(void);

/**
 * @brief Update OLED display with mirrored content
 * Call periodically (e.g., every 100ms) or after prints
 */
void oled_mirror_update(void);

/**
 * @brief Get number of lines currently displayed
 */
uint8_t oled_mirror_get_line_count(void);

#ifdef __cplusplus
}
#endif
