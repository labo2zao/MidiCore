#include "App/app_init.h"

#include "Hal/spi_bus.h"
#include "Hal/ainser64_hw/hal_ainser64_hw_step.h"
#include "Services/ain/ain.h"
#include "Hal/oled_ssd1322/oled_ssd1322.h"
#include "Services/router/router.h"
#include "Services/router/router_send.h"
#include "Services/midi/midi_din.h"
#include "Services/patch/patch.h"
#include "Services/patch/patch_router.h"
#include "Services/patch/patch_sd_mount.h"
#include "Services/patch/patch_system.h"
#include "Services/looper/looper.h"
#include "Services/ui/ui.h"
#include "Services/safe/safe_mode.h"
#include "Services/system/system_status.h"
#include "Services/watchdog/watchdog.h"
#include "Services/system/boot_reason.h"
#include "Services/log/log.h"
#include "Services/instrument/instrument_cfg.h"
#include "Services/humanize/humanize.h"
#include "App/ain_midi_task.h"
#include "Services/config/config.h"
#include "Services/srio/srio.h"
#include "Services/srio/srio_user_config.h"
#include "Services/usb_midi/usb_midi.h"
#include "App/midi_io_task.h"
#include "App/looper_selftest.h"

#include "cmsis_os2.h"
#include <string.h>

static void AinTask(void *argument);
static void OledDemoTask(void *argument);

void app_init_and_start(void)
{
  // Init shared services
  spibus_init();
  hal_ainser64_init();
  ain_init();
  oled_init();
  router_init(router_send_default);
  midi_din_init();
  usb_midi_init();
  patch_init();


// ---- Safety / boot mode ----
// Try mount SD early to read global config
int sd_ok = (patch_sd_mount_init() == 0);
safe_mode_set_sd_ok(sd_ok ? 1u : 0u);

config_t global_cfg;
config_set_defaults(&global_cfg);
if (sd_ok) {
  (void)config_load_from_sd(&global_cfg, "0:/cfg/global.ngc");

  instrument_cfg_t icfg; instrument_cfg_defaults(&icfg);
  if (sd_ok) { (void)instrument_cfg_load_sd(&icfg, "0:/cfg/instrument.ngc"); }
  instrument_cfg_set(&icfg);

  zones_cfg_t zcfg; zones_cfg_defaults(&zcfg);
  if (sd_ok) { (void)zones_cfg_load_sd(&zcfg, "0:/cfg/zones.ngc"); }
  zones_cfg_set(&zcfg);

  expr_cfg_t ecfg; expression_cfg_defaults(&ecfg);
  if (sd_ok) { (void)expression_cfg_load_sd(&ecfg, "0:/cfg/expression.ngc"); }
  expression_set_cfg(&ecfg);
  pressure_cfg_t pcfg; pressure_defaults(&pcfg);
  if (sd_ok) { (void)pressure_load_sd(&pcfg, "0:/cfg/pressure.ngc"); }
  pressure_set_cfg(&pcfg);
  // Debug: scan I2C bus to confirm pressure sensor address
  app_i2c_scan_and_log(pcfg.i2c_bus);
  humanize_init(osKernelGetTickCount());
}

// Hold SHIFT at boot to force SAFE_MODE
uint8_t shift_held = boot_shift_held(global_cfg.global_shift_active_low);
safe_mode_set_forced(shift_held ? 1u : 0u);
safe_mode_set_cfg(global_cfg.global_safe_mode ? 1u : 0u);
// SD/FATFS mount + load patch then apply router rules from [router]
    // SD/FATFS mount + load router_default + last active patch (bank/state)
  if (!system_is_fatal()) {
    patch_system_init();
  } else {
    // SD required but missing: stay alive with minimal UI
    ui_set_status_line("SD REQUIRED");
  }
  // OLED header init
  const patch_manager_t* pm = patch_system_get();
  const char* bank = pm->bank.bank_id[0] ? pm->bank.bank_id : pm->bank.bank_name;
  const char* patch = pm->bank.patches[pm->state.patch_index].label[0] ? pm->bank.patches[pm->state.patch_index].label : "patch";
  ui_set_patch_status(bank, patch);
looper_init();
  ui_init();
  boot_reason_init();
  watchdog_init();
  log_init();
  log_printf("BOOT", "reason=%d", (int)boot_reason_get());
  // Default routing examples
  router_set_route(ROUTER_NODE_DIN_IN1, ROUTER_NODE_DIN_OUT1, 1);
  router_set_route(ROUTER_NODE_DIN_IN2, ROUTER_NODE_DIN_OUT1, 1);
  router_set_route(ROUTER_NODE_DIN_IN3, ROUTER_NODE_DIN_OUT1, 1);
  router_set_route(ROUTER_NODE_DIN_IN4, ROUTER_NODE_DIN_OUT1, 1);
  // Default: Looper playback -> DIN OUT1
  router_set_route(ROUTER_NODE_LOOPER, ROUTER_NODE_DIN_OUT1, 1);

  // Create tasks
  const osThreadAttr_t ain_attr = {
    .name = "AinTask",
    .priority = osPriorityNormal,
    .stack_size = 1024
  };
  (void)osThreadNew(AinTask, NULL, &ain_attr);
  app_start_ain_midi_task();
  app_start_pressure_task();
  app_start_calibration_task();

  const osThreadAttr_t oled_attr = {
    .name = "OledDemo",
    .priority = osPriorityLow,
    .stack_size = 1024
  };
  (void)osThreadNew(OledDemoTask, NULL, &oled_attr);
  app_start_midi_io_task();
  app_start_looper_selftest();
}

static void AinTask(void *argument)
{
  (void)argument;
  for (;;) {
    ain_tick_5ms();
    osDelay(5);
  }
}

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
  for (uint16_t dy=0; dy<2; dy++) {
    for (uint16_t dx=0; dx<2; dx++) {
      uint16_t xx = x + dx;
      uint16_t yy = y + dy;
      if (xx >= 256 || yy >= 64) continue;

      uint32_t idx = (yy * 256u + xx);
      uint32_t byte = idx >> 1; // 2 pixels per byte
      uint8_t hi = (idx & 1u) == 0; // even pixel -> high nibble
      uint8_t v = fb[byte];
      if (hi) v = (uint8_t)((v & 0x0F) | (gray << 4));
      else    v = (uint8_t)((v & 0xF0) | (gray & 0x0F));
      fb[byte] = v;
    }
  }
}

static void OledDemoTask(void *argument)
{
  (void)argument;

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
}static uint8_t din_get_bit(const uint8_t* din, uint16_t phys) {
  uint16_t byte = (uint16_t)(phys >> 3);
  uint8_t bit = (uint8_t)(phys & 7u);
  return (din[byte] & (1u<<bit)) ? 1u : 0u;
}

/** Boot-time SHIFT detection.
    SHIFT is phys_id 10 (see Services/input/input.c).
    active_low=1 means pressed -> 0 on the DIN line.
*/
static uint8_t boot_shift_held(uint8_t active_low) {
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
  if (srio_read_din(din) < 0) return 0;
  uint8_t raw = din_get_bit(din, 10);
  return active_low ? (raw ? 0u : 1u) : (raw ? 1u : 0u);
#else
  (void)active_low;
  return 0;
#endif
}


