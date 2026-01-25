/**
 * @file perf_monitor.h
 * @brief Performance monitoring and benchmarking for production use
 * 
 * Provides runtime performance monitoring, benchmarking, and metrics collection
 * that can be used in both testing and production environments.
 * 
 * Features:
 * - Millisecond-precision timing
 * - Per-operation metrics tracking
 * - CSV export for analysis
 * - UART and SD card reporting
 * - Configurable metric storage
 * 
 * Usage in production:
 * - Monitor critical operation timing
 * - Identify performance bottlenecks
 * - Track performance trends over time
 * - Export data for offline analysis
 */

#ifndef PERF_MONITOR_H
#define PERF_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// =============================================================================
// CONFIGURATION
// =============================================================================

#ifndef PERF_MONITOR_MAX_METRICS
#define PERF_MONITOR_MAX_METRICS 32  // Maximum tracked operations
#endif

// =============================================================================
// TYPES
// =============================================================================

/**
 * @brief Performance metric identifier
 */
typedef uint16_t perf_metric_id_t;

/**
 * @brief Performance metrics for an operation
 */
typedef struct {
  uint32_t start_time_ms;        // Operation start timestamp
  uint32_t end_time_ms;          // Operation end timestamp
  uint32_t duration_ms;          // Total duration
  uint32_t call_count;           // Number of times measured
  uint32_t total_duration_ms;    // Cumulative duration
  uint32_t min_duration_ms;      // Minimum duration
  uint32_t max_duration_ms;      // Maximum duration
  const char* name;              // Operation name
} perf_metrics_t;

// =============================================================================
// API - INITIALIZATION
// =============================================================================

/**
 * @brief Initialize performance monitoring system
 * @return 0 on success, negative on error
 */
int perf_monitor_init(void);

/**
 * @brief Register a new metric for tracking
 * @param name Operation name (must be static string)
 * @return Metric ID (0-31), or negative on error
 */
perf_metric_id_t perf_monitor_register(const char* name);

// =============================================================================
// API - MEASUREMENT
// =============================================================================

/**
 * @brief Start measuring an operation
 * @param id Metric ID from perf_monitor_register()
 */
void perf_monitor_start(perf_metric_id_t id);

/**
 * @brief End measuring an operation
 * @param id Metric ID
 * @return Duration in milliseconds
 */
uint32_t perf_monitor_end(perf_metric_id_t id);

/**
 * @brief Measure a single operation (convenience function)
 * @param name Operation name
 * @param duration_ms Duration to record
 */
void perf_monitor_record(const char* name, uint32_t duration_ms);

// =============================================================================
// API - QUERIES
// =============================================================================

/**
 * @brief Get metrics for a specific operation
 * @param id Metric ID
 * @return Pointer to metrics, or NULL if not found
 */
const perf_metrics_t* perf_monitor_get(perf_metric_id_t id);

/**
 * @brief Get metrics by name
 * @param name Operation name
 * @return Pointer to metrics, or NULL if not found
 */
const perf_metrics_t* perf_monitor_get_by_name(const char* name);

/**
 * @brief Get average duration for an operation
 * @param id Metric ID
 * @return Average duration in ms, 0 if not found or no data
 */
uint32_t perf_monitor_get_average(perf_metric_id_t id);

// =============================================================================
// API - REPORTING
// =============================================================================

/**
 * @brief Print all metrics to UART
 */
void perf_monitor_report_uart(void);

/**
 * @brief Save metrics to CSV file on SD card
 * @param filename Output file path
 * @return 0 on success, negative on error
 */
int perf_monitor_save_csv(const char* filename);

/**
 * @brief Clear all collected metrics
 */
void perf_monitor_reset(void);

/**
 * @brief Clear metrics for specific operation
 * @param id Metric ID
 */
void perf_monitor_reset_metric(perf_metric_id_t id);

#ifdef __cplusplus
}
#endif

#endif // PERF_MONITOR_H
