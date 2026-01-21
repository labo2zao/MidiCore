#pragma once
#include "stm32f4xx_hal.h"

extern SPI_HandleTypeDef hspi2;

// OLED SSD1322 on SPI2
// Pin assignments from LoopA hardware:
// LCD_RS (DC/Data-Command) -> PA8
// LCDE1 (CS/Chip Select)   -> PC8
// LCD_RW (RST/Reset)        -> PC11

#define OLED_DC_GPIO_Port  GPIOA
#define OLED_DC_Pin        GPIO_PIN_8

#define OLED_CS_GPIO_Port  GPIOC
#define OLED_CS_Pin        GPIO_PIN_8

#define OLED_RST_GPIO_Port GPIOC
#define OLED_RST_Pin       GPIO_PIN_11
