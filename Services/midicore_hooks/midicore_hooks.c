/**
 * @file midicore_hooks.c
 * @brief MIOS32-Style Terminal Hooks Implementation
 * 
 * MIOS32 PRINCIPLES:
 * - NO printf / dbg_printf
 * - Thread-safe terminal I/O
 * - Minimal stack usage
 */

#include "midicore_hooks.h"
#include "cmsis_os2.h"
#include "Config/module_config.h"
#include "Services/midicore_query/midicore_query.h"
#include "Services/usb_cdc/usb_cdc.h"

#include <string.h>

/* Terminal mutex for thread-safe I/O */
static osMutexId_t g_terminal_mutex = NULL;
static volatile bool g_hooks_initialized = false;

/* Statistics (visible in debugger) */
static volatile uint32_t g_lock_count = 0;
static volatile uint32_t g_timeout_count = 0;
static volatile uint32_t g_contention_count = 0;

bool midicore_hooks_init(void)
{
  if (g_hooks_initialized) {
    return true;
  }

  const osMutexAttr_t mutex_attr = {
    .name = "terminal",
    .attr_bits = osMutexRecursive,
    .cb_mem = NULL,
    .cb_size = 0U
  };

  g_terminal_mutex = osMutexNew(&mutex_attr);
  if (g_terminal_mutex == NULL) {
    return false;  /* Mutex creation failed - visible in debugger */
  }

  g_hooks_initialized = true;
  return true;
}

size_t midicore_hooks_write(const char* data, size_t len)
{
  if (!g_hooks_initialized || data == NULL || len == 0) {
    return 0;
  }

  osStatus_t status = osMutexAcquire(g_terminal_mutex, 100);
  if (status != osOK) {
    g_timeout_count++;
    return 0;
  }

  g_lock_count++;
  size_t written = 0;

  /* Route to appropriate terminal based on CLI configuration */
#if MODULE_CLI_OUTPUT == CLI_OUTPUT_MIOS
  if (midicore_debug_send_message(data, 0)) {
    written = len;
  }
#elif MODULE_CLI_OUTPUT == CLI_OUTPUT_USB_CDC
  if (usb_cdc_send((const uint8_t*)data, len)) {
    written = len;
  }
#else
  /* UART/Debug modes disabled in MIOS32-style build */
  (void)data;
  written = 0;
#endif

  osMutexRelease(g_terminal_mutex);
  return written;
}

size_t midicore_hooks_read(char* buffer, size_t max_len)
{
  if (!g_hooks_initialized || buffer == NULL || max_len == 0) {
    return 0;
  }

  // Acquire mutex
  osStatus_t status = osMutexAcquire(g_terminal_mutex, 100);
  if (status != osOK) {
    g_timeout_count++;
    return 0;
  }

  g_lock_count++;
  size_t read_bytes = 0;

  // Read from appropriate source based on CLI configuration
  // Note: This is placeholder - actual implementation depends on input architecture
  // For now, return 0 (no data available)

  // Release mutex
  osMutexRelease(g_terminal_mutex);

  return read_bytes;
}

/**
 * @brief Acquire terminal mutex for exclusive access
 */
bool midicore_hooks_lock(uint32_t timeout_ms)
{
  if (!g_hooks_initialized) {
    return false;
  }

  // Check if already locked by this task (contention detection)
  osThreadId_t current_task = osThreadGetId();
  osThreadId_t mutex_owner = osMutexGetOwner(g_terminal_mutex);
  
  if (mutex_owner != NULL && mutex_owner != current_task) {
    g_contention_count++;
  }

  osStatus_t status = osMutexAcquire(g_terminal_mutex, timeout_ms);
  if (status == osOK) {
    g_lock_count++;
    return true;
  }

  g_timeout_count++;
  return false;
}

/**
 * @brief Release terminal mutex
 */
void midicore_hooks_unlock(void)
{
  if (g_hooks_initialized) {
    osMutexRelease(g_terminal_mutex);
  }
}

/**
 * @brief Check if terminal hooks are initialized
 */
bool midicore_hooks_is_initialized(void)
{
  return g_hooks_initialized;
}

/**
 * @brief Get terminal mutex statistics
 */
void midicore_hooks_get_stats(uint32_t* lock_count, uint32_t* timeout_count, uint32_t* contention_count)
{
  if (lock_count) {
    *lock_count = g_lock_count;
  }
  if (timeout_count) {
    *timeout_count = g_timeout_count;
  }
  if (contention_count) {
    *contention_count = g_contention_count;
  }
}
