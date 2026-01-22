#pragma once
#include "stm32f4xx_hal.h"

// OLED SSD1322 - Software SPI (bit-bang)
// Pin assignments from LoopA hardware (MIOS32 compatible):
// In MIOS32 LoopA, the LCD pins are used as follows:
// LCD_RS (Data/Command)    -> PA8  (this is the DC signal)
// LCD_E (Clock)            -> PC8  (SCL)
// LCD_RW (MOSI/Data)       -> PC11 (SDA)
// RST uses the DC pin initially, then DC takes over
// CS is hardwired to GND (not connected to STM32)

#define OLED_DC_GPIO_Port  GPIOA
#define OLED_DC_Pin        GPIO_PIN_8  // LCD_RS = Data/Command select

#define OLED_SCL_GPIO_Port GPIOC
#define OLED_SCL_Pin       GPIO_PIN_8  // LCD_E = Clock

#define OLED_SDA_GPIO_Port GPIOC
#define OLED_SDA_Pin       GPIO_PIN_11 // LCD_RW = Data

// RST shares pin with DC (we pulse it during init, then use as DC)
#define OLED_RST_GPIO_Port OLED_DC_GPIO_Port
#define OLED_RST_Pin       OLED_DC_Pin
