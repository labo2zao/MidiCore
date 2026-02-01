/**
 * @file cli.h
 * @brief Command Line Interface for MidiCore - MIOS32 Style
 * 
 * MIOS32 PRINCIPLES:
 * - NO printf / snprintf / vsnprintf
 * - Fixed string outputs only
 * - Minimal stack usage
 * - No dynamic allocation
 * 
 * Output via:
 * - MIOS Studio Terminal (SysEx protocol)
 * - USB CDC (optional)
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
#define CLI_MAX_COMMANDS 32
#endif

#ifndef CLI_MAX_COMMAND_LEN
#define CLI_MAX_COMMAND_LEN 64
#endif

#ifndef CLI_MAX_LINE_LEN
#define CLI_MAX_LINE_LEN 128
#endif

#ifndef CLI_MAX_ARGS
#define CLI_MAX_ARGS 8
#endif

#ifndef CLI_HISTORY_SIZE
#define CLI_HISTORY_SIZE 0  /* Disabled to save RAM */
#endif

// =============================================================================
// TYPES
// =============================================================================

typedef enum {
  CLI_OK = 0,
  CLI_ERROR = -1,
  CLI_INVALID_ARGS = -2,
  CLI_NOT_FOUND = -3,
  CLI_NO_PERMISSION = -4
} cli_result_t;

typedef cli_result_t (*cli_command_handler_t)(int argc, char* argv[]);

typedef struct {
  char name[CLI_MAX_COMMAND_LEN];
  cli_command_handler_t handler;
  const char* description;
  const char* usage;
  const char* category;
} cli_command_t;

// =============================================================================
// API - INITIALIZATION
// =============================================================================

int cli_init(void);
void cli_task(void);

// =============================================================================
// API - COMMAND REGISTRATION
// =============================================================================

int cli_register_command(const char* name,
                         cli_command_handler_t handler,
                         const char* description,
                         const char* usage,
                         const char* category);

int cli_unregister_command(const char* name);

// =============================================================================
// API - COMMAND EXECUTION
// =============================================================================

cli_result_t cli_execute(const char* line);
cli_result_t cli_execute_argv(int argc, char* argv[]);

// =============================================================================
// API - OUTPUT HELPERS (MIOS32 STYLE - NO printf!)
// =============================================================================

/* Fixed string output */
void cli_puts(const char* str);
void cli_putc(char c);
void cli_newline(void);

/* Number output (no printf!) */
void cli_print_u32(uint32_t val);
void cli_print_hex8(uint8_t val);

/* Status messages (fixed string only!) */
void cli_error(const char* msg);
void cli_success(const char* msg);
void cli_warning(const char* msg);

// =============================================================================
// API - UTILITIES
// =============================================================================

void cli_print_help(const char* command_name);
void cli_print_commands(void);
uint32_t cli_get_command_count(void);
void cli_print_banner(void);
void cli_print_prompt(void);

// =============================================================================
// API - MIOS STUDIO TERMINAL INTERFACE
// =============================================================================

/**
 * @brief Process a command received from MIOS Studio terminal via SysEx
 * @param cmd Null-terminated command string
 */
void cli_process_mios_command(const char* cmd);

#ifdef __cplusplus
}
#endif
