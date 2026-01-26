/**
 * @file test_debug.h
 * @brief MIOS32-compatible debug output for module tests
 * 
 * This module provides debug print functions compatible with MIOS32 hardware.
 * 
 * **Test Mode Debug Configuration:**
 * - Debug UART: USART3 (PD8/PD9) @ 115200 baud (default TEST_DEBUG_UART_PORT=1)
 * - All other UARTs available for MIDI @ 31250 baud
 * - Baud rate automatically configured by test_debug_init()
 * 
 * **OLED Debug Mode (when MODULE_ENABLE_OLED active):**
 * - Debug output goes to OLED display (no UART required)
 * - All 4 MIDI DIN ports available for MIDI @ 31250 baud
 * - Call dbg_mirror_update() periodically to refresh OLED
 * 
 * **Production Mode - All 4 MIDI DIN ports @ 31250 baud:**
 * - Port 0 (DIN1): USART2 PA2=TX,  PA3=RX  @ 31250 baud
 * - Port 1 (DIN2): USART1 PA9=TX,  PA10=RX @ 31250 baud
 * - Port 2 (DIN3): USART3 PD8=TX,  PD9=RX  @ 31250 baud
 * - Port 3 (DIN4): UART5  PC12=TX, PD2=RX  @ 31250 baud
 */

#ifndef TEST_DEBUG_H
#define TEST_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdarg.h>

// =============================================================================
// UART PORT CONFIGURATION (MIOS32 Compatible)
// =============================================================================

/**
 * @brief UART port used for debug output (dbg_print)
 * 
 * MIOS32 UART Mapping:
 * 0 = USART2 - PA2/PA3   - MIDI DIN1 (production) or Debug (if selected)
 * 1 = USART3 - PD8/PD9   - MIDI DIN2 (production) or Debug (recommended for test mode)
 * 2 = USART1 - PA9/PA10  - MIDI DIN3 (production) or Debug (if selected)
 * 3 = UART5  - PC12/PD2  - MIDI DIN4
 * 
 * **Test Mode Default: Port 1 (USART3/PD8-PD9) @ 115200 baud**
 * This frees all other UARTs for MIDI @ 31250 baud
 * 
 * When a UART is configured as TEST_DEBUG_UART_PORT:
 * - It runs at 115200 baud for debug output
 * - It's NOT available for MIDI DIN
 * All other UARTs run at 31250 baud for MIDI
 */
#ifndef TEST_DEBUG_UART_PORT
#define TEST_DEBUG_UART_PORT 1  // USART3 (PD8/PD9)
#endif

/**
 * @brief UART port used for MIDI DIN communication
 * 
 * MIOS32 UART Mapping (same as above)
 * 
 * Default: UART1 (port 0) for MIDI DIN
 */
#ifndef TEST_MIDI_DIN_UART_PORT
#define TEST_MIDI_DIN_UART_PORT 0
#endif

/**
 * @brief Baud rate for debug UART
 * Options: 31250 (MIDI), 115200 (standard debug), 38400, 57600
 * 
 * Default: 115200 for easier terminal reading
 */
#ifndef TEST_DEBUG_UART_BAUD
#define TEST_DEBUG_UART_BAUD 115200
#endif

/**
 * @brief Baud rate for MIDI DIN UART
 * Must be 31250 for MIDI compatibility
 */
#ifndef TEST_MIDI_DIN_UART_BAUD
#define TEST_MIDI_DIN_UART_BAUD 31250
#endif

// =============================================================================
// DEBUG PRINT API (MIOS32-style)
// =============================================================================

/**
 * @brief Initialize debug output system
 * 
 * Call this before using dbg_print functions.
 * 
 * **Test Mode with OLED (MODULE_ENABLE_OLED active):**
 * - Automatically initializes OLED debug mirroring
 * - Enables OLED as PRIMARY debug output (no UART debug)
 * - All dbg_print() and dbg_printf() calls go to OLED only
 * - All 4 MIDI DIN ports remain available for MIDI @ 31250 baud
 * - Call dbg_mirror_update() periodically (e.g. every 100ms) to refresh display
 * 
 * **Test Mode without OLED:**
 * - Uses configured UART for debug @ 115200 baud
 * - MIDI DIN ports still available on other UARTs
 * 
 * @return 0 on success, negative on error
 */
int test_debug_init(void);

/**
 * @brief Print a string to debug UART (MIOS32-compatible)
 * @param str String to print (null-terminated)
 * 
 * Example:
 *   dbg_print("AINSER64 test started\n");
 */
void dbg_print(const char* str);

/**
 * @brief Print formatted string to debug UART (printf-style)
 * @param format Printf-style format string
 * @param ... Variable arguments
 * 
 * Example:
 *   dbg_printf("Channel %d: value=%d\n", ch, val);
 */
void dbg_printf(const char* format, ...);

/**
 * @brief Print a single character to debug UART
 * @param c Character to print
 */
void dbg_putc(char c);

/**
 * @brief Print a byte as hexadecimal (2 digits)
 * @param b Byte to print
 * 
 * Example:
 *   dbg_print_hex8(0xA5);  // Outputs: A5
 */
void dbg_print_hex8(uint8_t b);

/**
 * @brief Print a 16-bit value as hexadecimal (4 digits)
 * @param w 16-bit value to print
 * 
 * Example:
 *   dbg_print_hex16(0x1234);  // Outputs: 1234
 */
void dbg_print_hex16(uint16_t w);

/**
 * @brief Print a 32-bit value as hexadecimal (8 digits)
 * @param dw 32-bit value to print
 */
void dbg_print_hex32(uint32_t dw);

/**
 * @brief Print an unsigned decimal number
 * @param n Number to print
 */
void dbg_print_uint(uint32_t n);

/**
 * @brief Print a signed decimal number
 * @param n Number to print
 */
void dbg_print_int(int32_t n);

/**
 * @brief Print a byte array as hexadecimal
 * @param data Pointer to data
 * @param len Length of data
 * @param separator Character between bytes (e.g., ' ', ':', 0 for none)
 * 
 * Example:
 *   uint8_t buf[] = {0x90, 0x3C, 0x7F};
 *   dbg_print_bytes(buf, 3, ' ');  // Outputs: 90 3C 7F
 */
void dbg_print_bytes(const uint8_t* data, uint16_t len, char separator);

/**
 * @brief Print newline (CRLF)
 */
void dbg_println(void);

// =============================================================================
// MIOS32 COMPATIBILITY MACROS
// =============================================================================

/**
 * @brief MIOS32-style debug message macro
 * Usage: DEBUG_MSG("Value: %d\n", value);
 */
#define DEBUG_MSG(fmt, ...) dbg_printf(fmt, ##__VA_ARGS__)

/**
 * @brief Get configured debug UART port
 * @return UART port number (0-3)
 */
static inline uint8_t test_debug_get_uart_port(void) {
  return TEST_DEBUG_UART_PORT;
}

/**
 * @brief Get configured MIDI DIN UART port
 * @return UART port number (0-3)
 */
static inline uint8_t test_midi_din_get_uart_port(void) {
  return TEST_MIDI_DIN_UART_PORT;
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * @brief Print test header banner
 * @param test_name Name of the test
 * 
 * Example output:
 * =====================================
 * AINSER64 Module Test
 * =====================================
 */
void dbg_print_test_header(const char* test_name);

/**
 * @brief Print configuration info
 * Shows UART port assignments
 */
void dbg_print_config_info(void);

/**
 * @brief Print a separator line
 */
void dbg_print_separator(void);

/**
 * @brief Print SPI pinout with SCK/MISO/MOSI and RC1/RC2 pins.
 * @param label Optional label for the pinout (can be NULL)
 * @param hspi SPI handle for instance name
 * @param sck_port SCK GPIO port (NULL if not available)
 * @param sck_pin SCK GPIO pin (0 if not available)
 * @param miso_port MISO GPIO port (NULL if not available)
 * @param miso_pin MISO GPIO pin (0 if not available)
 * @param mosi_port MOSI GPIO port (NULL if not available)
 * @param mosi_pin MOSI GPIO pin (0 if not available)
 * @param rc1_port RC1 GPIO port (NULL if not available)
 * @param rc1_pin RC1 GPIO pin (0 if not available)
 * @param rc2_port RC2 GPIO port (NULL if not available)
 * @param rc2_pin RC2 GPIO pin (0 if not available)
 */
void gdb_ptin_SPI_Pinout(const char* label,
                         SPI_HandleTypeDef* hspi,
                         GPIO_TypeDef* sck_port, uint16_t sck_pin,
                         GPIO_TypeDef* miso_port, uint16_t miso_pin,
                         GPIO_TypeDef* mosi_port, uint16_t mosi_pin,
                         GPIO_TypeDef* rc1_port, uint16_t rc1_pin,
                         GPIO_TypeDef* rc2_port, uint16_t rc2_pin);

/**
 * @brief Update OLED mirror display (if enabled)
 * 
 * Call periodically (e.g. every 100ms) to refresh OLED with debug output.
 * 
 * **Test Mode with OLED (recommended pattern):**
 * @code
 *   test_debug_init();  // Auto-enables OLED as primary debug output
 *   
 *   while (1) {
 *     dbg_printf("Value: %d\n", sensor_value);
 *     osDelay(100);
 *     dbg_mirror_update();  // Refresh OLED every 100ms - REQUIRED!
 *   }
 * @endcode
 * 
 * When MODULE_ENABLE_OLED is active:
 * - OLED mirroring is automatically enabled by test_debug_init()
 * - All dbg_print() and dbg_printf() output is captured
 * - NO UART debug output (all 4 MIDI ports available for MIDI)
 * - THIS FUNCTION MUST BE CALLED to see output on OLED
 */
void dbg_mirror_update(void);

#ifdef __cplusplus
}
#endif

#endif // TEST_DEBUG_H
