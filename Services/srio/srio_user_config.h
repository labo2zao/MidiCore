#pragma once

// SRIO SPI wiring for MBHP_CORE_STM32F4-style pins.
//
// Enable by adding compiler symbol: SRIO_ENABLE
// MODULE_TEST_SRIO automatically enables SRIO when running the module test.
//
// IMPORTANT: MIOS32 SPI numbering is different from STM32 SPI numbering!
// SRIO uses MIOS32_SPI1 which maps to:
//   - SPI2 for SCK/MISO/MOSI: PB13(SCK) PB14(MISO) PB15(MOSI)
//   - RC1 (74HC595 RCLK for DOUT): PB12 (OLED_CS_Pin in main.h - rename OLED_CS in CubeMX for actual SSD display)
//   - RC2 (74HC165 /PL for DIN): PD10 (MIOS_SPI1_RC2_Pin in main.h)
//
// This is different from AINSER64 which uses MIOS32_SPI0 (STM32 SPI3, RC=PA14)

#include "main.h"
#include "stm32f4xx_hal.h"

#if defined(MODULE_TEST_SRIO) && !defined(SRIO_ENABLE)
#define SRIO_ENABLE 1
#endif

#ifdef SRIO_ENABLE

// SPI handle used by SRIO: SPI2 (MIOS32_SPI1 → STM32 SPI2)
extern SPI_HandleTypeDef hspi2;
#define SRIO_SPI_HANDLE (&hspi2)

#ifndef SRIO_SPI_PRESCALER
// Match MIOS32 default: CLK1_PHASE1 with prescaler 128.
#define SRIO_SPI_PRESCALER SPI_BAUDRATEPRESCALER_128
#endif

#ifndef SRIO_APPLY_SPI_CONFIG
// Set to 0 if you must avoid changing SPI settings at runtime.
#define SRIO_APPLY_SPI_CONFIG 1
#endif

#ifndef SRIO_SPI_CPOL
#define SRIO_SPI_CPOL SPI_POLARITY_HIGH
#endif

#ifndef SRIO_SPI_CPHA
#define SRIO_SPI_CPHA SPI_PHASE_2EDGE
#endif

// 74HC165 /PL uses RC2 and 74HC595 RCLK uses RC1.
// Default to MIOS32-compatible pins; opt into explicit SRIO_RC pins via SRIO_USE_EXPLICIT_PINS.
#if defined(SRIO_USE_EXPLICIT_PINS) && defined(SRIO_RC1_GPIO_Port) && defined(SRIO_RC1_Pin) \
  && defined(SRIO_RC2_GPIO_Port) && defined(SRIO_RC2_Pin)
#define SRIO_DIN_PL_PORT SRIO_RC2_GPIO_Port
#define SRIO_DIN_PL_PIN  SRIO_RC2_Pin
#define SRIO_DOUT_RCLK_PORT SRIO_RC1_GPIO_Port
#define SRIO_DOUT_RCLK_PIN  SRIO_RC1_Pin
#else
#define SRIO_DIN_PL_PORT MIOS_SPI1_RC2_GPIO_Port
#define SRIO_DIN_PL_PIN  MIOS_SPI1_RC2_Pin
#define SRIO_DOUT_RCLK_PORT OLED_CS_GPIO_Port
#define SRIO_DOUT_RCLK_PIN  OLED_CS_Pin
#endif

// 64 inputs / 64 outputs by default (8 bytes each)
#define SRIO_DIN_BYTES  8
#define SRIO_DOUT_BYTES 8

#endif // SRIO_ENABLE
