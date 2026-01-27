/**
 * @file cli.h
 * @brief Command Line Interface for MidiCore - UART Terminal Control
 * 
 * Provides a command-line interface over UART for runtime configuration
 * and control of all MidiCore modules. Commands can be entered interactively
 * via terminal or programmatically via the UI.
 * 
 * Features:
 * - Command registration and execution
 * - Help system
 * - Line editing (backspace, cursor navigation)
 * - Command history
 * - Auto-completion
 * - Module registry integration
 * 
 * Architecture:
 * - Works with existing test_debug.h UART infrastructure
 * - Commands registered per-module
 * - Thread-safe command execution
 * - No dynamic memory allocation
 * 
 * Usage:
 * 1. Call cli_init() during system initialization
 * 2. Register module commands with cli_register_command()
 * 3. Call cli_task() periodically (e.g., from FreeRTOS task)
 * 4. Commands are executed as they arrive via UART
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONFIGURATION
// =============================================================================

#ifndef CLI_MAX_COMMANDS
#define CLI_MAX_COMMANDS 64  // Maximum number of registered commands (reduced from 128 to save ~5KB RAM)
#endif

#ifndef CLI_MAX_COMMAND_LEN
#define CLI_MAX_COMMAND_LEN 64  // Maximum command name length
#endif

#ifndef CLI_MAX_LINE_LEN
#define CLI_MAX_LINE_LEN 256  // Maximum input line length
#endif

#ifndef CLI_MAX_ARGS
#define CLI_MAX_ARGS 8  // Maximum arguments per command
#endif

#ifndef CLI_HISTORY_SIZE
#define CLI_HISTORY_SIZE 16  // Number of commands in history (optimized for RAM usage)
#endif

// =============================================================================
// TYPES
// =============================================================================

/**
 * @brief Command execution result
 */
typedef enum {
  CLI_OK = 0,              // Command executed successfully
  CLI_ERROR = -1,          // Command execution error
  CLI_INVALID_ARGS = -2,   // Invalid arguments
  CLI_NOT_FOUND = -3,      // Command not found
  CLI_NO_PERMISSION = -4   // Permission denied
} cli_result_t;

/**
 * @brief Command handler function type
 * @param argc Number of arguments
 * @param argv Array of argument strings (argv[0] is command name)
 * @return CLI result code
 */
typedef cli_result_t (*cli_command_handler_t)(int argc, char* argv[]);

/**
 * @brief Command descriptor
 */
typedef struct {
  char name[CLI_MAX_COMMAND_LEN];          // Command name
  cli_command_handler_t handler;           // Handler function
  const char* description;                 // Short description
  const char* usage;                       // Usage string
  const char* category;                    // Category (e.g., "looper", "midi")
} cli_command_t;

// =============================================================================
// API - INITIALIZATION
// =============================================================================

/**
 * @brief Initialize CLI system
 * @return 0 on success, negative on error
 */
int cli_init(void);

/**
 * @brief CLI task - call periodically to process input
 * Should be called from a FreeRTOS task or main loop
 */
void cli_task(void);

// =============================================================================
// API - COMMAND REGISTRATION
// =============================================================================

/**
 * @brief Register a command
 * @param name Command name (case-insensitive)
 * @param handler Command handler function
 * @param description Short description for help
 * @param usage Usage string (e.g., "module enable <name>")
 * @param category Category for grouping (e.g., "looper", "midi")
 * @return 0 on success, negative on error
 */
int cli_register_command(const char* name,
                         cli_command_handler_t handler,
                         const char* description,
                         const char* usage,
                         const char* category);

/**
 * @brief Unregister a command
 * @param name Command name
 * @return 0 on success, negative if not found
 */
int cli_unregister_command(const char* name);

// =============================================================================
// API - COMMAND EXECUTION
// =============================================================================

/**
 * @brief Execute a command string
 * @param line Command line string
 * @return CLI result code
 */
cli_result_t cli_execute(const char* line);

/**
 * @brief Execute a command with arguments
 * @param argc Number of arguments
 * @param argv Array of argument strings
 * @return CLI result code
 */
cli_result_t cli_execute_argv(int argc, char* argv[]);

// =============================================================================
// API - OUTPUT HELPERS
// =============================================================================

/**
 * @brief Print formatted output to CLI
 * @param fmt Printf-style format string
 * @param ... Variable arguments
 */
void cli_printf(const char* fmt, ...);

/**
 * @brief Print error message
 * @param fmt Printf-style format string
 * @param ... Variable arguments
 */
void cli_error(const char* fmt, ...);

/**
 * @brief Print success message
 * @param fmt Printf-style format string
 * @param ... Variable arguments
 */
void cli_success(const char* fmt, ...);

/**
 * @brief Print warning message
 * @param fmt Printf-style format string
 * @param ... Variable arguments
 */
void cli_warning(const char* fmt, ...);

// =============================================================================
// API - UTILITIES
// =============================================================================

/**
 * @brief Print help for all commands or specific command
 * @param command_name Command name (NULL for all commands)
 */
void cli_print_help(const char* command_name);

/**
 * @brief Print list of registered commands grouped by category
 */
void cli_print_commands(void);

/**
 * @brief Get number of registered commands
 * @return Count of registered commands
 */
uint32_t cli_get_command_count(void);

/**
 * @brief Print welcome banner
 */
void cli_print_banner(void);

/**
 * @brief Print command prompt
 */
void cli_print_prompt(void);

// =============================================================================
// BUILT-IN COMMANDS (auto-registered by cli_init)
// =============================================================================

/**
 * Built-in commands:
 * - help [command]       - Show help for all or specific command
 * - list                 - List all registered commands
 * - clear                - Clear screen
 * - version              - Show firmware version
 * - uptime               - Show system uptime
 * - status               - Show system status
 * - reboot               - Reboot system
 * - config load <file>   - Load configuration from file
 * - config save <file>   - Save configuration to file
 * - config get <key>     - Get configuration value
 * - config set <key> <val> - Set configuration value
 * - config list          - List all configuration entries
 */

#ifdef __cplusplus
}
#endif
