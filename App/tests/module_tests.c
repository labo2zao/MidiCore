/**
 * @file module_tests.c
 * @brief Implementation of unified module testing framework
 */

#include "App/tests/module_tests.h"
#include "App/tests/test_debug.h"
#include "Config/module_config.h"

#include "cmsis_os2.h"
#include <string.h>
#include <stdbool.h>

// Conditional includes for all modules that might be tested
#if MODULE_ENABLE_AINSER64
#include "Hal/spi_bus.h"
#include "Hal/ainser64_hw/hal_ainser64_hw_step.h"
#include "Hal/uart_midi/hal_uart_midi.h"
#endif

#if MODULE_ENABLE_SRIO
#include "Services/srio/srio.h"
#include "Services/srio/srio_user_config.h"

static const char* gpio_port_name(GPIO_TypeDef* port)
{
  if (port == GPIOA) return "GPIOA";
  if (port == GPIOB) return "GPIOB";
  if (port == GPIOC) return "GPIOC";
  if (port == GPIOD) return "GPIOD";
  if (port == GPIOE) return "GPIOE";
  if (port == GPIOF) return "GPIOF";
  if (port == GPIOG) return "GPIOG";
  if (port == GPIOH) return "GPIOH";
  if (port == GPIOI) return "GPIOI";
  return "GPIO?";
}

static int gpio_pin_index(uint16_t pin)
{
  for (int i = 0; i < 16; i++) {
    if (pin & (1u << i)) {
      return i;
    }
  }
  return -1;
}

static void dbg_print_gpio_pin(const char* label, GPIO_TypeDef* port, uint16_t pin)
{
  dbg_print("  ");
  dbg_print(label);
  dbg_print(": ");
  dbg_print(gpio_port_name(port));

  int index = gpio_pin_index(pin);
  if (index >= 0) {
    dbg_print_uint((uint32_t)index);
  } else {
    dbg_print("0x");
    dbg_print_hex16(pin);
  }
  dbg_print("\r\n");
}

static const char* spi_instance_name(SPI_HandleTypeDef* hspi)
{
  if (!hspi || !hspi->Instance) {
    return "UNKNOWN";
  }
  if (hspi->Instance == SPI1) return "SPI1";
  if (hspi->Instance == SPI2) return "SPI2";
  if (hspi->Instance == SPI3) return "SPI3";
  return "SPI?";
}

static void dbg_print_srio_pinout(void)
{
  dbg_print("SRIO Pinout:\r\n");
  dbg_printf("  SPI Instance: %s\r\n", spi_instance_name(SRIO_SPI_HANDLE));
#ifdef MIOS_SPI1_SCK_GPIO_Port
  dbg_print_gpio_pin("SPI SCK", MIOS_SPI1_SCK_GPIO_Port, MIOS_SPI1_SCK_Pin);
#endif
#ifdef MIOS_SPI1_MISO_GPIO_Port
  dbg_print_gpio_pin("SPI MISO", MIOS_SPI1_MISO_GPIO_Port, MIOS_SPI1_MISO_Pin);
#endif
#ifdef MIOS_SPI1_S0_GPIO_Port
  dbg_print_gpio_pin("SPI MOSI", MIOS_SPI1_S0_GPIO_Port, MIOS_SPI1_S0_Pin);
#endif
  dbg_print_gpio_pin("DIN /PL (RC2)", SRIO_DIN_PL_PORT, SRIO_DIN_PL_PIN);
  dbg_print_gpio_pin("DOUT RCLK (RC1)", SRIO_DOUT_RCLK_PORT, SRIO_DOUT_RCLK_PIN);
}
#endif

#if MODULE_ENABLE_MIDI_DIN
#include "Services/midi/midi_din.h"
#endif

#if MODULE_ENABLE_ROUTER
#include "Services/router/router.h"
#include "Services/router/router_send.h"
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
  if (test >= MODULE_TEST_NONE_ID && test <= MODULE_TEST_ALL_ID) {
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
#if defined(MODULE_TEST_GDB_DEBUG)
  return MODULE_TEST_GDB_DEBUG_ID;
#elif defined(MODULE_TEST_AINSER64)
  return MODULE_TEST_AINSER64_ID;
#elif defined(MODULE_TEST_SRIO)
  return MODULE_TEST_SRIO_ID;
#elif defined(MODULE_TEST_MIDI_DIN) || defined(APP_TEST_DIN_MIDI)
  return MODULE_TEST_MIDI_DIN_ID;
#elif defined(MODULE_TEST_ROUTER)
  return MODULE_TEST_ROUTER_ID;
#elif defined(MODULE_TEST_LOOPER) || defined(LOOPER_SELFTEST)
  return MODULE_TEST_LOOPER_ID;
#elif defined(MODULE_TEST_UI)
  return MODULE_TEST_UI_ID;
#elif defined(MODULE_TEST_PATCH_SD)
  return MODULE_TEST_PATCH_SD_ID;
#elif defined(MODULE_TEST_PRESSURE)
  return MODULE_TEST_PRESSURE_ID;
#elif defined(MODULE_TEST_USB_HOST_MIDI)
  return MODULE_TEST_USB_HOST_MIDI_ID;
#elif defined(MODULE_TEST_ALL)
  return MODULE_TEST_ALL_ID;
#elif defined(APP_TEST_AINSER_MIDI)
  return MODULE_TEST_AINSER64_ID;
#else
  return MODULE_TEST_NONE_ID;
#endif
}

// =============================================================================
// TEST RUNNER
// =============================================================================

int module_tests_run(module_test_t test)
{
  switch (test) {
    case MODULE_TEST_GDB_DEBUG_ID:
      module_test_gdb_debug_run();
      break;
      
    case MODULE_TEST_AINSER64_ID:
      module_test_ainser64_run();
      break;
      
    case MODULE_TEST_SRIO_ID:
      module_test_srio_run();
      break;
      
    case MODULE_TEST_MIDI_DIN_ID:
      module_test_midi_din_run();
      break;
      
    case MODULE_TEST_ROUTER_ID:
      module_test_router_run();
      break;
      
    case MODULE_TEST_LOOPER_ID:
      module_test_looper_run();
      break;
      
    case MODULE_TEST_UI_ID:
      module_test_ui_run();
      break;
      
    case MODULE_TEST_PATCH_SD_ID:
      return module_test_patch_sd_run();
      
    case MODULE_TEST_PRESSURE_ID:
      module_test_pressure_run();
      break;
      
    case MODULE_TEST_USB_HOST_MIDI_ID:
      module_test_usb_host_midi_run();
      break;
      
    case MODULE_TEST_ALL_ID:
      // Run all tests sequentially (where possible)
      // Most tests loop forever, so this is limited
      return -1; // Not implemented for now
      
    case MODULE_TEST_NONE_ID:
    default:
      return -1;
  }
  
  return 0;
}

// =============================================================================
// INDIVIDUAL MODULE TEST IMPLEMENTATIONS
// =============================================================================

void module_test_gdb_debug_run(void)
{
  // Simple UART verification test - ideal for GDB debugging
  dbg_print("\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("UART Debug Verification: OK\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("\r\n");
  
  dbg_print_test_header("GDB Debug / UART Verification Test");
  
  dbg_print("This test confirms UART communication is working.\r\n");
  dbg_print("\r\n");
  dbg_print("Configuration:\r\n");
  dbg_printf("  - UART Port: UART%d (Port %d)\r\n", TEST_DEBUG_UART_PORT + 1, TEST_DEBUG_UART_PORT);
  dbg_printf("  - Baud Rate: %d\r\n", TEST_DEBUG_UART_BAUD);
  dbg_print("  - Data: 8-N-1\r\n");
  dbg_print("\r\n");
  
  dbg_print("Hardware Pin Mapping (MIOS32-compatible):\r\n");
  dbg_print("  Port 0 (UART1/USART1): PA9/PA10   - MIDI OUT1/IN1\r\n");
  dbg_print("  Port 1 (UART2/USART2): PA2/PA3    - MIDI OUT2/IN2 (Debug)\r\n");
  dbg_print("  Port 2 (UART3/USART3): PB10/PB11  - MIDI OUT3/IN3\r\n");
  dbg_print("  Port 3 (UART5/UART5):  PC12/PD2   - MIDI OUT4/IN4\r\n");
  dbg_print("\r\n");
  
  dbg_print_separator();
  dbg_print("Test Output - Continuous Counter\r\n");
  dbg_print_separator();
  dbg_print("\r\n");
  
  uint32_t counter = 0;
  uint32_t last_print_ms = 0;
  
  for (;;) {
    uint32_t now_ms = osKernelGetTickCount();
    
    // Print every 1000ms
    if (now_ms - last_print_ms >= 1000) {
      last_print_ms = now_ms;
      counter++;
      
      // Print various formats to test output
      dbg_printf("Count: %lu | Time: %lu ms | Hex: 0x%08lX | Status: ", 
                 counter, now_ms, counter);
      
      // Test colored output indicators
      if (counter % 3 == 0) {
        dbg_print("OK");
      } else if (counter % 3 == 1) {
        dbg_print("TESTING");
      } else {
        dbg_print("ACTIVE");
      }
      
      dbg_print("\r\n");
      
      // Every 10 seconds, print a detailed status
      if (counter % 10 == 0) {
        dbg_print("\r\n");
        dbg_print("--- 10 Second Status ---\r\n");
        dbg_printf("Total iterations: %lu\r\n", counter);
        dbg_printf("FreeRTOS ticks: %lu\r\n", now_ms);
        dbg_print("UART is functioning correctly.\r\n");
        dbg_print("You can set breakpoints and inspect variables in GDB.\r\n");
        dbg_print("\r\n");
      }
    }
    
    osDelay(100); // 100ms delay
  }
}

void module_test_ainser64_run(void)
{
#if defined(APP_TEST_AINSER_MIDI)
  // Use existing AINSER test
  app_test_ainser_midi_run_forever();
#elif MODULE_ENABLE_AINSER64
  // Early UART verification
  dbg_print("\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("UART Debug Verification: OK\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("\r\n");
  osDelay(100); // Give time for UART transmission
  
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
  dbg_print("Values update on every scan (no delays between channels)\r\n");
  dbg_print("Press Ctrl+C to stop\r\n");
  dbg_print_separator();
  
  uint32_t scan_count = 0;
  uint16_t all_vals[8][8]; // [step][channel]
  
  for (;;) {
    // IMPORTANT: Read all 8 steps (mux channels) continuously without delays
    // This matches MIOS32 behavior and prevents stale/discontinuous values
    // The multiplexer needs continuous scanning to maintain stable readings
    for (uint8_t step = 0; step < 8; ++step) {
      if (hal_ainser64_read_bank_step(0u, step, all_vals[step]) != 0) {
        // Error reading - fill with zeros
        for (uint8_t m = 0; m < 8; m++) {
          all_vals[step][m] = 0;
        }
      }
      // NO DELAY HERE - immediate next step for continuous scanning
    }
    
    // Print every 100th scan to avoid flooding
    if ((scan_count % 100) == 0) {
      dbg_println();
      dbg_print("=== Scan #");
      dbg_print_uint(scan_count);
      dbg_print(" ===\r\n");
      dbg_println();
      
      // Print transposed: each line is one channel across all 8 modules
      for (uint8_t ch = 0; ch < 8; ++ch) {
        // Print channel header
        dbg_print("Channel ");
        dbg_print_uint(ch);
        dbg_print(" [M0-M7]: ");
        
        // Print values from all 8 modules for this channel
        for (uint8_t module = 0; module < 8; module++) {
          dbg_print_uint(all_vals[ch][module]);
          if (module < 7) dbg_print(", ");
        }
        dbg_println();
      }
    }
    
    scan_count++;
    
    // Small delay only AFTER complete scan to avoid flooding UART
    // In production code, this delay would not be needed
    osDelay(1); // 1ms delay between complete scans (not between channels!)
  }
#else
  // Module not enabled
  dbg_print("ERROR: AINSER64 module not enabled\r\n");
  dbg_print("Enable MODULE_ENABLE_AINSER64 in Config/module_config.h\r\n");
  for (;;) osDelay(1000);
#endif
}

void module_test_srio_run(void)
{
  // Early UART verification
  dbg_print("\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("UART Debug Verification: OK\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("\r\n");
  osDelay(100); // Give time for UART transmission
  
#if defined(DIN_SELFTEST) && defined(SRIO_ENABLE)
  // Use existing DIN selftest
  din_selftest_run();
#elif MODULE_ENABLE_SRIO && defined(SRIO_ENABLE)
  dbg_print_test_header("SRIO Module Test");
  
  // Initialize SRIO
  dbg_print("Initializing SRIO...");
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
  dbg_print(" OK\r\n");
  
  dbg_print_separator();
  GPIO_TypeDef* sck_port = NULL;
  uint16_t sck_pin = 0;
  GPIO_TypeDef* miso_port = NULL;
  uint16_t miso_pin = 0;
  GPIO_TypeDef* mosi_port = NULL;
  uint16_t mosi_pin = 0;
#ifdef MIOS_SPI1_SCK_GPIO_Port
  sck_port = MIOS_SPI1_SCK_GPIO_Port;
  sck_pin = MIOS_SPI1_SCK_Pin;
#endif
#ifdef MIOS_SPI1_MISO_GPIO_Port
  miso_port = MIOS_SPI1_MISO_GPIO_Port;
  miso_pin = MIOS_SPI1_MISO_Pin;
#endif
#ifdef MIOS_SPI1_S0_GPIO_Port
  mosi_port = MIOS_SPI1_S0_GPIO_Port;
  mosi_pin = MIOS_SPI1_S0_Pin;
#endif
  gdb_ptin_SPI_Pinout("SRIO", SRIO_SPI_HANDLE,
                      sck_port, sck_pin,
                      miso_port, miso_pin,
                      mosi_port, mosi_pin,
                      SRIO_DOUT_RCLK_PORT, SRIO_DOUT_RCLK_PIN,
                      SRIO_DIN_PL_PORT, SRIO_DIN_PL_PIN);
  dbg_print_separator();
  dbg_printf("Configuration: %d DIN bytes, %d DOUT bytes\r\n", 
             SRIO_DIN_BYTES, SRIO_DOUT_BYTES);
  dbg_printf("Total buttons: %d (8 per byte)\r\n", SRIO_DIN_BYTES * 8);
  dbg_print("Monitoring button presses (press any button)...\r\n");
  dbg_printf("Button numbers: 0-%d\r\n", (SRIO_DIN_BYTES * 8) - 1);
  dbg_print_separator();
  dbg_print("\r\n");
  
  uint8_t din[SRIO_DIN_BYTES];
  
  // Initialize first state
  int init_result = srio_read_din(din);
  if (init_result != 0) {
    dbg_printf("ERROR: SRIO init read failed with code %d\r\n", init_result);
    dbg_print("Check SPI and GPIO configuration!\r\n");
  } else {
    dbg_print("Initial DIN state read: ");
    for (uint8_t i = 0; i < SRIO_DIN_BYTES; i++) {
      dbg_printf("0x%02X ", din[i]);
    }
    dbg_print("\r\n");
    dbg_print("Expected: 0xFF 0xFF... (all buttons released with pull-ups)\r\n");
    dbg_print("If you see 0x00: inputs may be inverted or no pull-ups\r\n");
    dbg_print("If you see other values: some buttons may be stuck\r\n");
  }
  
  uint32_t scan_counter = 0;
  uint32_t last_activity_ms = osKernelGetTickCount();
  uint32_t last_debug_ms = osKernelGetTickCount();
  
  for (;;) {
    int result = srio_read_din(din);
    if (result != 0) {
      dbg_printf("ERROR: SRIO read failed with code %d\r\n", result);
      osDelay(1000);
      continue;
    }
    
    scan_counter++;
    
    // Check for button state changes using MIOS32-style change flags.
    bool changed = false;
    for (uint8_t byte_idx = 0; byte_idx < SRIO_DIN_BYTES; byte_idx++) {
      uint8_t diff = srio_din_changed_get_and_clear(byte_idx, 0xFF);
      if (!diff) continue;
      changed = true;

      uint8_t state = srio_din_get(byte_idx);
      // Check each bit in the byte
      for (uint8_t bit = 0; bit < 8; bit++) {
        if (diff & (1 << bit)) {
          uint16_t button_num = (byte_idx * 8) + bit;
          bool pressed = (state & (1 << bit)) == 0; // Active low
          
          dbg_printf("[Scan #%lu] Button %3d: %s\r\n", 
                     scan_counter, 
                     button_num, 
                     pressed ? "PRESSED " : "RELEASED");
        }
      }
    }
    
    if (changed) {
      last_activity_ms = osKernelGetTickCount();
    }
    
    // Print idle message and current DIN state every 5 seconds if no activity
    uint32_t now_ms = osKernelGetTickCount();
    if (now_ms - last_activity_ms >= 5000 && now_ms - last_debug_ms >= 5000) {
      dbg_printf("Waiting for button press... (scan count: %lu)\r\n", scan_counter);
      dbg_print("Current DIN state: ");
      for (uint8_t i = 0; i < SRIO_DIN_BYTES; i++) {
        dbg_printf("0x%02X ", srio_din_get(i));
      }
      dbg_print("\r\n");
      dbg_print("Raw last read: ");
      for (uint8_t i = 0; i < SRIO_DIN_BYTES; i++) {
        dbg_printf("0x%02X ", din[i]);
      }
      dbg_print("\r\n");
      last_debug_ms = now_ms;
    }
    
    osDelay(10); // 10ms scan rate = 100 Hz
  }
#else
  dbg_print_test_header("SRIO Test");
  dbg_print("ERROR: SRIO module not enabled!\r\n");
  dbg_print("Please enable MODULE_ENABLE_SRIO and SRIO_ENABLE\r\n");
  for (;;) {
    osDelay(1000);
  }
#endif
}

void module_test_midi_din_run(void)
{
  // Early UART verification
  dbg_print("\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("UART Debug Verification: OK\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("\r\n");
  osDelay(100);
  
#if defined(APP_TEST_DIN_MIDI)
  // Use existing DIN MIDI test
  app_test_din_midi_run_forever();
#elif MODULE_ENABLE_MIDI_DIN
  dbg_print_test_header("MIDI DIN Module Test");
  dbg_print("Initializing MIDI DIN service...");
  midi_din_init();
  dbg_print(" OK\r\n");
  dbg_print("\r\n");
  dbg_print("Listening for incoming MIDI bytes.\r\n");
  dbg_print("Press keys or send MIDI data from your controller.\r\n");
  dbg_print("Monitor output on the debug UART for activity.\r\n");
  dbg_print_separator();

  midi_din_stats_t prev_stats[MIDI_DIN_PORTS];
  midi_din_stats_t cur_stats[MIDI_DIN_PORTS];
  memset(prev_stats, 0, sizeof(prev_stats));
  memset(cur_stats, 0, sizeof(cur_stats));

  uint32_t last_poll_ms = osKernelGetTickCount();
  uint32_t last_idle_ms = last_poll_ms;

  for (;;) {
    midi_din_tick();

    uint32_t now_ms = osKernelGetTickCount();
    if (now_ms - last_poll_ms >= 50) {
      last_poll_ms = now_ms;
      bool any_activity = false;

      for (uint8_t port = 0; port < MIDI_DIN_PORTS; ++port) {
        midi_din_get_stats(port, &cur_stats[port]);

        if (cur_stats[port].rx_bytes != prev_stats[port].rx_bytes ||
            cur_stats[port].rx_msgs != prev_stats[port].rx_msgs ||
            cur_stats[port].rx_sysex_chunks != prev_stats[port].rx_sysex_chunks ||
            cur_stats[port].rx_drops != prev_stats[port].rx_drops ||
            cur_stats[port].rx_stray_data != prev_stats[port].rx_stray_data) {
          any_activity = true;

          dbg_printf("DIN%d: bytes=%lu msgs=%lu sysex=%lu drops=%lu stray=%lu",
                     port + 1,
                     cur_stats[port].rx_bytes,
                     cur_stats[port].rx_msgs,
                     cur_stats[port].rx_sysex_chunks,
                     cur_stats[port].rx_drops,
                     cur_stats[port].rx_stray_data);

          if (cur_stats[port].last_len > 0) {
            dbg_print(" last=");
            dbg_print_bytes(cur_stats[port].last_bytes,
                            cur_stats[port].last_len,
                            ' ');
            uint8_t status = cur_stats[port].last_bytes[0];
            if (status >= 0x80) {
              const char* label = "UNKNOWN";
              uint8_t channel = (uint8_t)((status & 0x0F) + 1);
              switch (status & 0xF0) {
                case 0x80: label = "NOTE_OFF"; break;
                case 0x90: label = "NOTE_ON"; break;
                case 0xA0: label = "POLY_AFTERTOUCH"; break;
                case 0xB0: label = "CONTROL_CHANGE"; break;
                case 0xC0: label = "PROGRAM_CHANGE"; break;
                case 0xD0: label = "CHANNEL_AFTERTOUCH"; break;
                case 0xE0: label = "PITCH_BEND"; break;
                case 0xF0:
                  switch (status) {
                    case 0xF0: label = "SYSEX_START"; break;
                    case 0xF1: label = "MTC_QUARTER_FRAME"; break;
                    case 0xF2: label = "SONG_POSITION"; break;
                    case 0xF3: label = "SONG_SELECT"; break;
                    case 0xF6: label = "TUNE_REQUEST"; break;
                    case 0xF8: label = "CLOCK"; break;
                    case 0xFA: label = "START"; break;
                    case 0xFB: label = "CONTINUE"; break;
                    case 0xFC: label = "STOP"; break;
                    case 0xFE: label = "ACTIVE_SENSE"; break;
                    case 0xFF: label = "RESET"; break;
                    default: label = "SYSTEM"; break;
                  }
                  break;
                default:
                  label = "UNKNOWN";
                  break;
              }
              dbg_printf(" msg=%s", label);
              if (status < 0xF0) {
                dbg_printf(" ch=%u", channel);
              }
            }
          }
          dbg_print("\r\n");

          prev_stats[port] = cur_stats[port];
        }
      }

      if (any_activity) {
        last_idle_ms = now_ms;
      } else if (now_ms - last_idle_ms >= 5000) {
        dbg_print("Waiting for MIDI DIN input...\r\n");
        last_idle_ms = now_ms;
      }
    }

    osDelay(1);
  }
#else
  // Module not enabled
  for (;;) osDelay(1000);
#endif
}

void module_test_router_run(void)
{
  // Early UART verification
  dbg_print("\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("UART Debug Verification: OK\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("\r\n");
  osDelay(100);
  
#if MODULE_ENABLE_ROUTER
  router_init(router_send_default);
  
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
  // Early UART verification
  dbg_print("\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("UART Debug Verification: OK\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("\r\n");
  osDelay(100);
  
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
  // Early UART verification
  dbg_print("\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("UART Debug Verification: OK\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("\r\n");
  osDelay(100);
  
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
  // Early UART verification
  dbg_print("\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("UART Debug Verification: OK\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("\r\n");
  osDelay(100);
  
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
  // Early UART verification
  dbg_print("\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("UART Debug Verification: OK\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("\r\n");
  osDelay(100);
  
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
  // Early UART verification
  dbg_print("\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("UART Debug Verification: OK\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("\r\n");
  osDelay(100);
  
#if MODULE_ENABLE_USBH_MIDI
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
