// SPDX-License-Identifier: MIT
#pragma once

#include "main.h"

/**
 * @file footswitch_pins.h
 * @brief Footswitch GPIO pin configuration
 * 
 * Defines GPIO pins for 8 footswitch inputs.
 * Supports two input methods:
 * - GPIO mode: Direct pin reading (default, simpler)
 * - SRIO mode: Bit-bang SPI with 74HC165 shift register
 * 
 * Select method with FOOTSWITCH_USE_SRIO define:
 * - Undefined or 0: GPIO mode (default)
 * - 1: SRIO bit-bang mode
 */

// =============================================================================
// GPIO Mode Configuration (Default)
// =============================================================================
// Uses 8 dedicated GPIO pins from J10A and J10B connectors
// Pins configured as inputs with internal pull-ups
// Active-low logic: pressed = LOW, released = HIGH

#define FOOTSWITCH_GPIO_FS0_PORT   GPIOE
#define FOOTSWITCH_GPIO_FS0_PIN    GPIO_PIN_2    // J10B_D3 (PE2)

#define FOOTSWITCH_GPIO_FS1_PORT   GPIOE
#define FOOTSWITCH_GPIO_FS1_PIN    GPIO_PIN_4    // J10B_D4 (PE4)

#define FOOTSWITCH_GPIO_FS2_PORT   GPIOE
#define FOOTSWITCH_GPIO_FS2_PIN    GPIO_PIN_5    // J10B_D5 (PE5)

#define FOOTSWITCH_GPIO_FS3_PORT   GPIOE
#define FOOTSWITCH_GPIO_FS3_PIN    GPIO_PIN_6    // J10B_D6 (PE6)

#define FOOTSWITCH_GPIO_FS4_PORT   GPIOB
#define FOOTSWITCH_GPIO_FS4_PIN    GPIO_PIN_8    // J10A_D0 (PB8)

#define FOOTSWITCH_GPIO_FS5_PORT   GPIOB
#define FOOTSWITCH_GPIO_FS5_PIN    GPIO_PIN_9    // J10A_D1 (PB9)

#define FOOTSWITCH_GPIO_FS6_PORT   GPIOB
#define FOOTSWITCH_GPIO_FS6_PIN    GPIO_PIN_10   // J10A_D2 (PB10)

#define FOOTSWITCH_GPIO_FS7_PORT   GPIOB
#define FOOTSWITCH_GPIO_FS7_PIN    GPIO_PIN_11   // J10A_D3 (PB11)

// =============================================================================
// SRIO Bit-Bang Mode Configuration (Alternative)
// =============================================================================
// Uses bit-bang software SPI with 74HC165 shift register
// Requires external 74HC165 hardware
// Independent from main SRIO bus

#define FOOTSWITCH_SRIO_SCK_PORT   GPIOB
#define FOOTSWITCH_SRIO_SCK_PIN    GPIO_PIN_12   // J10A_D4 (PB12)

#define FOOTSWITCH_SRIO_MISO_PORT  GPIOB
#define FOOTSWITCH_SRIO_MISO_PIN   GPIO_PIN_14   // J10A_D6 (PB14)

#define FOOTSWITCH_SRIO_PL_PORT    GPIOB
#define FOOTSWITCH_SRIO_PL_PIN     GPIO_PIN_15   // J10A_D7 (PB15)

// =============================================================================
// Constants
// =============================================================================

#define FOOTSWITCH_COUNT           8              // Number of footswitches
#define FOOTSWITCH_DEBOUNCE_MS     30             // Debounce time in milliseconds
#define FOOTSWITCH_SCAN_RATE_MS    10             // Scan rate in milliseconds (100 Hz)
