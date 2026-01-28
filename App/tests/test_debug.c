/**
 * @file test_debug.c
 * @brief Implementation of MIOS32-compatible debug output
 */

#include "App/tests/test_debug.h"
#include "App/tests/test_oled_mirror.h"
#include "Config/module_config.h"
#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#if MODULE_ENABLE_OLED
#include "Hal/oled_ssd1322/oled_ssd1322.h"  // For oled_init_newhaven()
#endif

#if MODULE_ENABLE_USB_CDC
#include "Services/usb_cdc/usb_cdc.h"  // For USB CDC output
#endif

// External UART handles from main.c
extern UART_HandleTypeDef huart1; // USART1 - MIDI DIN3 (PA9/PA10) or Debug in test mode
extern UART_HandleTypeDef huart2; // USART2 - MIDI DIN1 (PA2/PA3)
extern UART_HandleTypeDef huart3; // USART3 - MIDI DIN2 (PD8/PD9)
extern UART_HandleTypeDef huart5; // UART5  - MIDI DIN4 (PC12/PD2)

// =============================================================================
// UART HANDLE SELECTION
// =============================================================================

static UART_HandleTypeDef* get_debug_uart_handle(void)
{
  // Map TEST_DEBUG_UART_PORT to actual UART handles
  // MIDI DIN ports (0-3): USART2, USART3, USART1, UART5
  // Port 2 (USART1) can be used for either MIDI DIN3 or Debug
  switch (TEST_DEBUG_UART_PORT) {
    case 0: return &huart2;  // USART2 PA2/PA3   [MIDI DIN1 - MIOS32 UART1]
    case 1: return &huart3;  // USART3 PD8/PD9   [MIDI DIN2 - MIOS32 UART2]
    case 2: return &huart1;  // USART1 PA9/PA10  [MIDI DIN3 - MIOS32 UART3 / Debug]
    case 3: return &huart5;  // UART5  PC12/PD2  [MIDI DIN4 - MIOS32 UART4]
    default: return &huart1; // Default to USART1 for debug
  }
}

// =============================================================================
// INITIALIZATION
// =============================================================================

int test_debug_init(void)
{
#if MODULE_ENABLE_USB_CDC
  // USB CDC is already initialized in main.c before this function
  // Just print confirmation message via USB CDC
  dbg_print("\r\n==============================================\r\n");
  dbg_print("Debug output routed to USB CDC (Virtual COM)\r\n");
  dbg_print("==============================================\r\n");
#else
  // Fallback to UART if USB CDC not available
  // UART handles are initialized in main.c by CubeMX to 31250 (MIDI baud)
  // In TEST MODE, we reconfigure the debug UART to 115200 baud
  // In PRODUCTION MODE, all UARTs stay at 31250 baud
  
  UART_HandleTypeDef* huart = get_debug_uart_handle();
  
  // CRITICAL: Reconfigure to 115200 BEFORE any dbg_print() calls!
  // Do NOT call dbg_print() before this reconfiguration!
  HAL_UART_DeInit(huart);
  huart->Init.BaudRate = TEST_DEBUG_UART_BAUD;  // 115200 for debug
  huart->Init.WordLength = UART_WORDLENGTH_8B;
  huart->Init.StopBits = UART_STOPBITS_1;
  huart->Init.Parity = UART_PARITY_NONE;
  huart->Init.Mode = UART_MODE_TX_RX;
  huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart->Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(huart) != HAL_OK) {
    Error_Handler();
  }
  
  // NOW we can print at 115200 baud
  dbg_print("\r\n==============================================\r\n");
  dbg_print("Debug UART initialized at 115200 baud\r\n");
  dbg_print("==============================================\r\n");
#endif
  
#if MODULE_ENABLE_OLED
  // Always initialize OLED for debug mirroring (enabled by default)
  // This provides visual feedback even if UART debug is not connected
  
  dbg_print("Initializing OLED hardware (NHD-3.12-25664)...\r\n");
  
  // Initialize OLED hardware (production-grade Newhaven NHD-3.12-25664 init)
  oled_init_newhaven();
  
  dbg_print("OLED hardware initialized, initializing text display...\r\n");
  
  // Initialize mirroring (framebuffer-based text display)
  oled_mirror_init();
  oled_mirror_set_enabled(1);  // Enable for text output
  
  dbg_print("OLED mirroring initialized, printing test text...\r\n");
  
  // Print test text directly to OLED
  oled_mirror_print("*** MidiCore OLED Test ***\r\n");
  oled_mirror_print("Hardware: STM32F407VGT6\r\n");
  oled_mirror_print("Display: NHD-3.12-25664\r\n");
  oled_mirror_print("Status: READY\r\n");
  oled_mirror_print("Debug output active...\r\n");
  oled_mirror_print("\r\n");
  oled_mirror_print("You should see this text!\r\n");
  
  // Update the display to show the text
  dbg_mirror_update();
  
  dbg_print("OLED test text displayed, debug mirroring ready\r\n");
#else
  // OLED not compiled in - debug output only via UART
  dbg_print("OLED disabled (MODULE_ENABLE_OLED=0), using UART debug only\r\n");
#endif
  
  return 0; // Success
}

// =============================================================================
// BASIC OUTPUT FUNCTIONS
// =============================================================================

void dbg_putc(char c)
{
#if MODULE_ENABLE_USB_CDC
  // Primary output: USB CDC (virtual COM port)
  usb_cdc_send((const uint8_t*)&c, 1);
#else
  // Fallback to UART if USB CDC not available
  UART_HandleTypeDef* huart = get_debug_uart_handle();
  HAL_UART_Transmit(huart, (uint8_t*)&c, 1, 100);
#endif
  
  // Also mirror to OLED if enabled (optional secondary output)
  if (oled_mirror_is_enabled()) {
    char str[2] = {c, '\0'};
    oled_mirror_print(str);
  }
}

void dbg_print(const char* str)
{
  if (!str) return;
  
  size_t len = strlen(str);
  if (len == 0) return;
  
#if MODULE_ENABLE_USB_CDC
  // Primary output: USB CDC (virtual COM port) for TeraTerm, etc.
  usb_cdc_send((const uint8_t*)str, len);
#else
  // Fallback to UART if USB CDC not available
  UART_HandleTypeDef* huart = get_debug_uart_handle();
  HAL_UART_Transmit(huart, (const uint8_t*)str, len, 1000);
#endif

#if MODULE_ENABLE_USB_MIDI
  // Secondary output: MIOS32 debug message via USB MIDI for MIOS Studio terminal
  // Send as MIOS32 SysEx: F0 00 00 7E 32 00 0D <text> F7
  // NOTE: Delayed start to avoid interfering with USB enumeration and MIOS32 queries
  extern bool mios32_debug_send_message(const char* text, uint8_t cable);
  
  // CRITICAL: Check if we're in interrupt context
  // NEVER send USB MIDI from ISR - causes reentrancy issues and breaks USB stack!
  // Check IPSR register (Interrupt Program Status Register)
  // IPSR = 0 means thread mode (safe to send)
  // IPSR != 0 means exception/interrupt mode (NOT safe to send)
  uint32_t ipsr = __get_IPSR();
  bool in_interrupt = (ipsr != 0);
  
  if (in_interrupt) {
    // We're in ISR context - DO NOT send MIOS32 debug!
    // USB MIDI transmission from ISR causes reentrancy with RX ISR and breaks USB
    // CDC debug is fine from ISR (different mechanism)
    return;
  }
  
  // Rate limiting to prevent flooding USB MIDI bandwidth
  // Allow maximum 10 messages per second (100ms interval)
  // This allows debug monitoring without blocking normal MIDI traffic
  static uint32_t start_tick = 0;
  static uint32_t last_send_tick = 0;
  static uint32_t dropped_count = 0;
  static uint32_t sent_count = 0;
  
  // Get current tick on first call
  // CRITICAL: Use HAL_GetTick() not osKernelGetTickCount() - works before RTOS starts!
  if (start_tick == 0) {
    start_tick = HAL_GetTick();
  }
  
  uint32_t now = HAL_GetTick();
  uint32_t elapsed_since_boot = now - start_tick;
  
  // Wait 3 seconds after boot before sending any debug to MIOS Studio
  // This allows USB enumeration and MIOS32 query processing to complete first
  if (elapsed_since_boot >= 3000) {
    // Send test message once after boot delay
    static bool test_msg_sent = false;
    if (!test_msg_sent) {
      mios32_debug_send_message("\r\n*** MIOS Terminal Test ***\r\n", 0);
      test_msg_sent = true;
      sent_count++;
    }
    
    // Rate limiting: Allow maximum 10 messages per second
    // This prevents flooding USB MIDI bandwidth while still providing debug output
    #define DEBUG_MSG_MIN_INTERVAL_MS 100  // 100ms = 10 msg/sec
    
    uint32_t elapsed_since_last = now - last_send_tick;
    
    if (elapsed_since_last >= DEBUG_MSG_MIN_INTERVAL_MS) {
      // Enough time has passed, send this message
      bool sent = mios32_debug_send_message(str, 0);
      
      if (sent) {
        last_send_tick = now;
        sent_count++;
        
        // Report counters periodically (every 100 messages sent, only to CDC)
        if (sent_count % 100 == 0) {
          char stats_msg[120];
          snprintf(stats_msg, sizeof(stats_msg), 
                   "[MIOS Stats] Sent:%lu Dropped:%lu\r\n", 
                   (unsigned long)sent_count, (unsigned long)dropped_count);
          usb_cdc_send((uint8_t*)stats_msg, strlen(stats_msg));
        }
        
        // If we dropped messages, report it once (only to CDC to avoid recursion)
        if (dropped_count > 0) {
          char drop_msg[80];
          snprintf(drop_msg, sizeof(drop_msg), 
                   "[DBG] %lu debug messages dropped (rate limiting)\r\n", 
                   (unsigned long)dropped_count);
#if MODULE_ENABLE_USB_CDC
          usb_cdc_send((const uint8_t*)drop_msg, strlen(drop_msg));
#endif
          dropped_count = 0;
        }
      }
    } else {
      // Too soon since last message, drop this one (rate limiting active)
      dropped_count++;
    }
  }
#endif
  
  // Also mirror to OLED if enabled (optional tertiary output)
  if (oled_mirror_is_enabled()) {
    oled_mirror_print(str);
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

static const char* gpio_port_name(GPIO_TypeDef* port)
{
  if (port == GPIOA) return "GPIOA";
  if (port == GPIOB) return "GPIOB";
  if (port == GPIOC) return "GPIOC";
  if (port == GPIOD) return "GPIOD";
  if (port == GPIOE) return "GPIOE";
  if (port == GPIOF) return "GPIOF";
  if (port == GPIOG) return "GPIOG";
  if (port == GPIOH) return "GPIOH";
  if (port == GPIOI) return "GPIOI";
  return "GPIO?";
}

static int gpio_pin_index(uint16_t pin)
{
  for (int i = 0; i < 16; i++) {
    if (pin & (1u << i)) {
      return i;
    }
  }
  return -1;
}

static const char* spi_cpol_name(SPI_HandleTypeDef* hspi);
static const char* spi_cpha_name(SPI_HandleTypeDef* hspi);
static const char* spi_prescaler_name(SPI_HandleTypeDef* hspi);

static void dbg_print_gpio_pin(const char* label,
                               GPIO_TypeDef* port,
                               uint16_t pin,
                               SPI_HandleTypeDef* hspi)
{
  dbg_print("  ");
  dbg_print(label);
  dbg_print(": ");

  if (!port || !pin) {
    dbg_print("n/a");
  } else {
    dbg_print(gpio_port_name(port));

    int index = gpio_pin_index(pin);
    if (index >= 0) {
      dbg_print_uint((uint32_t)index);
    } else {
      dbg_print("0x");
      dbg_print_hex16(pin);
    }
  }
  if (hspi) {
    dbg_print(" (CPOL=");
    dbg_print(spi_cpol_name(hspi));
    dbg_print(", CPHA=");
    dbg_print(spi_cpha_name(hspi));
    dbg_print(", Prescaler=");
    dbg_print(spi_prescaler_name(hspi));
    dbg_print(")");
  }
  dbg_print("\r\n");
}

static const char* spi_instance_name(SPI_HandleTypeDef* hspi)
{
  if (!hspi || !hspi->Instance) {
    return "UNKNOWN";
  }
  if (hspi->Instance == SPI1) return "SPI1";
  if (hspi->Instance == SPI2) return "SPI2";
  if (hspi->Instance == SPI3) return "SPI3";
  return "SPI?";
}

static const char* spi_cpol_name(SPI_HandleTypeDef* hspi)
{
  if (!hspi) {
    return "n/a";
  }
  switch (hspi->Init.CLKPolarity) {
    case SPI_POLARITY_LOW:
      return "LOW";
    case SPI_POLARITY_HIGH:
      return "HIGH";
    default:
      return "UNKNOWN";
  }
}

static const char* spi_cpha_name(SPI_HandleTypeDef* hspi)
{
  if (!hspi) {
    return "n/a";
  }
  switch (hspi->Init.CLKPhase) {
    case SPI_PHASE_1EDGE:
      return "1EDGE";
    case SPI_PHASE_2EDGE:
      return "2EDGE";
    default:
      return "UNKNOWN";
  }
}

static const char* spi_prescaler_name(SPI_HandleTypeDef* hspi)
{
  if (!hspi) {
    return "n/a";
  }
  switch (hspi->Init.BaudRatePrescaler) {
    case SPI_BAUDRATEPRESCALER_2:
      return "2";
    case SPI_BAUDRATEPRESCALER_4:
      return "4";
    case SPI_BAUDRATEPRESCALER_8:
      return "8";
    case SPI_BAUDRATEPRESCALER_16:
      return "16";
    case SPI_BAUDRATEPRESCALER_32:
      return "32";
    case SPI_BAUDRATEPRESCALER_64:
      return "64";
    case SPI_BAUDRATEPRESCALER_128:
      return "128";
    case SPI_BAUDRATEPRESCALER_256:
      return "256";
    default:
      return "UNKNOWN";
  }
}

void gdb_ptin_SPI_Pinout(const char* label,
                         SPI_HandleTypeDef* hspi,
                         GPIO_TypeDef* sck_port, uint16_t sck_pin,
                         GPIO_TypeDef* miso_port, uint16_t miso_pin,
                         GPIO_TypeDef* mosi_port, uint16_t mosi_pin,
                         GPIO_TypeDef* rc1_port, uint16_t rc1_pin,
                         GPIO_TypeDef* rc2_port, uint16_t rc2_pin)
{
  dbg_print("SPI Pinout");
  if (label && label[0] != '\0') {
    dbg_print(" (");
    dbg_print(label);
    dbg_print(")");
  }
  dbg_print(":\r\n");
  dbg_printf("  SPI Instance: %s\r\n", spi_instance_name(hspi));
  dbg_printf("  SPI CPOL: %s\r\n", spi_cpol_name(hspi));
  dbg_printf("  SPI CPHA: %s\r\n", spi_cpha_name(hspi));
  dbg_printf("  SPI Prescaler: %s\r\n", spi_prescaler_name(hspi));
  dbg_print_gpio_pin("SPI SCK", sck_port, sck_pin, hspi);
  dbg_print_gpio_pin("SPI MISO", miso_port, miso_pin, hspi);
  dbg_print_gpio_pin("SPI MOSI", mosi_port, mosi_pin, hspi);
  dbg_print_gpio_pin("SPI RC1", rc1_port, rc1_pin, hspi);
  dbg_print_gpio_pin("SPI RC2", rc2_port, rc2_pin, hspi);
}

// =============================================================================
// OLED MIRROR SUPPORT
// =============================================================================

/**
 * @brief Update OLED mirror display
 * 
 * Call this periodically (e.g. every 100ms) to refresh the OLED screen
 * with mirrored debug output.
 * 
 * Note: OLED mirroring is automatically enabled when MODULE_ENABLE_OLED
 * is active and test_debug_init() is called.
 */
void dbg_mirror_update(void)
{
#if MODULE_ENABLE_OLED
  if (oled_mirror_is_enabled()) {
    oled_mirror_update();
  }
#endif
}
