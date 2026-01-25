/**
 * @file oled_config.h
 * @brief OLED Configuration Header
 * 
 * This file provides OLED configuration compatibility.
 * Pin definitions are in oled_pins.h (software SPI).
 */

#ifndef OLED_CONFIG_H
#define OLED_CONFIG_H

#include "oled_pins.h"

// OLED Display Configuration
#define OLED_WIDTH   256
#define OLED_HEIGHT  64
#define OLED_DRIVER  "SSD1322"

// Software SPI Configuration (bit-bang)
// Pins defined in oled_pins.h:
//   - RST = PA8 (Reset control)
//   - SCL = PC8 (Clock, bit-banged)
//   - SDA = PC11 (Data, bit-banged)
//   - CS = Hardwired to GND (not connected to STM32)

#endif // OLED_CONFIG_H
