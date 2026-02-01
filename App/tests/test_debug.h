/**
 * @file test_debug.h
 * @brief MIOS32-compatible debug output for module tests
 * 
 * This module provides debug print functions compatible with MidiCore hardware.
 * 
 * **USB CDC Debug Mode (when MODULE_ENABLE_USB_CDC=1 - RECOMMENDED):**
 * - Debug output routed to USB CDC (Virtual COM port)
 * - No UART reconfiguration needed - all MIDI DIN ports stay at 31250 baud
 * - Works with MIOS Studio terminal or any serial terminal software
 * - Cleaner hardware setup - no separate UART cable needed
 * - All 4 MIDI DIN ports available for MIDI @ 31250 baud
 * 
 * **Test Mode UART Debug (when MODULE_ENABLE_USB_CDC=0):**
 * - Debug UART: UART5 (PC12/PD2) reconfigured to 115200 baud in test mode
 * - Production: All UARTs at 31250 baud for MIDI
 * - Test mode: test_debug_init() changes debug UART to 115200 baud
 * - 2 MIDI DIN ports available for MIDI @ 31250 baud (DIN1, DIN2)
 * - PA9/PA10 (USART1) reserved for USB OTG
 * - Baud rate automatically configured by test_debug_init()
 * 
 * **OLED Debug Mode (when MODULE_ENABLE_OLED active - OPTIONAL):**
 * - Debug output ALSO mirrored to OLED display
 * - Works with both USB CDC and UART modes
 * - Call dbg_mirror_update() periodically to refresh OLED
 * 
 * **Production Mode - 3 MIDI DIN ports @ 31250 baud:**
 * - Port 0 (DIN1): USART2 PA2=TX,  PA3=RX   @ 31250 baud [MidiCore UART1]
 * - Port 1 (DIN2): USART3 PD8=TX,  PD9=RX   @ 31250 baud [MidiCore UART2]
 * - Port 2 (DIN3): Reserved for USB OTG (PA9/PA10)
 * - Port 3 (DIN4): UART5  PC12=TX, PD2=RX   @ 31250 baud [MidiCore UART4]
 * 
 * **Important**: 
 * - When USB CDC is enabled, test_debug_init() is optional (no UART reconfig needed)
 * - When USB CDC is disabled, call test_debug_init() before using debug functions
 * - Do NOT call test_debug_init() in production mode!
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
// UART PORT CONFIGURATION (MidiCore Compatible)
// =============================================================================

/**
 * @brief UART port used for debug output (dbg_print)
 * 
 * STM32F4 Discovery UART Mapping:
 * 0 = USART2 - PA2/PA3   - MIDI DIN1 [MidiCore UART1]
 * 1 = USART3 - PD8/PD9   - MIDI DIN2 [MidiCore UART2] - **RECOMMENDED FOR DEBUG**
 * 2 = USART1 - PA9/PA10  - USB OTG (not available for MIDI/debug)
 * 3 = UART5  - PC12/PD2  - MIDI DIN4 [MidiCore UART4]
 * 
 * **MidiCore COMPATIBILITY: Port 1 (USART3, PD8/PD9) is recommended**
 * - Matches MidiCore terminal configuration (avoids MIDI port conflicts)
 * - USART2 (PA2/PA3) would also work but is typically used for MIDI DIN
 * - Configure at 115200 baud at compile time for best results
 * 
 * **Alternative: Runtime reconfiguration (less reliable)**
 * Call test_debug_init() to reconfigure at runtime.
 * 
 * When a UART is configured as TEST_DEBUG_UART_PORT:
 * - It runs at 115200 baud for debug output
 * - All other MIDI DIN ports run at 31250 baud for MIDI
 */
#ifndef TEST_DEBUG_UART_PORT
#define TEST_DEBUG_UART_PORT 3  // UART5 (PC12/PD2) - User's debug port configuration
#endif

/**
 * @brief UART port used for MIDI DIN communication
 * 
 * MidiCore UART Mapping (same as above)
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
 * **When MODULE_ENABLE_USB_CDC=1 (Recommended):**
 * - Debug output automatically routed to USB CDC (Virtual COM port)
 * - This function is optional - no UART reconfiguration needed
 * - Call it if you want OLED mirroring or startup messages
 * 
 * **When MODULE_ENABLE_USB_CDC=0 (Fallback):**
 * - Reconfigures TEST_DEBUG_UART_PORT to 115200 baud for debug output
 * - All other UARTs remain at 31250 baud for MIDI
 * - MUST call this before using dbg_print() functions
 * 
 * **Behavior:**
 * - USB CDC mode: Prints confirmation message via USB CDC
 * - UART mode: Reconfigures UART to 115200 baud, then prints confirmation
 * - If MODULE_ENABLE_OLED is active, also initializes OLED mirroring
 * 
 * **Production Mode**: Do not call this function! Leave all UARTs at 31250 baud.
 * 
 * @return 0 on success, negative on error
 */
int test_debug_init(void);

/**
 * @brief Print a string to debug output (MIOS32-compatible)
 * @param str String to print (null-terminated)
 * 
 * Output routing:
 * - When MODULE_ENABLE_USB_CDC=1: Sends to USB CDC (Virtual COM port)
 * - When MODULE_ENABLE_USB_CDC=0: Sends to TEST_DEBUG_UART_PORT
 * - When MODULE_ENABLE_OLED=1: Also mirrors to OLED display
 * 
 * Example:
 *   dbg_print("AINSER64 test started\n");
 */
void dbg_print(const char* str);

/**
 * @brief Print formatted string to debug output (printf-style)
 * 
 * DEPRECATED: dbg_printf removed due to stack overflow risk (uses 500+ bytes stack).
 * Use MIOS32-style alternatives instead:
 *   - dbg_print("string") for fixed strings
 *   - dbg_print_u32(n) for unsigned numbers
 *   - dbg_print_hex8/16/32(n) for hex values
 * 
 * Example (old - DON'T USE):
 *   dbg_printf("Channel %d: value=%d\n", ch, val);
 * 
 * Example (new - USE THIS):
 *   dbg_print("Channel "); dbg_print_u32(ch); dbg_print(": value="); dbg_print_u32(val); dbg_print("\n");
 */
/* dbg_printf REMOVED - causes stack overflow in ISR/callback context */

/**
 * @brief Print a single character to debug output
 * @param c Character to print
 * 
 * Output routing:
 * - When MODULE_ENABLE_USB_CDC=1: Sends to USB CDC (Virtual COM port)
 * - When MODULE_ENABLE_USB_CDC=0: Sends to TEST_DEBUG_UART_PORT
 * - When MODULE_ENABLE_OLED=1: Also mirrors to OLED display
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
 * @brief Print an unsigned 32-bit number (alias for dbg_print_uint)
 * @param n Number to print
 * 
 * MIOS32-STYLE: Consistent naming with cli_print_u32()
 */
#define dbg_print_u32(n) dbg_print_uint(n)

/**
 * @brief Print a signed decimal number
 * @param n Number to print
 */
void dbg_print_int(int32_t n);

/**
 * @brief Print a signed 32-bit number (alias for dbg_print_int)
 * @param n Number to print
 * 
 * MIOS32-STYLE: Consistent naming with cli_print_i32()
 */
#define dbg_print_i32(n) dbg_print_int(n)

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
// MidiCore COMPATIBILITY MACROS
// =============================================================================

/**
 * @brief MIOS32-style debug message macro
 * 
 * DEPRECATED: DEBUG_MSG now only outputs the first argument as a fixed string.
 * For formatted output, use MIOS32-style helpers:
 *   dbg_print("string"), dbg_print_u32(n), dbg_print_hex8(n), etc.
 * 
 * Old usage (no longer works for formatted strings):
 *   DEBUG_MSG("Value: %d\n", value);
 * 
 * New usage:
 *   dbg_print("Value: "); dbg_print_u32(value); dbg_print("\n");
 */
#define DEBUG_MSG(fmt, ...) dbg_print(fmt)

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
 * When MODULE_ENABLE_OLED is active:
 * - OLED mirroring is automatically enabled by test_debug_init()
 * - Debug output appears on BOTH UART and OLED
 * - Call this function periodically to refresh OLED display
 * 
 * When MODULE_ENABLE_OLED is NOT active:
 * - This function does nothing
 * - Debug output only appears on UART
 */
void dbg_mirror_update(void);

#ifdef __cplusplus
}
#endif

#endif // TEST_DEBUG_H
