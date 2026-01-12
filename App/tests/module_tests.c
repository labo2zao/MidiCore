/**
 * @file module_tests.c
 * @brief Implementation of unified module testing framework
 */

#include "App/tests/module_tests.h"
#include "App/tests/test_debug.h"
#include "Config/module_config.h"

#include "cmsis_os2.h"
#include <string.h>

// Conditional includes for all modules that might be tested
#if MODULE_ENABLE_AINSER64 && MODULE_ENABLE_AIN
#include "Hal/spi_bus.h"
#include "Hal/ainser64_hw/hal_ainser64_hw_step.h"
#include "Hal/uart_midi/hal_uart_midi.h"
#endif

#if MODULE_ENABLE_SRIO
#include "Services/srio/srio.h"
#include "Services/srio/srio_user_config.h"
#endif

#if MODULE_ENABLE_MIDI_DIN
#include "Services/midi/midi_din.h"
#endif

#if MODULE_ENABLE_ROUTER
#include "Services/router/router.h"
#endif

#if MODULE_ENABLE_LOOPER
#include "Services/looper/looper.h"
#endif

#if MODULE_ENABLE_UI && MODULE_ENABLE_OLED
#include "Services/ui/ui.h"
#endif

#if MODULE_ENABLE_PATCH
#include "Services/patch/patch_sd_mount.h"
#include "Services/patch/patch.h"
#endif

#if MODULE_ENABLE_PRESSURE
#include "Services/pressure/pressure_i2c.h"
#endif

#if MODULE_ENABLE_USBH_MIDI
#include "Services/usb_host_midi/usb_host_midi.h"
#endif

// =============================================================================
// FORWARD DECLARATIONS FOR EXISTING TEST IMPLEMENTATIONS
// =============================================================================

// Existing test functions from other files
#if defined(APP_TEST_DIN_MIDI)
extern void app_test_din_midi_run_forever(void);
#endif

#if defined(APP_TEST_AINSER_MIDI)
extern void app_test_ainser_midi_run_forever(void);
#endif

#ifdef DIN_SELFTEST
extern void din_selftest_run(void);
#endif

#ifdef LOOPER_SELFTEST
extern void app_start_looper_selftest(void);
#endif

// =============================================================================
// TEST NAME TABLE
// =============================================================================

static const char* test_names[] = {
  "NONE",
  "AINSER64",
  "SRIO",
  "MIDI_DIN",
  "ROUTER",
  "LOOPER",
  "UI",
  "PATCH_SD",
  "PRESSURE",
  "USB_HOST_MIDI",
  "ALL"
};

const char* module_tests_get_name(module_test_t test)
{
  if (test >= MODULE_TEST_NONE && test <= MODULE_TEST_ALL) {
    return test_names[test];
  }
  return "UNKNOWN";
}

// =============================================================================
// INITIALIZATION
// =============================================================================

void module_tests_init(void)
{
  // Initialize debug UART
  test_debug_init();
  
  // Print startup banner
  dbg_print_test_header("MidiCore Module Test Framework");
  dbg_print_config_info();
}

// =============================================================================
// COMPILE-TIME TEST SELECTION
// =============================================================================

module_test_t module_tests_get_compile_time_selection(void)
{
#if defined(MODULE_TEST_AINSER64)
  return MODULE_TEST_AINSER64;
#elif defined(MODULE_TEST_SRIO)
  return MODULE_TEST_SRIO;
#elif defined(MODULE_TEST_MIDI_DIN) || defined(APP_TEST_DIN_MIDI)
  return MODULE_TEST_MIDI_DIN;
#elif defined(MODULE_TEST_ROUTER)
  return MODULE_TEST_ROUTER;
#elif defined(MODULE_TEST_LOOPER) || defined(LOOPER_SELFTEST)
  return MODULE_TEST_LOOPER;
#elif defined(MODULE_TEST_UI)
  return MODULE_TEST_UI;
#elif defined(MODULE_TEST_PATCH_SD)
  return MODULE_TEST_PATCH_SD;
#elif defined(MODULE_TEST_PRESSURE)
  return MODULE_TEST_PRESSURE;
#elif defined(MODULE_TEST_USB_HOST_MIDI)
  return MODULE_TEST_USB_HOST_MIDI;
#elif defined(MODULE_TEST_ALL)
  return MODULE_TEST_ALL;
#elif defined(APP_TEST_AINSER_MIDI)
  return MODULE_TEST_AINSER64;
#else
  return MODULE_TEST_NONE;
#endif
}

// =============================================================================
// TEST RUNNER
// =============================================================================

int module_tests_run(module_test_t test)
{
  switch (test) {
    case MODULE_TEST_AINSER64:
      module_test_ainser64_run();
      break;
      
    case MODULE_TEST_SRIO:
      module_test_srio_run();
      break;
      
    case MODULE_TEST_MIDI_DIN:
      module_test_midi_din_run();
      break;
      
    case MODULE_TEST_ROUTER:
      module_test_router_run();
      break;
      
    case MODULE_TEST_LOOPER:
      module_test_looper_run();
      break;
      
    case MODULE_TEST_UI:
      module_test_ui_run();
      break;
      
    case MODULE_TEST_PATCH_SD:
      return module_test_patch_sd_run();
      
    case MODULE_TEST_PRESSURE:
      module_test_pressure_run();
      break;
      
    case MODULE_TEST_USB_HOST_MIDI:
      module_test_usb_host_midi_run();
      break;
      
    case MODULE_TEST_ALL:
      // Run all tests sequentially (where possible)
      // Most tests loop forever, so this is limited
      return -1; // Not implemented for now
      
    case MODULE_TEST_NONE:
    default:
      return -1;
  }
  
  return 0;
}

// =============================================================================
// INDIVIDUAL MODULE TEST IMPLEMENTATIONS
// =============================================================================

void module_test_ainser64_run(void)
{
#if defined(APP_TEST_AINSER_MIDI)
  // Use existing AINSER test
  app_test_ainser_midi_run_forever();
#elif MODULE_ENABLE_AINSER64 && MODULE_ENABLE_AIN
  // Print test header
  dbg_print_test_header("AINSER64 Module Test");
  
  // Initialize hardware
  dbg_print("Initializing SPI bus...");
  spibus_init();
  dbg_print(" OK\r\n");
  
  dbg_print("Initializing AINSER64...");
  hal_ainser64_init();
  dbg_print(" OK\r\n");
  
  dbg_print("Initializing UART MIDI...");
  hal_uart_midi_init();
  dbg_print(" OK\r\n");
  
  dbg_print_separator();
  dbg_print("Scanning 64 channels continuously...\r\n");
  dbg_print("Press Ctrl+C to stop\r\n");
  dbg_print_separator();
  
  uint32_t scan_count = 0;
  
  for (;;) {
    for (uint8_t step = 0; step < 8; ++step) {
      uint16_t vals[8];
      if (hal_ainser64_read_bank_step(0u, step, vals) == 0) {
        // Successfully read values
        // Print every 100th scan to avoid flooding
        if ((scan_count % 100) == 0) {
          dbg_print("Step ");
          dbg_print_uint(step);
          dbg_print(": ");
          for (uint8_t ch = 0; ch < 8; ch++) {
            uint8_t idx = step * 8 + ch;
            dbg_print("CH");
            if (idx < 10) dbg_putc('0');
            dbg_print_uint(idx);
            dbg_print("=");
            dbg_print_uint(vals[ch]);
            if (ch < 7) dbg_print(" ");
          }
          dbg_println();
        }
      }
    }
    scan_count++;
    osDelay(10);
  }
#else
  // Module not enabled
  dbg_print("ERROR: AINSER64 module not enabled\r\n");
  dbg_print("Enable MODULE_ENABLE_AINSER64 and MODULE_ENABLE_AIN\r\n");
  for (;;) osDelay(1000);
#endif
}

void module_test_srio_run(void)
{
#ifdef DIN_SELFTEST
  // Use existing DIN selftest
  din_selftest_run();
#elif MODULE_ENABLE_SRIO
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
    srio_read_din(din);
    osDelay(10);
  }
#else
  // Module not enabled
  for (;;) osDelay(1000);
#endif
}

void module_test_midi_din_run(void)
{
#if defined(APP_TEST_DIN_MIDI)
  // Use existing DIN MIDI test
  app_test_din_midi_run_forever();
#elif MODULE_ENABLE_MIDI_DIN
  // Simple MIDI echo test
  for (;;) {
    osDelay(10);
    // Could implement MIDI echo here
  }
#else
  // Module not enabled
  for (;;) osDelay(1000);
#endif
}

void module_test_router_run(void)
{
#if MODULE_ENABLE_ROUTER
  router_init();
  
  // Test basic router functionality
  for (;;) {
    osDelay(100);
    // Could send test messages through router
  }
#else
  // Module not enabled
  for (;;) osDelay(1000);
#endif
}

void module_test_looper_run(void)
{
#ifdef LOOPER_SELFTEST
  // Use existing looper selftest
  app_start_looper_selftest();
  for (;;) osDelay(1000);
#elif MODULE_ENABLE_LOOPER
  looper_init();
  
  // Basic looper test cycle
  for (;;) {
    osDelay(1000);
    // Could implement test recording/playback cycle
  }
#else
  // Module not enabled
  for (;;) osDelay(1000);
#endif
}

void module_test_ui_run(void)
{
#if MODULE_ENABLE_UI && MODULE_ENABLE_OLED
  // Test UI drawing
  for (;;) {
    osDelay(100);
    // UI task would update display
  }
#else
  // Module not enabled
  for (;;) osDelay(1000);
#endif
}

int module_test_patch_sd_run(void)
{
#if MODULE_ENABLE_PATCH
  // Test SD card mounting
  int result = patch_sd_mount_retry(3);
  if (result == 0) {
    // SD mounted successfully
    // Could test patch loading here
    return 0;
  }
  return -1;
#else
  // Module not enabled
  return -1;
#endif
}

void module_test_pressure_run(void)
{
#if MODULE_ENABLE_PRESSURE
  // Test pressure sensor
  for (;;) {
    osDelay(100);
    // Could read and display pressure values
  }
#else
  // Module not enabled
  for (;;) osDelay(1000);
#endif
}

void module_test_usb_host_midi_run(void)
{
#if MODULE_ENABLE_USBH_MIDI
  #include "Services/usb_host_midi/usb_host_midi.h"
  
  usb_host_midi_init();
  
  // Test USB host MIDI
  for (;;) {
    usb_host_midi_task();
    osDelay(1);
  }
#else
  // Module not enabled
  for (;;) osDelay(1000);
#endif
}
