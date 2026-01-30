/**
 * @file test_debug_cli.h
 * @brief CLI commands for debug system control and inspection
 * 
 * Provides CLI commands to control and inspect the debug output system.
 * 
 * Commands:
 * - debug      - Show current debug configuration
 * - debug port - Show detailed UART port information
 * - debug test - Test debug output on all channels
 * 
 * GDB Functions:
 * - gdb_show_debug_config() - Display debug config from GDB
 * - gdb_test_output()       - Test output from GDB
 * 
 * @author MidiCore
 * @date 2026-01-30
 */

#ifndef TEST_DEBUG_CLI_H
#define TEST_DEBUG_CLI_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register CLI debug commands
 * 
 * Call this during initialization to register the 'debug' command
 * with the CLI system.
 * 
 * @return 0 on success, negative on error
 */
int test_debug_cli_register(void);

/**
 * @brief Show debug configuration from GDB
 * 
 * Usage in GDB:
 *   (gdb) break app_init_and_start
 *   (gdb) continue
 *   (gdb) call gdb_show_debug_config()
 * 
 * Displays:
 * - Output mode (SWV/CDC/UART/None)
 * - UART port and pins
 * - UART instance address
 * - Baud rates before/after init
 */
void gdb_show_debug_config(void);

/**
 * @brief Test debug output from GDB
 * 
 * Usage in GDB:
 *   (gdb) call gdb_test_output()
 * 
 * Sends a test message to the configured debug output channel.
 * Useful for verifying debug output is working.
 */
void gdb_test_output(void);

#ifdef __cplusplus
}
#endif

#endif // TEST_DEBUG_CLI_H
