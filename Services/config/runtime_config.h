/**
 * @file runtime_config.h
 * @brief Runtime configuration management for production use
 * 
 * Provides INI-style configuration file management that can be used
 * in both testing and production environments.
 * 
 * Features:
 * - Load/save configurations from SD card
 * - INI-style human-readable format
 * - Key-value pairs with sections
 * - Safe parsing with validation
 * - No recompilation needed for config changes
 * 
 * Usage in production:
 * - User-configurable parameters
 * - Runtime behavior modification
 * - Per-device configurations
 * - A/B testing configurations
 */

#ifndef RUNTIME_CONFIG_H
#define RUNTIME_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// =============================================================================
// CONFIGURATION
// =============================================================================

#ifndef CONFIG_MAX_ENTRIES
#define CONFIG_MAX_ENTRIES 64  // Maximum config entries
#endif

#ifndef CONFIG_MAX_KEY_LEN
#define CONFIG_MAX_KEY_LEN 64  // Maximum key length
#endif

#ifndef CONFIG_MAX_VALUE_LEN
#define CONFIG_MAX_VALUE_LEN 128  // Maximum value length
#endif

// =============================================================================
// TYPES
// =============================================================================

/**
 * @brief Configuration entry
 */
typedef struct {
  char key[CONFIG_MAX_KEY_LEN];
  char value[CONFIG_MAX_VALUE_LEN];
  const char* section;  // Section name (or NULL for global)
} config_entry_t;

/**
 * @brief Configuration change callback
 * @param key Configuration key that changed
 * @param old_value Previous value (NULL if new key)
 * @param new_value New value (NULL if deleted)
 */
typedef void (*config_change_callback_t)(const char* key, 
                                         const char* old_value,
                                         const char* new_value);

// =============================================================================
// API - INITIALIZATION
// =============================================================================

/**
 * @brief Initialize runtime configuration system
 * @return 0 on success, negative on error
 */
int runtime_config_init(void);

/**
 * @brief Load configuration from file
 * @param filename Config file path (e.g., "0:/config.ini")
 * @return 0 on success, negative on error
 */
int runtime_config_load(const char* filename);

/**
 * @brief Save configuration to file
 * @param filename Output file path
 * @return 0 on success, negative on error
 */
int runtime_config_save(const char* filename);

// =============================================================================
// API - GET VALUES
// =============================================================================

/**
 * @brief Get string value
 * @param key Configuration key
 * @param default_value Default if not found (can be NULL)
 * @return Value string or default
 */
const char* runtime_config_get_string(const char* key, const char* default_value);

/**
 * @brief Get integer value
 * @param key Configuration key
 * @param default_value Default if not found or invalid
 * @return Integer value
 */
int32_t runtime_config_get_int(const char* key, int32_t default_value);

/**
 * @brief Get boolean value
 * @param key Configuration key
 * @param default_value Default if not found
 * @return 1 for true, 0 for false
 */
uint8_t runtime_config_get_bool(const char* key, uint8_t default_value);

/**
 * @brief Get float value
 * @param key Configuration key
 * @param default_value Default if not found or invalid
 * @return Float value
 */
float runtime_config_get_float(const char* key, float default_value);

// =============================================================================
// API - SET VALUES
// =============================================================================

/**
 * @brief Set string value
 * @param key Configuration key
 * @param value Value to set
 * @return 0 on success, negative on error
 */
int runtime_config_set_string(const char* key, const char* value);

/**
 * @brief Set integer value
 * @param key Configuration key
 * @param value Value to set
 * @return 0 on success, negative on error
 */
int runtime_config_set_int(const char* key, int32_t value);

/**
 * @brief Set boolean value
 * @param key Configuration key
 * @param value Value to set (0 or 1)
 * @return 0 on success, negative on error
 */
int runtime_config_set_bool(const char* key, uint8_t value);

/**
 * @brief Set float value
 * @param key Configuration key
 * @param value Value to set
 * @return 0 on success, negative on error
 */
int runtime_config_set_float(const char* key, float value);

// =============================================================================
// API - MANAGEMENT
// =============================================================================

/**
 * @brief Check if key exists
 * @param key Configuration key
 * @return 1 if exists, 0 otherwise
 */
uint8_t runtime_config_exists(const char* key);

/**
 * @brief Delete a configuration entry
 * @param key Configuration key
 * @return 0 on success, negative if not found
 */
int runtime_config_delete(const char* key);

/**
 * @brief Clear all configuration entries
 */
void runtime_config_clear(void);

/**
 * @brief Register callback for configuration changes
 * @param callback Function to call when config changes
 */
void runtime_config_set_change_callback(config_change_callback_t callback);

// =============================================================================
// API - DEBUGGING
// =============================================================================

/**
 * @brief Print all configuration entries to UART
 */
void runtime_config_print(void);

/**
 * @brief Get number of entries
 * @return Count of configuration entries
 */
uint32_t runtime_config_get_count(void);

#ifdef __cplusplus
}
#endif

#endif // RUNTIME_CONFIG_H
