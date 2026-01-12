#pragma once
// Include main.h for portable STM32 HAL (F4/F7/H7 compatibility)
#include "main.h"
#include "cmsis_os2.h"

typedef enum {
  SPIBUS_DEV_SD = 0,
  SPIBUS_DEV_AIN,
  SPIBUS_DEV_OLED,
} spibus_dev_t;

void spibus_init(void);
HAL_StatusTypeDef spibus_begin(spibus_dev_t dev);
void spibus_end(spibus_dev_t dev);

HAL_StatusTypeDef spibus_tx(spibus_dev_t dev, const uint8_t* tx, uint16_t len, uint32_t timeout);
HAL_StatusTypeDef spibus_rx(spibus_dev_t dev, uint8_t* rx, uint16_t len, uint32_t timeout);
HAL_StatusTypeDef spibus_txrx(spibus_dev_t dev, const uint8_t* tx, uint8_t* rx, uint16_t len, uint32_t timeout);
