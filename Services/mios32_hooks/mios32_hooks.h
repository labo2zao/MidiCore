/**
 * @file mios32_hooks.h
 * @brief MIOS32-Style Terminal Hooks and Thread-Safe I/O
 * 
 * Implements MIOS32-compatible hooks for terminal I/O redirection and
 * thread-safe terminal access using mutex protection. This allows multiple
 * FreeRTOS tasks to safely write to the terminal without race conditions.
 * 
 * Architecture:
 * - Terminal output hook: Redirects stdout/debug output to MIOS Studio terminal
 * - Terminal input hook: Captures stdin from MIOS Studio terminal
 * - Mutex protection: Ensures thread-safe access to terminal resources
 * 
 * Usage:
 *   1. Call mios32_hooks_init() during system initialization
 *   2. Use mios32_hooks_write() for thread-safe terminal output
 *   3. Use mios32_hooks_read() for thread-safe terminal input
 * 
 * MIOS32 Compatibility:
 * - Matches MIOS32 terminal architecture
 * - Compatible with MIOS Studio terminal capture
 * - Thread-safe by design
 * - Low overhead (~10-20 bytes per call)
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize MIOS32 terminal hooks system
 * 
 * Creates terminal mutex and initializes hook infrastructure.
 * Must be called after FreeRTOS scheduler starts.
 * 
 * @return true if initialization successful, false otherwise
 */
bool mios32_hooks_init(void);

/**
 * @brief Write data to terminal with mutex protection
 * 
 * Thread-safe terminal write operation. Automatically acquires terminal
 * mutex, writes data, and releases mutex. Supports multiple destinations
 * based on MODULE_CLI_OUTPUT configuration.
 * 
 * @param data Pointer to data buffer
 * @param len Length of data in bytes
 * @return Number of bytes written, or 0 on error
 */
size_t mios32_hooks_write(const char* data, size_t len);

/**
 * @brief Read data from terminal with mutex protection
 * 
 * Thread-safe terminal read operation. Automatically acquires terminal
 * mutex, reads data, and releases mutex.
 * 
 * @param buffer Pointer to buffer to receive data
 * @param max_len Maximum number of bytes to read
 * @return Number of bytes read, or 0 if no data available
 */
size_t mios32_hooks_read(char* buffer, size_t max_len);

/**
 * @brief Acquire terminal mutex for exclusive access
 * 
 * Use for critical sections that need exclusive terminal access.
 * Must be paired with mios32_hooks_unlock().
 * 
 * @param timeout_ms Timeout in milliseconds (0 = no wait, 0xFFFFFFFF = wait forever)
 * @return true if mutex acquired, false on timeout
 */
bool mios32_hooks_lock(uint32_t timeout_ms);

/**
 * @brief Release terminal mutex
 * 
 * Releases terminal mutex after exclusive access section.
 * Must be called after successful mios32_hooks_lock().
 */
void mios32_hooks_unlock(void);

/**
 * @brief Check if terminal hooks are initialized
 * 
 * @return true if hooks system is ready, false otherwise
 */
bool mios32_hooks_is_initialized(void);

/**
 * @brief Get terminal mutex statistics
 * 
 * Returns diagnostic information about terminal mutex usage.
 * 
 * @param lock_count Pointer to receive total lock count (NULL to ignore)
 * @param timeout_count Pointer to receive timeout count (NULL to ignore)
 * @param contention_count Pointer to receive contention count (NULL to ignore)
 */
void mios32_hooks_get_stats(uint32_t* lock_count, uint32_t* timeout_count, uint32_t* contention_count);

#ifdef __cplusplus
}
#endif
