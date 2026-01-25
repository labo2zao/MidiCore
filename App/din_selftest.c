/**
 * @file din_selftest.c
 * @brief DEPRECATED: Legacy DIN test - use MODULE_TEST_SRIO instead
 * 
 * This file contains the old DIN self-test which has been superseded by
 * the comprehensive MODULE_TEST_SRIO test in App/tests/module_tests.c
 * 
 * The new test provides:
 * - Full button press/release detection
 * - Debounce support (MIOS32-compatible)
 * - Comprehensive diagnostics
 * - Pin mapping verification
 * 
 * This legacy code is kept for reference only.
 * To use the new test, set: MODULE_TEST_SRIO=1
 */

#include "App/din_selftest.h"

#include "Hal/uart_midi/hal_uart_midi.h"
#include "Services/srio/srio.h"
#include "Services/srio/srio_user_config.h"
#include "cmsis_os2.h"

#ifndef DIN_SELFTEST_UART_PORT
// 0=UART1, 1=UART2, 2=UART3, 3=UART5 (see Hal/uart_midi)
#define DIN_SELFTEST_UART_PORT 1u
#endif

static void uart_putc(char c) {
  hal_uart_midi_send_byte(DIN_SELFTEST_UART_PORT, (uint8_t)c);
}

static void uart_puts(const char* s) {
  if (!s) return;
  while (*s) uart_putc(*s++);
}

static void uart_hex8(uint8_t v) {
  static const char* h = "0123456789ABCDEF";
  uart_putc(h[(v >> 4) & 0x0F]);
  uart_putc(h[v & 0x0F]);
}

void din_selftest_run(void)
{
  // UART debug (assumes CubeMX already initialized USARTx)
  hal_uart_midi_init();

  uart_puts("\r\n[DIN] selftest start\r\n");
  uart_puts("SRIO: SPI2 PB13/PB14/PB15, RC1=PA15, RC2=PE1\r\n");
  uart_puts("Tip: if you use MIOS32-style single RC line, set SRIO_RC2 to same pin as RC1.\r\n");

#ifdef SRIO_ENABLE
  srio_config_t scfg = {
    .hspi = SRIO_SPI_HANDLE,
    .din_pl_port = SRIO_DIN_PL_PORT,
    .din_pl_pin = SRIO_DIN_PL_PIN,
    .dout_rclk_port = SRIO_DOUT_RCLK_PORT,
    .dout_rclk_pin = SRIO_DOUT_RCLK_PIN,
    .dout_oe_port = NULL,
    .dout_oe_pin = 0,
    .dout_oe_active_low = 1,
    .din_bytes = SRIO_DIN_BYTES,
    .dout_bytes = SRIO_DOUT_BYTES,
  };
  srio_init(&scfg);

  uint8_t din[SRIO_DIN_BYTES];

  for (;;) {
    int r = srio_read_din(din);
    if (r < 0) {
      uart_puts("DIN: read error\r\n");
    } else {
      uart_puts("DIN:");
      for (uint16_t i=0; i<SRIO_DIN_BYTES; i++) {
        uart_putc(' ');
        uart_hex8(din[i]);
      }
      uart_puts("\r\n");
    }
    osDelay(50);
  }
#else
  uart_puts("[DIN] ERROR: SRIO_ENABLE not defined at compile time.\r\n");
  for(;;) osDelay(1000);
#endif
}
