/**
 * @file test_debug_cli.c
 * @brief CLI commands for debug system control and inspection
 * 
 * MIOS32 PRINCIPLES:
 * - NO printf / snprintf / vsnprintf (causes stack overflow!)
 * - Fixed string outputs only
 * - Use dbg_print() + dbg_print_u32() instead of dbg_printf()
 * 
 * @author MidiCore
 * @date 2026-01-30
 */

#include "test_debug.h"
#include "test_debug_cli.h"
#include "Services/cli/cli.h"
#include "Config/module_config.h"
#include <string.h>

/* NO stdio.h - we don't use printf! */

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
    
    /* MIOS32-STYLE: Fixed strings + dbg_print_u32 */
    dbg_print("  Output Mode: ");
    dbg_print(debug_get_output_mode_name());
    dbg_print("\r\n");
    
#if MODULE_DEBUG_OUTPUT == DEBUG_OUTPUT_UART
    dbg_print("  UART Port:   ");
    dbg_print(debug_get_port_name(TEST_DEBUG_UART_PORT));
    dbg_print(" (port ");
    dbg_print_u32(TEST_DEBUG_UART_PORT);
    dbg_print(")\r\n");
    dbg_print("  Pins:        ");
    dbg_print(debug_get_port_pins(TEST_DEBUG_UART_PORT));
    dbg_print("\r\n");
    dbg_print("  Baud Rate:   ");
    dbg_print_u32(TEST_DEBUG_UART_BAUD);
    dbg_print("\r\n");
#endif

    // Show global diagnostic variables
    extern volatile uint32_t g_debug_uart_port;
    extern volatile void* g_debug_uart_instance;
    extern volatile uint32_t g_debug_uart_baud_before;
    extern volatile uint32_t g_debug_uart_baud_after;
    
    dbg_print("\r\nGDB Diagnostic Variables:\r\n");
    dbg_print("  g_debug_uart_port:        ");
    dbg_print_u32((uint32_t)g_debug_uart_port);
    dbg_print("\r\n");
    dbg_print("  g_debug_uart_instance:    0x");
    dbg_print_hex32((uint32_t)(uintptr_t)g_debug_uart_instance);
    dbg_print("\r\n");
    dbg_print("  g_debug_uart_baud_before: ");
    dbg_print_u32((uint32_t)g_debug_uart_baud_before);
    dbg_print("\r\n");
    dbg_print("  g_debug_uart_baud_after:  ");
    dbg_print_u32((uint32_t)g_debug_uart_baud_after);
    dbg_print("\r\n");
    
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
    
    dbg_print("  Port:         ");
    dbg_print(debug_get_port_name(TEST_DEBUG_UART_PORT));
    dbg_print(" (port ");
    dbg_print_u32(TEST_DEBUG_UART_PORT);
    dbg_print(")\r\n");
    dbg_print("  Instance:     0x");
    dbg_print_hex32((uint32_t)(uintptr_t)g_debug_uart_instance);
    dbg_print("\r\n");
    dbg_print("  Pins:         ");
    dbg_print(debug_get_port_pins(TEST_DEBUG_UART_PORT));
    dbg_print("\r\n");
    dbg_print("  Baud Before:  ");
    dbg_print_u32((uint32_t)g_debug_uart_baud_before);
    dbg_print("\r\n");
    dbg_print("  Baud After:   ");
    dbg_print_u32((uint32_t)g_debug_uart_baud_after);
    dbg_print("\r\n");
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
    dbg_print("[TEST] Formatted: int=");
    dbg_print_u32(42);
    dbg_print(", hex=0x");
    dbg_print_hex16(0xDEAD);
    dbg_print(", str=Hello\r\n");
    
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
 */
void gdb_show_debug_config(void) {
  extern volatile uint32_t g_debug_uart_port;
  extern volatile void* g_debug_uart_instance;
  extern volatile uint32_t g_debug_uart_baud_before;
  extern volatile uint32_t g_debug_uart_baud_after;
  
  dbg_print("\r\n=== GDB Debug Configuration ===\r\n");
  dbg_print("Output Mode:   ");
  dbg_print(debug_get_output_mode_name());
  dbg_print("\r\n");
  dbg_print("UART Port:     ");
  dbg_print_u32((uint32_t)g_debug_uart_port);
  dbg_print(" (");
  dbg_print(debug_get_port_name((uint8_t)g_debug_uart_port));
  dbg_print(")\r\n");
  dbg_print("UART Instance: 0x");
  dbg_print_hex32((uint32_t)(uintptr_t)g_debug_uart_instance);
  dbg_print("\r\n");
  dbg_print("Baud Before:   ");
  dbg_print_u32((uint32_t)g_debug_uart_baud_before);
  dbg_print("\r\n");
  dbg_print("Baud After:    ");
  dbg_print_u32((uint32_t)g_debug_uart_baud_after);
  dbg_print("\r\n");
  dbg_print("===============================\r\n");
}

/**
 * @brief Test debug output from GDB
 * 
 * Usage in GDB:
 *   (gdb) call gdb_test_output()
 */
void gdb_test_output(void) {
  dbg_print("\r\n[GDB TEST] Debug output working!\r\n");
  dbg_print("[GDB TEST] Timestamp: ");
  dbg_print_u32(HAL_GetTick());
  dbg_print("\r\n");
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
