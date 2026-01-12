#pragma once

// SRIO SPI wiring for MBHP_CORE_STM32F4-style pins.
//
// Enable by adding compiler symbol: SRIO_ENABLE
//
// According to MidiCore.ioc:
//   SPI2: PB13(SCK) PB14(MISO) PB15(MOSI)
//   RC1:  PA15 (SRIO_RC1_* in main.h)
//   RC2:  PE1  (SRIO_RC2_* in main.h)

#include "main.h"
#include "stm32f4xx_hal.h"

#ifdef SRIO_ENABLE

// SPI handle used by SRIO: SPI2
extern SPI_HandleTypeDef hspi2;
#define SRIO_SPI_HANDLE (&hspi2)

// 74HC165 /PL uses RC2
#define SRIO_DIN_PL_PORT SRIO_RC2_GPIO_Port
#define SRIO_DIN_PL_PIN  SRIO_RC2_Pin

// 74HC595 RCLK uses RC1
#define SRIO_DOUT_RCLK_PORT SRIO_RC1_GPIO_Port
#define SRIO_DOUT_RCLK_PIN  SRIO_RC1_Pin

// 64 inputs / 64 outputs by default (8 bytes each)
#define SRIO_DIN_BYTES  8
#define SRIO_DOUT_BYTES 8

#endif // SRIO_ENABLE
