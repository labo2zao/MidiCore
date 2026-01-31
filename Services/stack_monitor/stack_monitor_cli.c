/**
 * @file stack_monitor_cli.c
 * @brief CLI commands for stack monitor
 */

#include "Services/cli/cli.h"
#include "Services/stack_monitor/stack_monitor.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// =============================================================================
// CLI COMMANDS
// =============================================================================

/**
 * @brief stack command - Show stack usage for current task or specified task
 * Usage: stack [task_name]
 */
static cli_result_t cmd_stack(int argc, char* argv[])
{
  if (argc == 1) {
    // Show current task
    stack_monitor_print_task(NULL);
  } else if (argc == 2) {
    // Show specific task
    stack_info_t info;
    if (stack_monitor_get_info_by_name(argv[1], &info) == 0) {
      const char* status_str = "OK";
      if (info.status == STACK_STATUS_WARNING) status_str = "WARNING";
      else if (info.status == STACK_STATUS_CRITICAL) status_str = "CRITICAL";
      else if (info.status == STACK_STATUS_OVERFLOW) status_str = "OVERFLOW";

      cli_printf("\r\nTask: %s\r\n", info.task_name);
      cli_printf("  Stack size:    %lu bytes (%lu words)\r\n",
                 (unsigned long)info.stack_size_bytes,
                 (unsigned long)info.stack_size);
      cli_printf("  Used:          %lu bytes (%lu%%)\r\n",
                 (unsigned long)info.used_bytes,
                 (unsigned long)info.used_percent);
      cli_printf("  Free:          %lu bytes (%lu%%)\r\n",
                 (unsigned long)info.free_bytes,
                 (unsigned long)info.free_percent);
      cli_printf("  High-water:    %lu bytes\r\n",
                 (unsigned long)info.high_water_mark_bytes);
      cli_printf("  Status:        %s\r\n\r\n", status_str);
    } else {
      cli_error("Task '%s' not found\r\n", argv[1]);
      return CLI_ERROR;
    }
  } else {
    cli_error("Usage: stack [task_name]\r\n");
    return CLI_INVALID_ARGS;
  }

  return CLI_OK;
}

/**
 * @brief stack_all command - Show stack usage for all tasks
 * Usage: stack_all [-v]
 */
static cli_result_t cmd_stack_all(int argc, char* argv[])
{
  uint8_t verbose = 0;

  if (argc == 2 && (strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--verbose") == 0)) {
    verbose = 1;
  }

  stack_monitor_print_all(verbose);
  return CLI_OK;
}

/**
 * @brief stack_monitor command - Control stack monitor
 * Usage: stack_monitor <start|stop|stats|config|check|export>
 */
static cli_result_t cmd_stack_monitor(int argc, char* argv[])
{
  if (argc < 2) {
    cli_error("Usage: stack_monitor <start|stop|stats|config|check|export>\r\n");
    return CLI_INVALID_ARGS;
  }

  const char* subcmd = argv[1];

  if (strcmp(subcmd, "start") == 0) {
    if (stack_monitor_start() == 0) {
      cli_success("Stack monitoring started\r\n");
    } else {
      cli_error("Failed to start stack monitoring\r\n");
      return CLI_ERROR;
    }
  }
  else if (strcmp(subcmd, "stop") == 0) {
    if (stack_monitor_stop() == 0) {
      cli_success("Stack monitoring stopped\r\n");
    } else {
      cli_error("Failed to stop stack monitoring\r\n");
      return CLI_ERROR;
    }
  }
  else if (strcmp(subcmd, "stats") == 0) {
    stack_monitor_print_stats();
  }
  else if (strcmp(subcmd, "check") == 0) {
    cli_printf("Forcing immediate stack check...\r\n");
    stack_monitor_check_now();
    cli_success("Stack check completed\r\n");
  }
  else if (strcmp(subcmd, "export") == 0) {
    cli_printf("Exporting stack data as CSV...\r\n\r\n");
    stack_monitor_export_csv();
  }
  else if (strcmp(subcmd, "config") == 0) {
    if (argc == 2) {
      // Show current configuration
      stack_monitor_stats_t stats;
      stack_monitor_get_stats(&stats);
      cli_printf("\r\nStack Monitor Configuration:\r\n");
      cli_printf("  Status:           %s\r\n", "Running");  // TODO: track running state
      cli_printf("  Interval:         %lu ms\r\n", (unsigned long)STACK_MONITOR_INTERVAL_MS);
      cli_printf("  Warning threshold: %lu%%\r\n", (unsigned long)STACK_MONITOR_WARNING_THRESHOLD);
      cli_printf("  Critical threshold: %lu%%\r\n\r\n", (unsigned long)STACK_MONITOR_CRITICAL_THRESHOLD);
    } else if (argc == 4) {
      // Set configuration: stack_monitor config <param> <value>
      const char* param = argv[2];
      uint32_t value = (uint32_t)atoi(argv[3]);

      if (strcmp(param, "interval") == 0) {
        stack_monitor_set_interval(value);
        cli_success("Monitor interval set to %lu ms\r\n", (unsigned long)value);
      } else if (strcmp(param, "warning") == 0) {
        stack_monitor_set_warning_threshold(value);
        cli_success("Warning threshold set to %lu%%\r\n", (unsigned long)value);
      } else if (strcmp(param, "critical") == 0) {
        stack_monitor_set_critical_threshold(value);
        cli_success("Critical threshold set to %lu%%\r\n", (unsigned long)value);
      } else {
        cli_error("Unknown parameter '%s'. Valid: interval, warning, critical\r\n", param);
        return CLI_INVALID_ARGS;
      }
    } else {
      cli_error("Usage: stack_monitor config [<param> <value>]\r\n");
      cli_error("  Parameters: interval (ms), warning (%%),  critical (%%)\r\n");
      return CLI_INVALID_ARGS;
    }
  }
  else {
    cli_error("Unknown subcommand '%s'\r\n", subcmd);
    cli_error("Valid: start, stop, stats, config, check, export\r\n");
    return CLI_INVALID_ARGS;
  }

  return CLI_OK;
}

/**
 * @brief stack_free command - Show free stack space for all tasks (quick view)
 * Usage: stack_free
 */
static cli_result_t cmd_stack_free(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  
  stack_info_t tasks[STACK_MONITOR_MAX_TASKS];
  uint32_t count = 0;
  
  if (stack_monitor_get_all_tasks(tasks, STACK_MONITOR_MAX_TASKS, &count) != 0) {
    cli_error("Failed to get task list\r\n");
    return CLI_ERROR;
  }
  
  cli_printf("\r\nTask Stack Free Space:\r\n");
  for (uint32_t i = 0; i < count; i++) {
    cli_printf("  %-15s: %5lu / %5lu bytes (%lu%% free)\r\n",
               tasks[i].task_name,
               (unsigned long)tasks[i].free_bytes,
               (unsigned long)tasks[i].stack_size_bytes,
               (unsigned long)tasks[i].free_percent);
  }
  cli_printf("\r\n");
  
  return CLI_OK;
}

// =============================================================================
// REGISTRATION
// =============================================================================

/**
 * @brief Initialize stack monitor CLI commands
 */
int stack_monitor_cli_init(void)
{
  cli_register_command("stack", cmd_stack,
                       "Show stack usage for task",
                       "stack [task_name]",
                       "system");

  cli_register_command("stack_all", cmd_stack_all,
                       "Show stack usage for all tasks",
                       "stack_all [-v]",
                       "system");

  cli_register_command("stack_monitor", cmd_stack_monitor,
                       "Control stack monitor",
                       "stack_monitor <start|stop|stats|config|check|export>",
                       "system");

  cli_register_command("stack_free", cmd_stack_free,
                       "Show free stack space (quick view)",
                       "stack_free",
                       "system");

  return 0;
}
