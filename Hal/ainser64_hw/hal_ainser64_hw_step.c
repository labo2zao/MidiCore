// hal_ainser64_hw_step.c
// MIOS32-compatible AINSER64 backend (MCP3208 + 74HC595)

#include "Hal/ainser64_hw/hal_ainser64_hw_step.h"

#include <string.h>

#include "Hal/spi_bus.h"
#include "Config/ainser64_pins.h" // AIN_CS_* used by SPIBUS_DEV_AIN

// Include main.h for portable STM32 HAL (F4/F7/H7 compatibility)
#include "main.h"

// -----------------------------------------------------------------------------
// Mapping & options
// -----------------------------------------------------------------------------

// Default MidiCore mapping for MBHP_AINSER64 routing.
// In MidiCore (modules/ainser/ainser.c) this is used to map mux address -> pin base.
// Here we expose it so the higher level can turn it into a key index.
static const uint8_t k_default_mux_port_map[8] = { 0, 5, 2, 7, 4, 1, 6, 3 };
static uint8_t g_mux_port_map[8];

// MIOS32-style PWM LED modulation for smooth breathing effect
static uint8_t g_link_led_enable = 1;
static uint16_t g_link_status_ctr = 0;

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

#if AINSER64_LED_MODE_PWM
  // MIOS32-style PWM breathing effect
  // Link LED will flash with PWM effect (breathing in/out over ~2 seconds)
  // This matches the behavior in MidiCore modules/ainser/ainser.c lines 295-302
  //
  // NOTE: In MIOS32, this counter increments once per complete scan of all modules (~1ms).
  // In MidiCore with continuous scanning, we increment on every channel (8 channels per step,
  // 8 steps = 64 increments per scan). To match MidiCore timing, we increment every 64 calls.
  static uint8_t call_counter = 0;
  if (++call_counter >= 64) {
    call_counter = 0;
    ++g_link_status_ctr;
  }
  
  const uint32_t pwm_period = 20;       // *1ms -> 20ms
  const uint32_t pwm_sweep_steps = 100; // *20ms -> 2000ms (2 second sweep)
  
  uint32_t pwm_duty = ((g_link_status_ctr / pwm_period) % pwm_sweep_steps) / (pwm_sweep_steps / pwm_period);
  
  // Reverse direction every 2 seconds (creates breathing effect)
  if ((g_link_status_ctr % (2 * pwm_period * pwm_sweep_steps)) > (pwm_period * pwm_sweep_steps))
    pwm_duty = pwm_period - pwm_duty;
  
  uint32_t link_status = ((g_link_status_ctr % pwm_period) > pwm_duty) ? 1 : 0;
  
  return (uint8_t)(link_status & 1u);
#else
  // Simple on/off toggle mode (low memory usage, ~2Hz blink)
  // Uses system tick, not call counter, to avoid dependency on scan rate
  uint32_t t = HAL_GetTick();
  return (uint8_t)((t >> 8u) & 1u);  // Toggle every 256ms
#endif
}

// -----------------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------------

int32_t hal_ainser64_init(void)
{
  memcpy(g_mux_port_map, k_default_mux_port_map, sizeof(g_mux_port_map));
  g_link_led_enable = 1;
  g_link_status_ctr = 0;

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
    // (MidiCore does this on channel 7). Here we keep mux constant for this call.
    uint8_t sr_byte = (uint8_t)((mux_ctr << 5) | (link_bit & 1u));

    uint16_t sample = 0;
    if (mcp3208_read_channel_with_sr(ch, sr_byte, &sample) != 0) {
      return -4;
    }
    out8[ch] = sample;
  }
  
  return 0;
}
