#include "Hal/ainser64_hw/hal_ainser64_hw_step.h"
#include "Hal/spi_bus.h"
#include "Hal/delay_us.h"
#include "Config/ainser64_pins.h"
#include "stm32f4xx_hal.h"

static inline void set_bank(uint8_t bank) {
  HAL_GPIO_WritePin(BANK_A_GPIO_Port, BANK_A_Pin, (bank & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(BANK_B_GPIO_Port, BANK_B_Pin, (bank & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(BANK_C_GPIO_Port, BANK_C_Pin, (bank & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static inline void set_mux_step(uint8_t step) {
  HAL_GPIO_WritePin(MUX_S0_GPIO_Port, MUX_S0_Pin, (step & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(MUX_S1_GPIO_Port, MUX_S1_Pin, (step & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
  HAL_GPIO_WritePin(MUX_S2_GPIO_Port, MUX_S2_Pin, (step & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint16_t mcp3208_read_ch(uint8_t ch) {
  uint8_t tx[3];
  uint8_t rx[3];
  tx[0] = 0x06 | ((ch & 0x04) >> 2);
  tx[1] = (uint8_t)((ch & 0x03) << 6);
  tx[2] = 0x00;
  spibus_txrx(SPIBUS_DEV_AIN, tx, rx, 3, 10);
  return (uint16_t)(((rx[1] & 0x0F) << 8) | rx[2]);
}

void hal_ainser64_init(void) {
  // Ensure CS high
  HAL_GPIO_WritePin(AIN_CS_GPIO_Port, AIN_CS_Pin, GPIO_PIN_SET);
  set_bank(0);
  set_mux_step(0);
}

int hal_ainser64_read_bank_step(uint8_t bank, uint8_t step, uint16_t out8[8]) {
  if (bank > 7 || step > 7) return -1;

  set_bank(bank);
  delay_us(AIN_BANK_SETTLE_US);

  set_mux_step(step);
  delay_us(AIN_MUX_SETTLE_US);

  if (spibus_begin(SPIBUS_DEV_AIN) != HAL_OK) return -2;
  for (uint8_t ch = 0; ch < 8; ch++) out8[ch] = mcp3208_read_ch(ch);
  spibus_end(SPIBUS_DEV_AIN);
  return 0;
}
