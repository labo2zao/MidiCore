/**
 * @file performance_cli.c
 * @brief CLI integration for performance monitoring
 * 
 * CPU usage, memory stats, and operation benchmarking
 */

#include "Services/performance/perf_monitor.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int perf_param_get_metric_count(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = PERF_MONITOR_MAX_METRICS;
  return 0;
}

static int perf_param_get_metric_name(uint8_t track, param_value_t* out) {
  if (track >= PERF_MONITOR_MAX_METRICS) return -1;
  const perf_metrics_t* metrics = perf_monitor_get(track);
  if (!metrics || !metrics->name) {
    out->str_val = "(empty)";
  } else {
    out->str_val = metrics->name;
  }
  return 0;
}

static int perf_param_get_call_count(uint8_t track, param_value_t* out) {
  if (track >= PERF_MONITOR_MAX_METRICS) return -1;
  const perf_metrics_t* metrics = perf_monitor_get(track);
  if (!metrics) return -1;
  out->int_val = metrics->call_count;
  return 0;
}

static int perf_param_get_avg_duration(uint8_t track, param_value_t* out) {
  if (track >= PERF_MONITOR_MAX_METRICS) return -1;
  out->int_val = perf_monitor_get_average(track);
  return 0;
}

static int perf_param_get_min_duration(uint8_t track, param_value_t* out) {
  if (track >= PERF_MONITOR_MAX_METRICS) return -1;
  const perf_metrics_t* metrics = perf_monitor_get(track);
  if (!metrics) return -1;
  out->int_val = metrics->min_duration_ms;
  return 0;
}

static int perf_param_get_max_duration(uint8_t track, param_value_t* out) {
  if (track >= PERF_MONITOR_MAX_METRICS) return -1;
  const perf_metrics_t* metrics = perf_monitor_get(track);
  if (!metrics) return -1;
  out->int_val = metrics->max_duration_ms;
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int perf_cli_enable(uint8_t track) {
  (void)track;
  return 0;
}

static int perf_cli_disable(uint8_t track) {
  (void)track;
  return 0;
}

static int perf_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_performance_descriptor = {
  .name = "performance",
  .description = "Performance monitoring and benchmarking",
  .category = MODULE_CATEGORY_SYSTEM,
  .init = perf_monitor_init,
  .enable = perf_cli_enable,
  .disable = perf_cli_disable,
  .get_status = perf_cli_get_status,
  .has_per_track_state = 1,  // Per-metric data
  .is_global = 0,
  .max_tracks = PERF_MONITOR_MAX_METRICS
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_performance_parameters(void) {
  module_param_t params[] = {
    {
      .name = "metric_count",
      .description = "Maximum tracked metrics",
      .type = PARAM_TYPE_INT,
      .min = PERF_MONITOR_MAX_METRICS,
      .max = PERF_MONITOR_MAX_METRICS,
      .read_only = 1,
      .get_value = perf_param_get_metric_count,
      .set_value = NULL
    },
    {
      .name = "name",
      .description = "Metric operation name",
      .type = PARAM_TYPE_STRING,
      .read_only = 1,
      .get_value = perf_param_get_metric_name,
      .set_value = NULL
    },
    {
      .name = "call_count",
      .description = "Number of measurements",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 0x7FFFFFFF,
      .read_only = 1,
      .get_value = perf_param_get_call_count,
      .set_value = NULL
    },
    {
      .name = "avg_ms",
      .description = "Average duration (ms)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 0x7FFFFFFF,
      .read_only = 1,
      .get_value = perf_param_get_avg_duration,
      .set_value = NULL
    },
    {
      .name = "min_ms",
      .description = "Minimum duration (ms)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 0x7FFFFFFF,
      .read_only = 1,
      .get_value = perf_param_get_min_duration,
      .set_value = NULL
    },
    {
      .name = "max_ms",
      .description = "Maximum duration (ms)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 0x7FFFFFFF,
      .read_only = 1,
      .get_value = perf_param_get_max_duration,
      .set_value = NULL
    }
  };
  
  s_performance_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_performance_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int performance_register_cli(void) {
  setup_performance_parameters();
  return module_registry_register(&s_performance_descriptor);
}
