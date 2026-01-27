#include "App/tests/app_test_din_midi.h"

#include "cmsis_os2.h"
#include "main.h"

#include "Services/srio/srio.h"
#include "Services/srio/srio_user_config.h"
#include "Services/din/din_map.h"
#include "Services/patch/patch_sd_mount.h"
#include "Services/midi/midi_router.h"
#include "Services/router/router.h"
#include "Services/router/router_send.h"
#include "Services/usb_host_midi/usb_host_midi.h"
#include "Services/cli/cli.h"
#include "Services/cli/router_cli.h"
#include "Services/ui/ui.h"
#include "Hal/oled_ssd1322/oled_ssd1322.h"
#include "App/tests/test_debug.h"

#include <string.h>

// External UART handles for baudrate validation
extern UART_HandleTypeDef huart5; // UART5 - Debug port

#ifndef APP_TEST_MIDI_BASE_NOTE
#define APP_TEST_MIDI_BASE_NOTE 36
#endif

#ifndef APP_TEST_MIDI_CH
#define APP_TEST_MIDI_CH 0
#endif

#ifndef APP_TEST_MIDI_VELOCITY
#define APP_TEST_MIDI_VELOCITY 96
#endif

#ifndef APP_TEST_MIDI_USE_USBH
#define APP_TEST_MIDI_USE_USBH 1
#endif

#ifdef SRIO_ENABLE
static uint8_t din_prev[SRIO_DIN_BYTES];
#endif

static void app_test_din_output_cb(DIN_MapType type, uint8_t channel,
                                   uint8_t number, uint8_t value)
{
#ifdef SRIO_ENABLE
  if (type == DIN_MAP_TYPE_NOTE) {
    if (value) {
      midi_router_note_on(MIDI_ROUTER_SRC_DIN, channel, number, value);
    } else {
      midi_router_note_off(MIDI_ROUTER_SRC_DIN, channel, number, 0u);
    }
  } else if (type == DIN_MAP_TYPE_CC) {
    midi_router_cc(MIDI_ROUTER_SRC_DIN, channel, number, value);
  }
#else
  (void)type;
  (void)channel;
  (void)number;
  (void)value;
#endif
}

void app_test_din_midi_run_forever(void)
{
#ifdef SRIO_ENABLE
  // Initialize UART debug FIRST before anything else
  // NOTE: test_debug_init() also initializes OLED if MODULE_ENABLE_OLED is set
  test_debug_init();
  osDelay(100);
  
  dbg_print("\r\n==============================================\r\n");
  dbg_print("DIN MIDI Test Mode\r\n");
  dbg_print("==============================================\r\n\r\n");
  
  // Initialize UI (OLED already initialized by test_debug_init)
  dbg_print("Initializing UI subsystem... ");
  ui_init();
  dbg_print("OK\r\n");
  
  // Initialize CLI BEFORE router (so router CLI can register)
  dbg_print("Initializing CLI... ");
  cli_init();
  dbg_print("OK\r\n");
  dbg_print("  Type 'help' for available commands\r\n");
  dbg_print("  Type 'router matrix' to view routing\r\n\r\n");
  
  // Initialize router first (required for CLI commands)
  dbg_print("Initializing MIDI Router... ");
  router_init(router_send_default);
  dbg_print("OK\r\n");
  
  // Register router CLI commands AFTER router is initialized
  router_cli_register();
  
  // 0) SD + DIN mapping
  dbg_print("Initializing SD and DIN mapping... ");
  if (patch_sd_mount_retry(3) == 0) {
    din_map_init_defaults((uint8_t)APP_TEST_MIDI_BASE_NOTE);
    din_map_set_output_cb(app_test_din_output_cb);
    (void)din_map_load_sd("0:/cfg/din_map.ngc");
    dbg_print("OK (config loaded from SD)\r\n");
  } else {
    din_map_init_defaults((uint8_t)APP_TEST_MIDI_BASE_NOTE);
    din_map_set_output_cb(app_test_din_output_cb);
    dbg_print("OK (using defaults)\r\n");
  }

  // 1) UART MIDI - CRITICAL: Do NOT call hal_uart_midi_init() here!
  // IMPORTANT: Calling hal_uart_midi_init() would reconfigure ALL UARTs to 31250 baud,
  // including the debug UART (UART5) which test_debug_init() set to 115200 baud.
  // The router will handle MIDI output directly via router_send_default().
  //
  // Runtime validation: Debug UART must remain at 115200 baud
  UART_HandleTypeDef* debug_uart = &huart5;  // UART5 is debug port
  if (debug_uart->Init.BaudRate != 115200) {
    dbg_print("WARNING: Debug UART baudrate changed from 115200!\r\n");
    dbg_printf("Current baudrate: %lu\r\n", (unsigned long)debug_uart->Init.BaudRate);
    dbg_print("Reconfiguring to 115200...\r\n");
    HAL_UART_DeInit(debug_uart);
    debug_uart->Init.BaudRate = 115200;
    HAL_UART_Init(debug_uart);
  }
  dbg_print("MIDI routing via router (hal_uart_midi skipped to preserve 115200 debug baud)... OK\r\n");

  // 2) SRIO
  dbg_print("Initializing SRIO... ");
  srio_config_t scfg = {
    .hspi              = SRIO_SPI_HANDLE,
    .din_pl_port       = SRIO_DIN_PL_PORT,
    .din_pl_pin        = SRIO_DIN_PL_PIN,
    .dout_rclk_port    = SRIO_DOUT_RCLK_PORT,
    .dout_rclk_pin     = SRIO_DOUT_RCLK_PIN,
    .dout_oe_port      = NULL,
    .dout_oe_pin       = 0,
    .dout_oe_active_low= 1,
    .din_bytes         = SRIO_DIN_BYTES,
    .dout_bytes        = SRIO_DOUT_BYTES
  };
  srio_init(&scfg);

  memset(din_prev, 0xFF, sizeof(din_prev));
  (void)srio_read_din(din_prev);
  dbg_print("OK\r\n\r\n");
  
  dbg_print("==============================================\r\n");
  dbg_print("Test running. Press DIN buttons to send MIDI.\r\n");
  dbg_print("Use CLI commands to control routing.\r\n");
  dbg_print("\r\n");
  dbg_print("** UART DEBUG @ 115200 BAUD **\r\n");
  dbg_print("  Port: UART5 (PC12/PD2)\r\n");
  dbg_print("  Verify your terminal is set to 115200 baud!\r\n");
  dbg_print("\r\n");
  dbg_print("Available commands:\r\n");
  dbg_print("  help          - Show all commands\r\n");
  dbg_print("  router matrix - Show routing matrix\r\n");
  dbg_print("  router enable IN OUT - Enable route\r\n");
  dbg_print("  router disable IN OUT - Disable route\r\n");
  dbg_print("==============================================\r\n\r\n");

  // 3) Main loop
  for (;;) {
    // Process CLI commands
    cli_task();
    
    // Update UI
    ui_task();
    
#if APP_TEST_MIDI_USE_USBH
    usb_host_midi_task();
#endif
    uint8_t din[SRIO_DIN_BYTES];
    if (srio_read_din(din) == 0) {
      for (uint16_t bit = 0; bit < (uint16_t)(SRIO_DIN_BYTES * 8u); ++bit) {
        uint16_t byte = (uint16_t)(bit >> 3);
        uint8_t  mask = (uint8_t)(1u << (bit & 7u));
        uint8_t prev = (din_prev[byte] & mask) ? 1u : 0u;
        uint8_t cur  = (din[byte]      & mask) ? 1u : 0u;
        if (prev != cur) {
          // active-low : pressed quand la ligne passe à 0
          uint8_t pressed = (cur == 0u) ? 1u : 0u;
          din_map_process_event((uint8_t)bit, pressed);
        }
      }
      memcpy(din_prev, din, SRIO_DIN_BYTES);
    }
    osDelay(1);
  }
#else
  for (;;)
    osDelay(1000);
#endif
}
