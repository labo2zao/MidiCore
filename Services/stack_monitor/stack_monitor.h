/**
 * @file stack_monitor.h
 * @brief FreeRTOS Stack Usage Monitor
 * 
 * Provides runtime monitoring of FreeRTOS task stack usage to detect
 * and prevent stack overflows. Tracks high-water marks and detects
 * corruption of the 0xA5 fill pattern.
 * 
 * Features:
 * - Runtime stack usage tracking via uxTaskGetStackHighWaterMark()
 * - Detection of 0xA5 pattern corruption in stack guard regions
 * - Configurable warning/critical thresholds
 * - Periodic monitoring task
 * - CLI interface for stack inspection
 * - Logging and telemetry export
 * 
 * Architecture:
 * - Service layer (no HAL dependencies)
 * - Periodic background task with low priority
 * - Thread-safe API
 * - No dynamic memory allocation
 * 
 * Usage:
 * 1. Call stack_monitor_init() during system initialization
 * 2. Monitoring task runs automatically
 * 3. Query stack usage via CLI or stack_monitor_get_info()
 * 4. Warnings/critical alerts logged via dbg_printf
 * 
 * @note Requires FreeRTOS with:
 *       - INCLUDE_uxTaskGetStackHighWaterMark = 1
 *       - configCHECK_FOR_STACK_OVERFLOW = 2 (recommended)
 *       - configUSE_TRACE_FACILITY = 1
 */

#pragma once

#include <stdint.h>
#include "cmsis_os.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONFIGURATION
// =============================================================================

/** @brief Enable/disable stack monitoring (can be disabled to save resources) */
#ifndef STACK_MONITOR_ENABLED
#define STACK_MONITOR_ENABLED 1
#endif

/** @brief Stack monitor task priority (low to not interfere with real-time tasks) */
#ifndef STACK_MONITOR_PRIORITY
#define STACK_MONITOR_PRIORITY osPriorityBelowNormal
#endif

/** @brief Stack monitor task stack size (bytes)
 * 
 * CRITICAL: Must be large enough for:
 * - osThreadEnumerate() array (~64 bytes for 16 tasks)
 * - Local variables and task_info structs (~200 bytes)
 * - dbg_printf() formatting buffers (~256 bytes) 
 * - Function call stack frames (~100 bytes)
 * - Stack guard 0xA5 pattern (~100 bytes)
 * - Safety margin for diagnostics (~300 bytes)
 * Total minimum: ~1020 bytes
 * 
 * WARNING: 512 bytes causes stack overflow in monitor task itself!
 */
#ifndef STACK_MONITOR_STACK_SIZE
#define STACK_MONITOR_STACK_SIZE 1024  // Increased from 512 to prevent overflow
#endif

/** @brief Monitoring interval in milliseconds */
#ifndef STACK_MONITOR_INTERVAL_MS
#define STACK_MONITOR_INTERVAL_MS 5000  // Check every 5 seconds
#endif

/** @brief Warning threshold (percentage of stack remaining) */
#ifndef STACK_MONITOR_WARNING_THRESHOLD
#define STACK_MONITOR_WARNING_THRESHOLD 20  // Warn at 20% remaining
#endif

/** @brief Critical threshold (percentage of stack remaining) */
#ifndef STACK_MONITOR_CRITICAL_THRESHOLD
#define STACK_MONITOR_CRITICAL_THRESHOLD 5   // Critical at 5% remaining
#endif

/** @brief Maximum number of tasks to track */
#ifndef STACK_MONITOR_MAX_TASKS
#define STACK_MONITOR_MAX_TASKS 16
#endif

// =============================================================================
// TYPES
// =============================================================================

/**
 * @brief Stack status level
 */
typedef enum {
  STACK_STATUS_OK = 0,        ///< Stack usage is healthy
  STACK_STATUS_WARNING = 1,   ///< Stack usage approaching limit (warning threshold)
  STACK_STATUS_CRITICAL = 2,  ///< Stack usage critically low (critical threshold)
  STACK_STATUS_OVERFLOW = 3   ///< Stack overflow detected (0xA5 pattern corrupted)
} stack_status_t;

/**
 * @brief Task stack information
 */
typedef struct {
  char task_name[16];             ///< Task name (from FreeRTOS)
  uint32_t stack_size;            ///< Allocated stack size (words)
  uint32_t stack_size_bytes;      ///< Allocated stack size (bytes)
  uint32_t high_water_mark;       ///< Minimum free stack ever (words)
  uint32_t high_water_mark_bytes; ///< Minimum free stack ever (bytes)
  uint32_t used_bytes;            ///< Stack used (bytes)
  uint32_t used_percent;          ///< Stack usage percentage
  uint32_t free_bytes;            ///< Stack remaining (bytes)
  uint32_t free_percent;          ///< Stack free percentage
  stack_status_t status;          ///< Current status
  uint32_t timestamp;             ///< Last check timestamp (ms)
} stack_info_t;

/**
 * @brief Stack monitor statistics
 */
typedef struct {
  uint32_t total_checks;          ///< Total monitoring cycles completed
  uint32_t warning_count;         ///< Number of warnings issued
  uint32_t critical_count;        ///< Number of critical alerts issued
  uint32_t overflow_count;        ///< Number of overflows detected
  uint32_t last_check_time;       ///< Timestamp of last check (ms)
} stack_monitor_stats_t;

// =============================================================================
// API - INITIALIZATION
// =============================================================================

/**
 * @brief Initialize stack monitor
 * Creates the monitoring task and initializes internal state
 * @return 0 on success, negative on error
 */
int stack_monitor_init(void);

/**
 * @brief Start stack monitoring (if not auto-started)
 * @return 0 on success, negative on error
 */
int stack_monitor_start(void);

/**
 * @brief Stop stack monitoring
 * @return 0 on success, negative on error
 */
int stack_monitor_stop(void);

// =============================================================================
// API - QUERY
// =============================================================================

/**
 * @brief Get stack info for a specific task by handle
 * @param task_handle FreeRTOS task handle (NULL for current task)
 * @param info Pointer to stack_info_t to fill
 * @return 0 on success, negative on error
 */
int stack_monitor_get_info(osThreadId_t task_handle, stack_info_t* info);

/**
 * @brief Get stack info for a specific task by name
 * @param task_name Task name string
 * @param info Pointer to stack_info_t to fill
 * @return 0 on success, negative on error
 */
int stack_monitor_get_info_by_name(const char* task_name, stack_info_t* info);

/**
 * @brief Get list of all tasks with stack info
 * @param info_array Array to fill with stack_info_t (caller-allocated)
 * @param max_tasks Maximum number of tasks to return
 * @param num_tasks Output: actual number of tasks returned
 * @return 0 on success, negative on error
 */
int stack_monitor_get_all_tasks(stack_info_t* info_array, uint32_t max_tasks, uint32_t* num_tasks);

/**
 * @brief Get monitor statistics
 * @param stats Pointer to stack_monitor_stats_t to fill
 * @return 0 on success, negative on error
 */
int stack_monitor_get_stats(stack_monitor_stats_t* stats);

// =============================================================================
// API - CONTROL
// =============================================================================

/**
 * @brief Set warning threshold (percentage)
 * @param threshold Warning threshold (0-100)
 */
void stack_monitor_set_warning_threshold(uint32_t threshold);

/**
 * @brief Set critical threshold (percentage)
 * @param threshold Critical threshold (0-100)
 */
void stack_monitor_set_critical_threshold(uint32_t threshold);

/**
 * @brief Set monitoring interval (milliseconds)
 * @param interval_ms Monitoring interval
 */
void stack_monitor_set_interval(uint32_t interval_ms);

/**
 * @brief Force an immediate stack check
 * Bypasses the normal monitoring interval
 */
void stack_monitor_check_now(void);

// =============================================================================
// API - REPORTING
// =============================================================================

/**
 * @brief Print stack info for a specific task
 * @param task_handle Task handle (NULL for current task)
 */
void stack_monitor_print_task(osThreadId_t task_handle);

/**
 * @brief Print stack info for all tasks
 * @param verbose If true, include detailed information
 */
void stack_monitor_print_all(uint8_t verbose);

/**
 * @brief Print summary statistics
 */
void stack_monitor_print_stats(void);

/**
 * @brief Export stack data as CSV
 * Useful for telemetry and logging
 */
void stack_monitor_export_csv(void);

// =============================================================================
// API - ALERT CALLBACKS (optional)
// =============================================================================

/**
 * @brief Stack alert callback type
 * @param task_name Name of the task with alert
 * @param info Stack info structure
 * @param status Alert status level
 */
typedef void (*stack_alert_callback_t)(const char* task_name, 
                                       const stack_info_t* info,
                                       stack_status_t status);

/**
 * @brief Register alert callback
 * Called when a task triggers warning/critical/overflow status
 * @param callback Callback function (NULL to unregister)
 */
void stack_monitor_register_callback(stack_alert_callback_t callback);

#ifdef __cplusplus
}
#endif
