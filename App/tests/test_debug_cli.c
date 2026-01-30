/**
 * @file test_debug_cli.c
 * @brief CLI commands for debug system control and inspection
 * 
 * Provides CLI commands to control and inspect the debug output system.
 * Allows runtime configuration and testing of debug output.
 * 
 * Features:
 * - Show current debug configuration
 * - Test debug output on all channels
 * - Inspect UART port details
 * - Change debug mode at runtime (if supported)
 * 
 * @author MidiCore
 * @date 2026-01-30
 */

#include "test_debug.h"
#include "test_debug_cli.h"
#include "cli.h"
#include "module_config.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// HELPER FUNCTIONS - Get Human-Readable Names
// =============================================================================

/**
 * @brief Get human-readable UART port name
 * @param port Port number (0-3)
 * @return Port name string (e.g., "USART2", "UART5")
 */
static inline const char* debug_get_port_name(uint8_t port) {
  switch (port) {
    case 0: return "USART2";
    case 1: return "USART3";
    case 2: return "USART1";
    case 3: return "UART5";
    default: return "UNKNOWN";
  }
}

/**
 * @brief Get pin information for UART port
 * @param port Port number (0-3)
 * @return Pin string (e.g., "PA2/PA3", "PC12/PD2")
 */
static inline const char* debug_get_port_pins(uint8_t port) {
  switch (port) {
    case 0: return "PA2/PA3";
    case 1: return "PD8/PD9";
    case 2: return "PA9/PA10";
    case 3: return "PC12/PD2";
    default: return "UNKNOWN";
  }
}

/**
 * @brief Get human-readable output mode name
 * @return Output mode string
 */
static inline const char* debug_get_output_mode_name(void) {
#if MODULE_DEBUG_OUTPUT == DEBUG_OUTPUT_SWV
  return "SWV/ITM (ST-Link)";
#elif MODULE_DEBUG_OUTPUT == DEBUG_OUTPUT_USB_CDC
  return "USB CDC (Virtual COM)";
#elif MODULE_DEBUG_OUTPUT == DEBUG_OUTPUT_UART
  return "UART (Hardware)";
#elif MODULE_DEBUG_OUTPUT == DEBUG_OUTPUT_NONE
  return "None (Disabled)";
#else
  return "Unknown";
#endif
}

// =============================================================================
// CLI COMMAND HANDLERS
// =============================================================================

/**
 * @brief Show debug configuration
 * @param argc Argument count
 * @param argv Argument array
 * @return CLI_OK on success
 */
static cli_result_t cmd_debug(int argc, char* argv[]) {
  if (argc == 1) {
    // Show main configuration
    dbg_print("==============================================\r\n");
    dbg_print("Debug Output Configuration:\r\n");
    dbg_print("==============================================\r\n");
    
    dbg_printf("  Output Mode: %s\r\n", debug_get_output_mode_name());
    
#if MODULE_DEBUG_OUTPUT == DEBUG_OUTPUT_UART
    dbg_printf("  UART Port:   %s (port %d)\r\n", 
               debug_get_port_name(TEST_DEBUG_UART_PORT),
               TEST_DEBUG_UART_PORT);
    dbg_printf("  Pins:        %s\r\n", 
               debug_get_port_pins(TEST_DEBUG_UART_PORT));
    dbg_printf("  Baud Rate:   %d\r\n", TEST_DEBUG_UART_BAUD);
#endif

    // Show global diagnostic variables
    extern volatile uint32_t g_debug_uart_port;
    extern volatile void* g_debug_uart_instance;
    extern volatile uint32_t g_debug_uart_baud_before;
    extern volatile uint32_t g_debug_uart_baud_after;
    
    dbg_print("\r\nGDB Diagnostic Variables:\r\n");
    dbg_printf("  g_debug_uart_port:        %lu\r\n", (unsigned long)g_debug_uart_port);
    dbg_printf("  g_debug_uart_instance:    0x%08lX\r\n", (unsigned long)g_debug_uart_instance);
    dbg_printf("  g_debug_uart_baud_before: %lu\r\n", (unsigned long)g_debug_uart_baud_before);
    dbg_printf("  g_debug_uart_baud_after:  %lu\r\n", (unsigned long)g_debug_uart_baud_after);
    
    dbg_print("==============================================\r\n");
    
  } else if (argc == 2 && strcmp(argv[1], "port") == 0) {
    // Show detailed port information
    dbg_print("==============================================\r\n");
    dbg_print("UART Port Details:\r\n");
    dbg_print("==============================================\r\n");
    
#if MODULE_DEBUG_OUTPUT == DEBUG_OUTPUT_UART
    extern volatile void* g_debug_uart_instance;
    extern volatile uint32_t g_debug_uart_baud_before;
    extern volatile uint32_t g_debug_uart_baud_after;
    
    dbg_printf("  Port:         %s (port %d)\r\n", 
               debug_get_port_name(TEST_DEBUG_UART_PORT),
               TEST_DEBUG_UART_PORT);
    dbg_printf("  Instance:     0x%08lX\r\n", (unsigned long)g_debug_uart_instance);
    dbg_printf("  Pins:         %s\r\n", debug_get_port_pins(TEST_DEBUG_UART_PORT));
    dbg_printf("  Baud Before:  %lu\r\n", (unsigned long)g_debug_uart_baud_before);
    dbg_printf("  Baud After:   %lu\r\n", (unsigned long)g_debug_uart_baud_after);
#else
    dbg_print("  UART mode not active\r\n");
#endif
    
    dbg_print("==============================================\r\n");
    
  } else if (argc == 2 && strcmp(argv[1], "test") == 0) {
    // Test debug output
    dbg_print("==============================================\r\n");
    dbg_print("Testing Debug Output...\r\n");
    dbg_print("==============================================\r\n");
    
    dbg_print("[TEST] ASCII: abcdefghijklmnopqrstuvwxyz\r\n");
    dbg_print("[TEST] DIGITS: 0123456789\r\n");
    dbg_print("[TEST] SYMBOLS: !@#$%^&*()-_=+[]{};:'\"<>,.?/\r\n");
    dbg_printf("[TEST] Formatted: int=%d, hex=0x%04X, str=%s\r\n", 42, 0xDEAD, "Hello");
    
    // Test character-by-character
    dbg_print("[TEST] Character-by-character: ");
    for (char c = 'A'; c <= 'Z'; c++) {
      dbg_putc(c);
    }
    dbg_print("\r\n");
    
    dbg_print("==============================================\r\n");
    dbg_print("Test complete!\r\n");
    dbg_print("==============================================\r\n");
    
  } else {
    dbg_print("Usage: debug [port|test]\r\n");
    dbg_print("  debug      - Show debug configuration\r\n");
    dbg_print("  debug port - Show UART port details\r\n");
    dbg_print("  debug test - Test debug output\r\n");
    return CLI_INVALID_ARGS;
  }
  
  return CLI_OK;
}

// =============================================================================
// GDB CONVENIENCE FUNCTIONS
// =============================================================================

/**
 * @brief Show debug configuration from GDB
 * 
 * Usage in GDB:
 *   (gdb) call gdb_show_debug_config()
 * 
 * This function can be called from GDB to display the current
 * debug configuration without needing to inspect individual variables.
 */
void gdb_show_debug_config(void) {
  extern volatile uint32_t g_debug_uart_port;
  extern volatile void* g_debug_uart_instance;
  extern volatile uint32_t g_debug_uart_baud_before;
  extern volatile uint32_t g_debug_uart_baud_after;
  
  dbg_print("\r\n=== GDB Debug Configuration ===\r\n");
  dbg_printf("Output Mode:   %s\r\n", debug_get_output_mode_name());
  dbg_printf("UART Port:     %lu (%s)\r\n", 
             (unsigned long)g_debug_uart_port,
             debug_get_port_name(g_debug_uart_port));
  dbg_printf("UART Instance: 0x%08lX\r\n", (unsigned long)g_debug_uart_instance);
  dbg_printf("Baud Before:   %lu\r\n", (unsigned long)g_debug_uart_baud_before);
  dbg_printf("Baud After:    %lu\r\n", (unsigned long)g_debug_uart_baud_after);
  dbg_print("===============================\r\n");
}

/**
 * @brief Test debug output from GDB
 * 
 * Usage in GDB:
 *   (gdb) call gdb_test_output()
 * 
 * Sends a test message to the configured debug output.
 */
void gdb_test_output(void) {
  dbg_print("\r\n[GDB TEST] Debug output working!\r\n");
  dbg_printf("[GDB TEST] Timestamp: %lu\r\n", (unsigned long)HAL_GetTick());
}

// =============================================================================
// INITIALIZATION
// =============================================================================

/**
 * @brief Register CLI debug commands
 * @return 0 on success, negative on error
 */
int test_debug_cli_register(void) {
  return cli_register_command(
    "debug",
    cmd_debug,
    "Show/test debug configuration",
    "debug [port|test]",
    "debug"
  );
}
