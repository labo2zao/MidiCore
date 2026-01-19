#include "Services/srio/srio.h"
#include "Services/srio/srio_user_config.h"
#include "Config/module_config.h"

// Include project HAL umbrella via main.h for portability (F4/F7/H7).
#include "main.h"
#include "cmsis_os2.h"
#include <string.h>

static srio_config_t g;
static uint8_t g_inited = 0;
static uint8_t g_num_sr = 0;
static uint16_t g_debounce_time = 0;
static uint16_t g_debounce_ctr = 0;

static uint8_t* g_din = NULL;
static uint8_t* g_din_buffer = NULL;
static uint8_t* g_din_changed = NULL;

#if MODULE_ENABLE_AINSER64
extern SPI_HandleTypeDef hspi3;
#endif

static inline void gpio_write(GPIO_TypeDef* port, uint16_t pin, GPIO_PinState st) {
  if (!port) return;
  HAL_GPIO_WritePin(port, pin, st);
}

static void srio_set_spi_prescaler(SPI_HandleTypeDef* hspi, uint32_t prescaler)
{
  if (!hspi) return;
  __HAL_SPI_DISABLE(hspi);
  MODIFY_REG(hspi->Instance->CR1, SPI_CR1_BR, prescaler);
  hspi->Init.BaudRatePrescaler = prescaler;
  __HAL_SPI_ENABLE(hspi);
}

static void srio_set_spi_mode(SPI_HandleTypeDef* hspi, uint32_t cpol, uint32_t cpha)
{
  if (!hspi) return;
  __HAL_SPI_DISABLE(hspi);
  MODIFY_REG(hspi->Instance->CR1, SPI_CR1_CPOL | SPI_CR1_CPHA, cpol | cpha);
  hspi->Init.CLKPolarity = cpol;
  hspi->Init.CLKPhase = cpha;
  __HAL_SPI_ENABLE(hspi);
}

void srio_init(const srio_config_t* cfg) {
  if (cfg) g = *cfg;
  g_inited = (g.hspi && g.din_pl_port && g.din_bytes) ? 1u : 0u;

#if SRIO_APPLY_SPI_CONFIG
#if MODULE_ENABLE_AINSER64
  if (g.hspi != &hspi3) {
    srio_set_spi_mode(g.hspi, SRIO_SPI_CPOL, SRIO_SPI_CPHA);
    srio_set_spi_prescaler(g.hspi, SRIO_SPI_PRESCALER);
  }
#else
  srio_set_spi_mode(g.hspi, SRIO_SPI_CPOL, SRIO_SPI_CPHA);
  srio_set_spi_prescaler(g.hspi, SRIO_SPI_PRESCALER);
#endif
#endif

  // Ensure sane idle levels (MIOS32-style expects DIN /PL idle high)
  if (g.din_pl_port) {
#if SRIO_DIN_PL_ACTIVE_LOW
    HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_SET);
#else
    HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_RESET);
#endif
  }
  if (g.dout_rclk_port) HAL_GPIO_WritePin(g.dout_rclk_port, g.dout_rclk_pin, GPIO_PIN_RESET);
  srio_set_dout_enable(1);

  g_num_sr = (uint8_t)g.din_bytes;
  g_debounce_time = 0;
  g_debounce_ctr = 0;

  static uint8_t din[SRIO_DIN_BYTES];
  static uint8_t din_buffer[SRIO_DIN_BYTES];
  static uint8_t din_changed[SRIO_DIN_BYTES];
  g_din = din;
  g_din_buffer = din_buffer;
  g_din_changed = din_changed;
  for (uint8_t i = 0; i < g_num_sr; ++i) {
    g_din[i] = 0xFFu;
    g_din_buffer[i] = 0xFFu;
    g_din_changed[i] = 0u;
  }
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
#if SRIO_DIN_PL_ACTIVE_LOW
  HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_RESET);
#else
  HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_SET);
#endif
  // Increased delay for /PL pulse width - ensure 74HC165 has time to load
  for (volatile uint8_t i = 0; i < 16; ++i) { __NOP(); }
#if SRIO_DIN_PL_ACTIVE_LOW
  HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_SET);
#else
  HAL_GPIO_WritePin(g.din_pl_port, g.din_pl_pin, GPIO_PIN_RESET);
#endif
  // Small delay after releasing /PL before starting SPI clock
  for (volatile uint8_t i = 0; i < 8; ++i) { __NOP(); }

  // Optimize: no need to clear buffer, will be overwritten
  // Clock out data via SPI with dummy bytes.
  uint8_t dummy = 0x00;
  for (uint16_t i = 0; i < g.din_bytes; i++) {
    if (HAL_SPI_TransmitReceive(g.hspi, &dummy, &out[i], 1, 10) != HAL_OK) return -2;
  }

  if (g_din && g_din_buffer && g_din_changed) {
    for (uint8_t i = 0; i < g_num_sr; ++i) {
      g_din_buffer[i] = out[i];
    }

    if (g_debounce_time && g_debounce_ctr) {
      --g_debounce_ctr;
      for (uint8_t i = 0; i < g_num_sr; ++i) {
        g_din[i] ^= g_din_changed[i];
        g_din_changed[i] = 0u;
      }
    } else {
      for (uint8_t i = 0; i < g_num_sr; ++i) {
        uint8_t change_mask = g_din[i] ^ g_din_buffer[i];
        g_din_changed[i] |= change_mask;
        g_din[i] = g_din_buffer[i];
        if (change_mask && g_debounce_time) {
          g_debounce_ctr = g_debounce_time;
        }
      }
    }
  }

  return 0;
}

int srio_write_dout(const uint8_t* in) {
  if (!g_inited || !in) return -1;
  if (!g.dout_rclk_port || !g.dout_bytes) return -1;

  // Shift out to 595 chain.
  if (HAL_SPI_Transmit(g.hspi, (uint8_t*)in, g.dout_bytes, 10) != HAL_OK) return -2;

  // Latch: RCLK rising edge (idle low).
  HAL_GPIO_WritePin(g.dout_rclk_port, g.dout_rclk_pin, GPIO_PIN_SET);
  __NOP(); __NOP(); __NOP();
  HAL_GPIO_WritePin(g.dout_rclk_port, g.dout_rclk_pin, GPIO_PIN_RESET);

  return 0;
}

uint8_t srio_din_get(uint16_t sr)
{
  if (!g_din || sr >= g_num_sr) return 0xFFu;
  return g_din[sr];
}

uint8_t srio_din_changed_get_and_clear(uint16_t sr, uint8_t mask)
{
  if (!g_din_changed || sr >= g_num_sr) return 0u;
  uint8_t changed = g_din_changed[sr] & mask;
  g_din_changed[sr] &= (uint8_t)(~mask);
  return changed;
}

uint16_t srio_debounce_get(void)
{
  return g_debounce_time;
}

void srio_debounce_set(uint16_t debounce_ms)
{
  g_debounce_time = debounce_ms;
  if (g_debounce_ctr > g_debounce_time) {
    g_debounce_ctr = g_debounce_time;
  }
}

void srio_debounce_start(void)
{
  g_debounce_ctr = g_debounce_time;
}
