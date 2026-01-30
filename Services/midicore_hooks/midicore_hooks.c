/**
 * @file midicore_hooks.c
 * @brief MIOS32-Style Terminal Hooks Implementation
 */

#include "midicore_hooks.h"
#include "cmsis_os2.h"
#include "Config/module_config.h"
#include "Services/midicore_query/midicore_query.h"
#include "Services/usb_cdc/usb_cdc.h"
#include "App/tests/test_debug.h"

#include <string.h>

// Terminal mutex for thread-safe I/O
static osMutexId_t g_terminal_mutex = NULL;
static volatile bool g_hooks_initialized = false;

// Statistics
static volatile uint32_t g_lock_count = 0;
static volatile uint32_t g_timeout_count = 0;
static volatile uint32_t g_contention_count = 0;

/**
 * @brief Initialize MidiCore terminal hooks system
 */
bool midicore_hooks_init(void)
{
  if (g_hooks_initialized) {
    return true;  // Already initialized
  }

  // Create terminal mutex with name for debugging
  const osMutexAttr_t mutex_attr = {
    .name = "terminal",
    .attr_bits = osMutexRecursive,  // Allow recursive locking from same task
    .cb_mem = NULL,
    .cb_size = 0U
  };

  g_terminal_mutex = osMutexNew(&mutex_attr);
  if (g_terminal_mutex == NULL) {
    dbg_printf("[MIDICORE-HOOKS] ERROR: Failed to create terminal mutex\r\n");
    return false;
  }

  g_hooks_initialized = true;
  dbg_printf("[MIDICORE-HOOKS] Terminal hooks initialized\r\n");
  dbg_printf("[MIDICORE-HOOKS] Mutex: %p (recursive)\r\n", (void*)g_terminal_mutex);

  return true;
}

/**
 * @brief Write data to terminal with mutex protection
 */
size_t midicore_hooks_write(const char* data, size_t len)
{
  if (!g_hooks_initialized || data == NULL || len == 0) {
    return 0;
  }

  // Acquire mutex with timeout to prevent deadlock
  osStatus_t status = osMutexAcquire(g_terminal_mutex, 100);  // 100ms timeout
  if (status != osOK) {
    g_timeout_count++;
    return 0;  // Timeout or error
  }

  g_lock_count++;
  size_t written = 0;

  // Route to appropriate terminal based on CLI configuration
#if MODULE_CLI_OUTPUT == CLI_OUTPUT_MIOS
  // MIOS Studio terminal - use SysEx protocol
  if (midicore_debug_send_message(data, 0)) {
    written = len;
  }
#elif MODULE_CLI_OUTPUT == CLI_OUTPUT_USB_CDC
  // USB CDC text terminal
  if (usb_cdc_send((const uint8_t*)data, len)) {
    written = len;
  }
#elif MODULE_CLI_OUTPUT == CLI_OUTPUT_UART || MODULE_CLI_OUTPUT == CLI_OUTPUT_DEBUG
  // UART or debug output
  dbg_print(data, len);
  written = len;
#else
  // Default: debug output
  dbg_print(data, len);
  written = len;
#endif

  // Release mutex
  osMutexRelease(g_terminal_mutex);

  return written;
}

/**
 * @brief Read data from terminal with mutex protection
 */
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
