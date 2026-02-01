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

#if MODULE_ENABLE_STACK_MONITOR
#include "Services/stack_monitor/stack_monitor.h"
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

/* MidiCore cooperative architecture */
#include "App/midicore_main_task.h"

#include "cmsis_os2.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

/* Forward declarations */
static uint8_t boot_shift_held(uint8_t active_low);

void app_init_and_start(void)
{
  // NOTE: spibus_init() is called in main.c BEFORE osKernelStart()
  // DO NOT call it again here! Second call would reset g_spi1_mutex and g_spi3_mutex to NULL
  // after they were already created by spibus_begin(), causing NULL pointer access crashes!
  // See: docs/ROOT_CAUSE_DOUBLE_SPIBUS_INIT.md
  
  /* ========================================================================
   * MIOS32-STYLE INITIALIZATION
   * 
   * NO printf/dbg_printf during init - causes stack overflow!
   * Debug info available via:
   *   - MIOS Studio terminal (MIOS query SysEx)
   *   - Debugger variables
   * ======================================================================== */
  
  /* Heap diagnostics available in debugger via these globals: */
  /* (defined in freertos_hooks.c or here as needed) */
  
  /* Init shared services - NO logging! */

#if MODULE_ENABLE_AINSER64
  hal_ainser64_init();
#endif

#if MODULE_ENABLE_AIN
  ain_init();
#endif

#if MODULE_ENABLE_OLED
  // Production: Use complete Newhaven NHD-3.12 init (LoopA production code)
  // oled_init() is a simple MidiCore test init, not suitable for production
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
  // NOTE: Do NOT register echo callback here!
  // MIOS Studio terminal handles echoing on the PC side.
  // Echoing from firmware causes USB CDC conflicts with CLI output.
  // See: ROOT_CAUSE_USB_CDC_ECHO.md
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

  // Static config structures to avoid stack overflow
  // These were moved from local to static to prevent ~1000+ bytes on stack
  // See: STACK_ANALYSIS.md for rationale
  static config_t s_global_cfg;
  config_set_defaults(&s_global_cfg);
  
#if MODULE_ENABLE_PATCH
  if (sd_ok) {
    (void)config_load_from_sd(&s_global_cfg, "0:/cfg/global.ngc");

#if MODULE_ENABLE_INSTRUMENT
    static instrument_cfg_t s_icfg; 
    instrument_cfg_defaults(&s_icfg);
    if (sd_ok) { (void)instrument_cfg_load_sd(&s_icfg, "0:/cfg/instrument.ngc"); }
    instrument_cfg_set(&s_icfg);
#endif

#if MODULE_ENABLE_ZONES
    static zones_cfg_t s_zcfg; 
    zones_cfg_defaults(&s_zcfg);
    if (sd_ok) { (void)zones_cfg_load_sd(&s_zcfg, "0:/cfg/zones.ngc"); }
    zones_cfg_set(&s_zcfg);
#endif

#if MODULE_ENABLE_EXPRESSION
    static expr_cfg_t s_ecfg; 
    expression_cfg_defaults(&s_ecfg);
    if (sd_ok) { (void)expression_cfg_load_sd(&s_ecfg, "0:/cfg/expression.ngc"); }
    expression_set_cfg(&s_ecfg);
#endif

#if MODULE_ENABLE_PRESSURE
    static pressure_cfg_t s_pcfg; 
    pressure_defaults(&s_pcfg);
    if (sd_ok) { (void)pressure_load_sd(&s_pcfg, "0:/cfg/pressure.ngc"); }
    pressure_set_cfg(&s_pcfg);
    // Debug: scan I2C bus to confirm pressure sensor address
    app_i2c_scan_and_log(s_pcfg.i2c_bus);
#endif

#if MODULE_ENABLE_HUMANIZE
    humanize_init(osKernelGetTickCount());
#endif
  }
#endif

// Hold SHIFT at boot to force SAFE_MODE
  uint8_t shift_held = boot_shift_held(s_global_cfg.global_shift_active_low);
#if MODULE_ENABLE_SAFE_MODE
  safe_mode_set_forced(shift_held ? 1u : 0u);
  safe_mode_set_cfg(s_global_cfg.global_safe_mode ? 1u : 0u);
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
  /* Boot reason available via CLI 'status' command */
#endif

  /* System initialization complete - no logging needed! */

  /* Initialize CLI and module registry for terminal control */
#if MODULE_ENABLE_MODULE_REGISTRY
  module_registry_init();
#endif

#if MODULE_ENABLE_CLI
  cli_init();
  (void)cli_module_commands_init();
  
  /* Initialize MidiCore terminal hooks for thread-safe I/O */
  (void)midicore_hooks_init();
#endif

#if MODULE_ENABLE_TEST
  test_init();
  test_cli_init();
#endif

#if MODULE_ENABLE_STACK_MONITOR
  stack_monitor_init();
#endif

  /* Default routing examples */
#if MODULE_ENABLE_ROUTER && MODULE_ENABLE_MIDI_DIN
  router_set_route(ROUTER_NODE_DIN_IN1, ROUTER_NODE_DIN_OUT1, 1);
  router_set_route(ROUTER_NODE_DIN_IN2, ROUTER_NODE_DIN_OUT1, 1);
  router_set_route(ROUTER_NODE_DIN_IN3, ROUTER_NODE_DIN_OUT1, 1);
  router_set_route(ROUTER_NODE_DIN_IN4, ROUTER_NODE_DIN_OUT1, 1);
#endif

#if MODULE_ENABLE_ROUTER && MODULE_ENABLE_LOOPER && MODULE_ENABLE_MIDI_DIN
  /* Default: Looper playback -> DIN OUT1 */
  router_set_route(ROUTER_NODE_LOOPER, ROUTER_NODE_DIN_OUT1, 1);
#endif

  /* =========================================================================
   * TASK CREATION - Cooperative Architecture (Single Main Task)
   * =========================================================================
   * MidiCore uses a cooperative service-based design:
   * - Single MidiCore_MainTask handles all services cooperatively
   * - Deterministic 1ms tick, minimal stack usage
   * - Logic lives in service tick functions, not tasks
   * ========================================================================= */
  
  /* Calibration task runs once at startup and exits */
  app_start_calibration_task();
  
  /* Start the single MidiCore main task */
  (void)midicore_main_task_start();
  
  /* Optional debug stream - still allowed as separate task (rare) */
#if MODULE_ENABLE_AIN_RAW_DEBUG
  ain_raw_debug_task_create();
#endif

  /* Heap stats available via CLI 'heap' command or debugger */
}

/* ============================================================================
 * HELPER FUNCTIONS
 * ============================================================================
 */

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


