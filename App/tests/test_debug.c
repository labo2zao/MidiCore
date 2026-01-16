/**
 * @file test_debug.c
 * @brief Implementation of MIOS32-compatible debug output
 */

#include "App/tests/test_debug.h"
#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// External UART handles from main.c
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart5;

// =============================================================================
// UART HANDLE SELECTION
// =============================================================================

static UART_HandleTypeDef* get_debug_uart_handle(void)
{
  switch (TEST_DEBUG_UART_PORT) {
    case 0: return &huart1;
    case 1: return &huart2;
    case 2: return &huart3;
    case 3: return &huart5;
    default: return &huart2; // Default to UART2
  }
}

// =============================================================================
// INITIALIZATION
// =============================================================================

int test_debug_init(void)
{
  // UART is already initialized in main.c by CubeMX
  // Just verify the handle is ready
  UART_HandleTypeDef* huart = get_debug_uart_handle();
  if (huart->gState == HAL_UART_STATE_READY) {
    return 0; // Success
  }
  return 0; // Return success anyway - UART might be in other states but still usable
}

// =============================================================================
// BASIC OUTPUT FUNCTIONS
// =============================================================================

void dbg_putc(char c)
{
  UART_HandleTypeDef* huart = get_debug_uart_handle();
  HAL_UART_Transmit(huart, (uint8_t*)&c, 1, 100);
}

void dbg_print(const char* str)
{
  if (!str) return;
  
  UART_HandleTypeDef* huart = get_debug_uart_handle();
  // Send entire string at once for better performance
  size_t len = strlen(str);
  if (len > 0) {
    HAL_UART_Transmit(huart, (const uint8_t*)str, len, 1000);
  }
}

void dbg_println(void)
{
  dbg_putc('\r');
  dbg_putc('\n');
}

// =============================================================================
// FORMATTED OUTPUT
// =============================================================================

void dbg_printf(const char* format, ...)
{
  char buffer[256];
  va_list args;
  
  va_start(args, format);
  int len = vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  
  if (len > 0 && len < (int)sizeof(buffer)) {
    dbg_print(buffer);
  }
}

// =============================================================================
// HEXADECIMAL OUTPUT
// =============================================================================

static const char hex_chars[] = "0123456789ABCDEF";

void dbg_print_hex8(uint8_t b)
{
  dbg_putc(hex_chars[(b >> 4) & 0x0F]);
  dbg_putc(hex_chars[b & 0x0F]);
}

void dbg_print_hex16(uint16_t w)
{
  dbg_print_hex8((uint8_t)(w >> 8));
  dbg_print_hex8((uint8_t)(w & 0xFF));
}

void dbg_print_hex32(uint32_t dw)
{
  dbg_print_hex16((uint16_t)(dw >> 16));
  dbg_print_hex16((uint16_t)(dw & 0xFFFF));
}

void dbg_print_bytes(const uint8_t* data, uint16_t len, char separator)
{
  if (!data) return;
  
  for (uint16_t i = 0; i < len; i++) {
    dbg_print_hex8(data[i]);
    if (separator && i < len - 1) {
      dbg_putc(separator);
    }
  }
}

// =============================================================================
// DECIMAL OUTPUT
// =============================================================================

void dbg_print_uint(uint32_t n)
{
  char buffer[12];  // Max: 4294967295 (10 digits) + null
  int pos = 0;
  
  // Handle zero special case
  if (n == 0) {
    dbg_putc('0');
    return;
  }
  
  // Build string in reverse
  while (n > 0) {
    buffer[pos++] = '0' + (n % 10);
    n /= 10;
  }
  
  // Output in correct order
  while (pos > 0) {
    dbg_putc(buffer[--pos]);
  }
}

void dbg_print_int(int32_t n)
{
  if (n < 0) {
    dbg_putc('-');
    dbg_print_uint((uint32_t)(-n));
  } else {
    dbg_print_uint((uint32_t)n);
  }
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

void dbg_print_separator(void)
{
  for (int i = 0; i < 60; i++) {
    dbg_putc('=');
  }
  dbg_println();
}

void dbg_print_test_header(const char* test_name)
{
  dbg_println();
  dbg_print_separator();
  dbg_print(test_name);
  dbg_println();
  dbg_print_separator();
}

void dbg_print_config_info(void)
{
  dbg_print("UART Configuration (MIOS32 Compatible):");
  dbg_println();
  dbg_print("  Debug UART:    UART");
  dbg_print_uint(TEST_DEBUG_UART_PORT + 1);
  dbg_print(" (port ");
  dbg_print_uint(TEST_DEBUG_UART_PORT);
  dbg_print(") @ ");
  dbg_print_uint(TEST_DEBUG_UART_BAUD);
  dbg_print(" baud");
  dbg_println();
  
  dbg_print("  MIDI DIN UART: UART");
  dbg_print_uint(TEST_MIDI_DIN_UART_PORT + 1);
  dbg_print(" (port ");
  dbg_print_uint(TEST_MIDI_DIN_UART_PORT);
  dbg_print(") @ ");
  dbg_print_uint(TEST_MIDI_DIN_UART_BAUD);
  dbg_print(" baud");
  dbg_println();
  dbg_println();
  
  dbg_print("MIOS32 UART Mapping:");
  dbg_println();
  dbg_print("  Port 0 = UART1 (USART1) - PA9/PA10  - MIDI OUT1/IN1");
  dbg_println();
  dbg_print("  Port 1 = UART2 (USART2) - PA2/PA3   - MIDI OUT2/IN2");
  dbg_println();
  dbg_print("  Port 2 = UART3 (USART3) - PB10/PB11 - MIDI OUT3/IN3");
  dbg_println();
  dbg_print("  Port 3 = UART5 (UART5)  - PC12/PD2  - MIDI OUT4/IN4");
  dbg_println();
  dbg_print_separator();
}
