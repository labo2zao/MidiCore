// SPDX-License-Identifier: MIT
#pragma once

// SD card chip-select pin (optional).
//
// Note: This project currently doesn't depend on SD, but the SPI bus layer
// keeps a SD device slot.

#include "main.h"

// SPI peripheral used for the SD slot
extern SPI_HandleTypeDef hspi1;

// Chip select GPIO for SD
// (If you don't use SD, any free GPIO is acceptable.)
#ifndef SD_CS_GPIO_Port
#define SD_CS_GPIO_Port GPIOB
#define SD_CS_Pin       GPIO_PIN_2
#endif
