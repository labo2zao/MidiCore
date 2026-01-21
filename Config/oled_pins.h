#pragma once
#include "stm32f4xx_hal.h"

// OLED SSD1322 - Software SPI (bit-bang)
// Pin assignments from LoopA hardware (MIOS32 compatible):
// RST (Reset)              -> PA8
// SCL (Clock)              -> PC8
// SDA (MOSI/Data)          -> PC11
// CS is hardwired to GND (not connected to STM32)

#define OLED_RST_GPIO_Port GPIOA
#define OLED_RST_Pin       GPIO_PIN_8

#define OLED_SCL_GPIO_Port GPIOC
#define OLED_SCL_Pin       GPIO_PIN_8

#define OLED_SDA_GPIO_Port GPIOC
#define OLED_SDA_Pin       GPIO_PIN_11

// DC (Data/Command) uses same pin as SDA
// This is MIOS32 convention: multiplex SDA for both data and DC signaling
#define OLED_DC_GPIO_Port  OLED_SDA_GPIO_Port
#define OLED_DC_Pin        OLED_SDA_Pin
