#pragma once
#include "stm32f4xx_hal.h"

extern SPI_HandleTypeDef hspi2;

// OLED SSD1322 on SPI2
#define OLED_CS_GPIO_Port  GPIOB
#define OLED_CS_Pin        GPIO_PIN_12

#define OLED_DC_GPIO_Port  GPIOC
#define OLED_DC_Pin        GPIO_PIN_4

#define OLED_RST_GPIO_Port GPIOC
#define OLED_RST_Pin       GPIO_PIN_5
