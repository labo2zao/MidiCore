/**
 * @file oled.h
 * @brief OLED Display Driver Wrapper
 * 
 * This header selects the appropriate OLED driver based on OLED_DRIVER_SSD1306 configuration.
 * - SSD1322: 256×64 grayscale display (default MidiCore)
 * - SSD1306: 128×64 monochrome display (LoopA/MBHP compatible)
 * 
 * Both drivers expose the same API:
 * - void oled_init(void)
 * - uint8_t* oled_framebuffer(void)
 * - void oled_flush(void)
 * - void oled_clear(void)
 * - OLED_W, OLED_H macros for dimensions
 */

#pragma once

#include "Config/module_config.h"

#if OLED_DRIVER_SSD1306
  // SSD1306: 128×64 monochrome (1bpp), LoopA compatible
  #include "Hal/oled_ssd1306/oled_ssd1306.h"
#else
  // SSD1322: 256×64 grayscale (4bpp), MidiCore default
  #include "Hal/oled_ssd1322/oled_ssd1322.h"
#endif
