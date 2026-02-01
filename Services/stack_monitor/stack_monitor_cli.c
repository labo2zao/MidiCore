/**
 * @file stack_monitor_cli.c
 * @brief CLI commands for stack monitor
 * 
 * MIOS32-STYLE: NO printf / snprintf / vsnprintf
 * Uses fixed-string output only: cli_puts, cli_print_u32, etc.
 */

#include "Services/cli/cli.h"
#include "Services/stack_monitor/stack_monitor.h"
#include <string.h>
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

      cli_newline();
      cli_puts("Task: "); cli_puts(info.task_name); cli_newline();
      cli_puts("  Stack size:    "); cli_print_u32(info.stack_size_bytes); cli_puts(" bytes (");
      cli_print_u32(info.stack_size); cli_puts(" words)"); cli_newline();
      cli_puts("  Used:          "); cli_print_u32(info.used_bytes); cli_puts(" bytes (");
      cli_print_u32(info.used_percent); cli_puts("%)"); cli_newline();
      cli_puts("  Free:          "); cli_print_u32(info.free_bytes); cli_puts(" bytes (");
      cli_print_u32(info.free_percent); cli_puts("%)"); cli_newline();
      cli_puts("  High-water:    "); cli_print_u32(info.high_water_mark_bytes); cli_puts(" bytes"); cli_newline();
      cli_puts("  Status:        "); cli_puts(status_str); cli_newline();
      cli_newline();
    } else {
      cli_error("Task not found");
      return CLI_ERROR;
    }
  } else {
    cli_error("Usage: stack [task_name]");
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
    cli_error("Usage: stack_monitor <start|stop|stats|config|check|export>");
    return CLI_INVALID_ARGS;
  }

  const char* subcmd = argv[1];

  if (strcmp(subcmd, "start") == 0) {
    if (stack_monitor_start() == 0) {
      cli_success("Stack monitoring started");
    } else {
      cli_error("Failed to start stack monitoring");
      return CLI_ERROR;
    }
  }
  else if (strcmp(subcmd, "stop") == 0) {
    if (stack_monitor_stop() == 0) {
      cli_success("Stack monitoring stopped");
    } else {
      cli_error("Failed to stop stack monitoring");
      return CLI_ERROR;
    }
  }
  else if (strcmp(subcmd, "stats") == 0) {
    stack_monitor_print_stats();
  }
  else if (strcmp(subcmd, "check") == 0) {
    cli_puts("Forcing immediate stack check..."); cli_newline();
    stack_monitor_check_now();
    cli_success("Stack check completed");
  }
  else if (strcmp(subcmd, "export") == 0) {
    cli_puts("Exporting stack data as CSV..."); cli_newline();
    cli_newline();
    stack_monitor_export_csv();
  }
  else if (strcmp(subcmd, "config") == 0) {
    if (argc == 2) {
      // Show current configuration
      stack_monitor_stats_t stats;
      stack_monitor_get_stats(&stats);
      cli_newline();
      cli_puts("Stack Monitor Configuration:"); cli_newline();
      cli_puts("  Status:            Running"); cli_newline();
      cli_puts("  Interval:          "); cli_print_u32(STACK_MONITOR_INTERVAL_MS); cli_puts(" ms"); cli_newline();
      cli_puts("  Warning threshold: "); cli_print_u32(STACK_MONITOR_WARNING_THRESHOLD); cli_puts("%"); cli_newline();
      cli_puts("  Critical threshold: "); cli_print_u32(STACK_MONITOR_CRITICAL_THRESHOLD); cli_puts("%"); cli_newline();
      cli_newline();
    } else if (argc == 4) {
      // Set configuration: stack_monitor config <param> <value>
      const char* param = argv[2];
      uint32_t value = (uint32_t)atoi(argv[3]);

      if (strcmp(param, "interval") == 0) {
        stack_monitor_set_interval(value);
        cli_puts("Monitor interval set to "); cli_print_u32(value); cli_puts(" ms"); cli_newline();
      } else if (strcmp(param, "warning") == 0) {
        stack_monitor_set_warning_threshold(value);
        cli_puts("Warning threshold set to "); cli_print_u32(value); cli_puts("%"); cli_newline();
      } else if (strcmp(param, "critical") == 0) {
        stack_monitor_set_critical_threshold(value);
        cli_puts("Critical threshold set to "); cli_print_u32(value); cli_puts("%"); cli_newline();
      } else {
        cli_error("Unknown param. Valid: interval, warning, critical");
        return CLI_INVALID_ARGS;
      }
    } else {
      cli_error("Usage: stack_monitor config [<param> <value>]");
      return CLI_INVALID_ARGS;
    }
  }
  else {
    cli_error("Unknown subcommand. Valid: start, stop, stats, config, check, export");
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
    cli_error("Failed to get task list");
    return CLI_ERROR;
  }
  
  cli_newline();
  cli_puts("Task Stack Free Space:"); cli_newline();
  for (uint32_t i = 0; i < count; i++) {
    cli_puts("  ");
    cli_puts(tasks[i].task_name);
    cli_puts(": ");
    cli_print_u32(tasks[i].free_bytes);
    cli_puts(" / ");
    cli_print_u32(tasks[i].stack_size_bytes);
    cli_puts(" bytes (");
    cli_print_u32(tasks[i].free_percent);
    cli_puts("% free)");
    cli_newline();
  }
  cli_newline();
  
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
