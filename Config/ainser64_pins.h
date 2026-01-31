// SPDX-License-Identifier: MIT
#pragma once

// AINSER64 (MCP3208 + 74HC595) pin mapping.
//
// IMPORTANT: MidiCore SPI numbering is different from STM32 SPI numbering!
// AINSER64 uses MIOS32_SPI0 which maps to:
//   - SPI3 for SCK/MISO/MOSI (PB3/PB4/PB5)
//   - RC (CS): PA15 (MIOS_SPI2_RC1 in main.h - shared CS for MCP3208 and 74HC595 RCLK)
//
// This project uses the SPI bus abstraction (Hal/spi_bus.*).
// If your wiring differs, change the AIN_CS_* defines below.

// Keep main.h for the STM32Cube-generated GPIO definitions, but avoid relying
// on its AIN_CS_* macros (they can point to an unconnected pin in some
// CubeMX templates). We define our own canonical macros below.
#include "main.h"

// SPI peripheral used for AINSER64 (MIOS32_SPI0 → STM32 SPI3)
extern SPI_HandleTypeDef hspi3;

// Chip Select GPIO for MCP3208 (shared with 74HC595 RCLK on AINSER64)
// This maps to MIOS32_SPI0_RC in MidiCore terminology
// Using PA15 which is MIOS_SPI2_RC1 in main.h (note: MidiCore numbering ≠ STM32 numbering)
#ifndef AIN_CS_PORT
#define AIN_CS_PORT MIOS_SPI2_RC1_GPIO_Port
#define AIN_CS_PIN  MIOS_SPI2_RC1_Pin
#endif

// Backwards compatible aliases (avoid touching older code paths)
#ifndef AIN_CS_GPIO_Port
#define AIN_CS_GPIO_Port AIN_CS_PORT
#endif
#ifndef AIN_CS_Pin
#define AIN_CS_Pin AIN_CS_PIN
#endif
