/**
 * @file test_cli.c
 * @brief CLI commands for test module implementation
 */

#include "Services/test/test_cli.h"
#include "Services/test/test.h"
#include "Services/cli/cli.h"
#include "App/tests/test_debug.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// COMMAND HANDLERS
// =============================================================================

/**
 * @brief List all available tests
 */
static cli_result_t cmd_test_list(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  
  uint32_t count = test_get_count();
  
  // Buffer complete header to avoid fragmentation
  char header_buf[80];
  snprintf(header_buf, sizeof(header_buf), 
           "\r\n=== Available Tests ===\r\n\r\nCount: %lu tests\r\n\r\n",
           (unsigned long)count);
  dbg_print(header_buf);
  
  for (uint32_t i = 0; i < count; i++) {
    const char* name = test_get_name(i);
    const char* desc = test_get_description(name);
    
    // Buffer complete test entry to avoid fragmentation
    char test_buf[200];
    snprintf(test_buf, sizeof(test_buf), "  %lu. %s\r\n     %s\r\n\r\n",
             (unsigned long)(i + 1), name, desc);
    dbg_print(test_buf);
  }
  
  dbg_print("Usage: test run <name>\r\n");
  dbg_print("Example: test run ainser64\r\n\r\n");
  
  return CLI_OK;
}

/**
 * @brief Run a specific test
 */
static cli_result_t cmd_test_run(int argc, char* argv[])
{
  if (argc < 2) {
    dbg_print("ERROR: Test name required\r\n");
    dbg_print("Usage: test run <name>\r\n");
    dbg_print("Example: test run ainser64\r\n");
    dbg_print("Use 'test list' to see available tests\r\n");
    return CLI_INVALID_ARGS;
  }
  
  const char* test_name = argv[1];
  int32_t duration_ms = -1;  // Infinite by default
  
  // Optional duration argument
  if (argc >= 3) {
    // Parse duration if provided
    // For now, we'll run infinitely
  }
  
  dbg_print("\r\n=== Starting Test ===\r\n");
  dbg_print("Test: ");
  dbg_print(test_name);
  dbg_print("\r\n");
  dbg_print("Duration: Infinite (until reset)\r\n");
  dbg_print("\r\nNote: Most tests run in infinite loops.\r\n");
  dbg_print("Reset the device to stop the test.\r\n");
  dbg_print("======================\r\n\r\n");
  
  int result = test_run(test_name, duration_ms);
  
  if (result < 0) {
    dbg_print("\r\nERROR: Test failed to start (code: ");
    dbg_print_int(result);
    dbg_print(")\r\n");
    return CLI_ERROR;
  }
  
  return CLI_OK;
}

/**
 * @brief Stop current test
 */
static cli_result_t cmd_test_stop(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  
  if (!test_is_running()) {
    dbg_print("No test is currently running\r\n");
    return CLI_OK;
  }
  
  dbg_print("Attempting to stop test...\r\n");
  int result = test_stop();
  
  if (result < 0) {
    // Buffer error message to avoid fragmentation
    char err_buf[80];
    snprintf(err_buf, sizeof(err_buf), "ERROR: Could not stop test (code: %d)\r\n", result);
    dbg_print(err_buf);
    return CLI_ERROR;
  }
  
  return CLI_OK;
}

/**
 * @brief Show current test status
 */
static cli_result_t cmd_test_status(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  
  test_result_t result;
  
  if (test_get_status(&result) < 0) {
    dbg_print("ERROR: Could not get test status\r\n");
    return CLI_ERROR;
  }
  
  dbg_print("\r\n=== Test Status ===\r\n\r\n");
  
  if (result.test_name[0] == '\0') {
    dbg_print("Status: No test has been run\r\n");
  } else {
    const char* status_str;
    switch (result.status) {
      case TEST_STATUS_IDLE:    status_str = "IDLE"; break;
      case TEST_STATUS_RUNNING: status_str = "RUNNING"; break;
      case TEST_STATUS_PASSED:  status_str = "PASSED"; break;
      case TEST_STATUS_FAILED:  status_str = "FAILED"; break;
      case TEST_STATUS_ERROR:   status_str = "ERROR"; break;
      default:                  status_str = "UNKNOWN"; break;
    }
    
    // Buffer test info to avoid fragmentation
    char info_buf[150];
    snprintf(info_buf, sizeof(info_buf), "Test: %s\r\nStatus: %s\r\n",
             result.test_name, status_str);
    dbg_print(info_buf);
    
    // Buffer timing info to avoid fragmentation
    if (result.status == TEST_STATUS_RUNNING) {
      char time_buf[60];
      snprintf(time_buf, sizeof(time_buf), "Elapsed: %lu ms\r\n",
               (unsigned long)result.duration_ms);
      dbg_print(time_buf);
    } else if (result.duration_ms > 0) {
      char time_buf[60];
      snprintf(time_buf, sizeof(time_buf), "Duration: %lu ms\r\n",
               (unsigned long)result.duration_ms);
      dbg_print(time_buf);
    }
    
    if (result.error_message[0] != '\0') {
      char err_buf[150];
      snprintf(err_buf, sizeof(err_buf), "Error: %s\r\n", result.error_message);
      dbg_print(err_buf);
    }
  }
  
  dbg_print("===================\r\n\r\n");
  
  return CLI_OK;
}
      dbg_print("\r\n");
    }
  }
  
  dbg_print("\r\n");
  return CLI_OK;
}

/**
 * @brief Show information about a specific test
 */
static cli_result_t cmd_test_info(int argc, char* argv[])
{
  if (argc < 2) {
    dbg_print("ERROR: Test name required\r\n");
    dbg_print("Usage: test info <name>\r\n");
    dbg_print("Use 'test list' to see available tests\r\n");
    return CLI_INVALID_ARGS;
  }
  
  const char* test_name = argv[1];
  const char* desc = test_get_description(test_name);
  
  if (!desc) {
    dbg_print("ERROR: Test not found: ");
    dbg_print(test_name);
    dbg_print("\r\n");
    dbg_print("Use 'test list' to see available tests\r\n");
    return CLI_NOT_FOUND;
  }
  
  dbg_print("\r\n=== Test Information ===\r\n\r\n");
  dbg_print("Name: ");
  dbg_print(test_name);
  dbg_print("\r\n");
  dbg_print("Description: ");
  dbg_print(desc);
  dbg_print("\r\n\r\n");
  dbg_print("Usage: test run ");
  dbg_print(test_name);
  dbg_print("\r\n\r\n");
  
  return CLI_OK;
}

/**
 * @brief Clear test results
 */
static cli_result_t cmd_test_clear(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  
  if (test_clear_results() < 0) {
    dbg_print("ERROR: Could not clear test results\r\n");
    return CLI_ERROR;
  }
  
  dbg_print("Test results cleared\r\n");
  return CLI_OK;
}

/**
 * @brief Main test command dispatcher
 */
static cli_result_t cmd_test(int argc, char* argv[])
{
  if (argc < 2) {
    dbg_print("\r\nTest Module Commands:\r\n");
    dbg_print("  test list              - List all available tests\r\n");
    dbg_print("  test run <name>        - Run a specific test\r\n");
    dbg_print("  test stop              - Stop current test\r\n");
    dbg_print("  test status            - Show current test status\r\n");
    dbg_print("  test info <name>       - Show test information\r\n");
    dbg_print("  test clear             - Clear test results\r\n");
    dbg_print("\r\n");
    dbg_print("Examples:\r\n");
    dbg_print("  test list\r\n");
    dbg_print("  test run ainser64\r\n");
    dbg_print("  test status\r\n");
    dbg_print("\r\n");
    return CLI_OK;
  }
  
  const char* subcmd = argv[1];
  
  if (strcmp(subcmd, "list") == 0) {
    return cmd_test_list(argc - 1, &argv[1]);
  } else if (strcmp(subcmd, "run") == 0) {
    return cmd_test_run(argc - 1, &argv[1]);
  } else if (strcmp(subcmd, "stop") == 0) {
    return cmd_test_stop(argc - 1, &argv[1]);
  } else if (strcmp(subcmd, "status") == 0) {
    return cmd_test_status(argc - 1, &argv[1]);
  } else if (strcmp(subcmd, "info") == 0) {
    return cmd_test_info(argc - 1, &argv[1]);
  } else if (strcmp(subcmd, "clear") == 0) {
    return cmd_test_clear(argc - 1, &argv[1]);
  } else {
    dbg_print("ERROR: Unknown subcommand: ");
    dbg_print(subcmd);
    dbg_print("\r\n");
    dbg_print("Use 'test' to see available commands\r\n");
    return CLI_NOT_FOUND;
  }
}

// =============================================================================
// INITIALIZATION
// =============================================================================

int test_cli_init(void)
{
  cli_command_t cmd = {
    .name = "test",
    .handler = cmd_test,
    .description = "Module testing commands",
    .usage = "test <subcommand> [args]",
    .category = "testing"
  };
  
  return cli_register_command(&cmd);
}
