/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define J10B_D3_Pin GPIO_PIN_2
#define J10B_D3_GPIO_Port GPIOE
#define J10B_D4_Pin GPIO_PIN_4
#define J10B_D4_GPIO_Port GPIOE
#define J10B_D5_Pin GPIO_PIN_5
#define J10B_D5_GPIO_Port GPIOE
#define J10B_D6_Pin GPIO_PIN_6
#define J10B_D6_GPIO_Port GPIOE
#define J10B_D0_Pin GPIO_PIN_13
#define J10B_D0_GPIO_Port GPIOC
#define J10B_D1_Pin GPIO_PIN_14
#define J10B_D1_GPIO_Port GPIOC
#define J10B_D2_Pin GPIO_PIN_15
#define J10B_D2_GPIO_Port GPIOC
#define BANK_A_Pin GPIO_PIN_0
#define BANK_A_GPIO_Port GPIOC
#define BANK_B_Pin GPIO_PIN_1
#define BANK_B_GPIO_Port GPIOC
#define BANK_C_Pin GPIO_PIN_2
#define BANK_C_GPIO_Port GPIOC
#define AIN_CS_Pin GPIO_PIN_3
#define AIN_CS_GPIO_Port GPIOC
#define User_Button_Pin GPIO_PIN_0
#define User_Button_GPIO_Port GPIOA
#define MIOS_AN3_Pin GPIO_PIN_4
#define MIOS_AN3_GPIO_Port GPIOA
// OLED pin definitions moved to Config/oled_pins.h (software SPI on PA8/PC8/PC11)
#define MIOS_AN6_Pin GPIO_PIN_0
#define MIOS_AN6_GPIO_Port GPIOB
#define MIOS_AN7_Pin GPIO_PIN_1
#define MIOS_AN7_GPIO_Port GPIOB
#define MIOS_SPI0_RC1_Pin GPIO_PIN_2
#define MIOS_SPI0_RC1_GPIO_Port GPIOB
#define J10B_D15_Pin GPIO_PIN_7
#define J10B_D15_GPIO_Port GPIOE
#define J10A_D0_Pin GPIO_PIN_8
#define J10A_D0_GPIO_Port GPIOE
#define J10A_D1_Pin GPIO_PIN_9
#define J10A_D1_GPIO_Port GPIOE
#define J10A_D2_Pin GPIO_PIN_10
#define J10A_D2_GPIO_Port GPIOE
#define J10A_D3_Pin GPIO_PIN_11
#define J10A_D3_GPIO_Port GPIOE
#define J10A_D4_Pin GPIO_PIN_12
#define J10A_D4_GPIO_Port GPIOE
#define J10A_D6_Pin GPIO_PIN_14
#define J10A_D6_GPIO_Port GPIOE
#define J10A_D7_Pin GPIO_PIN_15
#define J10A_D7_GPIO_Port GPIOE
#define IOS_IIC0_SD_Pin GPIO_PIN_11
#define IOS_IIC0_SD_GPIO_Port GPIOB
#define OLED_CS_Pin GPIO_PIN_12
#define OLED_CS_GPIO_Port GPIOB
#define MIOS_SPI1_SCK_Pin GPIO_PIN_13
#define MIOS_SPI1_SCK_GPIO_Port GPIOB
#define MIOS_SPI1_MISO_Pin GPIO_PIN_14
#define MIOS_SPI1_MISO_GPIO_Port GPIOB
#define MIOS_SPI1_S0_Pin GPIO_PIN_15
#define MIOS_SPI1_S0_GPIO_Port GPIOB
#define MIDI2_OUT_Pin GPIO_PIN_8
#define MIDI2_OUT_GPIO_Port GPIOD
#define USART3_RX_Pin GPIO_PIN_9
#define USART3_RX_GPIO_Port GPIOD
#define MIOS_SPI1_RC2_Pin GPIO_PIN_10
#define MIOS_SPI1_RC2_GPIO_Port GPIOD
#define MIOS_SPI0_RC2_Pin GPIO_PIN_11
#define MIOS_SPI0_RC2_GPIO_Port GPIOD
#define GREEN_LED_Pin GPIO_PIN_12
#define GREEN_LED_GPIO_Port GPIOD
#define ORANGE_LED_Pin GPIO_PIN_13
#define ORANGE_LED_GPIO_Port GPIOD
#define RED_LED_Pin GPIO_PIN_14
#define RED_LED_GPIO_Port GPIOD
#define BLUE_LED_Pin GPIO_PIN_15
#define BLUE_LED_GPIO_Port GPIOD
#define MIDI3_OUT_Pin GPIO_PIN_6
#define MIDI3_OUT_GPIO_Port GPIOC
#define USB_DM_Pin GPIO_PIN_11
#define USB_DM_GPIO_Port GPIOA
#define USB_DP_Pin GPIO_PIN_12
#define USB_DP_GPIO_Port GPIOA
#define MIOS_SPI2_RC1_Pin GPIO_PIN_14
#define MIOS_SPI2_RC1_GPIO_Port GPIOA
#define SRIO_RC1_Pin GPIO_PIN_15
#define SRIO_RC1_GPIO_Port GPIOA
#define MIDI4_OUT_Pin GPIO_PIN_12
#define MIDI4_OUT_GPIO_Port GPIOC
#define MIOS_CAN_RX_Pin GPIO_PIN_0
#define MIOS_CAN_RX_GPIO_Port GPIOD
#define MIOS_CAN_TX_Pin GPIO_PIN_1
#define MIOS_CAN_TX_GPIO_Port GPIOD
#define MIDI4_IN_Pin GPIO_PIN_2
#define MIDI4_IN_GPIO_Port GPIOD
#define MUX_S0_Pin GPIO_PIN_3
#define MUX_S0_GPIO_Port GPIOD
#define MUX_S1_Pin GPIO_PIN_4
#define MUX_S1_GPIO_Port GPIOD
#define MUX_S2_Pin GPIO_PIN_5
#define MUX_S2_GPIO_Port GPIOD
#define MIOS_SPI2_SCL_Pin GPIO_PIN_3
#define MIOS_SPI2_SCL_GPIO_Port GPIOB
#define MIOS_SPI2_MISO_Pin GPIO_PIN_4
#define MIOS_SPI2_MISO_GPIO_Port GPIOB
#define MIOS_SPI2_MOSI_Pin GPIO_PIN_5
#define MIOS_SPI2_MOSI_GPIO_Port GPIOB
#define MIOS_IIC1_SCL_Pin GPIO_PIN_6
#define MIOS_IIC1_SCL_GPIO_Port GPIOB
#define MIDI3_IN_Pin GPIO_PIN_7
#define MIDI3_IN_GPIO_Port GPIOB
#define MIOS_SPI2_RC2_Pin GPIO_PIN_8
#define MIOS_SPI2_RC2_GPIO_Port GPIOB
#define MIOS_IIC1_SDA_Pin GPIO_PIN_9
#define MIOS_IIC1_SDA_GPIO_Port GPIOB
#define SRIO_RC2_Pin GPIO_PIN_1
#define SRIO_RC2_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
