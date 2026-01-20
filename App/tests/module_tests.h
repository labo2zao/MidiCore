/**
 * @file module_tests.h
 * @brief Unified module testing framework for MidiCore
 * 
 * This file provides a centralized testing infrastructure for all MidiCore modules.
 * Tests can be selected at compile time or runtime to isolate and validate individual modules.
 */

#ifndef MODULE_TESTS_H
#define MODULE_TESTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// =============================================================================
// TEST SELECTION
// =============================================================================

/**
 * @brief Available module tests
 * These can be enabled via compiler defines or runtime selection
 * 
 * Note: Enum values use _ID suffix to avoid conflicts with preprocessor defines
 * (e.g., MODULE_TEST_AINSER64_ID vs MODULE_TEST_AINSER64 define)
 */
typedef enum {
  MODULE_TEST_NONE_ID = 0,
  MODULE_TEST_GDB_DEBUG_ID,     // Test GDB debug / UART verification
  MODULE_TEST_AINSER64_ID,      // Test AINSER64 analog inputs
  MODULE_TEST_SRIO_ID,          // Test SRIO DIN (Digital Inputs)
  MODULE_TEST_SRIO_DOUT_ID,     // Test SRIO DOUT (Digital Outputs - LEDs)
  MODULE_TEST_MIDI_DIN_ID,      // Test MIDI DIN I/O
  MODULE_TEST_ROUTER_ID,        // Test MIDI router
  MODULE_TEST_LOOPER_ID,        // Test looper recording/playback
  MODULE_TEST_UI_ID,            // Test UI/OLED
  MODULE_TEST_PATCH_SD_ID,      // Test patch loading from SD
  MODULE_TEST_PRESSURE_ID,      // Test pressure sensor I2C
  MODULE_TEST_USB_HOST_MIDI_ID, // Test USB Host MIDI
  MODULE_TEST_ALL_ID,           // Run all tests sequentially
} module_test_t;

// =============================================================================
// TEST FUNCTIONS
// =============================================================================

/**
 * @brief Initialize the test framework
 * Call this before running any tests
 */
void module_tests_init(void);

/**
 * @brief Run a specific module test
 * @param test Test to run (see module_test_t)
 * @return 0 on success, negative on error
 * @note Most tests run in infinite loops and do not return
 */
int module_tests_run(module_test_t test);

/**
 * @brief Get the name of a test
 * @param test Test identifier
 * @return String name of the test
 */
const char* module_tests_get_name(module_test_t test);

// =============================================================================
// INDIVIDUAL MODULE TEST FUNCTIONS
// =============================================================================

/**
 * @brief Test GDB debug and UART verification
 * Simple test that prints to UART to verify debug connection
 * Ideal for verifying UART is working and baud rate is correct
 * @note This function runs forever
 */
void module_test_gdb_debug_run(void);

/**
 * @brief Test AINSER64 module (analog inputs)
 * Reads all 64 channels and outputs values via UART or OLED
 * @note This function runs forever
 */
void module_test_ainser64_run(void);

/**
 * @brief Test SRIO DIN module with MIDI output (Digital Inputs → MIDI)
 * 
 * Complete end-to-end test of the button input signal chain:
 * - Reads button inputs from 74HC165 shift registers (SRIO DIN)
 * - Generates MIDI Note On/Off messages when buttons are pressed/released
 * - Routes MIDI to USB MIDI OUT and DIN MIDI OUT1 (if router enabled)
 * 
 * Button to MIDI mapping:
 * - Button 0-63 → MIDI Notes 36-99 (C2 to D#7)
 * - Note On velocity: 100
 * - Note Off velocity: 0
 * - Channel: 1
 * 
 * Hardware tested:
 * - 74HC165 shift register chain (DIN inputs)
 * - SPI communication (MISO, SCK, /PL pins)
 * - MIDI Router (if enabled)
 * - USB MIDI output (if router enabled)
 * - DIN MIDI OUT1 (if router enabled)
 * 
 * Connect:
 * - Buttons to 74HC165 inputs (active low with pull-ups)
 * - USB cable to computer to receive MIDI notes
 * - Or DIN MIDI OUT1 to external synth/device
 * 
 * @note This function runs forever
 * @note Enable MODULE_ENABLE_ROUTER for MIDI output, otherwise only button detection
 */
void module_test_srio_run(void);

/**
 * @brief Test SRIO DOUT module (Digital Outputs - LEDs)
 * 
 * Comprehensive test of the LED output signal chain:
 * - Writes patterns to 74HC595 shift registers (SRIO DOUT)
 * - Cycles through 7 different visual patterns
 * - Tests all DOUT bytes and individual bits
 * - Verifies SPI MOSI, SCK, and RCLK signals
 * 
 * Test Patterns (2 seconds each):
 * 1. All LEDs ON (0x00) - Tests power and common connections
 * 2. All LEDs OFF (0xFF) - Tests LED disable
 * 3. Alternating (0xAA/0x55) - Tests adjacent bits independently
 * 4. Running light - Tests each individual LED sequentially
 * 5. Binary counter - Tests byte-level control and timing
 * 6. Wave pattern - Tests multi-chip synchronization
 * 7. Checkerboard (0x55) - Tests precise bit pattern accuracy
 * 
 * Hardware tested:
 * - 74HC595 shift register chain (DOUT outputs)
 * - SPI communication (MOSI, SCK pins)
 * - RCLK latch control (RC1 pin)
 * - All 8 DOUT bytes (64 LEDs)
 * - Daisy-chained register operation
 * 
 * Hardware connections (MIOS32 mbhp_doutx4):
 * - STM32 PB15 (SPI2 MOSI) → 74HC595 Pin 14 (SER - serial data)
 * - STM32 PB13 (SPI2 SCK)  → 74HC595 Pin 11 (SRCLK - shift clock)
 * - STM32 PB12 (RC1)       → 74HC595 Pin 12 (RCLK - register clock/latch)
 * - 74HC595 Pin 9 (QH')    → Next 74HC595 Pin 14 (daisy chain)
 * 
 * Visual verification:
 * - LEDs connected to 74HC595 outputs (active low typical)
 * - Watch patterns cycle to verify all outputs work
 * - Each pattern should be clearly visible for 2 seconds
 * 
 * Common issues diagnosed:
 * - No LEDs light: Check power, MOSI connection, RCLK pulse
 * - Random pattern: Check SPI clock, data line integrity
 * - Only some LEDs work: Check daisy chain connections (QH')
 * - Pattern frozen: Check RCLK (latch) signal
 * 
 * @note This function runs forever (continuous pattern cycling)
 * @note LEDs are typically active low (0 = ON, 1 = OFF)
 */
void module_test_srio_dout_run(void);

/**
 * @brief Test MIDI DIN module
 * Echoes MIDI input to output, shows status
 * @note This function runs forever
 */
void module_test_midi_din_run(void);

/**
 * @brief Test MIDI Router module
 * 
 * Comprehensive test of the MIDI routing matrix (16x16 nodes).
 * 
 * Tests:
 * - Route configuration (enable/disable)
 * - Channel filtering with chanmask
 * - Message routing between nodes (DIN, USB, Looper, etc.)
 * - Label assignment
 * - Multiple simultaneous routes
 * - Different MIDI message types (Note, CC, Sysex)
 * 
 * Hardware tested:
 * - DIN IN1-4 → DIN OUT1-4 routing
 * - USB Device IN/OUT → DIN routing
 * - Looper → Output routing with channel filtering
 * 
 * The test configures example routes and sends test messages to verify
 * the routing matrix operates correctly. Monitor UART output to see
 * routing table and test message flow.
 * 
 * @note This function runs forever
 */
void module_test_router_run(void);

/**
 * @brief Test Looper module
 * Tests recording, playback, overdub, quantization
 * @note This function runs forever
 */
void module_test_looper_run(void);

/**
 * @brief Test UI module (OLED display)
 * Tests graphics primitives, pages, encoders
 * @note This function runs forever
 */
/**
 * @brief Test UI/OLED module
 * 
 * Tests the complete UI system including OLED display, page navigation,
 * button/encoder input handling, and status line updates.
 * 
 * Features tested:
 * - OLED SSD1322 display initialization and rendering
 * - UI page cycling (Looper, Timeline, Pianoroll, Router, Patch)
 * - Button input simulation and handling
 * - Rotary encoder input simulation
 * - Status line updates
 * - Graphics rendering
 * 
 * Hardware requirements:
 * - OLED Display: SSD1322 256x64 (grayscale)
 * - Control Input: Buttons + rotary encoder (via SRIO or GPIO)
 * 
 * Test sequence:
 * 1. Initialize OLED and UI subsystem
 * 2. Cycle through all available UI pages (auto-demo)
 * 3. Simulate button presses
 * 4. Simulate encoder rotation
 * 5. Update status messages
 * 6. Enter manual testing mode for visual verification
 * 
 * Usage: Enable MODULE_TEST_UI=1 in test configuration
 * Connect OLED display and observe automatic page cycling,
 * then test with actual buttons/encoders.
 */
void module_test_ui_run(void);

/**
 * @brief Test Patch/SD module
 * Tests SD card mounting, file I/O, patch loading
 * @return 0 on success, negative on error
 */
int module_test_patch_sd_run(void);

/**
 * @brief Test Pressure sensor module
 * Tests I2C communication and value mapping
 * @note This function runs forever
 */
void module_test_pressure_run(void);

/**
 * @brief Test USB Host MIDI module
 * Tests USB device detection and MIDI communication
 * @note This function runs forever
 */
void module_test_usb_host_midi_run(void);

// =============================================================================
// COMPILE-TIME TEST SELECTION HELPERS
// =============================================================================

/**
 * @brief Get the test selected at compile time
 * @return Selected test or MODULE_TEST_NONE if no compile-time selection
 */
module_test_t module_tests_get_compile_time_selection(void);

#ifdef __cplusplus
}
#endif

#endif // MODULE_TESTS_H
