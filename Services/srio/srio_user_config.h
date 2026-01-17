#pragma once

// SRIO SPI wiring for MBHP_CORE_STM32F4-style pins.
//
// Enable by adding compiler symbol: SRIO_ENABLE
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

#ifdef SRIO_ENABLE

// SPI handle used by SRIO: SPI2 (MIOS32_SPI1 → STM32 SPI2)
extern SPI_HandleTypeDef hspi2;
#define SRIO_SPI_HANDLE (&hspi2)

// 74HC165 /PL uses RC2 (PD10 = MIOS_SPI1_RC2 as defined in main.h)
#define SRIO_DIN_PL_PORT MIOS_SPI1_RC2_GPIO_Port
#define SRIO_DIN_PL_PIN  MIOS_SPI1_RC2_Pin

// 74HC595 RCLK uses RC1 (PB12 = OLED_CS_Pin in main.h - to be MIOS32 compatible)
// Note: Rename OLED_CS in CubeMX to a different pin for actual SSD OLED display
#define SRIO_DOUT_RCLK_PORT OLED_CS_GPIO_Port
#define SRIO_DOUT_RCLK_PIN  OLED_CS_Pin

// 64 inputs / 64 outputs by default (8 bytes each)
#define SRIO_DIN_BYTES  8
#define SRIO_DOUT_BYTES 8

#endif // SRIO_ENABLE
