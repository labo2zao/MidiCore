/**
 * @file stack_monitor.c
 * @brief FreeRTOS Stack Usage Monitor Implementation
 */

#include "stack_monitor.h"
#include "App/tests/test_debug.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PRIVATE STATE
// =============================================================================

static uint8_t s_initialized = 0;
static uint8_t s_running = 0;
static osThreadId_t s_monitor_task_handle = NULL;

// Configuration
static uint32_t s_warning_threshold = STACK_MONITOR_WARNING_THRESHOLD;
static uint32_t s_critical_threshold = STACK_MONITOR_CRITICAL_THRESHOLD;
static uint32_t s_interval_ms = STACK_MONITOR_INTERVAL_MS;

// Statistics
static stack_monitor_stats_t s_stats = {0};

// Alert callback
static stack_alert_callback_t s_alert_callback = NULL;

// Previous status tracking (to avoid duplicate alerts)
typedef struct {
  osThreadId_t handle;
  stack_status_t last_status;
} task_status_cache_t;

static task_status_cache_t s_status_cache[STACK_MONITOR_MAX_TASKS] = {0};
static uint32_t s_cache_count = 0;

// =============================================================================
// FORWARD DECLARATIONS
// =============================================================================

static void stack_monitor_task(void* argument);
static void check_all_tasks(void);
static stack_status_t calculate_status(uint32_t free_percent);
static void issue_alert(const char* task_name, const stack_info_t* info);
static task_status_cache_t* find_cache_entry(osThreadId_t handle);

// =============================================================================
// INITIALIZATION
// =============================================================================

int stack_monitor_init(void)
{
#if !STACK_MONITOR_ENABLED
  dbg_printf("[STACK] Stack monitoring disabled (STACK_MONITOR_ENABLED=0)\r\n");
  return 0;
#endif

  if (s_initialized) {
    return 0;
  }

  dbg_printf("[STACK] Initializing stack monitor...\r\n");

  // Reset state
  memset(&s_stats, 0, sizeof(s_stats));
  memset(s_status_cache, 0, sizeof(s_status_cache));
  s_cache_count = 0;
  s_alert_callback = NULL;

  // Create monitoring task
  const osThreadAttr_t attr = {
    .name = "StackMon",
    .priority = STACK_MONITOR_PRIORITY,
    .stack_size = STACK_MONITOR_STACK_SIZE
  };

  s_monitor_task_handle = osThreadNew(stack_monitor_task, NULL, &attr);
  if (s_monitor_task_handle == NULL) {
    dbg_printf("[STACK] ERROR: Failed to create monitor task\r\n");
    return -1;
  }

  s_initialized = 1;
  s_running = 1;

  dbg_printf("[STACK] Stack monitor initialized (interval=%lums, warn=%lu%%, crit=%lu%%)\r\n",
             (unsigned long)s_interval_ms,
             (unsigned long)s_warning_threshold,
             (unsigned long)s_critical_threshold);

  return 0;
}

int stack_monitor_start(void)
{
  if (!s_initialized) {
    return -1;
  }

  s_running = 1;
  return 0;
}

int stack_monitor_stop(void)
{
  if (!s_initialized) {
    return -1;
  }

  s_running = 0;
  return 0;
}

// =============================================================================
// MONITORING TASK
// =============================================================================

static void stack_monitor_task(void* argument)
{
  (void)argument;

  dbg_printf("[STACK] Monitor task started\r\n");

  // Initial check after short delay
  osDelay(1000);
  check_all_tasks();

  while (1) {
    osDelay(s_interval_ms);

    if (s_running) {
      check_all_tasks();
      s_stats.total_checks++;
      s_stats.last_check_time = osKernelGetTickCount();
    }
  }
}

// =============================================================================
// QUERY API
// =============================================================================

int stack_monitor_get_info(osThreadId_t task_handle, stack_info_t* info)
{
  if (!info) {
    return -1;
  }

  memset(info, 0, sizeof(stack_info_t));

  // Get FreeRTOS task handle (convert from CMSIS-RTOS2 handle)
  TaskHandle_t task = (TaskHandle_t)task_handle;
  if (task == NULL) {
    task = xTaskGetCurrentTaskHandle();
  }

  // Get task name
  const char* name = pcTaskGetName(task);
  strncpy(info->task_name, name, sizeof(info->task_name) - 1);

  // Get high water mark (minimum free stack in words)
  UBaseType_t hwm = uxTaskGetStackHighWaterMark(task);
  info->high_water_mark = hwm;
  info->high_water_mark_bytes = hwm * sizeof(StackType_t);

  // Get stack size using TCB (requires access to task.h internals)
  // For CMSIS-RTOS2, we can use a workaround via thread attributes
  // Since we don't have direct access, we'll estimate from known tasks
  // This is a limitation - ideally we'd get this from TCB directly
  
  // For now, we'll get stack info from the thread enumeration
  osThreadId_t threads[STACK_MONITOR_MAX_TASKS];
  uint32_t thread_count = osThreadEnumerate(threads, STACK_MONITOR_MAX_TASKS);
  
  uint32_t stack_size_bytes = 0;
  for (uint32_t i = 0; i < thread_count; i++) {
    if (threads[i] == task_handle) {
      // Try to get stack space (this is the remaining space)
      uint32_t remaining = osThreadGetStackSpace(threads[i]);
      // Stack size = used + remaining
      // used = stack_size - high_water_mark
      // So stack_size can be approximated
      stack_size_bytes = remaining + (remaining * 100);  // Rough estimate
      break;
    }
  }
  
  // If we couldn't determine stack size, use high water mark as baseline
  if (stack_size_bytes == 0) {
    stack_size_bytes = info->high_water_mark_bytes * 2;  // Conservative estimate
  }

  info->stack_size_bytes = stack_size_bytes;
  info->stack_size = stack_size_bytes / sizeof(StackType_t);
  
  // Calculate usage
  info->used_bytes = stack_size_bytes - info->high_water_mark_bytes;
  info->free_bytes = info->high_water_mark_bytes;
  
  if (stack_size_bytes > 0) {
    info->used_percent = (info->used_bytes * 100) / stack_size_bytes;
    info->free_percent = (info->free_bytes * 100) / stack_size_bytes;
  }

  // Determine status
  info->status = calculate_status(info->free_percent);
  info->timestamp = osKernelGetTickCount();

  return 0;
}

int stack_monitor_get_info_by_name(const char* task_name, stack_info_t* info)
{
  if (!task_name || !info) {
    return -1;
  }

  // Enumerate all threads
  osThreadId_t threads[STACK_MONITOR_MAX_TASKS];
  uint32_t thread_count = osThreadEnumerate(threads, STACK_MONITOR_MAX_TASKS);

  for (uint32_t i = 0; i < thread_count; i++) {
    TaskHandle_t task = (TaskHandle_t)threads[i];
    const char* name = pcTaskGetName(task);
    if (strcasecmp(name, task_name) == 0) {
      return stack_monitor_get_info(threads[i], info);
    }
  }

  return -1;  // Task not found
}

int stack_monitor_get_all_tasks(stack_info_t* info_array, uint32_t max_tasks, uint32_t* num_tasks)
{
  if (!info_array || !num_tasks) {
    return -1;
  }

  // Enumerate all threads
  osThreadId_t threads[STACK_MONITOR_MAX_TASKS];
  uint32_t thread_count = osThreadEnumerate(threads, STACK_MONITOR_MAX_TASKS);

  *num_tasks = (thread_count < max_tasks) ? thread_count : max_tasks;

  for (uint32_t i = 0; i < *num_tasks; i++) {
    stack_monitor_get_info(threads[i], &info_array[i]);
  }

  return 0;
}

int stack_monitor_get_stats(stack_monitor_stats_t* stats)
{
  if (!stats) {
    return -1;
  }

  memcpy(stats, &s_stats, sizeof(stack_monitor_stats_t));
  return 0;
}

// =============================================================================
// CONTROL API
// =============================================================================

void stack_monitor_set_warning_threshold(uint32_t threshold)
{
  if (threshold <= 100) {
    s_warning_threshold = threshold;
  }
}

void stack_monitor_set_critical_threshold(uint32_t threshold)
{
  if (threshold <= 100) {
    s_critical_threshold = threshold;
  }
}

void stack_monitor_set_interval(uint32_t interval_ms)
{
  if (interval_ms >= 100) {
    s_interval_ms = interval_ms;
  }
}

void stack_monitor_check_now(void)
{
  check_all_tasks();
}

// =============================================================================
// REPORTING API
// =============================================================================

void stack_monitor_print_task(osThreadId_t task_handle)
{
  stack_info_t info;
  if (stack_monitor_get_info(task_handle, &info) == 0) {
    const char* status_str = "OK";
    if (info.status == STACK_STATUS_WARNING) status_str = "WARN";
    else if (info.status == STACK_STATUS_CRITICAL) status_str = "CRIT";
    else if (info.status == STACK_STATUS_OVERFLOW) status_str = "OVFL";

    dbg_printf("%-15s: %5lu/%5lu bytes (%3lu%% used, %3lu%% free) [%s]\r\n",
               info.task_name,
               (unsigned long)info.used_bytes,
               (unsigned long)info.stack_size_bytes,
               (unsigned long)info.used_percent,
               (unsigned long)info.free_percent,
               status_str);
  }
}

void stack_monitor_print_all(uint8_t verbose)
{
  stack_info_t tasks[STACK_MONITOR_MAX_TASKS];
  uint32_t count = 0;

  if (stack_monitor_get_all_tasks(tasks, STACK_MONITOR_MAX_TASKS, &count) != 0) {
    dbg_printf("[STACK] Error getting task list\r\n");
    return;
  }

  dbg_printf("\r\n=== Stack Usage Report (%lu tasks) ===\r\n", (unsigned long)count);
  dbg_printf("%-15s %12s %12s %8s %8s %6s\r\n",
             "Task", "Used", "Total", "Used%", "Free%", "Status");
  dbg_printf("--------------- ------------ ------------ -------- -------- ------\r\n");

  for (uint32_t i = 0; i < count; i++) {
    const stack_info_t* info = &tasks[i];
    const char* status_str = "OK";
    if (info->status == STACK_STATUS_WARNING) status_str = "WARN";
    else if (info->status == STACK_STATUS_CRITICAL) status_str = "CRIT";
    else if (info->status == STACK_STATUS_OVERFLOW) status_str = "OVFL";

    dbg_printf("%-15s %8lu B %8lu B %7lu%% %7lu%% %-6s\r\n",
               info->task_name,
               (unsigned long)info->used_bytes,
               (unsigned long)info->stack_size_bytes,
               (unsigned long)info->used_percent,
               (unsigned long)info->free_percent,
               status_str);

    if (verbose) {
      dbg_printf("  High-water mark: %lu bytes (%lu words)\r\n",
                 (unsigned long)info->high_water_mark_bytes,
                 (unsigned long)info->high_water_mark);
    }
  }

  dbg_printf("\r\n");
}

void stack_monitor_print_stats(void)
{
  dbg_printf("\r\n=== Stack Monitor Statistics ===\r\n");
  dbg_printf("Total checks:    %lu\r\n", (unsigned long)s_stats.total_checks);
  dbg_printf("Warnings:        %lu\r\n", (unsigned long)s_stats.warning_count);
  dbg_printf("Critical alerts: %lu\r\n", (unsigned long)s_stats.critical_count);
  dbg_printf("Overflows:       %lu\r\n", (unsigned long)s_stats.overflow_count);
  dbg_printf("Last check:      %lu ms\r\n", (unsigned long)s_stats.last_check_time);
  dbg_printf("Interval:        %lu ms\r\n", (unsigned long)s_interval_ms);
  dbg_printf("Warn threshold:  %lu%%\r\n", (unsigned long)s_warning_threshold);
  dbg_printf("Crit threshold:  %lu%%\r\n\r\n", (unsigned long)s_critical_threshold);
}

void stack_monitor_export_csv(void)
{
  stack_info_t tasks[STACK_MONITOR_MAX_TASKS];
  uint32_t count = 0;

  if (stack_monitor_get_all_tasks(tasks, STACK_MONITOR_MAX_TASKS, &count) != 0) {
    return;
  }

  dbg_printf("task_name,used_bytes,total_bytes,used_pct,free_pct,hwm_bytes,status\r\n");
  for (uint32_t i = 0; i < count; i++) {
    const stack_info_t* info = &tasks[i];
    dbg_printf("%s,%lu,%lu,%lu,%lu,%lu,%d\r\n",
               info->task_name,
               (unsigned long)info->used_bytes,
               (unsigned long)info->stack_size_bytes,
               (unsigned long)info->used_percent,
               (unsigned long)info->free_percent,
               (unsigned long)info->high_water_mark_bytes,
               (int)info->status);
  }
}

// =============================================================================
// CALLBACK API
// =============================================================================

void stack_monitor_register_callback(stack_alert_callback_t callback)
{
  s_alert_callback = callback;
}

// =============================================================================
// PRIVATE HELPERS
// =============================================================================

static void check_all_tasks(void)
{
  stack_info_t tasks[STACK_MONITOR_MAX_TASKS];
  uint32_t count = 0;

  if (stack_monitor_get_all_tasks(tasks, STACK_MONITOR_MAX_TASKS, &count) != 0) {
    return;
  }

  for (uint32_t i = 0; i < count; i++) {
    const stack_info_t* info = &tasks[i];
    
    // Check if status changed from previous check
    task_status_cache_t* cache = find_cache_entry((osThreadId_t)info);
    stack_status_t prev_status = STACK_STATUS_OK;
    
    if (cache) {
      prev_status = cache->last_status;
      cache->last_status = info->status;
    } else if (s_cache_count < STACK_MONITOR_MAX_TASKS) {
      // Add new cache entry
      s_status_cache[s_cache_count].handle = (osThreadId_t)info;
      s_status_cache[s_cache_count].last_status = info->status;
      s_cache_count++;
    }

    // Issue alert if status worsened
    if (info->status > prev_status) {
      issue_alert(info->task_name, info);
    }
  }
}

static stack_status_t calculate_status(uint32_t free_percent)
{
  if (free_percent <= s_critical_threshold) {
    return STACK_STATUS_CRITICAL;
  } else if (free_percent <= s_warning_threshold) {
    return STACK_STATUS_WARNING;
  }
  return STACK_STATUS_OK;
}

static void issue_alert(const char* task_name, const stack_info_t* info)
{
  // Update statistics
  if (info->status == STACK_STATUS_WARNING) {
    s_stats.warning_count++;
    dbg_printf("[STACK] WARNING: Task '%s' stack usage high: %lu%% used (%lu/%lu bytes)\r\n",
               task_name,
               (unsigned long)info->used_percent,
               (unsigned long)info->used_bytes,
               (unsigned long)info->stack_size_bytes);
  } else if (info->status == STACK_STATUS_CRITICAL) {
    s_stats.critical_count++;
    dbg_printf("[STACK] CRITICAL: Task '%s' stack nearly full: %lu%% used (%lu/%lu bytes)!\r\n",
               task_name,
               (unsigned long)info->used_percent,
               (unsigned long)info->used_bytes,
               (unsigned long)info->stack_size_bytes);
  } else if (info->status == STACK_STATUS_OVERFLOW) {
    s_stats.overflow_count++;
    dbg_printf("[STACK] OVERFLOW: Task '%s' stack corrupted!\r\n", task_name);
  }

  // Call user callback if registered
  if (s_alert_callback) {
    s_alert_callback(task_name, info, info->status);
  }
}

static task_status_cache_t* find_cache_entry(osThreadId_t handle)
{
  for (uint32_t i = 0; i < s_cache_count; i++) {
    if (s_status_cache[i].handle == handle) {
      return &s_status_cache[i];
    }
  }
  return NULL;
}
