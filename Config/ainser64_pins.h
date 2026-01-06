#pragma once
#include "stm32f4xx_hal.h"

// ===== SPI handles (adjust names to match CubeMX generated code) =====
extern SPI_HandleTypeDef hspi1; // SD + AINSER64 (MCP3208)
extern SPI_HandleTypeDef hspi2; // OLED SSD1322

// ===== SPI1 chip selects (GPIO) =====
#define SD_CS_GPIO_Port GPIOB
#define SD_CS_Pin       GPIO_PIN_2

#define AIN_CS_GPIO_Port GPIOC
#define AIN_CS_Pin       GPIO_PIN_3

// ===== AINSER64 bank select (74HC138 : 3->8) =====
#define BANK_A_GPIO_Port GPIOC
#define BANK_A_Pin       GPIO_PIN_0
#define BANK_B_GPIO_Port GPIOC
#define BANK_B_Pin       GPIO_PIN_1
#define BANK_C_GPIO_Port GPIOC
#define BANK_C_Pin       GPIO_PIN_2

// ===== AINSER64 mux select (4051 : 8->1) =====
#define MUX_S0_GPIO_Port GPIOD
#define MUX_S0_Pin       GPIO_PIN_3
#define MUX_S1_GPIO_Port GPIOD
#define MUX_S1_Pin       GPIO_PIN_4
#define MUX_S2_GPIO_Port GPIOD
#define MUX_S2_Pin       GPIO_PIN_5

// ===== Timing (tune later) =====
#define AIN_BANK_SETTLE_US   500u   // bank power-gating settle time
#define AIN_MUX_SETTLE_US     20u   // mux settle time per step
