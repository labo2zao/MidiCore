/**
 * @file cli.c
 * @brief Command Line Interface implementation
 */

#include "cli.h"
#include "App/tests/test_debug.h"
#include "Config/module_config.h"  // For MODULE_CLI_OUTPUT and DEBUG_OUTPUT modes
#include "main.h"  // For UART handle
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#if MODULE_ENABLE_USB_CDC
#include "Services/usb_cdc/usb_cdc.h"
#endif

#if MODULE_CLI_OUTPUT == CLI_OUTPUT_MIOS
#include "Services/midicore_query/midicore_query.h"  // For SysEx terminal protocol
#endif

// =============================================================================
// PRIVATE STATE
// =============================================================================

static cli_command_t s_commands[CLI_MAX_COMMANDS];
static uint32_t s_command_count = 0;
static char s_input_line[CLI_MAX_LINE_LEN];
static uint32_t s_input_pos = 0;
static uint8_t s_initialized = 0;

// Command history
static char s_history[CLI_HISTORY_SIZE][CLI_MAX_LINE_LEN];
static uint8_t s_history_count = 0;
static uint8_t s_history_index = 0;

#if MODULE_ENABLE_USB_CDC
// =============================================================================
// USB CDC INPUT BUFFER (ISR-safe circular buffer)
// =============================================================================
// USB CDC uses callbacks from ISR, not polling.
// We buffer input here and process in cli_task().

#define CLI_INPUT_BUFFER_SIZE 256

static uint8_t s_input_buffer[CLI_INPUT_BUFFER_SIZE];
static volatile uint16_t s_input_head = 0;
static volatile uint16_t s_input_tail = 0;

/**
 * @brief USB CDC RX callback - called from ISR context
 * @param buf Received data buffer
 * @param len Number of bytes received
 * 
 * CRITICAL: Called from USB interrupt! Keep it fast!
 * Just buffer the data and return immediately.
 */
static void cli_usb_cdc_rx_callback(const uint8_t *buf, uint32_t len)
{
  // Queue all received bytes into circular buffer
  for (uint32_t i = 0; i < len; i++) {
    uint16_t next_head = (s_input_head + 1) % CLI_INPUT_BUFFER_SIZE;
    
    // Check if buffer full (drop character if full)
    if (next_head != s_input_tail) {
      s_input_buffer[s_input_head] = buf[i];
      s_input_head = next_head;
    }
    // If buffer full, character is silently dropped (overflow protection)
  }
}

/**
 * @brief Get one character from input buffer (non-blocking)
 * @param ch Pointer to store received character
 * @return 1 if character available, 0 if buffer empty
 */
static uint8_t cli_getchar(uint8_t *ch)
{
  // Check if buffer empty
  if (s_input_head == s_input_tail) {
    return 0;  // No data available
  }
  
  // Get character from buffer
  *ch = s_input_buffer[s_input_tail];
  s_input_tail = (s_input_tail + 1) % CLI_INPUT_BUFFER_SIZE;
  
  return 1;  // Character retrieved
}
#endif

// =============================================================================
// FORWARD DECLARATIONS
// =============================================================================

static cli_result_t cmd_help(int argc, char* argv[]);
static cli_result_t cmd_list(int argc, char* argv[]);
static cli_result_t cmd_clear(int argc, char* argv[]);
static cli_result_t cmd_version(int argc, char* argv[]);
static cli_result_t cmd_uptime(int argc, char* argv[]);
static cli_result_t cmd_status(int argc, char* argv[]);
static cli_result_t cmd_reboot(int argc, char* argv[]);

// =============================================================================
// INITIALIZATION
// =============================================================================

int cli_init(void)
{
  if (s_initialized) {
    return 0;
  }

  dbg_printf("[CLI] Initializing CLI subsystem...\r\n");

  // Clear state
  memset(s_commands, 0, sizeof(s_commands));
  s_command_count = 0;
  memset(s_input_line, 0, sizeof(s_input_line));
  s_input_pos = 0;
  memset(s_history, 0, sizeof(s_history));
  s_history_count = 0;
  s_history_index = 0;

  // Register built-in commands
  cli_register_command("help", cmd_help, "Show help", "help [command]", "system");
  cli_register_command("list", cmd_list, "List commands", "list", "system");
  cli_register_command("clear", cmd_clear, "Clear screen", "clear", "system");
  cli_register_command("version", cmd_version, "Show version", "version", "system");
  cli_register_command("uptime", cmd_uptime, "Show uptime", "uptime", "system");
  cli_register_command("status", cmd_status, "Show status", "status", "system");
  cli_register_command("reboot", cmd_reboot, "Reboot system", "reboot", "system");

#if MODULE_ENABLE_USB_CDC
  // Register USB CDC receive callback for CLI input
  // This is CRITICAL - without this, CLI receives no input!
  dbg_printf("[CLI] Registering USB CDC receive callback...\r\n");
  usb_cdc_register_receive_callback(cli_usb_cdc_rx_callback);
  s_input_head = 0;
  s_input_tail = 0;
  dbg_printf("[CLI] USB CDC callback registered\r\n");
#endif

  s_initialized = 1;

  dbg_printf("[CLI] CLI initialization complete - %lu commands registered\r\n", (unsigned long)s_command_count);

  return 0;
}

// =============================================================================
// COMMAND REGISTRATION
// =============================================================================

int cli_register_command(const char* name,
                         cli_command_handler_t handler,
                         const char* description,
                         const char* usage,
                         const char* category)
{
  if (!name || !handler || s_command_count >= CLI_MAX_COMMANDS) {
    return -1;
  }

  // Check for duplicates
  for (uint32_t i = 0; i < s_command_count; i++) {
    if (strcasecmp(s_commands[i].name, name) == 0) {
      return -1; // Already registered
    }
  }

  // Register command
  strncpy(s_commands[s_command_count].name, name, CLI_MAX_COMMAND_LEN - 1);
  s_commands[s_command_count].handler = handler;
  s_commands[s_command_count].description = description ? description : "";
  s_commands[s_command_count].usage = usage ? usage : name;
  s_commands[s_command_count].category = category ? category : "other";
  s_command_count++;

  return 0;
}

int cli_unregister_command(const char* name)
{
  if (!name) {
    return -1;
  }

  for (uint32_t i = 0; i < s_command_count; i++) {
    if (strcasecmp(s_commands[i].name, name) == 0) {
      // Shift remaining commands
      for (uint32_t j = i; j < s_command_count - 1; j++) {
        s_commands[j] = s_commands[j + 1];
      }
      s_command_count--;
      return 0;
    }
  }

  return -1; // Not found
}

// =============================================================================
// COMMAND EXECUTION
// =============================================================================

cli_result_t cli_execute(const char* line)
{
  if (!line || strlen(line) == 0) {
    return CLI_OK;
  }

  // Parse command line into arguments
  char buffer[CLI_MAX_LINE_LEN];
  strncpy(buffer, line, sizeof(buffer) - 1);
  buffer[sizeof(buffer) - 1] = '\0';

  char* argv[CLI_MAX_ARGS];
  int argc = 0;

  char* token = strtok(buffer, " \t\r\n");
  while (token && argc < CLI_MAX_ARGS) {
    argv[argc++] = token;
    token = strtok(NULL, " \t\r\n");
  }

  if (argc == 0) {
    return CLI_OK;
  }

  return cli_execute_argv(argc, argv);
}

cli_result_t cli_execute_argv(int argc, char* argv[])
{
  if (argc == 0 || !argv[0]) {
    return CLI_INVALID_ARGS;
  }

  // Find command
  for (uint32_t i = 0; i < s_command_count; i++) {
    if (strcasecmp(s_commands[i].name, argv[0]) == 0) {
      // Execute handler
      cli_result_t result = s_commands[i].handler(argc, argv);
      return result;
    }
  }

  cli_error("Command not found: %s\n", argv[0]);
  cli_printf("Type 'help' for available commands.\n");
  return CLI_NOT_FOUND;
}

// =============================================================================
// INPUT PROCESSING
// =============================================================================

/**
 * @brief Get one character from CLI input (respects MODULE_CLI_OUTPUT setting)
 * @param ch Pointer to store received character
 * @return 1 if character available, 0 if no character
 * 
 * Reads CLI input from appropriate source based on MODULE_CLI_OUTPUT:
 * - CLI_OUTPUT_USB_CDC / CLI_OUTPUT_MIOS: Read from USB CDC
 * - CLI_OUTPUT_UART: Read from UART
 * - CLI_OUTPUT_DEBUG: Read based on MODULE_DEBUG_OUTPUT setting
 */
static uint8_t cli_get_input_char(uint8_t *ch)
{
#if MODULE_CLI_OUTPUT == CLI_OUTPUT_USB_CDC || MODULE_CLI_OUTPUT == CLI_OUTPUT_MIOS
  // USB CDC input modes
  #if MODULE_ENABLE_USB_CDC
    return cli_getchar(ch);  // Read from USB CDC callback buffer
  #else
    return 0;  // USB CDC not available
  #endif
  
#elif MODULE_CLI_OUTPUT == CLI_OUTPUT_UART
  // UART input mode - read from debug UART
  extern UART_HandleTypeDef huart5;  // User's debug UART (PC12/PD2)
  UART_HandleTypeDef* uart = &huart5;
  
  if (HAL_UART_Receive(uart, ch, 1, 0) == HAL_OK) {
    // Character received - echo it back (UART terminals often don't echo locally)
    HAL_UART_Transmit(uart, ch, 1, 10);
    return 1;
  }
  return 0;  // No character available
  
#elif MODULE_CLI_OUTPUT == CLI_OUTPUT_DEBUG
  // Follow MODULE_DEBUG_OUTPUT setting
  #if MODULE_DEBUG_OUTPUT == DEBUG_OUTPUT_UART
    // UART mode - read from debug UART
    extern UART_HandleTypeDef huart5;
    UART_HandleTypeDef* uart = &huart5;
    
    if (HAL_UART_Receive(uart, ch, 1, 0) == HAL_OK) {
      // Echo character for UART terminals
      HAL_UART_Transmit(uart, ch, 1, 10);
      return 1;
    }
    return 0;
    
  #elif MODULE_DEBUG_OUTPUT == DEBUG_OUTPUT_USB_CDC
    // USB CDC mode
    #if MODULE_ENABLE_USB_CDC
      return cli_getchar(ch);
    #else
      return 0;
    #endif
    
  #else
    // SWV or other modes - no input available
    return 0;
  #endif
  
#else
  #error "Invalid MODULE_CLI_OUTPUT setting"
#endif
}

void cli_task(void)
{
  if (!s_initialized) {
    return;
  }

  // Get one character (non-blocking) from appropriate input source
  uint8_t ch;
  if (!cli_get_input_char(&ch)) {
    return; // No character available
  }
  
  // NOTE: Echo handling is done in cli_get_input_char() for UART modes.
  // USB CDC modes rely on terminal emulator echo (standard MidiCore behavior).
  
  // Handle special characters
  if (ch == '\r' || ch == '\n') {
    // Enter pressed - execute command
    cli_printf("\r\n");
    
    if (s_input_pos > 0) {
      s_input_line[s_input_pos] = '\0';
      
      // Add to history
      if (s_history_count < CLI_HISTORY_SIZE) {
        strncpy(s_history[s_history_count], s_input_line, CLI_MAX_LINE_LEN - 1);
        s_history[s_history_count][CLI_MAX_LINE_LEN - 1] = '\0';
        s_history_count++;
      }
      
      // Execute command
      cli_execute(s_input_line);
      
      // Reset input line
      s_input_pos = 0;
      memset(s_input_line, 0, sizeof(s_input_line));
    }
    
    // Print new prompt
    cli_print_prompt();
  }
  else if (ch == 0x7F || ch == 0x08) {
    // Backspace or DEL
    if (s_input_pos > 0) {
      s_input_pos--;
      s_input_line[s_input_pos] = '\0';
      // Erase character on terminal: backspace + space + backspace
      cli_printf("\b \b");
    }
  }
  else if (ch >= 0x20 && ch < 0x7F) {
    // Printable character
    if (s_input_pos < CLI_MAX_LINE_LEN - 1) {
      s_input_line[s_input_pos++] = (char)ch;
      s_input_line[s_input_pos] = '\0';
      // Echo only for non-UART modes (UART already echoed in cli_get_input_char)
#if MODULE_CLI_OUTPUT == CLI_OUTPUT_USB_CDC || MODULE_CLI_OUTPUT == CLI_OUTPUT_MIOS || \
    (MODULE_CLI_OUTPUT == CLI_OUTPUT_DEBUG && MODULE_DEBUG_OUTPUT != DEBUG_OUTPUT_UART)
      cli_printf("%c", ch);
#endif
    }
  }
  // Ignore other control characters for now (arrows, etc.)
}

// =============================================================================
// OUTPUT HELPERS
// =============================================================================

// CLI output function - routes based on MODULE_CLI_OUTPUT configuration
// Allows user to choose CLI terminal independently from debug output
static void cli_print(const char* str)
{
  if (!str || strlen(str) == 0) {
    return;
  }

#if MODULE_CLI_OUTPUT == CLI_OUTPUT_USB_CDC
  // Route to USB CDC only
  usb_cdc_send((const uint8_t*)str, strlen(str));
  
#elif MODULE_CLI_OUTPUT == CLI_OUTPUT_UART
  // Route to UART (force UART mode for CLI)
  #if MODULE_DEBUG_OUTPUT == DEBUG_OUTPUT_UART
    // Already UART mode, use normal debug print
    dbg_print(str);
  #else
    // Not in UART mode, need to send directly to UART
    // This is a fallback - normally use DEBUG_OUTPUT_UART
    dbg_print(str);  // Will go to current debug output
  #endif
  
#elif MODULE_CLI_OUTPUT == CLI_OUTPUT_MIOS
  // MIOS terminal mode - use MIDI SysEx protocol (NOT USB CDC)
  // MIOS Studio terminal receives text via SysEx debug messages
  midicore_debug_send_message(str, 0);  // Cable 0
  
#elif MODULE_CLI_OUTPUT == CLI_OUTPUT_DEBUG
  // Follow MODULE_DEBUG_OUTPUT setting
  dbg_print(str);
  
#else
  #error "Invalid MODULE_CLI_OUTPUT setting"
#endif
}

void cli_printf(const char* fmt, ...)
{
  char buffer[256];
  va_list args;

  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  cli_print(buffer);  // Use cli_print() instead of dbg_print()
}

void cli_error(const char* fmt, ...)
{
  char buffer[256];
  va_list args;

  cli_printf("ERROR: ");
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  cli_print(buffer);  // Use cli_print() for consistency
}

void cli_success(const char* fmt, ...)
{
  char buffer[256];
  va_list args;

  cli_printf("OK: ");
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  cli_print(buffer);  // Use cli_print() for consistency
}

void cli_warning(const char* fmt, ...)
{
  char buffer[256];
  va_list args;

  cli_printf("WARNING: ");
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  cli_print(buffer);  // Use cli_print() for consistency
}

// =============================================================================
// UTILITIES
// =============================================================================

void cli_print_help(const char* command_name)
{
  if (command_name) {
    // Show help for specific command
    for (uint32_t i = 0; i < s_command_count; i++) {
      if (strcasecmp(s_commands[i].name, command_name) == 0) {
        cli_printf("\n");
        cli_printf("Command: %s\n", s_commands[i].name);
        cli_printf("Category: %s\n", s_commands[i].category);
        cli_printf("Description: %s\n", s_commands[i].description);
        cli_printf("Usage: %s\n", s_commands[i].usage);
        cli_printf("\n");
        return;
      }
    }
    cli_error("Command not found: %s\n", command_name);
  } else {
    // Show all commands grouped by category
    cli_printf("\n=== MidiCore CLI Help ===\n\n");
    
    // Group by category
    const char* current_category = NULL;
    for (uint32_t i = 0; i < s_command_count; i++) {
      if (current_category == NULL || strcmp(s_commands[i].category, current_category) != 0) {
        current_category = s_commands[i].category;
        cli_printf("\n[%s]\n", current_category);
      }
      cli_printf("  %-20s - %s\n", s_commands[i].name, s_commands[i].description);
    }
    cli_printf("\nType 'help <command>' for detailed usage.\n\n");
  }
}

void cli_print_commands(void)
{
  cli_printf("\n=== Registered Commands (%lu) ===\n\n", (unsigned long)s_command_count);
  
  const char* current_category = NULL;
  for (uint32_t i = 0; i < s_command_count; i++) {
    if (current_category == NULL || strcmp(s_commands[i].category, current_category) != 0) {
      current_category = s_commands[i].category;
      cli_printf("\n[%s]\n", current_category);
    }
    cli_printf("  %s\n", s_commands[i].name);
  }
  cli_printf("\n");
}

uint32_t cli_get_command_count(void)
{
  return s_command_count;
}

void cli_print_banner(void)
{
  cli_printf("\n");
  cli_printf("=====================================\n");
  cli_printf("   MidiCore CLI v1.0\n");
  cli_printf("   Firmware Configuration Interface\n");
  cli_printf("=====================================\n");
  cli_printf("\n");
#if MODULE_ENABLE_USB_CDC
  cli_printf("NOTE: USB CDC may take 2-5 seconds to enumerate.\n");
  cli_printf("      If you don't see prompt, press ENTER.\n");
  cli_printf("\n");
#endif
  cli_printf("Type 'help' for available commands.\n");
  cli_printf("\n");
}

void cli_print_prompt(void)
{
  // Add newline before prompt when CLI shares terminal with debug output
  // This prevents CLI prompt from overwriting MidiCore query debug messages
  #if (MODULE_CLI_OUTPUT == CLI_OUTPUT_DEBUG) || \
      (MODULE_CLI_OUTPUT == CLI_OUTPUT_UART && MODULE_DEBUG_OUTPUT == DEBUG_OUTPUT_UART)
    // CLI and debug on same terminal - add newline for separation
    #if MODULE_DEBUG_MIDICORE_QUERIES
      // MidiCore debug active - ensure clean line for prompt
      cli_printf("\r\nmidicore> ");
    #else
      // No MidiCore debug - normal prompt
      cli_printf("midicore> ");
    #endif
  #else
    // CLI on separate terminal from debug - normal prompt
    cli_printf("midicore> ");
  #endif
}

// =============================================================================
// BUILT-IN COMMANDS
// =============================================================================

static cli_result_t cmd_help(int argc, char* argv[])
{
  if (argc > 1) {
    cli_print_help(argv[1]);
  } else {
    cli_print_help(NULL);
  }
  return CLI_OK;
}

static cli_result_t cmd_list(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  cli_print_commands();
  return CLI_OK;
}

static cli_result_t cmd_clear(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  
  // ANSI escape sequence to clear screen
  cli_printf("\033[2J\033[H");
  cli_print_banner();
  return CLI_OK;
}

static cli_result_t cmd_version(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  
  cli_printf("\n");
  cli_printf("MidiCore Firmware\n");
  cli_printf("  Version: 1.0.0\n");
  cli_printf("  Build: %s %s\n", __DATE__, __TIME__);
  cli_printf("  Target: STM32F407VGT6\n");
  cli_printf("\n");
  return CLI_OK;
}

static cli_result_t cmd_uptime(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  
  extern uint32_t HAL_GetTick(void);
  uint32_t ticks = HAL_GetTick();
  uint32_t seconds = ticks / 1000;
  uint32_t minutes = seconds / 60;
  uint32_t hours = minutes / 60;
  
  cli_printf("\n");
  cli_printf("System Uptime: %lu:%02lu:%02lu\n", 
             (unsigned long)hours, 
             (unsigned long)(minutes % 60), 
             (unsigned long)(seconds % 60));
  cli_printf("\n");
  return CLI_OK;
}

static cli_result_t cmd_status(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  
  cli_printf("\n");
  cli_printf("=== System Status ===\n");
  cli_printf("  Commands registered: %lu\n", (unsigned long)s_command_count);
  cli_printf("\n");
  return CLI_OK;
}

static cli_result_t cmd_reboot(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  
  cli_printf("\nRebooting system...\n");
  
  // Delay to allow UART to flush
  extern void HAL_Delay(uint32_t Delay);
  HAL_Delay(100);
  
  // Perform system reset
  extern void NVIC_SystemReset(void);
  NVIC_SystemReset();
  
  return CLI_OK;
}

// =============================================================================
// MIOS STUDIO TERMINAL COMMAND PROCESSING
// =============================================================================

/**
 * @brief Process a command received from MIOS Studio terminal via SysEx
 * @param cmd Null-terminated command string (e.g., "help", "status", etc.)
 * 
 * Called from midicore_query.c when a debug message with type=0x00 (input)
 * is received. The response is sent back via midicore_debug_send_message().
 */
void cli_process_mios_command(const char* cmd)
{
  if (!cmd || !s_initialized) {
    return;
  }
  
  // Strip leading whitespace
  while (*cmd && isspace((unsigned char)*cmd)) {
    cmd++;
  }
  
  // Ignore empty commands
  if (*cmd == '\0') {
    return;
  }
  
  // Strip trailing newline/carriage return
  char clean_cmd[CLI_MAX_LINE_LEN];
  strncpy(clean_cmd, cmd, sizeof(clean_cmd) - 1);
  clean_cmd[sizeof(clean_cmd) - 1] = '\0';
  
  // Remove trailing whitespace/newlines
  size_t len = strlen(clean_cmd);
  while (len > 0 && (clean_cmd[len-1] == '\n' || clean_cmd[len-1] == '\r' || isspace((unsigned char)clean_cmd[len-1]))) {
    clean_cmd[--len] = '\0';
  }
  
  // Execute the command - response goes to cli_print() which uses midicore_debug_send_message()
  cli_execute(clean_cmd);
  
  // Send prompt for next command
  cli_print("> ");
}
