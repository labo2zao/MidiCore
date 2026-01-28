/**
 * @file tests_common.h
 * @brief Common utilities and macros for test modules
 * 
 * Provides shared functionality for all test modules to reduce duplication
 * and ensure consistent test patterns.
 * 
 * Note: This entire module is excluded from production builds.
 *       Set MODULE_ENABLE_TEST=1 in module_config.h to enable.
 */

#pragma once

#include <stdint.h>
#include "Config/module_config.h"

#if MODULE_ENABLE_TEST

#include "Services/test/test.h"
#include "Services/log/log.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TEST ASSERTION MACROS
// =============================================================================

/**
 * @brief Assert that condition is true
 * @param cond Condition to check
 * @param msg Message to log if assertion fails
 */
#define TEST_ASSERT(cond, msg) do { \
  if (!(cond)) { \
    log_printf("TEST_FAIL", "[%s:%d] ASSERTION FAILED: %s", __FILE__, __LINE__, msg); \
    return -1; \
  } \
} while(0)

/**
 * @brief Assert equality
 */
#define TEST_ASSERT_EQ(a, b, msg) do { \
  if ((a) != (b)) { \
    log_printf("TEST_FAIL", "[%s:%d] ASSERTION FAILED: %s (expected=%d, actual=%d)", \
               __FILE__, __LINE__, msg, (int)(b), (int)(a)); \
    return -1; \
  } \
} while(0)

/**
 * @brief Assert not equal
 */
#define TEST_ASSERT_NE(a, b, msg) do { \
  if ((a) == (b)) { \
    log_printf("TEST_FAIL", "[%s:%d] ASSERTION FAILED: %s (both=%d)", \
               __FILE__, __LINE__, msg, (int)(a)); \
    return -1; \
  } \
} while(0)

/**
 * @brief Assert greater than
 */
#define TEST_ASSERT_GT(a, b, msg) do { \
  if ((a) <= (b)) { \
    log_printf("TEST_FAIL", "[%s:%d] ASSERTION FAILED: %s (%d <= %d)", \
               __FILE__, __LINE__, msg, (int)(a), (int)(b)); \
    return -1; \
  } \
} while(0)

/**
 * @brief Assert less than
 */
#define TEST_ASSERT_LT(a, b, msg) do { \
  if ((a) >= (b)) { \
    log_printf("TEST_FAIL", "[%s:%d] ASSERTION FAILED: %s (%d >= %d)", \
               __FILE__, __LINE__, msg, (int)(a), (int)(b)); \
    return -1; \
  } \
} while(0)

// =============================================================================
// TEST LOOP CONTROL
// =============================================================================

/**
 * @brief Check if test should stop (call periodically in test loops)
 * @return 1 if test should stop, 0 to continue
 */
static inline uint8_t test_should_stop(void) {
  return test_is_stop_requested();
}

/**
 * @brief Sleep for a duration while checking stop flag
 * @param ms Milliseconds to sleep
 * @return 1 if stopped during sleep, 0 if completed normally
 */
static inline uint8_t test_delay_ms(uint32_t ms) {
  const uint32_t check_interval = 10;  // Check every 10ms
  uint32_t elapsed = 0;
  
  while (elapsed < ms) {
    if (test_should_stop()) {
      return 1;
    }
    HAL_Delay(check_interval);
    elapsed += check_interval;
  }
  return 0;
}

/**
 * @brief Standard test loop header
 * Use this at the start of every test loop
 */
#define TEST_LOOP_BEGIN() \
  uint32_t iteration = 0; \
  while (!test_should_stop()) { \
    iteration++;

/**
 * @brief Standard test loop footer
 * Use this at the end of every test loop
 */
#define TEST_LOOP_END(delay_ms) \
    if (test_delay_ms(delay_ms)) break; \
  } \
  log_printf("TEST", "Test stopped after %u iterations", iteration);

// =============================================================================
// TEST LOGGING MACROS
// =============================================================================

#define TEST_LOG_INFO(fmt, ...) log_printf("TEST_INFO", fmt, ##__VA_ARGS__)
#define TEST_LOG_WARN(fmt, ...) log_printf("TEST_WARN", fmt, ##__VA_ARGS__)
#define TEST_LOG_ERROR(fmt, ...) log_printf("TEST_ERROR", fmt, ##__VA_ARGS__)
#define TEST_LOG_PASS(fmt, ...) log_printf("TEST_PASS", fmt, ##__VA_ARGS__)
#define TEST_LOG_FAIL(fmt, ...) log_printf("TEST_FAIL", fmt, ##__VA_ARGS__)

// =============================================================================
// PERFORMANCE MEASUREMENT
// =============================================================================

/**
 * @brief Performance timer structure
 */
typedef struct {
  uint32_t start_time_ms;
  uint32_t count;
  uint32_t min_ms;
  uint32_t max_ms;
  uint32_t total_ms;
} test_perf_t;

/**
 * @brief Initialize performance timer
 */
static inline void test_perf_init(test_perf_t* perf) {
  perf->start_time_ms = 0;
  perf->count = 0;
  perf->min_ms = 0xFFFFFFFF;
  perf->max_ms = 0;
  perf->total_ms = 0;
}

/**
 * @brief Start performance measurement
 */
static inline void test_perf_start(test_perf_t* perf) {
  perf->start_time_ms = HAL_GetTick();
}

/**
 * @brief End performance measurement
 */
static inline void test_perf_end(test_perf_t* perf) {
  uint32_t elapsed = HAL_GetTick() - perf->start_time_ms;
  perf->count++;
  perf->total_ms += elapsed;
  if (elapsed < perf->min_ms) perf->min_ms = elapsed;
  if (elapsed > perf->max_ms) perf->max_ms = elapsed;
}

/**
 * @brief Get average duration
 */
static inline uint32_t test_perf_avg(const test_perf_t* perf) {
  return perf->count ? (perf->total_ms / perf->count) : 0;
}

// =============================================================================
// TEST PATTERN TEMPLATES
// =============================================================================

/**
 * @brief Basic hardware test template
 * Use for simple input/output tests
 */
#define DEFINE_BASIC_HW_TEST(name, init_func, test_func, cleanup_func) \
  void test_##name##_run(void) { \
    TEST_LOG_INFO("Starting " #name " test"); \
    if (init_func() != 0) { \
      TEST_LOG_ERROR("Initialization failed"); \
      return; \
    } \
    TEST_LOOP_BEGIN() \
      if (test_func() != 0) { \
        TEST_LOG_ERROR("Test iteration failed at #%u", iteration); \
        break; \
      } \
    TEST_LOOP_END(100) \
    if (cleanup_func) cleanup_func(); \
    TEST_LOG_PASS(#name " test completed"); \
  }

#endif  // MODULE_ENABLE_TEST

#ifdef __cplusplus
}
#endif
