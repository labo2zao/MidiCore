#include "App/app_init.h"
#include "Config/module_config.h"

#if MODULE_ENABLE_SPI_BUS
#include "Hal/spi_bus.h"
#endif

#if MODULE_ENABLE_AINSER64
#include "Hal/ainser64_hw/hal_ainser64_hw_step.h"
#endif

#if MODULE_ENABLE_AIN
#include "Services/ain/ain.h"
#endif

#if MODULE_ENABLE_OLED
#include "Hal/oled_ssd1322/oled_ssd1322.h"
#endif

#if MODULE_ENABLE_ROUTER
#include "Services/router/router.h"
#include "Services/router/router_send.h"
#endif

#if MODULE_ENABLE_MIDI_DIN
#include "Services/midi/midi_din.h"
#endif

#if MODULE_ENABLE_PATCH
#include "Services/patch/patch.h"
#include "Services/patch/patch_router.h"
#include "Services/patch/patch_sd_mount.h"
#include "Services/patch/patch_system.h"
#endif

#if MODULE_ENABLE_LOOPER
#include "Services/looper/looper.h"
#endif

#if MODULE_ENABLE_UI
#include "Services/ui/ui.h"
#endif

#if MODULE_ENABLE_SAFE_MODE
#include "Services/safe/safe_mode.h"
#endif

#if MODULE_ENABLE_SYSTEM_STATUS
#include "Services/system/system_status.h"
#endif

#if MODULE_ENABLE_WATCHDOG
#include "Services/watchdog/watchdog.h"
#endif

#if MODULE_ENABLE_BOOT_REASON
#include "Services/system/boot_reason.h"
#include "App/tests/test_debug.h"
#endif

#if MODULE_ENABLE_LOG
#include "Services/log/log.h"
#endif

#if MODULE_ENABLE_CLI
#include "Services/cli/cli.h"
#include "Services/cli/cli_module_commands.h"
#endif

#if MODULE_ENABLE_USB_CDC
#include "Services/usb_cdc/usb_cdc.h"
#endif

#if MODULE_ENABLE_MODULE_REGISTRY
#include "Services/module_registry/module_registry.h"
#endif

#if MODULE_ENABLE_TEST
#include "Services/test/test.h"
#include "Services/test/test_cli.h"
#endif

#if MODULE_ENABLE_INSTRUMENT
#include "Services/instrument/instrument_cfg.h"
#endif

#if MODULE_ENABLE_ZONES
#include "Services/zones/zones_cfg.h"
#endif

#if MODULE_ENABLE_EXPRESSION
#include "Services/expression/expression_cfg.h"
#endif

#if MODULE_ENABLE_PRESSURE
#include "Services/pressure/pressure_i2c.h"
#include "App/i2c_scan.h"
#endif

#if MODULE_ENABLE_HUMANIZE
#include "Services/humanize/humanize.h"
#endif

#if MODULE_ENABLE_LFO
#include "Services/lfo/lfo.h"
#endif

#include "App/ain_midi_task.h"

#if MODULE_ENABLE_AIN_RAW_DEBUG
#include "App/ain_raw_debug_task.h"
#endif

#include "Services/config/config.h"

#if MODULE_ENABLE_SRIO
#include "Services/srio/srio.h"
#include "Services/srio/srio_user_config.h"
#endif

#if MODULE_ENABLE_USB_MIDI
#include "Services/usb_midi/usb_midi.h"
#endif

#if MODULE_ENABLE_USB_CDC
#include "Services/usb_cdc/usb_cdc.h"
#endif

#include "App/midi_io_task.h"

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

static void AinTask(void *argument);
static void OledDemoTask(void *argument);
#if MODULE_ENABLE_CLI
static void CliTask(void *argument);
#endif
static uint8_t boot_shift_held(uint8_t active_low);

#if MODULE_ENABLE_USB_CDC
// CDC terminal echo callback for MIOS Studio terminal
static void cdc_terminal_echo(const uint8_t *data, uint32_t len) {
  // Echo back what was received (simple terminal test)
  usb_cdc_send(data, len);
}
#endif

void app_init_and_start(void)
{
  // Init shared services
#if MODULE_ENABLE_SPI_BUS
  spibus_init();
  dbg_printf("[INIT] After spibus_init - Stack free: %u bytes\r\n", stack_free * 4);
#endif

#if MODULE_ENABLE_AINSER64
  hal_ainser64_init();
#endif

#if MODULE_ENABLE_AIN
  ain_init();
#endif

#if MODULE_ENABLE_OLED
  // Production: Use complete Newhaven NHD-3.12 init (LoopA production code)
  // oled_init() is a simple MIOS32 test init, not suitable for production
  oled_init_newhaven();
#endif

#if MODULE_ENABLE_ROUTER
  router_init(router_send_default);
#endif

#if MODULE_ENABLE_MIDI_DIN
  midi_din_init();
#endif

#if MODULE_ENABLE_USB_MIDI
  usb_midi_init();
#endif

#if MODULE_ENABLE_USB_CDC
  usb_cdc_init();
  // Register CDC terminal echo callback for MIOS Studio terminal
  usb_cdc_register_receive_callback(cdc_terminal_echo);
#endif

#if MODULE_ENABLE_PATCH
  patch_init();
#endif

// ---- Safety / boot mode ----
// Try mount SD early to read global config
#if MODULE_ENABLE_PATCH
  int sd_ok = (patch_sd_mount_init() == 0);
#else
  int sd_ok = 0;
#endif

#if MODULE_ENABLE_SAFE_MODE
  safe_mode_set_sd_ok(sd_ok ? 1u : 0u);
#endif

  config_t global_cfg;
  config_set_defaults(&global_cfg);
  
#if MODULE_ENABLE_PATCH
  if (sd_ok) {
    (void)config_load_from_sd(&global_cfg, "0:/cfg/global.ngc");

#if MODULE_ENABLE_INSTRUMENT
    instrument_cfg_t icfg; 
    instrument_cfg_defaults(&icfg);
    if (sd_ok) { (void)instrument_cfg_load_sd(&icfg, "0:/cfg/instrument.ngc"); }
    instrument_cfg_set(&icfg);
#endif

#if MODULE_ENABLE_ZONES
    zones_cfg_t zcfg; 
    zones_cfg_defaults(&zcfg);
    if (sd_ok) { (void)zones_cfg_load_sd(&zcfg, "0:/cfg/zones.ngc"); }
    zones_cfg_set(&zcfg);
#endif

#if MODULE_ENABLE_EXPRESSION
    expr_cfg_t ecfg; 
    expression_cfg_defaults(&ecfg);
    if (sd_ok) { (void)expression_cfg_load_sd(&ecfg, "0:/cfg/expression.ngc"); }
    expression_set_cfg(&ecfg);
#endif

#if MODULE_ENABLE_PRESSURE
    pressure_cfg_t pcfg; 
    pressure_defaults(&pcfg);
    if (sd_ok) { (void)pressure_load_sd(&pcfg, "0:/cfg/pressure.ngc"); }
    pressure_set_cfg(&pcfg);
    // Debug: scan I2C bus to confirm pressure sensor address
    app_i2c_scan_and_log(pcfg.i2c_bus);
#endif

#if MODULE_ENABLE_HUMANIZE
    humanize_init(osKernelGetTickCount());
#endif
  }
#endif

// Hold SHIFT at boot to force SAFE_MODE
  uint8_t shift_held = boot_shift_held(global_cfg.global_shift_active_low);
#if MODULE_ENABLE_SAFE_MODE
  safe_mode_set_forced(shift_held ? 1u : 0u);
  safe_mode_set_cfg(global_cfg.global_safe_mode ? 1u : 0u);
#endif

// SD/FATFS mount + load patch then apply router rules from [router]
#if MODULE_ENABLE_PATCH && MODULE_ENABLE_SYSTEM_STATUS
  if (!system_is_fatal()) {
    patch_system_init();
  } else {
    // SD required but missing: stay alive with minimal UI
#if MODULE_ENABLE_UI
    ui_set_status_line("SD REQUIRED");
#endif
  }
#endif

  // OLED header init
#if MODULE_ENABLE_PATCH && MODULE_ENABLE_UI
  const patch_manager_t* pm = patch_system_get();
  const char* bank = pm->bank.bank_id[0] ? pm->bank.bank_id : pm->bank.bank_name;
  const char* patch = pm->bank.patches[pm->state.patch_index].label[0] ? pm->bank.patches[pm->state.patch_index].label : "patch";
  ui_set_patch_status(bank, patch);
#endif

#if MODULE_ENABLE_LOOPER
  looper_init();
#endif

#if MODULE_ENABLE_LFO
  lfo_init();
#endif

// Note: humanize_init() is already called earlier in the init sequence (line ~204)

#if MODULE_ENABLE_UI
  ui_init();
#endif

#if MODULE_ENABLE_BOOT_REASON
  boot_reason_init();
#endif

#if MODULE_ENABLE_WATCHDOG
  watchdog_init();
#endif

#if MODULE_ENABLE_LOG
  log_init();
#if MODULE_ENABLE_BOOT_REASON
  dbg_printf("BOOT: reason=%d\r\n", (int)boot_reason_get());
#endif
#endif

  dbg_printf("[INIT] System initialization complete\r\n");

  // Initialize CLI and module registry for terminal control
#if MODULE_ENABLE_MODULE_REGISTRY
  dbg_printf("[INIT] Initializing module registry...\r\n");
  module_registry_init();
#endif

#if MODULE_ENABLE_CLI
  dbg_printf("[INIT] CLI step 1: calling cli_init...\r\n");
  cli_init();
  dbg_printf("[INIT] CLI step 2: cli_init returned OK\r\n");
  dbg_printf("[INIT] CLI step 3: calling cli_module_commands_init...\r\n");
  int cli_cmd_result = cli_module_commands_init();
  dbg_printf("[INIT] CLI step 4: cli_module_commands_init returned %d\r\n", cli_cmd_result);
  dbg_printf("[INIT] CLI step 5: CLI system ready\r\n");
#else
  dbg_printf("[INIT] MODULE_ENABLE_CLI is NOT defined - CLI will not be available\r\n");
#endif

#if MODULE_ENABLE_TEST
  test_init();
  test_cli_init();
#endif

  // Default routing examples
#if MODULE_ENABLE_ROUTER && MODULE_ENABLE_MIDI_DIN
  router_set_route(ROUTER_NODE_DIN_IN1, ROUTER_NODE_DIN_OUT1, 1);
  router_set_route(ROUTER_NODE_DIN_IN2, ROUTER_NODE_DIN_OUT1, 1);
  router_set_route(ROUTER_NODE_DIN_IN3, ROUTER_NODE_DIN_OUT1, 1);
  router_set_route(ROUTER_NODE_DIN_IN4, ROUTER_NODE_DIN_OUT1, 1);
#endif

#if MODULE_ENABLE_ROUTER && MODULE_ENABLE_LOOPER && MODULE_ENABLE_MIDI_DIN
  // Default: Looper playback -> DIN OUT1
  router_set_route(ROUTER_NODE_LOOPER, ROUTER_NODE_DIN_OUT1, 1);
#endif

  // Create tasks
#if MODULE_ENABLE_AIN
  const osThreadAttr_t ain_attr = {
    .name = "AinTask",
    .priority = osPriorityNormal,
    .stack_size = 1024
  };
  (void)osThreadNew(AinTask, NULL, &ain_attr);
  app_start_ain_midi_task();
#endif

#if MODULE_ENABLE_PRESSURE
  app_start_pressure_task();
#endif

  app_start_calibration_task();

#if MODULE_ENABLE_OLED
  const osThreadAttr_t oled_attr = {
    .name = "OledDemo",
    .priority = osPriorityLow,
    .stack_size = 1024
  };
  (void)osThreadNew(OledDemoTask, NULL, &oled_attr);
#endif

#if MODULE_ENABLE_CLI
  dbg_printf("[INIT] Creating CLI task...\r\n");
  // CLI task for processing terminal commands via UART
  // Stack size: 5KB balanced for optimized CLI (buffers 128B, no history)
  // With nested calls (cli_execute→cmd_xxx→cli_printf) need ~3KB + debug margin
  // 5KB provides stability while still saving 3KB vs original 8KB
  const osThreadAttr_t cli_attr = {
    .name = "CliTask",
    .priority = osPriorityBelowNormal,
    .stack_size = 5120  // 5KB - Balanced (was 3KB=too small, 8KB=too much)
  };
  osThreadId_t cli_handle = osThreadNew(CliTask, NULL, &cli_attr);
  if (cli_handle == NULL) {
    dbg_printf("[ERROR] Failed to create CLI task!\r\n");
  } else {
    dbg_printf("[INIT] CLI task created successfully\r\n");
  }
#else
  dbg_printf("[WARNING] MODULE_ENABLE_CLI not defined - CLI disabled\r\n");
#endif

  dbg_printf("[INIT] About to start MIDI IO task...\r\n");
  
  // Optional UART debug stream (raw ADC values)
#if MODULE_ENABLE_AIN_RAW_DEBUG
  ain_raw_debug_task_create();
#endif

  app_start_midi_io_task();
  
  dbg_printf("[INIT] MIDI IO task started\r\n");
  dbg_printf("[INIT] app_init_and_start() complete - returning to scheduler\r\n");
}

static void AinTask(void *argument)
{
  (void)argument;
#if MODULE_ENABLE_AIN
  for (;;) {
    ain_tick_5ms();
    osDelay(5);
  }
#else
  // Module disabled, just idle
  for (;;) {
    osDelay(1000);
  }
#endif
}

#if MODULE_ENABLE_OLED && MODULE_ENABLE_AIN
// Super simple visualization:
// - each NOTE_ON draws a bright pixel at (x=key, y=velocity scaled)
// - NOTE_OFF draws a dim pixel
static void plot_event(uint8_t key, uint8_t vel, uint8_t on)
{
  uint8_t* fb = oled_framebuffer();

  // Map key (0..63) to x (0..255)
  uint16_t x = (uint16_t)key * 4u; // 0..252
  // Map vel (1..127) to y (0..63)
  uint16_t y = 63u - (uint16_t)((vel * 63u) / 127u);

  // 4-bit grayscale: 0..15
  uint8_t gray = on ? 0xF : 0x3;

  // Set a 2x2 block for visibility.
  for (uint16_t dy=0; dy<2u; dy++) {
    for (uint16_t dx=0; dx<2u; dx++) {
      uint16_t xx = x + dx;
      uint16_t yy = y + dy;
      if (xx >= 256u || yy >= 64u) continue;

      uint32_t idx = (yy << 8u) + xx; // Optimize: yy * 256 as shift
      uint32_t byte = idx >> 1u; // 2 pixels per byte
      uint8_t hi = (idx & 1u) == 0u; // even pixel -> high nibble
      uint8_t v = fb[byte];
      if (hi) v = (uint8_t)((v & 0x0Fu) | (gray << 4));
      else    v = (uint8_t)((v & 0xF0u) | (gray & 0x0Fu));
      fb[byte] = v;
    }
  }
}
#endif

static void OledDemoTask(void *argument)
{
  (void)argument;

#if MODULE_ENABLE_OLED && MODULE_ENABLE_AIN
  oled_clear();
  oled_flush();

  uint32_t last_flush = osKernelGetTickCount();

  for (;;) {
    ain_event_t ev;
    while (ain_pop_event(&ev)) {
      if (ev.type == AIN_EV_NOTE_ON) plot_event(ev.key, ev.velocity, 1);
      else if (ev.type == AIN_EV_NOTE_OFF) plot_event(ev.key, 1, 0);
    }

    // Flush at ~20 FPS max to reduce SPI load
    uint32_t now = osKernelGetTickCount();
    if ((now - last_flush) >= 50) {
      oled_flush();
      last_flush = now;
    }

    osDelay(10);
  }
#else
  // Module disabled, just idle
  for (;;) {
    osDelay(1000);
  }
#endif
}

#if MODULE_ENABLE_SRIO
static inline uint8_t din_get_bit(const uint8_t* din, uint16_t phys) {
  uint16_t byte = phys >> 3u;
  uint8_t bit = phys & 7u;
  return (din[byte] & (1u << bit)) ? 1u : 0u;
}
#endif

/** Boot-time SHIFT detection.
    SHIFT is phys_id 10 (see Services/input/input.c).
    active_low=1 means pressed -> 0 on the DIN line.
*/
static uint8_t boot_shift_held(uint8_t active_low) {
#if MODULE_ENABLE_SRIO && defined(SRIO_ENABLE)
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
  if (srio_read_din(din) < 0) return 0;
  uint8_t raw = din_get_bit(din, 10);
  return active_low ? (raw ? 0u : 1u) : (raw ? 1u : 0u);
#else
  (void)active_low;
  return 0;
#endif
}

#if MODULE_ENABLE_CLI
/**
 * @brief CLI task for processing UART terminal commands
 * 
 * This task continuously calls cli_task() to process incoming
 * commands from the UART terminal (MIOS Studio compatible).
 * Commands are parsed and executed in real-time.
 */
static void CliTask(void *argument)
{
  (void)argument;
  
  // Diagnostic trace: numbered checkpoints to identify exact blocking point
  dbg_printf("[CLI-TASK-01] Entry point reached\r\n");
  
  dbg_printf("[CLI-TASK-02] Before first osDelay(10)\r\n");
  osDelay(10);
  dbg_printf("[CLI-TASK-03] After first osDelay(10) - SUCCESS\r\n");
  
  // Wait for USB CDC to be fully enumerated if using USB CDC
  // This typically takes 2-5 seconds after boot
#if MODULE_ENABLE_USB_CDC
  dbg_printf("[CLI-TASK-04] USB CDC mode - before osDelay(2000)\r\n");
  osDelay(2000);  // 2 seconds for USB CDC to fully enumerate
  dbg_printf("[CLI-TASK-05] USB CDC mode - after osDelay(2000) - SUCCESS\r\n");
#else
  dbg_printf("[CLI-TASK-04] UART mode - before osDelay(100)\r\n");
  osDelay(100);  // Small delay to let UART stabilize
  dbg_printf("[CLI-TASK-05] UART mode - after osDelay(100) - SUCCESS\r\n");
#endif
  
  // Now print welcome banner - USB CDC should be ready
#if MODULE_ENABLE_USB_CDC
  // If USB CDC just connected, repeat important boot info that was likely lost
  extern uint8_t boot_reason_get(void);
  dbg_printf("[CLI-TASK-06] Before cli_printf system ready\r\n");
  cli_printf("\r\n");
  cli_printf("=== MidiCore System Ready ===\r\n");
  cli_printf("Boot reason: %d\r\n", (int)boot_reason_get());
  cli_printf("CLI commands: %lu registered\r\n", (unsigned long)cli_get_command_count());
  cli_printf("\r\n");
  dbg_printf("[CLI-TASK-07] After cli_printf system ready - SUCCESS\r\n");
#endif
  
  dbg_printf("[CLI-TASK-08] Before cli_print_banner()\r\n");
  cli_print_banner();
  dbg_printf("[CLI-TASK-09] After cli_print_banner() - SUCCESS\r\n");
  
  dbg_printf("[CLI-TASK-10] Before cli_print_prompt()\r\n");
  cli_print_prompt();
  dbg_printf("[CLI-TASK-11] After cli_print_prompt() - SUCCESS\r\n");
  
  dbg_printf("[CLI-TASK-12] Entering main command processing loop\r\n");
  
  // CLI processing loop
  uint32_t loop_count = 0;
  for (;;) {
    if (loop_count == 0) {
      dbg_printf("[CLI-TASK-13] First loop iteration - calling cli_task()\r\n");
    }
    
    cli_task();
    
    if (loop_count == 0) {
      dbg_printf("[CLI-TASK-14] First cli_task() call completed - SUCCESS\r\n");
      dbg_printf("[CLI-TASK-15] CLI fully operational - entering normal operation\r\n");
    }
    
    osDelay(10);  // 10ms polling interval
    loop_count++;
  }
}
#endif


