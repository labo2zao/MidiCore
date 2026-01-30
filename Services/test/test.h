/**
 * @file test.h
 * @brief Test Module - Runtime module testing service
 * 
 * Provides a service module for running module tests at runtime via CLI.
 * Integrates with the existing module_tests.c framework but allows
 * test selection and execution via CLI commands.
 * 
 * Features:
 * - Run individual module tests via CLI
 * - Query test status and results
 * - List available tests
 * - Enable/disable test execution
 * - Integration with module registry
 * 
 * Usage:
 * 1. Call test_init() during system initialization
 * 2. Use CLI commands: "test run <module>", "test status", "test list"
 * 3. Test results are reported via UART
 * 
 * Note: This entire module is excluded from production builds.
 *       Set MODULE_ENABLE_TEST=1 in module_config.h to enable.
 */

#pragma once

#include <stdint.h>
#include "Config/module_config.h"

// Test module is only available when MODULE_ENABLE_TEST is enabled
#if MODULE_ENABLE_TEST

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONFIGURATION
// =============================================================================

#ifndef TEST_MAX_NAME_LEN
#define TEST_MAX_NAME_LEN 32
#endif

#ifndef TEST_MAX_DESCRIPTION_LEN
#define TEST_MAX_DESCRIPTION_LEN 128
#endif

// =============================================================================
// TYPES
// =============================================================================

/**
 * @brief Test execution status
 */
typedef enum {
  TEST_STATUS_IDLE = 0,       // No test running
  TEST_STATUS_RUNNING,        // Test in progress
  TEST_STATUS_PAUSED,         // Test paused
  TEST_STATUS_STOPPING,       // Test stop requested
  TEST_STATUS_STOPPED,        // Test stopped gracefully
  TEST_STATUS_PASSED,         // Test passed
  TEST_STATUS_FAILED,         // Test failed
  TEST_STATUS_TIMEOUT,        // Test timed out
  TEST_STATUS_ERROR           // Test error
} test_status_t;

/**
 * @brief Test result structure
 */
typedef struct {
  char test_name[TEST_MAX_NAME_LEN];
  test_status_t status;
  uint32_t start_time_ms;
  uint32_t end_time_ms;
  uint32_t duration_ms;
  uint32_t iteration_count;
  uint32_t assertions_total;
  uint32_t assertions_passed;
  uint32_t assertions_failed;
  char error_message[TEST_MAX_DESCRIPTION_LEN];
} test_result_t;

/**
 * @brief Test configuration
 */
typedef struct {
  uint8_t enabled;              // Test module enabled
  uint8_t auto_run;             // Auto-run tests on startup
  uint32_t timeout_ms;          // Test timeout in milliseconds
  uint8_t verbose;              // Verbose output
} test_config_t;

// =============================================================================
// API - INITIALIZATION
// =============================================================================

/**
 * @brief Initialize the test module
 * @return 0 on success, negative on error
 */
int test_init(void);

/**
 * @brief Get test module initialization status
 * @return 1 if initialized, 0 if not
 */
uint8_t test_is_initialized(void);

// =============================================================================
// API - TEST EXECUTION
// =============================================================================

/**
 * @brief Run a specific module test
 * @param test_name Name of the test to run (e.g., "ainser64", "srio", "router")
 * @param duration_ms Duration to run test (0 = one iteration, -1 = infinite)
 * @return 0 on success, negative on error
 */
int test_run(const char* test_name, int32_t duration_ms);

/**
 * @brief Stop currently running test gracefully
 * Sets stop flag that tests should check periodically
 * @return 0 on success, negative on error
 */
int test_stop(void);

/**
 * @brief Pause currently running test
 * @return 0 on success, negative on error
 */
int test_pause(void);

/**
 * @brief Resume paused test
 * @return 0 on success, negative on error
 */
int test_resume(void);

/**
 * @brief Check if a test is currently running
 * @return 1 if running, 0 if not
 */
uint8_t test_is_running(void);

/**
 * @brief Check if test stop has been requested
 * Tests should call this periodically and exit gracefully if true
 * @return 1 if stop requested, 0 otherwise
 */
uint8_t test_is_stop_requested(void);

/**
 * @brief Check if test is paused
 * @return 1 if paused, 0 otherwise
 */
uint8_t test_is_paused(void);

// =============================================================================
// API - TEST STATUS & RESULTS
// =============================================================================

/**
 * @brief Get current test status
 * @param result Pointer to result structure to fill
 * @return 0 on success, negative on error
 */
int test_get_status(test_result_t* result);

/**
 * @brief Get last test result
 * @param result Pointer to result structure to fill
 * @return 0 on success, negative on error
 */
int test_get_last_result(test_result_t* result);

/**
 * @brief Clear test results
 * @return 0 on success, negative on error
 */
int test_clear_results(void);

// =============================================================================
// API - TEST DISCOVERY
// =============================================================================

/**
 * @brief Get number of available tests
 * @return Number of tests
 */
uint32_t test_get_count(void);

/**
 * @brief Get test name by index
 * @param index Test index (0 to test_get_count()-1)
 * @return Test name string, or NULL if index invalid
 */
const char* test_get_name(uint32_t index);

/**
 * @brief Get test description by name
 * @param test_name Test name
 * @return Description string, or NULL if not found
 */
const char* test_get_description(const char* test_name);

// =============================================================================
// API - CONFIGURATION
// =============================================================================

/**
 * @brief Enable test module
 * @param enabled 1 to enable, 0 to disable
 * @return 0 on success, negative on error
 */
int test_set_enabled(uint8_t enabled);

/**
 * @brief Get test module enabled status
 * @return 1 if enabled, 0 if disabled
 */
uint8_t test_get_enabled(void);

/**
 * @brief Set verbose output mode
 * @param verbose 1 for verbose, 0 for quiet
 * @return 0 on success, negative on error
 */
int test_set_verbose(uint8_t verbose);

/**
 * @brief Get verbose output mode
 * @return 1 if verbose, 0 if quiet
 */
uint8_t test_get_verbose(void);

/**
 * @brief Set test timeout
 * @param timeout_ms Timeout in milliseconds
 * @return 0 on success, negative on error
 */
int test_set_timeout(uint32_t timeout_ms);

/**
 * @brief Get test timeout
 * @return Timeout in milliseconds
 */
uint32_t test_get_timeout(void);

// =============================================================================
// API - MODULE REGISTRY INTEGRATION
// =============================================================================

/**
 * @brief Register test module with module registry
 * Called automatically by test_init() if MODULE_REGISTRY is enabled
 * @return 0 on success, negative on error
 */
int test_register_with_registry(void);

#else  // !MODULE_ENABLE_TEST

// Provide stub functions when test module is disabled
static inline int test_init(void) { return 0; }
static inline uint8_t test_is_initialized(void) { return 0; }
static inline int test_run(const char* test_name, int32_t duration_ms) { (void)test_name; (void)duration_ms; return -1; }
static inline int test_stop(void) { return -1; }
static inline uint8_t test_is_running(void) { return 0; }
static inline uint32_t test_get_count(void) { return 0; }
static inline const char* test_get_name(uint32_t index) { (void)index; return NULL; }
static inline const char* test_get_description(const char* test_name) { (void)test_name; return NULL; }
static inline int test_set_enabled(uint8_t enabled) { (void)enabled; return -1; }
static inline uint8_t test_get_enabled(void) { return 0; }
static inline int test_set_verbose(uint8_t verbose) { (void)verbose; return -1; }
static inline uint8_t test_get_verbose(void) { return 0; }
static inline int test_set_timeout(uint32_t timeout_ms) { (void)timeout_ms; return -1; }
static inline uint32_t test_get_timeout(void) { return 0; }
static inline int test_register_with_registry(void) { return 0; }

#endif  // MODULE_ENABLE_TEST

#ifdef __cplusplus
}
#endif
