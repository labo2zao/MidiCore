#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int i2c_hal_read(uint8_t bus, uint8_t addr7, uint8_t reg, uint8_t* data, uint16_t len, uint32_t timeout_ms);
int i2c_hal_probe(uint8_t bus, uint8_t addr7, uint32_t timeout_ms);

int i2c_hal_write(uint8_t bus, uint8_t addr7, uint8_t reg, const uint8_t* data, uint16_t len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif
