// SPDX-License-Identifier: MIT
#pragma once

// AINSER64 (MCP3208 + 74HC595) pin mapping.
//
// This project uses the SPI bus abstraction (Hal/spi_bus.*).
// The AINSER64 uses:
//   - SPI3 for SCK/MISO/MOSI (PB3/PB4/PB5 with CubeMX default)
//   - A dedicated GPIO for CS (default: PA15, which matches many MIOS32 STM32F4 core J19 mappings)
//
// If your wiring differs, change the AIN_CS_* defines below.

#include "main.h"

// SPI peripheral used for AINSER64
extern SPI_HandleTypeDef hspi3;

// Chip Select GPIO for MCP3208
#ifndef AIN_CS_GPIO_Port
#define AIN_CS_GPIO_Port GPIOA
#define AIN_CS_Pin GPIO_PIN_15
#endif
