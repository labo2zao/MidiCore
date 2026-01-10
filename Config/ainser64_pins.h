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

// Keep main.h for the STM32Cube-generated GPIO definitions, but avoid relying
// on its AIN_CS_* macros (they can point to an unconnected pin in some
// CubeMX templates). We define our own canonical macros below.
#include "main.h"

// SPI peripheral used for AINSER64
extern SPI_HandleTypeDef hspi3;

// Chip Select GPIO for MCP3208 (shared with 74HC595 RCLK on AINSER64)
// This should match the physical wire used for the AINSER64 "RC" line.
#ifndef AIN_CS_PORT
#define AIN_CS_PORT GPIOA
#define AIN_CS_PIN  GPIO_PIN_15
#endif

// Backwards compatible aliases (avoid touching older code paths)
#ifndef AIN_CS_GPIO_Port
#define AIN_CS_GPIO_Port AIN_CS_PORT
#endif
#ifndef AIN_CS_Pin
#define AIN_CS_Pin AIN_CS_PIN
#endif
