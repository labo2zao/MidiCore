#include "Hal/i2c_hal.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;

static I2C_HandleTypeDef* pick(uint8_t bus){
  switch(bus){
    default:
    case 1: return &hi2c1;
    case 2: return &hi2c2;
  }
}

int i2c_hal_read(uint8_t bus, uint8_t addr7, uint8_t reg, uint8_t* data, uint16_t len, uint32_t timeout_ms){
  I2C_HandleTypeDef* h = pick(bus);
  if(!h || !data || len==0) return -1;
  if (HAL_I2C_Mem_Read(h, (uint16_t)(addr7<<1), reg, I2C_MEMADD_SIZE_8BIT, data, len, timeout_ms) != HAL_OK) return -2;
  return 0;
}

int i2c_hal_write(uint8_t bus, uint8_t addr7, uint8_t reg, const uint8_t* data, uint16_t len, uint32_t timeout_ms){
  I2C_HandleTypeDef* h = pick(bus);
  if(!h || !data || len==0) return -1;
  if (HAL_I2C_Mem_Write(h, (uint16_t)(addr7<<1), reg, I2C_MEMADD_SIZE_8BIT, (uint8_t*)data, len, timeout_ms) != HAL_OK) return -2;
  return 0;
}

int i2c_hal_probe(uint8_t bus, uint8_t addr7, uint32_t timeout_ms){
  I2C_HandleTypeDef* h = pick(bus);
  if(!h) return -1;
  // HAL expects 8-bit address
  if (HAL_I2C_IsDeviceReady(h, (uint16_t)(addr7<<1), 2, timeout_ms) != HAL_OK) return -2;
  return 0;
}
