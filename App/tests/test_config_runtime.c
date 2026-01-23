/**
 * @file test_config_runtime.c
 * @brief Runtime test configuration and control implementation
 */

#include "App/tests/test_config_runtime.h"
#include "App/tests/test_debug.h"
#include "App/tests/module_tests.h"
#include "cmsis_os2.h"
#include "ff.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PRIVATE STATE
// =============================================================================

static test_exec_config_t g_exec_config = {
  .timeout_ms = 30000,           // 30 second default timeout
  .enable_benchmarking = 1,      // Benchmarking on by default
  .enable_logging = 0,           // Logging off by default
  .abort_on_failure = 0,         // Continue on failure
  .verbose_output = 1            // Verbose output on
};

static test_selection_t g_selection = {
  .test_enabled = {0},           // All disabled initially
  .run_count = 1                 // Run once
};

static test_perf_metrics_t g_perf_metrics[MODULE_TEST_ALL_ID + 1] = {0};

static struct {
  uint32_t start_time;
  uint32_t timeout_ms;
  uint8_t active;
} g_timeout = {0};

static FIL g_log_file;
static uint8_t g_log_open = 0;

// =============================================================================
// CONFIGURATION MANAGEMENT
// =============================================================================

void test_config_init(void)
{
  // Set defaults
  g_exec_config.timeout_ms = 30000;
  g_exec_config.enable_benchmarking = 1;
  g_exec_config.enable_logging = 0;
  g_exec_config.abort_on_failure = 0;
  g_exec_config.verbose_output = 1;
  
  // Enable all finite tests by default
  memset(g_selection.test_enabled, 0, sizeof(g_selection.test_enabled));
  g_selection.test_enabled[MODULE_TEST_OLED_SSD1322_ID] = 1;
  g_selection.test_enabled[MODULE_TEST_PATCH_SD_ID] = 1;
  g_selection.run_count = 1;
  
  // Clear metrics
  memset(g_perf_metrics, 0, sizeof(g_perf_metrics));
}

int test_config_load(const char* filename)
{
  FIL fp;
  if (f_open(&fp, filename, FA_READ) != FR_OK) {
    return -1;
  }
  
  char line[128];
  while (f_gets(line, sizeof(line), &fp)) {
    // Remove newline
    line[strcspn(line, "\r\n")] = 0;
    
    // Skip comments and empty lines
    if (line[0] == '#' || line[0] == 0) continue;
    
    // Parse key=value
    char* eq = strchr(line, '=');
    if (!eq) continue;
    
    *eq = 0;
    char* key = line;
    char* val = eq + 1;
    
    // Trim whitespace
    while (*key == ' ') key++;
    while (*val == ' ') val++;
    
    // Parse configuration
    if (strcmp(key, "timeout_ms") == 0) {
      g_exec_config.timeout_ms = (uint32_t)atoi(val);
    } else if (strcmp(key, "enable_benchmarking") == 0) {
      g_exec_config.enable_benchmarking = (uint8_t)atoi(val);
    } else if (strcmp(key, "enable_logging") == 0) {
      g_exec_config.enable_logging = (uint8_t)atoi(val);
    } else if (strcmp(key, "abort_on_failure") == 0) {
      g_exec_config.abort_on_failure = (uint8_t)atoi(val);
    } else if (strcmp(key, "verbose_output") == 0) {
      g_exec_config.verbose_output = (uint8_t)atoi(val);
    } else if (strcmp(key, "run_count") == 0) {
      g_selection.run_count = (uint8_t)atoi(val);
    }
    // Test-specific enables
    else if (strncmp(key, "enable_", 7) == 0) {
      // Parse test name and enable/disable
      char* test_name = key + 7;
      uint8_t enabled = (uint8_t)atoi(val);
      
      // Map test names to IDs
      if (strcmp(test_name, "oled") == 0) {
        g_selection.test_enabled[MODULE_TEST_OLED_SSD1322_ID] = enabled;
      } else if (strcmp(test_name, "patch_sd") == 0) {
        g_selection.test_enabled[MODULE_TEST_PATCH_SD_ID] = enabled;
      }
    }
  }
  
  f_close(&fp);
  return 0;
}

int test_config_save(const char* filename)
{
  FIL fp;
  if (f_open(&fp, filename, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
    return -1;
  }
  
  f_printf(&fp, "# MidiCore Test Configuration\n");
  f_printf(&fp, "# Generated automatically\n\n");
  
  f_printf(&fp, "[execution]\n");
  f_printf(&fp, "timeout_ms=%lu\n", g_exec_config.timeout_ms);
  f_printf(&fp, "enable_benchmarking=%d\n", g_exec_config.enable_benchmarking);
  f_printf(&fp, "enable_logging=%d\n", g_exec_config.enable_logging);
  f_printf(&fp, "abort_on_failure=%d\n", g_exec_config.abort_on_failure);
  f_printf(&fp, "verbose_output=%d\n", g_exec_config.verbose_output);
  f_printf(&fp, "run_count=%d\n", g_selection.run_count);
  
  f_printf(&fp, "\n[tests]\n");
  f_printf(&fp, "enable_oled=%d\n", g_selection.test_enabled[MODULE_TEST_OLED_SSD1322_ID]);
  f_printf(&fp, "enable_patch_sd=%d\n", g_selection.test_enabled[MODULE_TEST_PATCH_SD_ID]);
  
  f_close(&fp);
  return 0;
}

const test_exec_config_t* test_config_get_exec(void)
{
  return &g_exec_config;
}

const test_selection_t* test_config_get_selection(void)
{
  return &g_selection;
}

void test_config_set_exec(const test_exec_config_t* config)
{
  if (config) {
    memcpy(&g_exec_config, config, sizeof(test_exec_config_t));
  }
}

void test_config_enable_test(module_test_t test_id, uint8_t enabled)
{
  if (test_id <= MODULE_TEST_ALL_ID) {
    g_selection.test_enabled[test_id] = enabled ? 1 : 0;
  }
}

uint8_t test_config_is_enabled(module_test_t test_id)
{
  if (test_id <= MODULE_TEST_ALL_ID) {
    return g_selection.test_enabled[test_id];
  }
  return 0;
}

// =============================================================================
// PERFORMANCE BENCHMARKING
// =============================================================================

void test_perf_start(module_test_t test_id)
{
  if (test_id > MODULE_TEST_ALL_ID) return;
  
  g_perf_metrics[test_id].start_time_ms = osKernelGetTickCount();
  g_perf_metrics[test_id].end_time_ms = 0;
  g_perf_metrics[test_id].duration_ms = 0;
}

test_perf_metrics_t test_perf_end(module_test_t test_id, int result)
{
  test_perf_metrics_t metrics = {0};
  
  if (test_id > MODULE_TEST_ALL_ID) return metrics;
  
  g_perf_metrics[test_id].end_time_ms = osKernelGetTickCount();
  g_perf_metrics[test_id].duration_ms = 
    g_perf_metrics[test_id].end_time_ms - g_perf_metrics[test_id].start_time_ms;
  
  return g_perf_metrics[test_id];
}

const test_perf_metrics_t* test_perf_get(module_test_t test_id)
{
  if (test_id > MODULE_TEST_ALL_ID) return NULL;
  return &g_perf_metrics[test_id];
}

void test_perf_report(module_test_t test_id)
{
  dbg_print("\r\n");
  dbg_print("==============================================\r\n");
  dbg_print("       PERFORMANCE REPORT\r\n");
  dbg_print("==============================================\r\n");
  
  if (test_id == MODULE_TEST_ALL_ID) {
    // Report all tests
    for (int i = 0; i <= MODULE_TEST_ALL_ID; i++) {
      if (g_perf_metrics[i].duration_ms > 0) {
        const char* name = module_tests_get_name((module_test_t)i);
        dbg_printf("%-20s : %lu ms\r\n", name, g_perf_metrics[i].duration_ms);
      }
    }
  } else if (test_id < MODULE_TEST_ALL_ID) {
    // Report single test
    const char* name = module_tests_get_name(test_id);
    const test_perf_metrics_t* m = &g_perf_metrics[test_id];
    
    dbg_printf("Test: %s\r\n", name);
    dbg_printf("Duration: %lu ms\r\n", m->duration_ms);
    dbg_printf("Start:    %lu ms\r\n", m->start_time_ms);
    dbg_printf("End:      %lu ms\r\n", m->end_time_ms);
  }
  
  dbg_print("==============================================\r\n");
  dbg_print("\r\n");
}

int test_perf_save(const char* filename)
{
  FIL fp;
  if (f_open(&fp, filename, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
    return -1;
  }
  
  f_printf(&fp, "# MidiCore Test Performance Metrics\n");
  f_printf(&fp, "# Timestamp: %lu ms\n\n", osKernelGetTickCount());
  
  f_printf(&fp, "Test,Duration_ms,Start_ms,End_ms\n");
  
  for (int i = 0; i <= MODULE_TEST_ALL_ID; i++) {
    if (g_perf_metrics[i].duration_ms > 0) {
      const char* name = module_tests_get_name((module_test_t)i);
      f_printf(&fp, "%s,%lu,%lu,%lu\n",
               name,
               g_perf_metrics[i].duration_ms,
               g_perf_metrics[i].start_time_ms,
               g_perf_metrics[i].end_time_ms);
    }
  }
  
  f_close(&fp);
  return 0;
}

// =============================================================================
// TIMEOUT CONTROL
// =============================================================================

void test_timeout_init(uint32_t timeout_ms)
{
  g_timeout.timeout_ms = timeout_ms;
  g_timeout.start_time = osKernelGetTickCount();
  g_timeout.active = (timeout_ms > 0) ? 1 : 0;
}

void test_timeout_reset(void)
{
  if (g_timeout.active) {
    g_timeout.start_time = osKernelGetTickCount();
  }
}

uint8_t test_timeout_expired(void)
{
  if (!g_timeout.active) return 0;
  
  uint32_t elapsed = osKernelGetTickCount() - g_timeout.start_time;
  return (elapsed >= g_timeout.timeout_ms) ? 1 : 0;
}

uint32_t test_timeout_remaining(void)
{
  if (!g_timeout.active) return 0;
  
  uint32_t elapsed = osKernelGetTickCount() - g_timeout.start_time;
  if (elapsed >= g_timeout.timeout_ms) return 0;
  
  return g_timeout.timeout_ms - elapsed;
}

// =============================================================================
// RESULT LOGGING
// =============================================================================

int test_log_init(const char* filename)
{
  if (g_log_open) {
    f_close(&g_log_file);
    g_log_open = 0;
  }
  
  if (f_open(&g_log_file, filename, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
    return -1;
  }
  
  g_log_open = 1;
  
  // Write header
  f_printf(&g_log_file, "# MidiCore Test Log\n");
  f_printf(&g_log_file, "# Timestamp: %lu ms\n\n", osKernelGetTickCount());
  
  return 0;
}

int test_log_result(const test_result_extended_t* result)
{
  if (!g_log_open || !result) return -1;
  
  f_printf(&g_log_file, "[%lu ms] Test: %s\n",
           osKernelGetTickCount(), result->test_name);
  
  if (result->skipped) {
    f_printf(&g_log_file, "  Status: SKIPPED\n");
  } else if (result->timed_out) {
    f_printf(&g_log_file, "  Status: TIMEOUT\n");
  } else if (result->result == 0) {
    f_printf(&g_log_file, "  Status: PASS\n");
  } else {
    f_printf(&g_log_file, "  Status: FAIL (code %d)\n", result->result);
  }
  
  f_printf(&g_log_file, "  Duration: %lu ms\n", result->metrics.duration_ms);
  f_printf(&g_log_file, "\n");
  
  f_sync(&g_log_file);
  return 0;
}

int test_log_message(const char* message)
{
  if (!g_log_open || !message) return -1;
  
  f_printf(&g_log_file, "[%lu ms] %s\n", osKernelGetTickCount(), message);
  f_sync(&g_log_file);
  return 0;
}

void test_log_close(void)
{
  if (g_log_open) {
    f_close(&g_log_file);
    g_log_open = 0;
  }
}

// =============================================================================
// ENHANCED TEST RUNNER
// =============================================================================

int test_run_single_timed(module_test_t test_id,
                          uint32_t timeout_ms,
                          test_result_extended_t* result)
{
  if (!result) return -1;
  
  // Initialize result
  memset(result, 0, sizeof(test_result_extended_t));
  result->test_id = test_id;
  result->test_name = module_tests_get_name(test_id);
  
  // Start performance measurement
  if (g_exec_config.enable_benchmarking) {
    test_perf_start(test_id);
  }
  
  // Initialize timeout
  if (timeout_ms > 0) {
    test_timeout_init(timeout_ms);
  }
  
  // Run the test
  result->result = module_tests_run(test_id);
  
  // End performance measurement
  if (g_exec_config.enable_benchmarking) {
    result->metrics = test_perf_end(test_id, result->result);
  }
  
  // Check timeout
  result->timed_out = test_timeout_expired();
  
  return 0;
}

int test_run_configured(const test_exec_config_t* config,
                       const test_selection_t* selection,
                       test_result_extended_t* results,
                       uint32_t max_results)
{
  // Use provided config or default
  if (config) {
    test_config_set_exec(config);
  }
  
  // Use provided selection or default
  const test_selection_t* sel = selection ? selection : &g_selection;
  
  // Initialize logging if enabled
  if (g_exec_config.enable_logging) {
    test_log_init("0:/test_results.log");
  }
  
  int test_count = 0;
  
  // Iterate through tests
  for (module_test_t tid = MODULE_TEST_OLED_SSD1322_ID; 
       tid < MODULE_TEST_ALL_ID; 
       tid = (module_test_t)(tid + 1)) {
    
    // Check if test is enabled
    if (!sel->test_enabled[tid]) continue;
    
    // Check if we have space in results array
    if (results && test_count >= max_results) break;
    
    // Run the test
    test_result_extended_t* res = results ? &results[test_count] : NULL;
    test_result_extended_t temp_result;
    if (!res) res = &temp_result;
    
    test_run_single_timed(tid, g_exec_config.timeout_ms, res);
    
    // Log result
    if (g_exec_config.enable_logging) {
      test_log_result(res);
    }
    
    // Print to UART if verbose
    if (g_exec_config.verbose_output) {
      dbg_printf("[%s] %s\r\n",
                 res->result == 0 ? "PASS" : "FAIL",
                 res->test_name);
    }
    
    test_count++;
    
    // Abort on failure if configured
    if (g_exec_config.abort_on_failure && res->result != 0) {
      break;
    }
  }
  
  // Close log
  if (g_exec_config.enable_logging) {
    test_log_close();
  }
  
  // Print performance report if enabled
  if (g_exec_config.enable_benchmarking) {
    test_perf_report(MODULE_TEST_ALL_ID);
  }
  
  return test_count;
}
