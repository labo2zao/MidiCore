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
  // Map TEST_DEBUG_UART_PORT to actual UART handles
  // Port 0 = USART2, Port 1 = USART3, Port 2 = USART1, Port 3 = UART5
  switch (TEST_DEBUG_UART_PORT) {
    case 0: return &huart2;  // USART2 PA2/PA3
    case 1: return &huart3;  // USART3 PD8/PD9
    case 2: return &huart1;  // USART1 PA9/PA10
    case 3: return &huart5;  // UART5  PC12/PD2
    default: return &huart3; // Default to USART3
  }
}

// =============================================================================
// INITIALIZATION
// =============================================================================

int test_debug_init(void)
{
  // UART handles are initialized in main.c by CubeMX
  // We need to reconfigure the debug UART to 115200 baud
  // (CubeMX initializes all UARTs to 31250 for MIDI by default)
  
  UART_HandleTypeDef* huart = get_debug_uart_handle();
  
  // Reconfigure debug UART to 115200 baud (from default 31250 MIDI baud)
  if (huart->Init.BaudRate != TEST_DEBUG_UART_BAUD) {
    HAL_UART_DeInit(huart);
    huart->Init.BaudRate = TEST_DEBUG_UART_BAUD;  // 115200 for debug
    huart->Init.WordLength = UART_WORDLENGTH_8B;
    huart->Init.StopBits = UART_STOPBITS_1;
    huart->Init.Parity = UART_PARITY_NONE;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(huart);
  }
  
#if MODULE_ENABLE_OLED && MODULE_ENABLE_UI
  // Automatically initialize and enable OLED debug mirroring when OLED is active
  oled_mirror_init();
  oled_mirror_set_enabled(1);
#endif
  
  return 0; // Success
}

// =============================================================================
// BASIC OUTPUT FUNCTIONS
// =============================================================================

void dbg_putc(char c)
{
  // Always output to UART debug
  UART_HandleTypeDef* huart = get_debug_uart_handle();
  HAL_UART_Transmit(huart, (uint8_t*)&c, 1, 100);
  
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
  
  // Always output to UART debug
  UART_HandleTypeDef* huart = get_debug_uart_handle();
  HAL_UART_Transmit(huart, (const uint8_t*)str, len, 1000);
  
  // Also mirror to OLED if enabled (optional secondary output)
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
#if MODULE_ENABLE_OLED && MODULE_ENABLE_UI
  if (oled_mirror_is_enabled()) {
    oled_mirror_update();
  }
#endif
}
