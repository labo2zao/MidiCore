// hal_ainser64_hw_step.c
// MIOS32-compatible AINSER64 backend (MCP3208 + 74HC595)

#include "Hal/ainser64_hw/hal_ainser64_hw_step.h"

#include <string.h>

#include "Hal/spi_bus.h"
#include "Config/ainser64_pins.h" // AIN_CS_* used by SPIBUS_DEV_AIN

#include "stm32f4xx_hal.h"

// -----------------------------------------------------------------------------
// Mapping & options
// -----------------------------------------------------------------------------

// Default MIOS32 mapping for MBHP_AINSER64 routing.
// In MIOS32 (modules/ainser/ainser.c) this is used to map mux address -> pin base.
// Here we expose it so the higher level can turn it into a key index.
static const uint8_t k_default_mux_port_map[8] = { 0, 5, 2, 7, 4, 1, 6, 3 };
static uint8_t g_mux_port_map[8];

// Simple, cheap LED modulation (MIOS32 uses a PWM-modulated slow flash).
static uint8_t g_link_led_enable = 1;
static uint16_t g_link_led_phase = 0;

// This project currently supports a single AINSER64 module on a single CS line.
// Keeping the "module" parameter in the API allows later extension.
static inline int module_supported(uint8_t module) { return module == 0; }

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

// MCP3208 transaction (3 bytes) + 74HC595 update byte in the 3rd byte.
// Returns 12-bit sample in *out.
static int32_t mcp3208_read_channel_with_sr(uint8_t channel, uint8_t sr_byte, uint16_t *out)
{
  // MCP3208 command format (single-ended):
  // b0: 0b00000110 | (channel >> 2)
  // b1: (channel << 6)
  // b2: don't care for MCP, but is used as 74HC595 data (via MOSI)
  uint8_t tx[3];
  uint8_t rx[3];

  tx[0] = (uint8_t)(0x06 | (channel >> 2));
  tx[1] = (uint8_t)(channel << 6);
  tx[2] = sr_byte;

  // IMPORTANT (AINSER64 wiring): RC (chip-select) is shared between
  // - MCP3208 CS (pin 10)
  // - 74HC595 RCLK (pin 12)
  // So we MUST assert CS low for the transfer, then deassert it high so that
  // the 74HC595 latches the last shifted byte (sr_byte).
  if (spibus_begin(SPIBUS_DEV_AIN) != HAL_OK)
    return -1;

  HAL_StatusTypeDef st = spibus_txrx(SPIBUS_DEV_AIN, tx, rx, 3, 10);

  // CS rising edge latches 74HC595 outputs (Link LED + MUX A/B/C)
  (void)spibus_end(SPIBUS_DEV_AIN);

  if (st != HAL_OK)
    return -1;

  // 12-bit sample: low 4 bits in rx[1], then rx[2]
  *out = (uint16_t)(((rx[1] & 0x0Fu) << 8) | rx[2]);
  return 0;
}

static inline uint8_t compute_link_led_bit(void)
{
  if (!g_link_led_enable)
    return 0;

  // Slow blink: ~2 Hz, cheap PWM-ish (good enough for a status LED)
  // (toggle every 250 ms)
  uint32_t t = HAL_GetTick();
  return (uint8_t)(((t / 250u) & 1u) ? 1u : 0u);
}

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

int32_t hal_ainser64_init(void)
{
  memcpy(g_mux_port_map, k_default_mux_port_map, sizeof(g_mux_port_map));
  g_link_led_enable = 1;
  g_link_led_phase = 0;

  // SPI bus init is handled by the app (see app_init.c).
  return 0;
}

void hal_ainser64_set_link_led_enable(uint8_t enable)
{
  g_link_led_enable = enable ? 1 : 0;
}

void hal_ainser64_set_mux_port_map(const uint8_t map[8])
{
  if (!map) {
    memcpy(g_mux_port_map, k_default_mux_port_map, sizeof(g_mux_port_map));
    return;
  }
  memcpy(g_mux_port_map, map, sizeof(g_mux_port_map));
}

int32_t hal_ainser64_read_bank_step(uint8_t module, uint8_t step, uint16_t out8[8])
{
  if (!out8)
    return -1;
  if (!module_supported(module))
    return -2;

  step &= 0x7u;

  // MIOS32: mux control value goes in bits 7..5 of the 74HC595 byte.
  // LSB is the LINK LED.
  // We keep other bits 0.
  uint8_t link_bit = compute_link_led_bit();

  // If you want strict MIOS32-style port order, the mux control that must be
  // shifted is the *physical* mux address. The mapping array is used later to
  // map results to pin numbers. Here we keep it simple: mux_ctr == step.
  uint8_t mux_ctr = step;

  for (uint8_t ch = 0; ch < 8; ++ch) {
    // For best behaviour we can preload the next mux ctr at the end of the scan
    // (MIOS32 does this on channel 7). Here we keep mux constant for this call.
    uint8_t sr_byte = (uint8_t)((mux_ctr << 5) | (link_bit & 1u));

    uint16_t sample = 0;
    if (mcp3208_read_channel_with_sr(ch, sr_byte, &sample) != 0) {
      return -4;
    }
    out8[ch] = sample;
  }
  (void)g_link_led_phase; // reserved for future PWM
  return 0;
}
