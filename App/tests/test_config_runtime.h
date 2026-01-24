/**
 * @file test_config_runtime.h
 * @brief Runtime test configuration and control
 * 
 * Provides runtime configuration for test execution including:
 * - Dynamic test selection
 * - Performance benchmarking
 * - Test timeout control
 * - Result persistence
 */

#ifndef TEST_CONFIG_RUNTIME_H
#define TEST_CONFIG_RUNTIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "App/tests/module_tests.h"

// =============================================================================
// CONFIGURATION STRUCTURES
// =============================================================================

/**
 * @brief Test execution configuration
 */
typedef struct {
  uint32_t timeout_ms;           // Test timeout (0 = no timeout)
  uint8_t  enable_benchmarking;  // Enable performance measurement
  uint8_t  enable_logging;       // Log results to SD card
  uint8_t  abort_on_failure;     // Stop on first failure
  uint8_t  verbose_output;       // Detailed UART output
} test_exec_config_t;

/**
 * @brief Test selection configuration
 */
typedef struct {
  uint8_t test_enabled[MODULE_TEST_ALL_ID + 1];  // Enable/disable each test
  uint8_t run_count;                              // Number of times to run
} test_selection_t;

/**
 * @brief Performance metrics for a test
 */
typedef struct {
  uint32_t start_time_ms;        // Test start timestamp
  uint32_t end_time_ms;          // Test end timestamp
  uint32_t duration_ms;          // Total execution time
  uint32_t peak_stack_usage;     // Peak stack usage (bytes)
  uint32_t memory_allocated;     // Heap allocation (if any)
} test_perf_metrics_t;

/**
 * @brief Complete test result with metrics
 */
typedef struct {
  module_test_t test_id;         // Test identifier
  const char* test_name;         // Test name string
  int result;                    // Test result (0 = pass, <0 = fail)
  uint8_t skipped;               // Test was skipped
  uint8_t timed_out;             // Test timed out
  test_perf_metrics_t metrics;   // Performance metrics
} test_result_extended_t;

// =============================================================================
// RUNTIME CONFIGURATION API
// =============================================================================

/**
 * @brief Initialize runtime test configuration with defaults
 */
void test_config_init(void);

/**
 * @brief Load test configuration from SD card
 * @param filename Config file path (e.g., "0:/test_config.txt")
 * @return 0 on success, negative on error
 */
int test_config_load(const char* filename);

/**
 * @brief Save current test configuration to SD card
 * @param filename Config file path
 * @return 0 on success, negative on error
 */
int test_config_save(const char* filename);

/**
 * @brief Get current execution configuration
 * @return Pointer to current config (read-only)
 */
const test_exec_config_t* test_config_get_exec(void);

/**
 * @brief Get current test selection
 * @return Pointer to current selection (read-only)
 */
const test_selection_t* test_config_get_selection(void);

/**
 * @brief Set execution configuration
 * @param config New configuration
 */
void test_config_set_exec(const test_exec_config_t* config);

/**
 * @brief Enable/disable specific test
 * @param test_id Test to configure
 * @param enabled 1 to enable, 0 to disable
 */
void test_config_enable_test(module_test_t test_id, uint8_t enabled);

/**
 * @brief Check if test is enabled
 * @param test_id Test to check
 * @return 1 if enabled, 0 if disabled
 */
uint8_t test_config_is_enabled(module_test_t test_id);

// =============================================================================
// PERFORMANCE BENCHMARKING
// =============================================================================

/**
 * @brief Start performance measurement for a test
 * @param test_id Test being measured
 */
void test_perf_start(module_test_t test_id);

/**
 * @brief End performance measurement for a test
 * @param test_id Test being measured
 * @param result Test result code
 * @return Performance metrics
 */
test_perf_metrics_t test_perf_end(module_test_t test_id, int result);

/**
 * @brief Get performance metrics for last run of a test
 * @param test_id Test to query
 * @return Pointer to metrics (NULL if not available)
 */
const test_perf_metrics_t* test_perf_get(module_test_t test_id);

/**
 * @brief Print performance report to UART
 * @param test_id Test to report (or MODULE_TEST_ALL_ID for all)
 */
void test_perf_report(module_test_t test_id);

/**
 * @brief Save performance metrics to SD card
 * @param filename Output file path
 * @return 0 on success, negative on error
 */
int test_perf_save(const char* filename);

// =============================================================================
// TEST TIMEOUT CONTROL
// =============================================================================

/**
 * @brief Initialize test timeout watchdog
 * @param timeout_ms Timeout in milliseconds (0 = disable)
 */
void test_timeout_init(uint32_t timeout_ms);

/**
 * @brief Reset watchdog timer (call periodically in test)
 */
void test_timeout_reset(void);

/**
 * @brief Check if test has timed out
 * @return 1 if timed out, 0 otherwise
 */
uint8_t test_timeout_expired(void);

/**
 * @brief Get remaining time before timeout
 * @return Milliseconds remaining (0 if expired or disabled)
 */
uint32_t test_timeout_remaining(void);

// =============================================================================
// RESULT LOGGING
// =============================================================================

/**
 * @brief Initialize result logging to SD card
 * @param filename Log file path
 * @return 0 on success, negative on error
 */
int test_log_init(const char* filename);

/**
 * @brief Log test result to file
 * @param result Test result structure
 * @return 0 on success, negative on error
 */
int test_log_result(const test_result_extended_t* result);

/**
 * @brief Log text message to file
 * @param message Message to log
 * @return 0 on success, negative on error
 */
int test_log_message(const char* message);

/**
 * @brief Close log file
 */
void test_log_close(void);

// =============================================================================
// ENHANCED TEST RUNNER
// =============================================================================

/**
 * @brief Run tests with runtime configuration
 * @param config Execution configuration (NULL for defaults)
 * @param selection Test selection (NULL for all enabled)
 * @param results Output array for results (can be NULL)
 * @param max_results Size of results array
 * @return Number of tests run, negative on error
 */
int test_run_configured(const test_exec_config_t* config,
                       const test_selection_t* selection,
                       test_result_extended_t* results,
                       uint32_t max_results);

/**
 * @brief Run single test with timeout and benchmarking
 * @param test_id Test to run
 * @param timeout_ms Timeout in milliseconds (0 = no timeout)
 * @param result Output for test result
 * @return 0 on success, negative on error
 */
int test_run_single_timed(module_test_t test_id,
                          uint32_t timeout_ms,
                          test_result_extended_t* result);

#ifdef __cplusplus
}
#endif

#endif // TEST_CONFIG_RUNTIME_H
