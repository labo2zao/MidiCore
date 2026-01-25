// SPDX-License-Identifier: MIT
#pragma once

// SD card SPI pin configuration
//
// Hardware connections (MIOS32 STM32F4 standard):
// - PA5 = CLK  (SPI1 SCK)
// - PA6 = DAT0 (SPI1 MISO) 
// - PA7 = CMD  (SPI1 MOSI)
// - PA4 = CS   (Chip Select - MIOS32 default for SD card)
// - PB2 = CD   (Card Detect - optional, hardware sense)
//
// Note: SD cards in SPI mode require a CS pin for slave select.
// PB2 is Card Detect (CD) for sensing card insertion, not CS!

#include "main.h"

// SPI peripheral used for the SD slot
extern SPI_HandleTypeDef hspi1;

// Chip Select pin (required for SPI communication)
// MIOS32 default: PA4 for SD card CS
#ifndef SD_CS_GPIO_Port
#define SD_CS_GPIO_Port GPIOA
#define SD_CS_Pin       GPIO_PIN_4
#endif

// Card Detect pin (optional - for sensing card insertion)
// Not currently used by driver, but available for future enhancement
// User hardware: PB2 = CD (Card Detect)
#ifndef SD_CD_GPIO_Port
#define SD_CD_GPIO_Port GPIOB
#define SD_CD_Pin       GPIO_PIN_2
#endif
