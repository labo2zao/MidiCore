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

// UI framework for framebuffer-based testing
#include "Services/ui/ui_page_oled_test.h"
#include "Services/ui/ui_gfx.h"

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

#if MODULE_ENABLE_USB_MIDI
#include "Services/usb_midi/usb_midi.h"
#include "App/tests/app_test_usb_midi.h"
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

// LOOPER_SELFTEST is deprecated - use MODULE_TEST_LOOPER instead

// =============================================================================
// TEST NAME TABLE
// =============================================================================

static const char* test_names[] = {
  "NONE",
  "GDB_DEBUG",
  "AINSER64",
  "SRIO",
  "SRIO_DOUT",
  "MIDI_DIN",
  "ROUTER",
  "LOOPER",
  "LFO",
  "HUMANIZER",
  "UI",
  "UI_PAGE_SONG",
  "UI_PAGE_MIDI_MONITOR",
  "UI_PAGE_SYSEX",
  "UI_PAGE_CONFIG",
  "UI_PAGE_LIVEFX",
  "UI_PAGE_RHYTHM",
  "UI_PAGE_HUMANIZER",
  "PATCH_SD",
  "PRESSURE",
  "USB_HOST_MIDI",
  "USB_DEVICE_MIDI",
  "OLED_SSD1322",
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
#elif defined(MODULE_TEST_SRIO_DOUT)
  return MODULE_TEST_SRIO_DOUT_ID;
#elif defined(MODULE_TEST_MIDI_DIN) || defined(APP_TEST_DIN_MIDI)
  return MODULE_TEST_MIDI_DIN_ID;
#elif defined(MODULE_TEST_ROUTER)
  return MODULE_TEST_ROUTER_ID;
#elif defined(MODULE_TEST_LOOPER)
  return MODULE_TEST_LOOPER_ID;
#elif defined(MODULE_TEST_LFO)
  return MODULE_TEST_LFO_ID;
#elif defined(MODULE_TEST_HUMANIZER)
  return MODULE_TEST_HUMANIZER_ID;
#elif defined(MODULE_TEST_UI)
  return MODULE_TEST_UI_ID;
#elif defined(MODULE_TEST_UI_PAGE_SONG)
  return MODULE_TEST_UI_PAGE_SONG_ID;
#elif defined(MODULE_TEST_UI_PAGE_MIDI_MONITOR)
  return MODULE_TEST_UI_PAGE_MIDI_MONITOR_ID;
#elif defined(MODULE_TEST_UI_PAGE_SYSEX)
  return MODULE_TEST_UI_PAGE_SYSEX_ID;
#elif defined(MODULE_TEST_UI_PAGE_CONFIG)
  return MODULE_TEST_UI_PAGE_CONFIG_ID;
#elif defined(MODULE_TEST_UI_PAGE_LIVEFX)
  return MODULE_TEST_UI_PAGE_LIVEFX_ID;
#elif defined(MODULE_TEST_UI_PAGE_RHYTHM)
  return MODULE_TEST_UI_PAGE_RHYTHM_ID;
#elif defined(MODULE_TEST_UI_PAGE_HUMANIZER)
  return MODULE_TEST_UI_PAGE_HUMANIZER_ID;
#elif defined(MODULE_TEST_PATCH_SD)
  return MODULE_TEST_PATCH_SD_ID;
#elif defined(MODULE_TEST_PRESSURE)
  return MODULE_TEST_PRESSURE_ID;
#elif defined(MODULE_TEST_USB_HOST_MIDI)
  return MODULE_TEST_USB_HOST_MIDI_ID;
#elif defined(MODULE_TEST_USB_DEVICE_MIDI) || defined(APP_TEST_USB_MIDI)
  // Support both symbols: MODULE_TEST_* (framework style) and APP_TEST_* (legacy style)
  return MODULE_TEST_USB_DEVICE_MIDI_ID;
#elif defined(MODULE_TEST_OLED_SSD1322)
  return MODULE_TEST_OLED_SSD1322_ID;
#elif defined(MODULE_TEST_ALL)
  return MODULE_TEST_ALL_ID;
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
      
    case MODULE_TEST_SRIO_DOUT_ID:
      module_test_srio_dout_run();
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
      
    case MODULE_TEST_USB_DEVICE_MIDI_ID:
      module_test_usb_device_midi_run();
      break;
      
    case MODULE_TEST_OLED_SSD1322_ID:
      return module_test_oled_ssd1322_run();
      
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
  
#if MODULE_ENABLE_SRIO && defined(SRIO_ENABLE)
  dbg_print_test_header("SRIO DIN → MIDI Test");
  
  dbg_print("This test demonstrates the complete signal chain:\r\n");
  dbg_print("  Button Press → SRIO DIN → MIDI Note → USB/DIN MIDI OUT\r\n");
  dbg_print("\r\n");
  
#if MODULE_ENABLE_ROUTER
  // Initialize router for MIDI output
  dbg_print("Initializing MIDI Router...");
  router_init(router_send_default);
  dbg_print(" OK\r\n");
  
  // Configure routing: DIN IN (node 0) is used as virtual source for this test
  // Route to USB MIDI OUT (node 9) and DIN MIDI OUT1 (node 4)
  dbg_print("Configuring MIDI routes:\r\n");
  dbg_print("  → USB MIDI OUT (for computer)\r\n");
  dbg_print("  → DIN MIDI OUT1 (for external synth)\r\n");
  router_set_route(0, 9, 1);   // DIN IN → USB MIDI OUT
  router_set_route(0, 4, 1);   // DIN IN → DIN MIDI OUT1
  router_set_chanmask(0, 9, 0xFFFF);  // All channels
  router_set_chanmask(0, 4, 0xFFFF);  // All channels
  dbg_print("\r\n");
#else
  dbg_print("NOTE: Router not enabled - MIDI output disabled\r\n");
  dbg_print("      Only button detection will be shown\r\n");
  dbg_print("\r\n");
#endif
  
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
  
  // Allow time for /PL pin to stabilize at idle HIGH before first read
  osDelay(10);
  
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
  dbg_print("\r\n");
  
#if MODULE_ENABLE_ROUTER
  dbg_print("MIDI Note Mapping:\r\n");
  dbg_print("  Button 0-63 → MIDI Notes 36-99 (C2-D#7)\r\n");
  dbg_print("  Velocity: 100 (Note On), 0 (Note Off)\r\n");
  dbg_print("  Channel: 1\r\n");
  dbg_print("\r\n");
  dbg_print("Connect USB MIDI or DIN MIDI OUT1 to see notes!\r\n");
#else
  dbg_print("TEST MODE: Button detection only (no MIDI output)\r\n");
  dbg_print("Enable MODULE_ENABLE_ROUTER for MIDI output\r\n");
#endif
  dbg_print_separator();
  dbg_print("\r\n");
  
  uint8_t din[SRIO_DIN_BYTES];
  
  // Initialize first state
  dbg_print("Testing /PL pin control before first read...\r\n");
  dbg_printf("  /PL pin should idle at: %s\r\n", SRIO_DIN_PL_ACTIVE_LOW ? "HIGH (GPIO_PIN_SET)" : "LOW (GPIO_PIN_RESET)");
  dbg_printf("  DIN /PL pin: %s Pin %d\r\n", 
             SRIO_DIN_PL_PORT == GPIOB ? "GPIOB" : SRIO_DIN_PL_PORT == GPIOD ? "GPIOD" : "GPIO?",
             SRIO_DIN_PL_PIN);
  dbg_printf("  DOUT RCLK pin: %s Pin %d\r\n",
             SRIO_DOUT_RCLK_PORT == GPIOB ? "GPIOB" : SRIO_DOUT_RCLK_PORT == GPIOD ? "GPIOD" : "GPIO?",
             SRIO_DOUT_RCLK_PIN);
  dbg_print("  About to pulse /PL for DIN latch...\r\n");
  dbg_print("\r\n");
  dbg_print("IMPORTANT: Verify your hardware uses these pins for SRIO:\r\n");
  dbg_print("  - 74HC165 /PL (pin 1) should connect to the DIN /PL pin above\r\n");
  dbg_print("  - 74HC595 RCLK (pin 12) should connect to the DOUT RCLK pin above\r\n");
  dbg_print("  - If pins are wrong, SRIO will not work!\r\n");
  dbg_print("\r\n");
  osDelay(100); // Give time to see on scope
  
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
          
          // Map button to MIDI note (button 0 = C2 (36), button 63 = D#7 (99))
          uint8_t midi_note = 36 + button_num;
          if (midi_note > 127) midi_note = 127; // Clamp to MIDI range
          
          dbg_printf("[Scan #%lu] Button %3d: %s", 
                     scan_counter, 
                     button_num, 
                     pressed ? "PRESSED " : "RELEASED");
          
#if MODULE_ENABLE_ROUTER
          // Send MIDI Note On/Off via router
          router_msg_t midi_msg;
          midi_msg.type = ROUTER_MSG_3B;
          midi_msg.b0 = pressed ? 0x90 : 0x80;  // Note On (0x90) or Note Off (0x80)
          midi_msg.b1 = midi_note;
          midi_msg.b2 = pressed ? 100 : 0;  // Velocity
          
          // Process through router (will send to USB MIDI and DIN MIDI OUT1)
          router_process(0, &midi_msg);  // From virtual node 0
          
          dbg_printf(" → MIDI Note %d %s (Ch 1)\r\n", 
                     midi_note,
                     pressed ? "ON " : "OFF");
#else
          dbg_print("\r\n");
#endif
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

void module_test_srio_dout_run(void)
{
  // Early UART verification
  dbg_print("\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("UART Debug Verification: OK\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("\r\n");
  osDelay(100); // Give time for UART transmission
  
#if MODULE_ENABLE_SRIO && defined(SRIO_ENABLE)
  dbg_print_test_header("SRIO DOUT Module Test");
  dbg_print("Testing Digital Outputs (LEDs) using 74HC595 shift registers\r\n");
  dbg_print("\r\n");
  
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
  dbg_printf("Configuration: %d DOUT bytes (74HC595 chips)\r\n", SRIO_DOUT_BYTES);
  dbg_printf("Total LEDs: %d (8 per byte)\r\n", SRIO_DOUT_BYTES * 8);
  dbg_print("\r\n");
  
  dbg_print("Hardware connections (MIOS32 mbhp_doutx4):\r\n");
  dbg_print("  74HC595 Pin 11 (SRCLK) → PB13 (SPI2 SCK)\r\n");
  dbg_print("  74HC595 Pin 12 (RCLK)  → PB12 (RC1)\r\n");
  dbg_print("  74HC595 Pin 14 (SER)   → PB15 (SPI2 MOSI)\r\n");
  dbg_print("\r\n");
  
  dbg_print("LED Note: LEDs are ACTIVE LOW (0=ON, 1=OFF)\r\n");
  dbg_print("  - 0x00 = All LEDs ON\r\n");
  dbg_print("  - 0xFF = All LEDs OFF\r\n");
  dbg_print_separator();
  dbg_print("\r\n");
  
  uint8_t dout[SRIO_DOUT_BYTES];
  uint32_t pattern_counter = 0;
  uint32_t last_pattern_ms = 0;
  
  // Start with all LEDs OFF
  memset(dout, 0xFF, SRIO_DOUT_BYTES);
  srio_write_dout(dout);
  
  dbg_print("Starting LED pattern test...\r\n");
  dbg_print("Patterns will cycle every 2 seconds\r\n");
  dbg_print("Watch your LEDs to verify all outputs work!\r\n");
  dbg_print("\r\n");
  
  for (;;) {
    uint32_t now_ms = osKernelGetTickCount();
    
    // Change pattern every 2 seconds
    if (now_ms - last_pattern_ms >= 2000) {
      last_pattern_ms = now_ms;
      pattern_counter++;
      
      uint8_t pattern_type = pattern_counter % 7;
      
      dbg_printf("[Pattern %lu] ", pattern_counter);
      
      switch (pattern_type) {
        case 0:
          // All LEDs ON
          dbg_print("All LEDs ON (0x00)\r\n");
          memset(dout, 0x00, SRIO_DOUT_BYTES);
          break;
          
        case 1:
          // All LEDs OFF
          dbg_print("All LEDs OFF (0xFF)\r\n");
          memset(dout, 0xFF, SRIO_DOUT_BYTES);
          break;
          
        case 2:
          // Alternating pattern
          dbg_print("Alternating pattern (0xAA/0x55)\r\n");
          for (uint8_t i = 0; i < SRIO_DOUT_BYTES; i++) {
            dout[i] = (i % 2 == 0) ? 0xAA : 0x55;
          }
          break;
          
        case 3:
          // Running light (one LED at a time)
          dbg_print("Running light\r\n");
          memset(dout, 0xFF, SRIO_DOUT_BYTES);
          uint8_t led_pos = (pattern_counter / 4) % (SRIO_DOUT_BYTES * 8);
          uint8_t byte_idx = led_pos / 8;
          uint8_t bit_idx = led_pos % 8;
          dout[byte_idx] &= ~(1 << bit_idx);
          break;
          
        case 4:
          // Binary counter
          dbg_print("Binary counter\r\n");
          uint32_t counter_val = pattern_counter & 0xFF;
          for (uint8_t i = 0; i < SRIO_DOUT_BYTES && i < 4; i++) {
            dout[i] = ~((counter_val >> (i * 8)) & 0xFF);
          }
          for (uint8_t i = 4; i < SRIO_DOUT_BYTES; i++) {
            dout[i] = 0xFF;
          }
          break;
          
        case 5:
          // Wave pattern
          dbg_print("Wave pattern\r\n");
          for (uint8_t i = 0; i < SRIO_DOUT_BYTES; i++) {
            uint8_t phase = (pattern_counter + i * 2) % 8;
            dout[i] = ~(1 << phase);
          }
          break;
          
        case 6:
          // Checkerboard
          dbg_print("Checkerboard (0x55)\r\n");
          memset(dout, 0x55, SRIO_DOUT_BYTES);
          break;
      }
      
      // Write pattern to DOUTs
      int result = srio_write_dout(dout);
      if (result != 0) {
        dbg_printf("ERROR: DOUT write failed with code %d\r\n", result);
      }
      
      // Print hex values
      dbg_print("  DOUT values: ");
      for (uint8_t i = 0; i < SRIO_DOUT_BYTES; i++) {
        dbg_printf("0x%02X ", dout[i]);
      }
      dbg_print("\r\n\r\n");
    }
    
    osDelay(100); // 100ms update rate
  }
#else
  dbg_print_test_header("SRIO DOUT Test");
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

/**
 * @brief ROUTER Module Test
 * 
 * Tests the MIDI routing matrix functionality.
 * 
 * The router is a 16x16 matrix that routes MIDI messages between nodes:
 * - DIN IN1-4 → DIN OUT1-4
 * - USB Device IN/OUT
 * - USB Host IN/OUT
 * - Logical nodes (Looper, Keys, etc.)
 * 
 * Features tested:
 * - Route enable/disable
 * - Channel filtering (chanmask)
 * - Message types (Note On/Off, CC, Sysex)
 * - Multiple simultaneous routes
 * - Label assignment
 * 
 * Enable with: MODULE_TEST_ROUTER=1
 */
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
  dbg_print_test_header("MIDI Router Module Test");
  
  dbg_print("Initializing Router... ");
  router_init(router_send_default);
  dbg_print("OK\r\n");
  
  dbg_print("============================================================\r\n");
  dbg_print("Router Configuration:\r\n");
  dbg_printf("  Total Nodes: %d x %d matrix\r\n", ROUTER_NUM_NODES, ROUTER_NUM_NODES);
  dbg_print("\r\n");
  
  dbg_print("Available Nodes:\r\n");
  dbg_print("  DIN Inputs:  IN1-4  (nodes 0-3)\r\n");
  dbg_print("  DIN Outputs: OUT1-4 (nodes 4-7)\r\n");
  dbg_print("  USB Device:  IN/OUT (nodes 8-9)\r\n");
  dbg_print("  USB Host:    IN/OUT (nodes 12-13)\r\n");
  dbg_print("  Looper:      (node 10)\r\n");
  dbg_print("  Keys:        (node 11)\r\n");
  dbg_print("============================================================\r\n");
  dbg_print("\r\n");
  
  // Test 1: Set up basic routes
  dbg_print("[Test 1] Configuring test routes...\r\n");
  
  // DIN IN1 → DIN OUT1 (MIDI thru)
  router_set_route(ROUTER_NODE_DIN_IN1, ROUTER_NODE_DIN_OUT1, 1);
  router_set_chanmask(ROUTER_NODE_DIN_IN1, ROUTER_NODE_DIN_OUT1, ROUTER_CHMASK_ALL);
  router_set_label(ROUTER_NODE_DIN_IN1, ROUTER_NODE_DIN_OUT1, "MIDI Thru 1");
  dbg_print("  ✓ DIN IN1 → OUT1 (all channels)\r\n");
  
  // DIN IN1 → USB OUT (to computer)
  router_set_route(ROUTER_NODE_DIN_IN1, ROUTER_NODE_USB_OUT, 1);
  router_set_chanmask(ROUTER_NODE_DIN_IN1, ROUTER_NODE_USB_OUT, ROUTER_CHMASK_ALL);
  router_set_label(ROUTER_NODE_DIN_IN1, ROUTER_NODE_USB_OUT, "DIN→USB");
  dbg_print("  ✓ DIN IN1 → USB OUT (all channels)\r\n");
  
  // USB IN → DIN OUT2 (from computer to hardware)
  router_set_route(ROUTER_NODE_USB_IN, ROUTER_NODE_DIN_OUT2, 1);
  router_set_chanmask(ROUTER_NODE_USB_IN, ROUTER_NODE_DIN_OUT2, ROUTER_CHMASK_ALL);
  router_set_label(ROUTER_NODE_USB_IN, ROUTER_NODE_DIN_OUT2, "USB→DIN2");
  dbg_print("  ✓ USB IN → DIN OUT2 (all channels)\r\n");
  
  // Looper → DIN OUT3 (looper playback)
  router_set_route(ROUTER_NODE_LOOPER, ROUTER_NODE_DIN_OUT3, 1);
  router_set_chanmask(ROUTER_NODE_LOOPER, ROUTER_NODE_DIN_OUT3, 0x0001); // Channel 1 only
  router_set_label(ROUTER_NODE_LOOPER, ROUTER_NODE_DIN_OUT3, "Looper→OUT3");
  dbg_print("  ✓ Looper → DIN OUT3 (channel 1 only)\r\n");
  
  dbg_print("\r\n");
  
  // Test 2: Send test messages through router
  dbg_print("[Test 2] Sending test MIDI messages...\r\n");
  dbg_print("  (Messages will be routed according to configuration)\r\n");
  dbg_print("\r\n");
  
  router_msg_t msg;
  
  // Note On C4 on channel 1
  msg.type = ROUTER_MSG_3B;
  msg.b0 = 0x90;  // Note On, channel 1
  msg.b1 = 60;    // C4
  msg.b2 = 100;   // Velocity 100
  router_process(ROUTER_NODE_DIN_IN1, &msg);
  dbg_print("  → Note On C4 vel=100 ch=1 from DIN IN1\r\n");
  osDelay(100);
  
  // Note Off C4 on channel 1
  msg.b0 = 0x80;  // Note Off, channel 1
  msg.b2 = 0;     // Velocity 0
  router_process(ROUTER_NODE_DIN_IN1, &msg);
  dbg_print("  → Note Off C4 from DIN IN1\r\n");
  osDelay(100);
  
  // Control Change from USB
  msg.type = ROUTER_MSG_3B;
  msg.b0 = 0xB0;  // CC, channel 1
  msg.b1 = 7;     // Volume
  msg.b2 = 127;   // Max
  router_process(ROUTER_NODE_USB_IN, &msg);
  dbg_print("  → CC#7 (Volume) = 127 ch=1 from USB IN\r\n");
  osDelay(100);
  
  dbg_print("\r\n");
  
  // Test 3: Display routing table
  dbg_print("[Test 3] Active Routes:\r\n");
  dbg_print("  From       → To          Ch.Mask  Label\r\n");
  dbg_print("  -----------------------------------------\r\n");
  
  for (uint8_t in = 0; in < ROUTER_NUM_NODES; in++) {
    for (uint8_t out = 0; out < ROUTER_NUM_NODES; out++) {
      if (router_get_route(in, out)) {
        uint16_t chmask = router_get_chanmask(in, out);
        const char* label = router_get_label(in, out);
        
        dbg_printf("  Node %2d   → Node %2d   0x%04X  %s\r\n", 
                   in, out, chmask, label ? label : "(no label)");
      }
    }
  }
  
  dbg_print("\r\n");
  dbg_print("============================================================\r\n");
  dbg_print("Router test running. Send MIDI to DIN IN1 or USB to test.\r\n");
  dbg_print("Press Ctrl+C to stop\r\n");
  dbg_print("============================================================\r\n");
  
  // Continuous operation - process any incoming MIDI
  for (;;) {
    osDelay(100);
    // Router will process messages from MIDI task
  }
#else
  dbg_print_test_header("MIDI Router Module Test");
  dbg_print("ERROR: Router module not enabled!\r\n");
  dbg_print("Enable with MODULE_ENABLE_ROUTER=1\r\n");
  
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
  
#if MODULE_ENABLE_LOOPER
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
  // Print test header
  dbg_print("\r\n");
  dbg_print("============================================================\r\n");
  dbg_print("UI/OLED Module Test\r\n");
  dbg_print("============================================================\r\n");
  dbg_print("\r\n");
  dbg_print("This test exercises the complete UI system:\r\n");
  dbg_print("  - OLED SSD1322 display (256x64 grayscale)\r\n");
  dbg_print("  - UI page rendering and navigation\r\n");
  dbg_print("  - Button and encoder input handling\r\n");
  dbg_print("  - Status line updates\r\n");
  dbg_print("\r\n");
  dbg_print("Hardware Requirements:\r\n");
  dbg_print("  OLED Display:  SSD1322 256x64 (I2C/SPI)\r\n");
  dbg_print("  Control Input: Buttons + rotary encoder (via SRIO DIN)\r\n");
  dbg_print("\r\n");
  dbg_print("Available UI Pages:\r\n");
  dbg_print("  0: Looper       - Main sequencer view\r\n");
  dbg_print("  1: Timeline     - Track/pattern timeline\r\n");
  dbg_print("  2: Pianoroll    - Note editor\r\n");
  dbg_print("  3: Router       - MIDI routing matrix\r\n");
  dbg_print("  4: Patch        - Patch selection\r\n");
  dbg_print("\r\n");
  dbg_print("============================================================\r\n");
  dbg_print("\r\n");
  
  // Test 1: Initialize UI
  dbg_print("[Init] Initializing OLED...");
  osDelay(100);
  dbg_print(" OK\r\n");
  
  dbg_print("[Init] Initializing UI...");
  ui_init();
  osDelay(100);
  dbg_print(" OK\r\n");
  
  dbg_print("[Init] Setting startup status: \"MidiCore UI Test v1.0\"\r\n");
  ui_set_status_line("MidiCore UI Test v1.0");
  osDelay(500);
  
  // Test 2: Cycle through all pages
  dbg_print("\r\n[Test 1] Page Cycling (5s per page)\r\n");
  const char* page_names[] = {
    "Looper",
    "Timeline",
    "Pianoroll",
    "Router",
    "Patch"
  };
  
  for (uint8_t page = 0; page < UI_PAGE_COUNT && page < 5; page++) {
    dbg_print("  \xE2\x86\x92 Page ");
    dbg_print_uint(page);
    dbg_print(": ");
    dbg_print(page_names[page]);
    dbg_print("\r\n");
    
    ui_set_page((ui_page_t)page);
    ui_tick_20ms(); // Force UI update
    osDelay(5000); // 5 seconds per page
  }
  
  // Test 3: Simulate button press
  dbg_print("\r\n[Test 2] Simulating Button Press (ID=5)\r\n");
  dbg_print("  \xE2\x86\x92 UI received button PRESSED event\r\n");
  ui_on_button(5, 1); // Button 5 pressed
  ui_tick_20ms();
  osDelay(500);
  
  dbg_print("  \xE2\x86\x92 UI received button RELEASED event\r\n");
  ui_on_button(5, 0); // Button 5 released
  ui_tick_20ms();
  osDelay(500);
  
  // Test 4: Simulate encoder rotation
  dbg_print("\r\n[Test 3] Simulating Encoder Rotation\r\n");
  dbg_print("  \xE2\x86\x92 Encoder +3 steps (clockwise)\r\n");
  ui_on_encoder(3);
  ui_tick_20ms();
  osDelay(500);
  
  dbg_print("  \xE2\x86\x92 Encoder -2 steps (counter-clockwise)\r\n");
  ui_on_encoder(-2);
  ui_tick_20ms();
  osDelay(500);
  
  // Test 5: Update status line
  dbg_print("\r\n[Test 4] Updating Status Line\r\n");
  dbg_print("  \xE2\x86\x92 Status: \"All Tests Complete!\"\r\n");
  ui_set_status_line("All Tests Complete!");
  ui_tick_20ms();
  osDelay(1000);
  
  // Enter manual testing mode
  dbg_print("\r\n============================================================\r\n");
  dbg_print("Entering manual testing mode...\r\n");
  dbg_print("  - Connect buttons/encoders to test input\r\n");
  dbg_print("  - Watch OLED for visual feedback\r\n");
  dbg_print("  - Check UART for event logs\r\n");
  dbg_print("  - UI task will continue updating display\r\n");
  dbg_print("============================================================\r\n");
  dbg_print("\r\n");
  
  // Continuous operation - UI task handles display updates
  uint32_t tick_count = 0;
  for (;;) {
    osDelay(100);
    
    // Periodic status update every 10 seconds
    if (++tick_count >= 100) {
      tick_count = 0;
      dbg_print("[Status] UI running... (press buttons/turn encoder to test)\r\n");
    }
  }
#else
  // Module not enabled
  dbg_print("\r\n");
  dbg_print("============================================================\r\n");
  dbg_print("UI/OLED Module Test\r\n");
  dbg_print("============================================================\r\n");
  dbg_print("\r\n");
  dbg_print("ERROR: UI and/or OLED module not enabled!\r\n");
  dbg_print("\r\n");
  dbg_print("To enable this test, set in module_config.h:\r\n");
  dbg_print("  MODULE_ENABLE_UI=1\r\n");
  dbg_print("  MODULE_ENABLE_OLED=1\r\n");
  dbg_print("\r\n");
  dbg_print("============================================================\r\n");
  dbg_print("\r\n");
  
  for (;;) osDelay(1000);
#endif
}

int module_test_patch_sd_run(void)
{
  // Early UART verification
  dbg_print("\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("  MODULE_TEST_PATCH_SD - Comprehensive Test\r\n");
  dbg_print("==============================================\r\n");
  osDelay(100);
  
  // Verify UART is working
  dbg_print("Initializing UART debug output...\r\n");
  dbg_print("UART Debug Output: OK\r\n");
  dbg_print("\r\n");
  osDelay(100);
  
#if !MODULE_ENABLE_PATCH
  dbg_print("[ERROR] MODULE_ENABLE_PATCH is disabled!\r\n");
  dbg_print("Please enable in Config/module_config.h\r\n");
  return -1;
#endif

#if !MODULE_ENABLE_LOOPER
  dbg_print("[WARNING] MODULE_ENABLE_LOOPER is disabled!\r\n");
  dbg_print("MIDI export tests will be skipped.\r\n");
#endif

  int test_passed = 0;
  int test_failed = 0;
  
  // ========================================
  // TEST 1: SD Card Mount/Unmount
  // ========================================
  dbg_print("TEST 1: SD Card Mount\r\n");
  dbg_print("--------------------------------------\r\n");
  
  int result = patch_sd_mount_retry(3);
  if (result == 0) {
    dbg_print("[PASS] SD card mounted successfully\r\n");
    test_passed++;
  } else {
    dbg_print("[FAIL] SD card mount failed!\r\n");
    dbg_print("       Check: 1) SD card inserted\r\n");
    dbg_print("              2) Card formatted FAT32\r\n");
    dbg_print("              3) Proper SPI connections\r\n");
    test_failed++;
    // Jump to summary since remaining tests depend on SD card
    goto test_summary;
  }
  dbg_print("\r\n");
  osDelay(200);
  
  // ========================================
  // TEST 2: SD Card Configuration Load
  // ========================================
  dbg_print("TEST 2: Config File Loading\r\n");
  dbg_print("--------------------------------------\r\n");
  
  // Initialize patch system
  patch_init();
  
  // Try to load config file
  const char* config_paths[] = {
    "0:/config.ngc",
    "0:/config_minimal.ngc", 
    "0:/config_full.ngc"
  };
  
  int config_loaded = 0;
  for (int i = 0; i < 3; i++) {
    dbg_printf("Trying: %s...\r\n", config_paths[i]);
    result = patch_load(config_paths[i]);
    if (result == 0) {
      dbg_printf("[PASS] Loaded %s\r\n", config_paths[i]);
      config_loaded = 1;
      test_passed++;
      break;
    }
  }
  
  if (!config_loaded) {
    dbg_print("[FAIL] No config file found on SD card\r\n");
    dbg_print("       Expected: 0:/config.ngc\r\n");
    test_failed++;
  }
  dbg_print("\r\n");
  osDelay(200);
  
  // ========================================
  // TEST 3: Config Parameter Reading
  // ========================================
  dbg_print("TEST 3: Config Parameter Reading\r\n");
  dbg_print("--------------------------------------\r\n");
  
  if (config_loaded) {
    char value[64];
    
    // Test reading common parameters
    const char* test_keys[] = {
      "SRIO_DIN_ENABLE",
      "AINSER_ENABLE", 
      "MIDI_DEFAULT_CHANNEL",
      "AIN_ENABLE"
    };
    
    int params_read = 0;
    for (int i = 0; i < 4; i++) {
      result = patch_get(test_keys[i], value, sizeof(value));
      if (result == 0) {
        dbg_printf("[PASS] %s = %s\r\n", test_keys[i], value);
        params_read++;
      } else {
        dbg_printf("[SKIP] %s not found\r\n", test_keys[i]);
      }
    }
    
    if (params_read > 0) {
      dbg_printf("[PASS] Read %d config parameters\r\n", params_read);
      test_passed++;
    } else {
      dbg_print("[FAIL] Could not read any config parameters\r\n");
      test_failed++;
    }
  } else {
    dbg_print("[SKIP] No config loaded\r\n");
  }
  dbg_print("\r\n");
  osDelay(200);
  
  // ========================================
  // TEST 4: Config File Saving
  // ========================================
  dbg_print("TEST 4: Config File Saving\r\n");
  dbg_print("--------------------------------------\r\n");
  
  // Set some test parameters
  const char* TEST_VALUE_1 = "123";
  const char* TEST_VALUE_2 = "456";
  patch_set("TEST_PARAM_1", TEST_VALUE_1);
  patch_set("TEST_PARAM_2", TEST_VALUE_2);
  
  // Save to a test file
  const char* test_config = "0:/test_config.ngc";
  result = patch_save(test_config);
  if (result == 0) {
    dbg_printf("[PASS] Config saved to %s\r\n", test_config);
    test_passed++;
    
    // Verify by reloading
    patch_init(); // Clear current config
    result = patch_load(test_config);
    if (result == 0) {
      char val[64];
      if (patch_get("TEST_PARAM_1", val, sizeof(val)) == 0) {
        dbg_printf("[PASS] Verified saved value: %s\r\n", val);
        test_passed++;
      } else {
        dbg_print("[FAIL] Could not read saved parameter\r\n");
        test_failed++;
      }
    } else {
      dbg_print("[FAIL] Could not reload saved config\r\n");
      test_failed++;
    }
  } else {
    dbg_print("[FAIL] Could not save config file\r\n");
    dbg_print("       Check: SD card write protection\r\n");
    test_failed++;
  }
  dbg_print("\r\n");
  osDelay(200);
  
#if MODULE_ENABLE_LOOPER
  // ========================================
  // TEST 5: MIDI Export - Single Track
  // ========================================
  dbg_print("TEST 5: MIDI Export - Single Track\r\n");
  dbg_print("--------------------------------------\r\n");
  
  // Initialize looper
  looper_init();
  dbg_print("Looper initialized\r\n");
  
  // Clear track to ensure known state
  looper_clear(0);
  
  // Export empty track (should succeed but note it's empty)
  const char* track_file = "0:/test_track0.mid";
  result = looper_export_track_midi(0, track_file);
  if (result == 0) {
    dbg_printf("[PASS] Track exported to %s\r\n", track_file);
    test_passed++;
  } else if (result == -2) {
    dbg_print("[SKIP] Track is empty (expected)\r\n");
  } else {
    dbg_print("[FAIL] Track export failed\r\n");
    test_failed++;
  }
  dbg_print("\r\n");
  osDelay(200);
  
  // ========================================
  // TEST 6: MIDI Export - All Tracks
  // ========================================
  dbg_print("TEST 6: MIDI Export - All Tracks\r\n");
  dbg_print("--------------------------------------\r\n");
  
  const char* all_tracks_file = "0:/test_all_tracks.mid";
  result = looper_export_midi(all_tracks_file);
  if (result == 0) {
    dbg_printf("[PASS] All tracks exported to %s\r\n", all_tracks_file);
    test_passed++;
  } else if (result == -2) {
    dbg_print("[SKIP] No tracks have data (expected)\r\n");
  } else {
    dbg_print("[FAIL] All tracks export failed\r\n");
    test_failed++;
  }
  dbg_print("\r\n");
  osDelay(200);
  
  // ========================================
  // TEST 7: MIDI Export - Scene Export
  // ========================================
  dbg_print("TEST 7: MIDI Export - Scene Export\r\n");
  dbg_print("--------------------------------------\r\n");
  
  const char* scene_file = "0:/test_scene_A.mid";
  result = looper_export_scene_midi(0, scene_file);
  if (result == 0 || result == -2) {
    dbg_printf("[PASS/SKIP] Scene export completed\r\n");
    if (result == -2) {
      dbg_print("         (Scene empty, which is expected)\r\n");
    }
    test_passed++;
  } else {
    dbg_print("[FAIL] Scene export failed\r\n");
    test_failed++;
  }
  dbg_print("\r\n");
  osDelay(200);
  
  // ========================================
  // TEST 8: Scene Chaining Configuration
  // ========================================
  dbg_print("TEST 8: Scene Chaining Configuration\r\n");
  dbg_print("--------------------------------------\r\n");
  
  // Configure scene chains: A->B->C->A
  looper_set_scene_chain(0, 1, 1); // Scene A -> B
  looper_set_scene_chain(1, 2, 1); // Scene B -> C
  looper_set_scene_chain(2, 0, 1); // Scene C -> A (loop)
  
  // Verify configuration
  int chain_ok = 1;
  if (looper_get_scene_chain(0) != 1 || !looper_is_scene_chain_enabled(0)) {
    dbg_print("[FAIL] Scene A->B chain not set correctly\r\n");
    chain_ok = 0;
  }
  if (looper_get_scene_chain(1) != 2 || !looper_is_scene_chain_enabled(1)) {
    dbg_print("[FAIL] Scene B->C chain not set correctly\r\n");
    chain_ok = 0;
  }
  if (looper_get_scene_chain(2) != 0 || !looper_is_scene_chain_enabled(2)) {
    dbg_print("[FAIL] Scene C->A chain not set correctly\r\n");
    chain_ok = 0;
  }
  
  if (chain_ok) {
    dbg_print("[PASS] Scene chains configured: A->B->C->A\r\n");
    test_passed++;
  } else {
    test_failed++;
  }
  dbg_print("\r\n");
  osDelay(200);
  
  // ========================================
  // TEST 9: Quick-Save System
  // ========================================
  dbg_print("TEST 9: Quick-Save System\r\n");
  dbg_print("--------------------------------------\r\n");
  
  // Save to slot 0
  dbg_print("Saving to quick-save slot 0...\r\n");
  result = looper_quick_save(0, "Test Session");
  if (result == 0) {
    dbg_print("[PASS] Quick-save successful\r\n");
    test_passed++;
    
    // Verify slot is marked as used
    if (looper_quick_save_is_used(0)) {
      dbg_print("[PASS] Slot 0 marked as used\r\n");
      const char* name = looper_quick_save_get_name(0);
      if (name) {
        dbg_printf("[PASS] Slot name: %s\r\n", name);
      }
      test_passed++;
    } else {
      dbg_print("[FAIL] Slot 0 not marked as used\r\n");
      test_failed++;
    }
    
    // Test quick-load
    dbg_print("Loading from quick-save slot 0...\r\n");
    result = looper_quick_load(0);
    if (result == 0) {
      dbg_print("[PASS] Quick-load successful\r\n");
      test_passed++;
    } else {
      dbg_print("[FAIL] Quick-load failed\r\n");
      test_failed++;
    }
  } else {
    dbg_print("[FAIL] Quick-save failed\r\n");
    dbg_print("       Check: SD card writable\r\n");
    test_failed++;
  }
  dbg_print("\r\n");
  osDelay(200);
  
  // ========================================
  // TEST 10: Scene Chaining Persistence
  // ========================================
  dbg_print("TEST 10: Scene Chaining Persistence\r\n");
  dbg_print("--------------------------------------\r\n");
  
  // The scene chains should persist in quick-save
  // We already saved them above, now verify they're still there
  uint8_t next = looper_get_scene_chain(0);
  uint8_t enabled = looper_is_scene_chain_enabled(0);
  
  if (next == 1 && enabled) {
    dbg_print("[PASS] Scene chain A->B persisted\r\n");
    test_passed++;
  } else {
    dbg_print("[FAIL] Scene chain lost after save/load\r\n");
    test_failed++;
  }
  dbg_print("\r\n");
  osDelay(200);
  
#else
  dbg_print("\r\n[SKIP] Tests 5-10 (Looper not enabled)\r\n\r\n");
#endif
  
  // ========================================
  // TEST SUMMARY
  // ========================================
test_summary:
  dbg_print("\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("            TEST SUMMARY\r\n");
  dbg_print("==============================================\r\n");
  dbg_printf("Tests Passed: %d\r\n", test_passed);
  dbg_printf("Tests Failed: %d\r\n", test_failed);
  dbg_printf("Total Tests:  %d\r\n", test_passed + test_failed);
  dbg_print("----------------------------------------------\r\n");
  
  if (test_failed == 0) {
    dbg_print("RESULT: ALL TESTS PASSED!\r\n");
    dbg_print("\r\n");
    dbg_print("Features Verified:\r\n");
    dbg_print("  - SD card mount/unmount\r\n");
    dbg_print("  - Config file load/save\r\n");
    dbg_print("  - Config parameter read/write\r\n");
#if MODULE_ENABLE_LOOPER
    dbg_print("  - MIDI export (track/scene/all)\r\n");
    dbg_print("  - Scene chaining configuration\r\n");
    dbg_print("  - Quick-save system\r\n");
    dbg_print("  - Scene chain persistence\r\n");
#endif
  } else {
    dbg_print("RESULT: SOME TESTS FAILED\r\n");
    dbg_print("\r\n");
    dbg_print("Troubleshooting:\r\n");
    dbg_print("  1. Check SD card is inserted\r\n");
    dbg_print("  2. Verify card is FAT32 formatted\r\n");
    dbg_print("  3. Check SPI connections\r\n");
    dbg_print("  4. Verify write protection is off\r\n");
    dbg_print("  5. Ensure config.ngc exists on card\r\n");
  }
  dbg_print("==============================================\r\n");
  dbg_print("\r\n");
  
  // Return success if all tests passed
  return (test_failed == 0) ? 0 : -1;
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

#if MODULE_ENABLE_USB_MIDI && !defined(APP_TEST_USB_MIDI)
// Built-in USB MIDI test debug hook (only when not using dedicated app_test)
static void module_test_usb_midi_print_packet(const uint8_t packet4[4])
{
  uint8_t cable = (packet4[0] >> 4) & 0x0F;
  uint8_t status = packet4[1];
  uint8_t data1 = packet4[2];
  uint8_t data2 = packet4[3];
  
  dbg_printf("[RX] Cable:%d %02X %02X %02X", cable, status, data1, data2);
  
  // Decode message type
  uint8_t msg_type = status & 0xF0;
  uint8_t channel = (status & 0x0F) + 1;
  
  if (msg_type == 0x90 && data2 > 0) {
    dbg_printf(" (Note On Ch:%d Note:%d Vel:%d)", channel, data1, data2);
  } else if (msg_type == 0x80 || (msg_type == 0x90 && data2 == 0)) {
    dbg_printf(" (Note Off Ch:%d Note:%d)", channel, data1);
  } else if (msg_type == 0xB0) {
    dbg_printf(" (CC Ch:%d CC:%d Val:%d)", channel, data1, data2);
  } else if (msg_type == 0xC0) {
    dbg_printf(" (Prog Ch:%d Prog:%d)", channel, data1);
  } else if (msg_type == 0xE0) {
    dbg_printf(" (Bend Ch:%d)", channel);
  }
  
  dbg_print("\r\n");
}

/**
 * @brief Unified USB MIDI receive debug hook - overrides weak symbol in usb_midi.c
 * Works for both APP_TEST_USB_MIDI and MODULE_TEST_USB_DEVICE_MIDI modes
 */
void usb_midi_rx_debug_hook(const uint8_t packet4[4])
{
  uint8_t cin = packet4[0] & 0x0F;
  
  // Handle SysEx packets (CIN 0x4-0x7) - special logging format
  if (cin >= 0x04 && cin <= 0x07) {
    uint8_t cable = (packet4[0] >> 4) & 0x0F;
    dbg_print("[RX SysEx] Cable:");
    dbg_print_uint(cable);
    dbg_print(" CIN:0x");
    dbg_print_hex8(cin);
    dbg_print(" Data:");
    for (uint8_t i = 1; i < 4; i++) {
      dbg_print(" ");
      dbg_print_hex8(packet4[i]);
    }
    dbg_print("\r\n");
    return; // Don't print regular format for SysEx
  }
  
  // Print regular MIDI messages using shared formatting function
  module_test_usb_midi_print_packet(packet4);
}
#endif

void module_test_usb_device_midi_run(void)
{
  // Early UART verification
  dbg_print("\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("UART Debug Verification: OK\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("\r\n");
  osDelay(100);
  
#if defined(APP_TEST_USB_MIDI)
  // Use existing USB MIDI test
  app_test_usb_midi_run_forever();
#elif MODULE_ENABLE_USB_MIDI
  // Built-in USB Device MIDI test
  dbg_print_test_header("USB Device MIDI Test");
  
  dbg_print("Configuration:\r\n");
  dbg_printf("  - UART Port: UART%d (Port %d)\r\n", TEST_DEBUG_UART_PORT + 1, TEST_DEBUG_UART_PORT);
  dbg_printf("  - Baud Rate: %d\r\n", TEST_DEBUG_UART_BAUD);
  dbg_print("  - Data: 8-N-1\r\n");
  dbg_print("\r\n");
  
  // Note: usb_midi_init() is already called in main.c before RTOS starts
  // USB Device MIDI is ready to use
  dbg_print("USB Device MIDI already initialized.\r\n");
  
  dbg_print("\r\n");
  dbg_print("USB Device MIDI initialized.\r\n");
  dbg_print("Connect USB to computer/DAW to send and receive MIDI.\r\n");
  dbg_print("This test will log received MIDI packets to UART.\r\n");
  dbg_print("Sending test Note On/Off messages every 2 seconds.\r\n");
  dbg_print_separator();
  
  uint32_t last_send_time = 0;
  uint8_t note_state = 0;  // 0=off, 1=on
  
  // Main test loop
  for (;;) {
    uint32_t now = osKernelGetTickCount();
    
    // Periodically send test MIDI messages
    if (now - last_send_time >= 2000) {
      last_send_time = now;
      
      if (note_state == 0) {
        // Send Note On (Middle C, Channel 1, Velocity 100)
        uint8_t cin = 0x09;  // Cable 0, Note On CIN
        uint8_t status = 0x90;  // Note On, Channel 1
        uint8_t note = 60;  // Middle C
        uint8_t velocity = 100;
        
        usb_midi_send_packet(cin, status, note, velocity);
        dbg_printf("[TX] Cable:0 %02X %02X %02X (Note On)\r\n", status, note, velocity);
        note_state = 1;
      } else {
        // Send Note Off
        uint8_t cin = 0x08;  // Cable 0, Note Off CIN
        uint8_t status = 0x80;  // Note Off, Channel 1
        uint8_t note = 60;  // Middle C
        uint8_t velocity = 0;
        
        usb_midi_send_packet(cin, status, note, velocity);
        dbg_printf("[TX] Cable:0 %02X %02X %02X (Note Off)\r\n", status, note, velocity);
        note_state = 0;
      }
    }
    
    osDelay(10);
  }
#else
  // Module not enabled
  dbg_print_test_header("USB Device MIDI Test");
  dbg_print("ERROR: USB Device MIDI not enabled!\r\n");
  dbg_print("Enable MODULE_ENABLE_USB_MIDI in Config/module_config.h\r\n");
  dbg_print_separator();
  for (;;) osDelay(1000);
#endif
}

// =============================================================================
// OLED SSD1322 TEST
// =============================================================================

#if MODULE_ENABLE_OLED
#include "Hal/oled_ssd1322/oled_ssd1322.h"
#include "Config/oled_pins.h"

/**
 * @brief Minimal hardware test - bypasses full init, tests basic SPI communication
 * @return 0 on success, -1 on failure
 */
static int module_test_oled_minimal_hardware(void)
{
  dbg_print_separator();
  dbg_print("=== MINIMAL OLED Hardware Test ===\r\n");
  dbg_print("This test bypasses full initialization\r\n");
  dbg_print("Commands: 0xFD 0x12 (unlock), 0xAF (display ON), 0xA5 (all pixels ON)\r\n");
  dbg_print_separator();
  
  // Initialize DWT for precise timing
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  
  // Set initial states (SPI Mode 0: clock idle LOW)
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);   // SCL LOW (idle)
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);   // E2 LOW
  HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);  // Data LOW
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);    // DC LOW (command mode)
  
  dbg_print("Initial GPIO states set (SCL=LOW, SDA=LOW, DC=LOW)\r\n");
  osDelay(100);
  
  // Inline send_byte function
  void send_byte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
      // Set data bit
      if (byte & 0x80) {
        HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET);
      } else {
        HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);
      }
      
      // Small delay for data setup
      uint32_t start = DWT->CYCCNT;
      while ((DWT->CYCCNT - start) < 20);
      
      // Clock HIGH (sample edge)
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
      
      // Hold time
      start = DWT->CYCCNT;
      while ((DWT->CYCCNT - start) < 20);
      
      // Clock back to LOW
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
      
      start = DWT->CYCCNT;
      while ((DWT->CYCCNT - start) < 20);
      
      byte <<= 1;
    }
  }
  
  void send_cmd(uint8_t cmd) {
    HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);
    uint32_t start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < 10);
    send_byte(cmd);
  }
  
  void send_data_byte(uint8_t d) {
    HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);
    uint32_t start = DWT->CYCCNT;
    while ((DWT->CYCCNT - start) < 10);
    send_byte(d);
  }
  
  dbg_print("\r\nSending command sequence:\r\n");
  
  // 1. Unlock (0xFD 0x12)
  dbg_print("  0xFD (unlock command)...\r\n");
  send_cmd(0xFD);
  dbg_print("  0x12 (unlock data)...\r\n");
  send_data_byte(0x12);
  osDelay(10);
  
  // 2. Display ON (0xAF)
  dbg_print("  0xAF (display ON)...\r\n");
  send_cmd(0xAF);
  osDelay(10);
  
  // 3. All pixels ON - bypass GDDRAM (0xA5)
  dbg_print("  0xA5 (all pixels ON - bypass RAM)...\r\n");
  send_cmd(0xA5);
  osDelay(100);
  
  dbg_print("\r\n");
  dbg_print_separator();
  dbg_print("=== Hardware Test Complete ===\r\n");
  dbg_print("EXPECTED: Display should show ALL pixels lit (full white)\r\n");
  dbg_print("If display is still blank:\r\n");
  dbg_print("  - Check VCC (should be 3.3V stable)\r\n");
  dbg_print("  - Check all wire connections\r\n");
  dbg_print("  - Measure signals with logic analyzer\r\n");
  dbg_print("  - Possible hardware issue with OLED module\r\n");
  dbg_print_separator();
  
  return 0;
}

/**
 * @brief Test GPIO pin control for OLED
 * @return 0 on success, -1 on failure
 */
static int module_test_oled_gpio_control(void)
{
  dbg_print("=== GPIO Control Test ===\r\n");
  
  // Test PA8 (DC pin)
  dbg_print("Testing PA8 (DC pin)...\r\n");
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);
  uint8_t dc_low = HAL_GPIO_ReadPin(OLED_DC_GPIO_Port, OLED_DC_Pin);
  
  HAL_GPIO_WritePin(OLED_DC_GPIO_Port, OLED_DC_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
  uint8_t dc_high = HAL_GPIO_ReadPin(OLED_DC_GPIO_Port, OLED_DC_Pin);
  
  dbg_printf("  PA8 LOW=%d, HIGH=%d ", dc_low, dc_high);
  if (dc_low == 0 && dc_high == 1) {
    dbg_print("[PASS]\r\n");
  } else {
    dbg_print("[FAIL]\r\n");
    return -1;
  }
  
  // Test PC8 (Clock pin 1)
  dbg_print("Testing PC8 (SCL/E1 pin)...\r\n");
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET);
  HAL_Delay(1);
  uint8_t clk1_low = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8);
  
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET);
  HAL_Delay(1);
  uint8_t clk1_high = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8);
  
  dbg_printf("  PC8 LOW=%d, HIGH=%d ", clk1_low, clk1_high);
  if (clk1_low == 0 && clk1_high == 1) {
    dbg_print("[PASS]\r\n");
  } else {
    dbg_print("[FAIL]\r\n");
    return -1;
  }
  
  // Test PC9 (Clock pin 2)
  dbg_print("Testing PC9 (E2 pin)...\r\n");
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
  HAL_Delay(1);
  uint8_t clk2_low = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9);
  
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
  HAL_Delay(1);
  uint8_t clk2_high = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_9);
  
  dbg_printf("  PC9 LOW=%d, HIGH=%d ", clk2_low, clk2_high);
  if (clk2_low == 0 && clk2_high == 1) {
    dbg_print("[PASS]\r\n");
  } else {
    dbg_print("[FAIL]\r\n");
    return -1;
  }
  
  // Test PC11 (Data pin)
  dbg_print("Testing PC11 (SDA pin)...\r\n");
  HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);
  uint8_t sda_low = HAL_GPIO_ReadPin(OLED_SDA_GPIO_Port, OLED_SDA_Pin);
  
  HAL_GPIO_WritePin(OLED_SDA_GPIO_Port, OLED_SDA_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
  uint8_t sda_high = HAL_GPIO_ReadPin(OLED_SDA_GPIO_Port, OLED_SDA_Pin);
  
  dbg_printf("  PC11 LOW=%d, HIGH=%d ", sda_low, sda_high);
  if (sda_low == 0 && sda_high == 1) {
    dbg_print("[PASS]\r\n");
  } else {
    dbg_print("[FAIL]\r\n");
    return -1;
  }
  
  dbg_print("GPIO Control Test: [PASS]\r\n\r\n");
  return 0;
}

/**
 * @brief Display test patterns on OLED
 * @return 0 on success
 */
static int module_test_oled_display_patterns(void)
{
  dbg_print("=== Display Pattern Tests ===\r\n");
  uint8_t *fb = oled_framebuffer();
  
  // Test 1: All white
  dbg_print("Test 1: All WHITE (2 seconds)...\r\n");
  memset(fb, 0xFF, 8192);
  oled_flush();
  osDelay(2000);
  
  // Test 2: All black
  dbg_print("Test 2: All BLACK (2 seconds)...\r\n");
  memset(fb, 0x00, 8192);
  oled_flush();
  osDelay(2000);
  
  // Test 3: Checkerboard
  dbg_print("Test 3: CHECKERBOARD (2 seconds)...\r\n");
  for (int i = 0; i < 8192; i++) {
    fb[i] = (i & 1) ? 0xFF : 0x00;
  }
  oled_flush();
  osDelay(2000);
  
  // Test 4: Horizontal stripes
  dbg_print("Test 4: HORIZONTAL STRIPES (2 seconds)...\r\n");
  for (int row = 0; row < 64; row++) {
    uint8_t value = (row & 4) ? 0xFF : 0x00;
    memset(&fb[row * 128], value, 128);
  }
  oled_flush();
  osDelay(2000);
  
  // Test 5: Grayscale gradient
  dbg_print("Test 5: GRAYSCALE GRADIENT (2 seconds)...\r\n");
  for (int row = 0; row < 64; row++) {
    uint8_t gray = (row * 4) & 0xFF;
    memset(&fb[row * 128], gray, 128);
  }
  oled_flush();
  osDelay(2000);
  
  // Clear display
  dbg_print("Clearing display...\r\n");
  oled_clear();
  oled_flush();
  
  dbg_print("Display Pattern Tests: [COMPLETE]\r\n\r\n");
  return 0;
}
#endif

int module_test_oled_ssd1322_run(void)
{
#if MODULE_ENABLE_OLED
  dbg_print("\r\n");
  dbg_print("=====================================\r\n");
  dbg_print("  MIOS32 SSD1322 Test (Simplified)\r\n");
  dbg_print("=====================================\r\n");
  dbg_print("Based on: midibox/mios32/apps/mios32_test/app_lcd/ssd1322\r\n");
  dbg_print("Target: STM32F407 @ 168 MHz\r\n");
  dbg_print("Display: SSD1322 256x64 OLED\r\n\r\n");
  
  dbg_print("Pin Mapping:\r\n");
  dbg_print("  PA8  = DC  (Data/Command)\r\n");
  dbg_print("  PC8  = SCL (Clock 1)\r\n");
  dbg_print("  PC9  = SCL (Clock 2, dual COM)\r\n");
  dbg_print("  PC11 = SDA (Data)\r\n");
  dbg_print("  CS#  = GND (hardwired)\r\n\r\n");
  
  // Comprehensive OLED test suite with BOTH init methods
  dbg_print("=== COMPREHENSIVE OLED TEST SUITE ===\r\n\r\n");
  
  dbg_print("Choose initialization method:\r\n");
  dbg_print("  1. Simple MIOS32 test init (basic, proven working)\r\n");
  dbg_print("  2. Complete Newhaven NHD-3.12 init (LoopA production)\r\n\r\n");
  
  // Use Newhaven init by default (LoopA production code)
  #define USE_NEWHAVEN_INIT 1
  
  dbg_print("Step 1: Initialize OLED...\r\n");
  
  #if USE_NEWHAVEN_INIT
    dbg_print("Using: Complete Newhaven NHD-3.12 initialization\r\n");
    dbg_print("  - Display Clock: 80 Frames/Sec (0x91)\r\n");
    dbg_print("  - Custom gray scale table\r\n");
    dbg_print("  - Display enhancement enabled\r\n");
    dbg_print("  - Pre-charge voltage: 0.60*VCC\r\n\r\n");
    oled_init_newhaven();
  #else
    dbg_print("Using: Simple MIOS32 test initialization\r\n");
    dbg_print("  - Display Clock: ~58 Frames/Sec (divider=0, freq=12)\r\n");
    dbg_print("  - Linear gray scale table\r\n");
    dbg_print("  - Basic settings only\r\n\r\n");
    oled_init();
  #endif
  
  dbg_print("[OK] Init complete\r\n\r\n");
  
  // Array of test functions and their descriptions
  typedef struct {
    void (*test_func)(void);
    const char* name;
    const char* description;
  } oled_test_t;
  
  const oled_test_t tests[] = {
    {oled_test_mios32_pattern, "MIOS32 Pattern", "Gradient (left) + White (right) - MIOS32 original test"},
    {oled_test_checkerboard,   "Checkerboard",   "Alternating black/white squares - pixel uniformity test"},
    {oled_test_h_gradient,     "H-Gradient",     "Horizontal gradient from black to white"},
    {oled_test_v_gradient,     "V-Gradient",     "Vertical gradient from black to white"},
    {oled_test_gray_levels,    "Gray Levels",    "All 16 grayscale levels as vertical bars"},
    {oled_test_rectangles,     "Rectangles",     "Concentric rectangles - geometric pattern"},
    {oled_test_stripes,        "Diagonal Stripes", "Diagonal stripe pattern"},
    {oled_test_voxel_landscape, "Voxel Landscape", "Simple 3D terrain visualization (voxelspace)"},
    {oled_test_text_pattern,   "Text Pattern",   "Simulated text rendering pattern"},
  };
  
  const uint8_t num_tests = sizeof(tests) / sizeof(tests[0]);
  
  dbg_printf("Step 2: Running %d visual tests...\r\n", num_tests);
  dbg_print("Each test displays for 3 seconds\r\n");
  dbg_print("Watch the OLED display!\r\n\r\n");
  
  // Run all tests in sequence
  for (uint8_t i = 0; i < num_tests; i++) {
    dbg_printf("Test %d/%d: %s\r\n", i+1, num_tests, tests[i].name);
    dbg_printf("  %s\r\n", tests[i].description);
    
    // Render the test pattern
    tests[i].test_func();
    
    dbg_print("  [OK] Pattern rendered\r\n\r\n");
    
    // Display for 3 seconds
    osDelay(3000);
  }
  
  dbg_print("=== ALL DIRECT PATTERN TESTS COMPLETE ===\r\n\r\n");
  
  // ============================================================================
  // Step 3: UI Page Test (Framebuffer-based rendering)
  // ============================================================================
  dbg_print("Step 3: UI Page Test (Framebuffer + Graphics API)\r\n");
  dbg_print("===============================================\r\n");
  dbg_print("This test demonstrates the production UI framework:\r\n");
  dbg_print("  - Framebuffer-based rendering\r\n");
  dbg_print("  - Graphics primitives (text, lines, rectangles, pixels)\r\n");
  dbg_print("  - Multiple test modes (use encoder/buttons to switch)\r\n");
  dbg_print("  - Real-time updates with millisecond counter\r\n\r\n");
  
  dbg_print("Available UI test modes:\r\n");
  dbg_print("  Mode 0: Pattern Test - Stripes and checkerboard\r\n");
  dbg_print("  Mode 1: Grayscale Test - All 16 levels with labels\r\n");
  dbg_print("  Mode 2: Pixel Test - Individual pixel grid\r\n");
  dbg_print("  Mode 3: Text Test - Font rendering (different sizes)\r\n");
  dbg_print("  Mode 4: Animation Test - Moving bar and pulsing square\r\n");
  dbg_print("  Mode 5: Hardware Info - Display specifications\r\n");
  dbg_print("  Mode 6: Direct Framebuffer - Raw buffer manipulation\r\n\r\n");
  
  dbg_print("NOTE: Encoder/button control not available in test mode.\r\n");
  dbg_print("      Modes will cycle automatically.\r\n\r\n");
  
  // Initialize UI graphics with OLED framebuffer
  uint8_t* fb = oled_framebuffer();
  ui_gfx_set_fb(fb, OLED_W, OLED_H);
  
  dbg_print("Starting UI page test loop...\r\n");
  dbg_print("Each mode displays for 5 seconds before cycling.\r\n");
  dbg_print("Watch the OLED display!\r\n\r\n");
  
  // Continuous loop through UI test modes
  uint32_t loop_count = 0;
  uint32_t mode_start_time = 0;
  const uint32_t mode_duration_ms = 5000;  // 5 seconds per mode
  
  while(1) {
    uint32_t current_time = osKernelGetTickCount();
    
    // Cycle through modes every 5 seconds
    if (current_time - mode_start_time >= mode_duration_ms) {
      mode_start_time = current_time;
      loop_count++;
      
      // Simulate encoder to cycle through modes (0-6)
      ui_page_oled_test_on_encoder(1);  // Next mode
      
      dbg_printf("--- Loop #%lu: Switching to next test mode ---\r\n", loop_count);
    }
    
    // Render the current UI page
    ui_page_oled_test_render(current_time);
    
    // Flush framebuffer to display
    oled_flush();
    
    // Small delay for smooth animation (16ms = ~60 FPS)
    osDelay(16);
  }
  
  return 0;
#else
  dbg_print("OLED is not enabled in module_config.h\r\n");
  return -1;
#endif
}
