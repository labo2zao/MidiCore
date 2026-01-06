#pragma once
// SRIO SPI wiring for MBHP_CORE_STM32F4-style pins.
//
// Enable by adding compiler symbol: SRIO_ENABLE
//
// This project uses the standard MIOS32/MBHP SRIO bus naming:
// - SC = SPI SCK
// - SI = SPI MOSI (to 74HC595 SER)
// - SO = SPI MISO (from 74HC165 Q7)
// - RC1 = DOUT latch (74HC595 RCLK)
// - RC2 = DIN latch/load (74HC165 /PL)
//
// According to your .ioc (MidiCore.ioc):
//   SPI2: PB13(SCK) PB14(MISO) PB15(MOSI)
//   RC1: PA15
//   RC2: PE1
//
// Adjust DIN/DOUT byte counts to your chain length.

#ifdef SRIO_ENABLE
  #include "main.h"
  // CubeMX usually declares SPI handles in spi.h
  #include "spi.h"

  // SPI2 on PB13/14/15
  #define SRIO_SPI_HANDLE (&hspi2)

  // 74HC165 /PL uses RC2
  #define SRIO_DIN_PL_PORT SRIO_RC2_GPIO_Port
  #define SRIO_DIN_PL_PIN  SRIO_RC2_Pin

  // 74HC595 RCLK uses RC1
  #define SRIO_DOUT_RCLK_PORT SRIO_RC1_GPIO_Port
  #define SRIO_DOUT_RCLK_PIN  SRIO_RC1_Pin

  // 64 inputs / 64 outputs by default
  #define SRIO_DIN_BYTES  8
  #define SRIO_DOUT_BYTES 8
#endif
