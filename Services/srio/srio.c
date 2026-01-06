#include "Services/srio/srio.h"

// Include project HAL umbrella via main.h for portability (F4/F7/H7).
#include "main.h"
#include "cmsis_os2.h"
#include <string.h>

static srio_config_t g;
static uint8_t g_inited = 0;

static inline void gpio_write(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState st) {
  if (!port) return;
  HAL_GPIO_WritePin(port, pin, st);
}

void srio_init(const srio_config_t* cfg) {
  memset(&g, 0, sizeof(g));
  if (cfg) g = *cfg;
  g_inited = (g.hspi && g.din_pl_port && g.dout_rclk_port && g.din_bytes && g.dout_bytes);
  srio_set_dout_enable(1);
}

uint16_t srio_din_bytes(void) { return g.din_bytes; }
uint16_t srio_dout_bytes(void) { return g.dout_bytes; }

void srio_set_dout_enable(uint8_t enable) {
  if (!g.dout_oe_port) return;
  if (g.dout_oe_active_low) {
    gpio_write(g.dout_oe_port, g.dout_oe_pin, enable ? GPIO_PIN_RESET : GPIO_PIN_SET);
  } else {
    gpio_write(g.dout_oe_port, g.dout_oe_pin, enable ? GPIO_PIN_SET : GPIO_PIN_RESET);
  }
}

int srio_read_din(uint8_t* out) {
  if (!g_inited || !out) return -1;

  // Latch DIN parallel inputs into 165 shift regs: /PL low pulse (idle high).
  HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_RESET);
  __NOP(); __NOP(); __NOP();
  HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_SET);

  memset(out, 0, g.din_bytes);

  // Clock out data via SPI with dummy bytes.
  uint8_t dummy = 0x00;
  for (uint16_t i = 0; i < g.din_bytes; i++) {
    if (HAL_SPI_TransmitReceive(g.hspi, &dummy, &out[i], 1, 10) != HAL_OK) return -2;
  }

  return 0;
}

int srio_write_dout(const uint8_t* in) {
  if (!g_inited || !in) return -1;

  // Shift out to 595 chain.
  if (HAL_SPI_Transmit(g.hspi, (uint8_t*)in, g.dout_bytes, 10) != HAL_OK) return -2;

  // Latch: RCLK rising edge (idle low).
  HAL_GPIO_WritePin(g.dout_rclk_port, g.dout_rclk_pin, GPIO_PIN_SET);
  __NOP(); __NOP(); __NOP();
  HAL_GPIO_WritePin(g.dout_rclk_port, g.dout_rclk_pin, GPIO_PIN_RESET);

  return 0;
}
