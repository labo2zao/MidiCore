/**
 * @file test_cli.h
 * @brief CLI commands for test module
 * 
 * Provides UART terminal commands for running and managing module tests.
 * Commands are compatible with MIOS Studio-style terminal interaction.
 * 
 * Note: This entire module is excluded from production builds.
 *       Set MODULE_ENABLE_TEST=1 in module_config.h to enable.
 */

#pragma once

#include <stdint.h>
#include "Config/module_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#if MODULE_ENABLE_TEST

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

#else  // !MODULE_ENABLE_TEST

// Provide stub when test module is disabled
static inline int test_cli_init(void) { return 0; }

#endif  // MODULE_ENABLE_TEST

#ifdef __cplusplus
}
#endif
