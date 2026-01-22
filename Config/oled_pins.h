#pragma once
#include "stm32f4xx_hal.h"

// OLED SSD1322 - Software SPI (bit-bang)
// Pin assignments from LoopA hardware (MIOS32 compatible):
// In MIOS32 LoopA, the LCD pins are used as follows:
// LCD_RS (Data/Command)    -> PA8  (this is the DC signal, also called RS)
// LCD_E (Clock)            -> PC8  (SCL)
// LCD_RW (MOSI/Data)       -> PC11 (SDA)
// 
// RESET CIRCUIT:
// RST pin has a hardware RC (resistor-capacitor) circuit between 0V and 3.3V
// This automatically generates a reset pulse on power-up:
// - At power-on, capacitor is discharged → RST = 0V (reset asserted)
// - Capacitor charges through resistor → RST gradually rises to 3.3V
// - This creates the required reset pulse without MCU control
// The STM32 does NOT control RST - it's purely hardware RC timing
//
// CS is hardwired to GND (not connected to STM32)

#define OLED_DC_GPIO_Port  GPIOA
#define OLED_DC_Pin        GPIO_PIN_8  // LCD_RS = Data/Command select (DC/RS pin)

#define OLED_SCL_GPIO_Port GPIOC
#define OLED_SCL_Pin       GPIO_PIN_8  // LCD_E = Clock

#define OLED_SDA_GPIO_Port GPIOC
#define OLED_SDA_Pin       GPIO_PIN_11 // LCD_RW = Data

// Note: No RST pin controlled by STM32 (hardware RC circuit handles it)
