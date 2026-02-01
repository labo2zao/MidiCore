/**
 * @file cli.c
 * @brief Command Line Interface implementation - MIOS32 Style
 * 
 * MIOS32 PRINCIPLES:
 * - NO printf / snprintf / vsnprintf
 * - Fixed string outputs only
 * - Minimal stack usage
 * - No dynamic allocation
 */

#include "cli.h"
#include "Config/module_config.h"
#include "main.h"
#include <string.h>
#include <ctype.h>

/* NO stdio.h - we don't use printf! */
/* NO stdarg.h - we don't use variadic functions! */

#if MODULE_ENABLE_USB_CDC
#include "Services/usb_cdc/usb_cdc.h"
#endif

#if MODULE_CLI_OUTPUT == CLI_OUTPUT_MIOS
#include "Services/midicore_query/midicore_query.h"
#endif

/* ============================================================================
 * MIOS32-STYLE NUMBER TO STRING CONVERSION (NO printf!)
 * ============================================================================ */

/* Convert uint32 to decimal string - returns pointer to static buffer */
static const char* cli_u32_to_str(uint32_t val)
{
  static char buf[12];  /* Max: 4294967295 = 10 digits + null */
  char* p = &buf[11];
  *p = '\0';
  
  if (val == 0) {
    *--p = '0';
  } else {
    while (val > 0) {
      *--p = '0' + (val % 10);
      val /= 10;
    }
  }
  return p;
}

/* Convert uint8 to hex string (2 digits) */
static const char* cli_u8_to_hex(uint8_t val)
{
  static char buf[3];
  static const char hex[] = "0123456789ABCDEF";
  buf[0] = hex[(val >> 4) & 0x0F];
  buf[1] = hex[val & 0x0F];
  buf[2] = '\0';
  return buf;
}

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
// INITIALIZATION - MIOS32 STYLE (NO printf!)
// =============================================================================

int cli_init(void)
{
  if (s_initialized) {
    return 0;
  }

  /* Clear state */
  memset(s_commands, 0, sizeof(s_commands));
  s_command_count = 0;
  memset(s_input_line, 0, sizeof(s_input_line));
  s_input_pos = 0;
  memset(s_history, 0, sizeof(s_history));
  s_history_count = 0;
  s_history_index = 0;

  /* Register built-in commands */
  cli_register_command("help", cmd_help, "Show help", "help [command]", "system");
  cli_register_command("list", cmd_list, "List commands", "list", "system");
  cli_register_command("clear", cmd_clear, "Clear screen", "clear", "system");
  cli_register_command("version", cmd_version, "Show version", "version", "system");
  cli_register_command("uptime", cmd_uptime, "Show uptime", "uptime", "system");
  cli_register_command("status", cmd_status, "Show status", "status", "system");
  cli_register_command("reboot", cmd_reboot, "Reboot system", "reboot", "system");

#if MODULE_ENABLE_USB_CDC
  /* Register USB CDC receive callback for CLI input */
  usb_cdc_register_receive_callback(cli_usb_cdc_rx_callback);
  s_input_head = 0;
  s_input_tail = 0;
#endif

  s_initialized = 1;
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

  cli_print("Unknown command: ");
  cli_print(argv[0]);
  cli_newline();
  cli_print("Type 'help' for available commands.\n");
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
  
  // Handle special characters
  if (ch == '\r' || ch == '\n') {
    cli_newline();
    
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
      // Erase character on terminal
      cli_print("\b \b");
    }
  }
  else if (ch >= 0x20 && ch < 0x7F) {
    // Printable character
    if (s_input_pos < CLI_MAX_LINE_LEN - 1) {
      s_input_line[s_input_pos++] = (char)ch;
      s_input_line[s_input_pos] = '\0';
      // Echo character
#if MODULE_CLI_OUTPUT == CLI_OUTPUT_USB_CDC || MODULE_CLI_OUTPUT == CLI_OUTPUT_MIOS || \
    (MODULE_CLI_OUTPUT == CLI_OUTPUT_DEBUG && MODULE_DEBUG_OUTPUT != DEBUG_OUTPUT_UART)
      cli_putc((char)ch);
#endif
    }
  }
  // Ignore other control characters
}

// =============================================================================
// OUTPUT HELPERS - MIOS32 STYLE (NO printf!)
// =============================================================================

/* CLI output function - routes based on MODULE_CLI_OUTPUT configuration */
static void cli_print(const char* str)
{
  if (!str || strlen(str) == 0) {
    return;
  }

#if MODULE_CLI_OUTPUT == CLI_OUTPUT_USB_CDC
  usb_cdc_send((const uint8_t*)str, strlen(str));
  
#elif MODULE_CLI_OUTPUT == CLI_OUTPUT_UART
  /* Direct UART output - implement if needed */
  (void)str;
  
#elif MODULE_CLI_OUTPUT == CLI_OUTPUT_MIOS
  /* MIOS terminal mode - use MIDI SysEx protocol */
  midicore_debug_send_message(str, 0);
  
#elif MODULE_CLI_OUTPUT == CLI_OUTPUT_DEBUG
  /* Disabled - no debug output in MIOS32 style */
  (void)str;
  
#else
  #error "Invalid MODULE_CLI_OUTPUT setting"
#endif
}

/* MIOS32-style output functions - NO printf! */
void cli_puts(const char* str)
{
  cli_print(str);
}

void cli_putc(char c)
{
  char buf[2] = { c, '\0' };
  cli_print(buf);
}

void cli_print_u32(uint32_t val)
{
  cli_print(cli_u32_to_str(val));
}

void cli_print_i32(int32_t val)
{
  if (val < 0) {
    cli_putc('-');
    val = -val;
  }
  cli_print(cli_u32_to_str((uint32_t)val));
}

void cli_print_hex8(uint8_t val)
{
  cli_print(cli_u8_to_hex(val));
}

void cli_newline(void)
{
  cli_print("\r\n");
}

/* Legacy compatibility - redirect to fixed string output */
void cli_error(const char* msg)
{
  cli_print("ERROR: ");
  cli_print(msg);
  cli_newline();
}

void cli_success(const char* msg)
{
  cli_print("OK: ");
  cli_print(msg);
  cli_newline();
}

void cli_warning(const char* msg)
{
  cli_print("WARNING: ");
  cli_print(msg);
  cli_newline();
}

// =============================================================================
// UTILITIES
// =============================================================================
// HELP FUNCTIONS - MIOS32 STYLE (NO printf!)
// =============================================================================

void cli_print_help(const char* command_name)
{
  if (command_name) {
    /* Show help for specific command */
    for (uint32_t i = 0; i < s_command_count; i++) {
      if (strcasecmp(s_commands[i].name, command_name) == 0) {
        cli_newline();
        cli_print("Command: "); cli_print(s_commands[i].name); cli_newline();
        cli_print("Category: "); cli_print(s_commands[i].category); cli_newline();
        cli_print("Description: "); cli_print(s_commands[i].description); cli_newline();
        cli_print("Usage: "); cli_print(s_commands[i].usage); cli_newline();
        cli_newline();
        return;
      }
    }
    cli_print("Command not found: "); cli_print(command_name); cli_newline();
  } else {
    /* Show all commands */
    cli_print("\n=== MidiCore CLI Help ===\n");
    
    const char* current_category = NULL;
    for (uint32_t i = 0; i < s_command_count; i++) {
      if (current_category == NULL || strcmp(s_commands[i].category, current_category) != 0) {
        current_category = s_commands[i].category;
        cli_print("\n["); cli_print(current_category); cli_print("]\n");
      }
      cli_print("  "); cli_print(s_commands[i].name); 
      cli_print(" - "); cli_print(s_commands[i].description); cli_newline();
    }
    cli_print("\nType 'help <command>' for details.\n");
  }
}

void cli_print_commands(void)
{
  cli_print("\n=== Commands (");
  cli_print_u32(s_command_count);
  cli_print(") ===\n");
  
  const char* current_category = NULL;
  for (uint32_t i = 0; i < s_command_count; i++) {
    if (current_category == NULL || strcmp(s_commands[i].category, current_category) != 0) {
      current_category = s_commands[i].category;
      cli_print("\n["); cli_print(current_category); cli_print("]\n");
    }
    cli_print("  "); cli_print(s_commands[i].name); cli_newline();
  }
  cli_newline();
}

uint32_t cli_get_command_count(void)
{
  return s_command_count;
}

void cli_print_banner(void)
{
  cli_print("\n=====================================\n");
  cli_print("   MidiCore CLI v1.0\n");
  cli_print("   MIOS32-Style Terminal\n");
  cli_print("=====================================\n");
  cli_print("Type 'help' for commands.\n\n");
}

void cli_print_prompt(void)
{
  cli_print("midicore> ");
}

// =============================================================================
// BUILT-IN COMMANDS - MIOS32 STYLE (NO printf!)
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
  
  /* ANSI escape sequence to clear screen */
  cli_print("\033[2J\033[H");
  cli_print_banner();
  return CLI_OK;
}

static cli_result_t cmd_version(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  
  cli_print("\nMidiCore Firmware\n");
  cli_print("  Version: 1.0.0\n");
  cli_print("  Target: STM32F407VGT6\n\n");
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
  
  cli_print("\nUptime: ");
  cli_print_u32(hours); cli_print(":");
  cli_print_u32(minutes % 60); cli_print(":");
  cli_print_u32(seconds % 60); cli_newline();
  return CLI_OK;
}

static cli_result_t cmd_status(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  
  cli_print("\n=== Status ===\n");
  cli_print("  Commands: "); cli_print_u32(s_command_count); cli_newline();
  cli_newline();
  return CLI_OK;
}

static cli_result_t cmd_reboot(int argc, char* argv[])
{
  (void)argc;
  (void)argv;
  
  cli_print("\nRebooting...\n");
  
  /* Delay to allow output to flush */
  extern void HAL_Delay(uint32_t Delay);
  HAL_Delay(100);
  
  /* System reset */
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
