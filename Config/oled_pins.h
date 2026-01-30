#pragma once
#include "stm32f4xx_hal.h"

// OLED SSD1322 - Software SPI (bit-bang)
// Pin assignments from LoopA hardware (MidiCore compatible):
// 
// Only 3 signals connected from STM32 to OLED:
// LCD_RS (Data/Command)    -> PA8  (this is the DC signal, also called RS)
// LCD_E (Clock)            -> PC8  (SCL)
// LCD_RW (MOSI/Data)       -> PC11 (SDA)
// 
// RESET CIRCUIT (on OLED display module, NOT on STM32 board):
// The OLED display module has its own on-board RC (resistor-capacitor) circuit
// that automatically generates a reset pulse when power is applied:
// - At power-on, capacitor is discharged → RST = 0V (reset asserted)
// - Capacitor charges through resistor → RST gradually rises to 3.3V
// - This creates the required reset pulse without any MCU control
// The STM32 board does NOT have any RST connection to the OLED.
//
// CS is hardwired to GND on the OLED module (not connected to STM32)
//
// Summary: Only 3 wires from STM32 to OLED (DC, SCL, SDA) + Power/GND

#define OLED_DC_GPIO_Port  GPIOA
#define OLED_DC_Pin        GPIO_PIN_8  // LCD_RS = Data/Command select (DC/RS pin)

#define OLED_SCL_GPIO_Port GPIOC
#define OLED_SCL_Pin       GPIO_PIN_8  // LCD_E = Clock

#define OLED_SDA_GPIO_Port GPIOC
#define OLED_SDA_Pin       GPIO_PIN_11 // LCD_RW = Data

// Note: No RST pin from STM32 (OLED module has on-board RC reset circuit)
