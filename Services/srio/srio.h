#pragma once
// Include main.h for portable STM32 HAL (F4/F7/H7 compatibility)
#include "main.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __SPI_HandleTypeDef SPI_HandleTypeDef;
typedef struct {
  SPI_HandleTypeDef* hspi;

  // 74HC165 /PL (parallel load) pin (active low pulse)
  GPIO_TypeDef* din_pl_port;
  uint16_t      din_pl_pin;

  // 74HC595 RCLK (latch) pin (rising edge to latch shift register outputs)
  GPIO_TypeDef* dout_rclk_port;
  uint16_t      dout_rclk_pin;

  // Optional /OE for 74HC595 (active low). Set port=NULL to ignore.
  GPIO_TypeDef* dout_oe_port;
  uint16_t      dout_oe_pin;
  uint8_t       dout_oe_active_low; // 1 typical

  uint16_t din_bytes;   // number of bytes in DIN chain (165)
  uint16_t dout_bytes;  // number of bytes in DOUT chain (595)
} srio_config_t;

void srio_init(const srio_config_t* cfg);
int  srio_read_din(uint8_t* out);
int  srio_write_dout(const uint8_t* in);
void srio_set_dout_enable(uint8_t enable);
int  srio_scan(void);

uint16_t srio_din_bytes(void);
uint16_t srio_dout_bytes(void);

// MIOS32-inspired helpers for DIN state/changes.
uint8_t srio_din_get(uint16_t sr);
uint8_t srio_din_changed_get_and_clear(uint16_t sr, uint8_t mask);

// Debounce handling (milliseconds; 0 disables debouncing).
uint16_t srio_debounce_get(void);
void srio_debounce_set(uint16_t debounce_ms);
void srio_debounce_start(void);

#ifdef __cplusplus
}
#endif
