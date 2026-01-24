/**
 * @file perf_monitor.c
 * @brief Performance monitoring implementation
 */

#include "Services/performance/perf_monitor.h"
#include "cmsis_os2.h"
#include "ff.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PRIVATE STATE
// =============================================================================

static perf_metrics_t g_metrics[PERF_MONITOR_MAX_METRICS];
static uint8_t g_metric_count = 0;
static uint8_t g_initialized = 0;

// =============================================================================
// INITIALIZATION
// =============================================================================

int perf_monitor_init(void)
{
  memset(g_metrics, 0, sizeof(g_metrics));
  g_metric_count = 0;
  g_initialized = 1;
  return 0;
}

perf_metric_id_t perf_monitor_register(const char* name)
{
  if (!g_initialized) {
    perf_monitor_init();
  }
  
  if (g_metric_count >= PERF_MONITOR_MAX_METRICS) {
    return (perf_metric_id_t)-1;
  }
  
  // Check if already registered
  for (uint8_t i = 0; i < g_metric_count; i++) {
    if (g_metrics[i].name == name || 
        (g_metrics[i].name && strcmp(g_metrics[i].name, name) == 0)) {
      return i;
    }
  }
  
  // Register new metric
  perf_metric_id_t id = g_metric_count++;
  g_metrics[id].name = name;
  g_metrics[id].min_duration_ms = 0xFFFFFFFF;
  g_metrics[id].max_duration_ms = 0;
  g_metrics[id].call_count = 0;
  g_metrics[id].total_duration_ms = 0;
  
  return id;
}

// =============================================================================
// MEASUREMENT
// =============================================================================

void perf_monitor_start(perf_metric_id_t id)
{
  if (id >= g_metric_count) return;
  
  g_metrics[id].start_time_ms = osKernelGetTickCount();
}

uint32_t perf_monitor_end(perf_metric_id_t id)
{
  uint32_t end_time = osKernelGetTickCount();
  
  if (id >= g_metric_count) return 0;
  
  g_metrics[id].end_time_ms = end_time;
  g_metrics[id].duration_ms = end_time - g_metrics[id].start_time_ms;
  
  // Update statistics
  g_metrics[id].call_count++;
  g_metrics[id].total_duration_ms += g_metrics[id].duration_ms;
  
  if (g_metrics[id].duration_ms < g_metrics[id].min_duration_ms) {
    g_metrics[id].min_duration_ms = g_metrics[id].duration_ms;
  }
  
  if (g_metrics[id].duration_ms > g_metrics[id].max_duration_ms) {
    g_metrics[id].max_duration_ms = g_metrics[id].duration_ms;
  }
  
  return g_metrics[id].duration_ms;
}

void perf_monitor_record(const char* name, uint32_t duration_ms)
{
  perf_metric_id_t id = perf_monitor_register(name);
  if (id < 0) return;
  
  g_metrics[id].duration_ms = duration_ms;
  g_metrics[id].call_count++;
  g_metrics[id].total_duration_ms += duration_ms;
  
  if (duration_ms < g_metrics[id].min_duration_ms) {
    g_metrics[id].min_duration_ms = duration_ms;
  }
  
  if (duration_ms > g_metrics[id].max_duration_ms) {
    g_metrics[id].max_duration_ms = duration_ms;
  }
}

// =============================================================================
// QUERIES
// =============================================================================

const perf_metrics_t* perf_monitor_get(perf_metric_id_t id)
{
  if (id >= g_metric_count) return NULL;
  return &g_metrics[id];
}

const perf_metrics_t* perf_monitor_get_by_name(const char* name)
{
  for (uint8_t i = 0; i < g_metric_count; i++) {
    if (g_metrics[i].name && strcmp(g_metrics[i].name, name) == 0) {
      return &g_metrics[i];
    }
  }
  return NULL;
}

uint32_t perf_monitor_get_average(perf_metric_id_t id)
{
  if (id >= g_metric_count) return 0;
  if (g_metrics[id].call_count == 0) return 0;
  
  return g_metrics[id].total_duration_ms / g_metrics[id].call_count;
}

// =============================================================================
// REPORTING
// =============================================================================

void perf_monitor_report_uart(void)
{
  // Note: This uses printf which should be redirected to UART
  // In production, replace with actual UART output function
  
  printf("\r\n");
  printf("==============================================\r\n");
  printf("       PERFORMANCE METRICS\r\n");
  printf("==============================================\r\n");
  printf("Operation                 Calls    Avg(ms)  Min(ms)  Max(ms)\r\n");
  printf("--------------------------------------------------------------\r\n");
  
  for (uint8_t i = 0; i < g_metric_count; i++) {
    if (g_metrics[i].call_count > 0) {
      uint32_t avg = perf_monitor_get_average(i);
      printf("%-24s %6lu  %7lu  %7lu  %7lu\r\n",
             g_metrics[i].name ? g_metrics[i].name : "Unknown",
             g_metrics[i].call_count,
             avg,
             g_metrics[i].min_duration_ms,
             g_metrics[i].max_duration_ms);
    }
  }
  
  printf("==============================================\r\n");
  printf("\r\n");
}

int perf_monitor_save_csv(const char* filename)
{
  FIL fp;
  
  if (f_open(&fp, filename, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) {
    return -1;
  }
  
  // Write header
  f_printf(&fp, "# MidiCore Performance Metrics\r\n");
  f_printf(&fp, "# Timestamp: %lu ms\r\n\r\n", osKernelGetTickCount());
  f_printf(&fp, "Operation,Calls,Average_ms,Min_ms,Max_ms,Total_ms\r\n");
  
  // Write data
  for (uint8_t i = 0; i < g_metric_count; i++) {
    if (g_metrics[i].call_count > 0) {
      uint32_t avg = perf_monitor_get_average(i);
      f_printf(&fp, "%s,%lu,%lu,%lu,%lu,%lu\r\n",
               g_metrics[i].name ? g_metrics[i].name : "Unknown",
               g_metrics[i].call_count,
               avg,
               g_metrics[i].min_duration_ms,
               g_metrics[i].max_duration_ms,
               g_metrics[i].total_duration_ms);
    }
  }
  
  f_close(&fp);
  return 0;
}

void perf_monitor_reset(void)
{
  for (uint8_t i = 0; i < g_metric_count; i++) {
    g_metrics[i].call_count = 0;
    g_metrics[i].total_duration_ms = 0;
    g_metrics[i].min_duration_ms = 0xFFFFFFFF;
    g_metrics[i].max_duration_ms = 0;
    g_metrics[i].duration_ms = 0;
  }
}

void perf_monitor_reset_metric(perf_metric_id_t id)
{
  if (id >= g_metric_count) return;
  
  g_metrics[id].call_count = 0;
  g_metrics[id].total_duration_ms = 0;
  g_metrics[id].min_duration_ms = 0xFFFFFFFF;
  g_metrics[id].max_duration_ms = 0;
  g_metrics[id].duration_ms = 0;
}
