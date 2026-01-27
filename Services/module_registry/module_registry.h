/**
 * @file module_registry.h
 * @brief Module Registry - Central registry for all MidiCore modules
 * 
 * Provides a centralized system for registering, discovering, and managing
 * all firmware modules. Each module can register itself with metadata about
 * its capabilities, parameters, and state.
 * 
 * Features:
 * - Module discovery and enumeration
 * - Enable/disable module control
 * - Parameter registration and access
 * - Module status reporting
 * - Integration with CLI and UI systems
 * 
 * Architecture:
 * - Each module registers at init time
 * - Modules define their parameters with types and ranges
 * - CLI can query and modify any module parameter
 * - UI can build dynamic menus from registry
 * 
 * Usage:
 * 1. Call module_registry_init() at startup
 * 2. Each module calls module_registry_register() with its descriptor
 * 3. CLI/UI queries registry for available modules and parameters
 * 4. Users control modules via CLI commands or UI menus
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONFIGURATION
// =============================================================================

#ifndef MODULE_REGISTRY_MAX_MODULES
#define MODULE_REGISTRY_MAX_MODULES 32  // Maximum number of modules (reduced from 64 to save ~165 KB RAM)
#endif

#ifndef MODULE_REGISTRY_MAX_PARAMS
#define MODULE_REGISTRY_MAX_PARAMS 8   // Maximum parameters per module (reduced from 16)
#endif

#ifndef MODULE_REGISTRY_MAX_NAME_LEN
#define MODULE_REGISTRY_MAX_NAME_LEN 24  // Maximum module name length (reduced from 32)
#endif

#ifndef MODULE_REGISTRY_MAX_DESC_LEN
#define MODULE_REGISTRY_MAX_DESC_LEN 64  // Maximum description length (reduced from 128)
#endif

// =============================================================================
// TYPES
// =============================================================================

/**
 * @brief Module category
 */
typedef enum {
  MODULE_CATEGORY_SYSTEM = 0,
  MODULE_CATEGORY_MIDI,
  MODULE_CATEGORY_INPUT,
  MODULE_CATEGORY_OUTPUT,
  MODULE_CATEGORY_EFFECT,
  MODULE_CATEGORY_GENERATOR,
  MODULE_CATEGORY_LOOPER,
  MODULE_CATEGORY_UI,
  MODULE_CATEGORY_ACCORDION,
  MODULE_CATEGORY_OTHER
} module_category_t;

/**
 * @brief Parameter data type
 */
typedef enum {
  PARAM_TYPE_BOOL = 0,
  PARAM_TYPE_INT,
  PARAM_TYPE_FLOAT,
  PARAM_TYPE_ENUM,
  PARAM_TYPE_STRING
} param_type_t;

/**
 * @brief Parameter value union
 */
typedef union {
  uint8_t bool_val;
  int32_t int_val;
  float float_val;
  const char* string_val;
} param_value_t;

/**
 * @brief Parameter descriptor
 */
typedef struct {
  char name[MODULE_REGISTRY_MAX_NAME_LEN];          // Parameter name
  char description[MODULE_REGISTRY_MAX_DESC_LEN];   // Description
  param_type_t type;                                 // Data type
  int32_t min;                                       // Minimum value (for int/float)
  int32_t max;                                       // Maximum value (for int/float)
  const char** enum_values;                          // Enum value strings (if type is ENUM)
  uint8_t enum_count;                                // Number of enum values
  uint8_t read_only;                                 // 1 if read-only
  
  // Getter/setter function pointers
  int (*get_value)(uint8_t track, param_value_t* out);
  int (*set_value)(uint8_t track, const param_value_t* val);
} module_param_t;

/**
 * @brief Module status
 */
typedef enum {
  MODULE_STATUS_DISABLED = 0,
  MODULE_STATUS_ENABLED,
  MODULE_STATUS_ERROR
} module_status_t;

/**
 * @brief Module descriptor
 */
typedef struct {
  char name[MODULE_REGISTRY_MAX_NAME_LEN];          // Module name (e.g., "looper")
  char description[MODULE_REGISTRY_MAX_DESC_LEN];   // Short description
  module_category_t category;                        // Module category
  
  // Module control functions
  int (*init)(void);                                 // Initialize module
  int (*enable)(uint8_t track);                      // Enable for track (0xFF = global)
  int (*disable)(uint8_t track);                     // Disable for track
  int (*get_status)(uint8_t track);                  // Get status (module_status_t)
  
  // Parameter metadata
  module_param_t params[MODULE_REGISTRY_MAX_PARAMS];
  uint8_t param_count;
  
  // Flags
  uint8_t has_per_track_state;  // 1 if module has per-track configuration
  uint8_t is_global;             // 1 if module is global (not per-track)
  uint8_t registered;            // Internal: 1 if registered
} module_descriptor_t;

// =============================================================================
// API - INITIALIZATION
// =============================================================================

/**
 * @brief Initialize module registry
 * @return 0 on success, negative on error
 */
int module_registry_init(void);

// =============================================================================
// API - MODULE REGISTRATION
// =============================================================================

/**
 * @brief Register a module
 * @param descriptor Module descriptor (will be copied)
 * @return 0 on success, negative on error
 */
int module_registry_register(const module_descriptor_t* descriptor);

/**
 * @brief Unregister a module
 * @param name Module name
 * @return 0 on success, negative if not found
 */
int module_registry_unregister(const char* name);

// =============================================================================
// API - MODULE DISCOVERY
// =============================================================================

/**
 * @brief Get number of registered modules
 * @return Module count
 */
uint32_t module_registry_get_count(void);

/**
 * @brief Get module descriptor by index
 * @param index Module index (0 to count-1)
 * @return Pointer to descriptor or NULL if invalid index
 */
const module_descriptor_t* module_registry_get_by_index(uint32_t index);

/**
 * @brief Get module descriptor by name
 * @param name Module name
 * @return Pointer to descriptor or NULL if not found
 */
const module_descriptor_t* module_registry_get_by_name(const char* name);

/**
 * @brief List all modules in a category
 * @param category Module category
 * @param out_modules Array to receive module pointers
 * @param max_count Maximum number of modules to return
 * @return Number of modules found
 */
uint32_t module_registry_list_by_category(module_category_t category,
                                           const module_descriptor_t** out_modules,
                                           uint32_t max_count);

// =============================================================================
// API - MODULE CONTROL
// =============================================================================

/**
 * @brief Enable a module
 * @param name Module name
 * @param track Track index (0xFF for global modules)
 * @return 0 on success, negative on error
 */
int module_registry_enable(const char* name, uint8_t track);

/**
 * @brief Disable a module
 * @param name Module name
 * @param track Track index (0xFF for global modules)
 * @return 0 on success, negative on error
 */
int module_registry_disable(const char* name, uint8_t track);

/**
 * @brief Get module status
 * @param name Module name
 * @param track Track index (0xFF for global modules)
 * @return Module status or MODULE_STATUS_ERROR if not found
 */
module_status_t module_registry_get_status(const char* name, uint8_t track);

// =============================================================================
// API - PARAMETER ACCESS
// =============================================================================

/**
 * @brief Get parameter value
 * @param module_name Module name
 * @param param_name Parameter name
 * @param track Track index
 * @param out Output value
 * @return 0 on success, negative on error
 */
int module_registry_get_param(const char* module_name,
                               const char* param_name,
                               uint8_t track,
                               param_value_t* out);

/**
 * @brief Set parameter value
 * @param module_name Module name
 * @param param_name Parameter name
 * @param track Track index
 * @param value New value
 * @return 0 on success, negative on error
 */
int module_registry_set_param(const char* module_name,
                               const char* param_name,
                               uint8_t track,
                               const param_value_t* value);

/**
 * @brief Get parameter descriptor
 * @param module_name Module name
 * @param param_name Parameter name
 * @return Pointer to parameter descriptor or NULL if not found
 */
const module_param_t* module_registry_get_param_descriptor(const char* module_name,
                                                            const char* param_name);

// =============================================================================
// API - UTILITIES
// =============================================================================

/**
 * @brief Print all registered modules
 */
void module_registry_print_modules(void);

/**
 * @brief Print module information
 * @param name Module name
 */
void module_registry_print_module(const char* name);

/**
 * @brief Print module parameters
 * @param name Module name
 */
void module_registry_print_params(const char* name);

/**
 * @brief Convert category enum to string
 * @param category Module category
 * @return Category string
 */
const char* module_registry_category_to_string(module_category_t category);

/**
 * @brief Convert parameter type enum to string
 * @param type Parameter type
 * @return Type string
 */
const char* module_registry_param_type_to_string(param_type_t type);

#ifdef __cplusplus
}
#endif
