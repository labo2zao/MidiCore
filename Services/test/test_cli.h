/**
 * @file test_cli.h
 * @brief CLI commands for test module
 * 
 * Provides UART terminal commands for running and managing module tests.
 * Commands are compatible with MIOS Studio-style terminal interaction.
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register test CLI commands
 * 
 * Registers the following commands:
 * - test list                 - List all available tests
 * - test run <name>           - Run a specific test
 * - test stop                 - Stop current test
 * - test status               - Show current test status
 * - test info <name>          - Show test information
 * - test clear                - Clear test results
 * 
 * @return 0 on success, negative on error
 */
int test_cli_init(void);

#ifdef __cplusplus
}
#endif
