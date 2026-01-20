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
 * @brief Test SRIO DIN module (Digital Inputs - buttons)
 * Reads button inputs from 74HC165 shift registers
 * @note This function runs forever
 */
void module_test_srio_run(void);

/**
 * @brief Test SRIO DOUT module (Digital Outputs - LEDs)
 * Cycles through LED patterns on 74HC595 shift registers
 * Tests all DOUT bytes with various patterns to verify hardware
 * @note This function runs forever
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
 * Tests routing rules and message forwarding
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
