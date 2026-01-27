/**
 * @file module_cli_helpers.h
 * @brief Helper macros for easy CLI integration
 * 
 * This file provides convenience macros to simplify adding CLI support
 * to existing modules without significant code changes.
 * 
 * Usage:
 * 1. Include this header in your module .c file
 * 2. Use the DEFINE_PARAM_* macros to create wrappers
 * 3. Use MODULE_DESCRIPTOR() macro to define the module
 * 4. Call MODULE_REGISTER() in your init function
 */

#pragma once

#include "Services/module_registry/module_registry.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// PARAMETER WRAPPER MACROS - Simplify creating getter/setter functions
// =============================================================================

/**
 * @brief Define a boolean parameter with getter/setter
 * 
 * Example:
 *   DEFINE_PARAM_BOOL(arp, enabled, arp_get_enabled, arp_set_enabled)
 * 
 * This creates:
 *   - arp_param_get_enabled(track, out)
 *   - arp_param_set_enabled(track, val)
 */
#define DEFINE_PARAM_BOOL(module, param_name, get_fn, set_fn) \
  static int module##_param_get_##param_name(uint8_t track, param_value_t* out) { \
    (void)track; \
    out->bool_val = get_fn(); \
    return 0; \
  } \
  static int module##_param_set_##param_name(uint8_t track, const param_value_t* val) { \
    (void)track; \
    set_fn(val->bool_val); \
    return 0; \
  }

/**
 * @brief Define a per-track boolean parameter
 */
#define DEFINE_PARAM_BOOL_TRACK(module, param_name, get_fn, set_fn) \
  static int module##_param_get_##param_name(uint8_t track, param_value_t* out) { \
    out->bool_val = get_fn(track); \
    return 0; \
  } \
  static int module##_param_set_##param_name(uint8_t track, const param_value_t* val) { \
    set_fn(track, val->bool_val); \
    return 0; \
  }

/**
 * @brief Define an integer parameter
 */
#define DEFINE_PARAM_INT(module, param_name, get_fn, set_fn) \
  static int module##_param_get_##param_name(uint8_t track, param_value_t* out) { \
    (void)track; \
    out->int_val = (int32_t)get_fn(); \
    return 0; \
  } \
  static int module##_param_set_##param_name(uint8_t track, const param_value_t* val) { \
    (void)track; \
    set_fn(val->int_val); \
    return 0; \
  }

/**
 * @brief Define a per-track integer parameter
 */
#define DEFINE_PARAM_INT_TRACK(module, param_name, get_fn, set_fn) \
  static int module##_param_get_##param_name(uint8_t track, param_value_t* out) { \
    out->int_val = (int32_t)get_fn(track); \
    return 0; \
  } \
  static int module##_param_set_##param_name(uint8_t track, const param_value_t* val) { \
    set_fn(track, val->int_val); \
    return 0; \
  }

/**
 * @brief Define an enum parameter
 */
#define DEFINE_PARAM_ENUM(module, param_name, get_fn, set_fn, enum_type) \
  static int module##_param_get_##param_name(uint8_t track, param_value_t* out) { \
    (void)track; \
    out->int_val = (int32_t)get_fn(); \
    return 0; \
  } \
  static int module##_param_set_##param_name(uint8_t track, const param_value_t* val) { \
    (void)track; \
    if (val->int_val < 0) return -1; \
    set_fn((enum_type)val->int_val); \
    return 0; \
  }

/**
 * @brief Define a per-track enum parameter
 */
#define DEFINE_PARAM_ENUM_TRACK(module, param_name, get_fn, set_fn, enum_type) \
  static int module##_param_get_##param_name(uint8_t track, param_value_t* out) { \
    out->int_val = (int32_t)get_fn(track); \
    return 0; \
  } \
  static int module##_param_set_##param_name(uint8_t track, const param_value_t* val) { \
    if (val->int_val < 0) return -1; \
    set_fn(track, (enum_type)val->int_val); \
    return 0; \
  }

// =============================================================================
// MODULE CONTROL WRAPPER MACROS
// =============================================================================

/**
 * @brief Define enable/disable/status wrappers for global module
 */
#define DEFINE_MODULE_CONTROL_GLOBAL(module, set_enabled_fn, get_enabled_fn) \
  static int module##_cli_enable(uint8_t track) { \
    (void)track; \
    set_enabled_fn(1); \
    return 0; \
  } \
  static int module##_cli_disable(uint8_t track) { \
    (void)track; \
    set_enabled_fn(0); \
    return 0; \
  } \
  static int module##_cli_get_status(uint8_t track) { \
    (void)track; \
    return get_enabled_fn() ? MODULE_STATUS_ENABLED : MODULE_STATUS_DISABLED; \
  }

/**
 * @brief Define enable/disable/status wrappers for per-track module
 */
#define DEFINE_MODULE_CONTROL_TRACK(module, set_enabled_fn, get_enabled_fn) \
  static int module##_cli_enable(uint8_t track) { \
    set_enabled_fn(track, 1); \
    return 0; \
  } \
  static int module##_cli_disable(uint8_t track) { \
    set_enabled_fn(track, 0); \
    return 0; \
  } \
  static int module##_cli_get_status(uint8_t track) { \
    return get_enabled_fn(track) ? MODULE_STATUS_ENABLED : MODULE_STATUS_DISABLED; \
  }

// =============================================================================
// PARAMETER DESCRIPTOR MACROS
// =============================================================================

/**
 * @brief Create a boolean parameter descriptor
 */
#define PARAM_BOOL(module, name, desc) \
  { \
    .name = #name, \
    .description = desc, \
    .type = PARAM_TYPE_BOOL, \
    .min = 0, \
    .max = 1, \
    .read_only = 0, \
    .get_value = module##_param_get_##name, \
    .set_value = module##_param_set_##name \
  }

/**
 * @brief Create an integer parameter descriptor
 */
#define PARAM_INT(module, name, desc, min_val, max_val) \
  { \
    .name = #name, \
    .description = desc, \
    .type = PARAM_TYPE_INT, \
    .min = min_val, \
    .max = max_val, \
    .read_only = 0, \
    .get_value = module##_param_get_##name, \
    .set_value = module##_param_set_##name \
  }

/**
 * @brief Create an enum parameter descriptor
 */
#define PARAM_ENUM(module, name, desc, max_val, enum_strs, enum_cnt) \
  { \
    .name = #name, \
    .description = desc, \
    .type = PARAM_TYPE_ENUM, \
    .min = 0, \
    .max = max_val, \
    .enum_values = enum_strs, \
    .enum_count = enum_cnt, \
    .read_only = 0, \
    .get_value = module##_param_get_##name, \
    .set_value = module##_param_set_##name \
  }

/**
 * @brief Create a read-only integer parameter descriptor
 */
#define PARAM_INT_RO(module, name, desc) \
  { \
    .name = #name, \
    .description = desc, \
    .type = PARAM_TYPE_INT, \
    .read_only = 1, \
    .get_value = module##_param_get_##name, \
    .set_value = NULL \
  }

// =============================================================================
// MODULE DESCRIPTOR MACRO
// =============================================================================

/**
 * @brief Define a module descriptor
 * 
 * Example:
 *   MODULE_DESCRIPTOR(
 *     arpeggiator,
 *     "MIDI arpeggiator",
 *     MODULE_CATEGORY_EFFECT,
 *     arp_init,
 *     0,  // not per-track
 *     1   // is global
 *   )
 */
#define MODULE_DESCRIPTOR(module_name, desc_text, cat, init_fn, per_track, global) \
  static module_descriptor_t s_##module_name##_descriptor = { \
    .name = #module_name, \
    .description = desc_text, \
    .category = cat, \
    .init = init_fn, \
    .enable = module_name##_cli_enable, \
    .disable = module_name##_cli_disable, \
    .get_status = module_name##_cli_get_status, \
    .has_per_track_state = per_track, \
    .is_global = global \
  }

/**
 * @brief Register module with CLI
 */
#define MODULE_REGISTER(module_name) \
  module_registry_register(&s_##module_name##_descriptor)

// =============================================================================
// USAGE EXAMPLE
// =============================================================================

#if 0
/*
 * In your module .c file:
 */

#include "my_module.h"
#include "Services/cli/module_cli_helpers.h"

// Define parameter wrappers
DEFINE_PARAM_BOOL(my_module, enabled, my_module_get_enabled, my_module_set_enabled)
DEFINE_PARAM_INT(my_module, rate, my_module_get_rate, my_module_set_rate)
DEFINE_PARAM_ENUM(my_module, mode, my_module_get_mode, my_module_set_mode, my_mode_t)

// Define module control wrappers
DEFINE_MODULE_CONTROL_GLOBAL(my_module, my_module_set_enabled, my_module_get_enabled)

// Enum strings
static const char* s_mode_names[] = { "OFF", "ON", "AUTO" };

// Define module descriptor
MODULE_DESCRIPTOR(
  my_module,
  "My awesome module",
  MODULE_CATEGORY_EFFECT,
  my_module_init,
  0,  // not per-track
  1   // is global
);

// Add parameters to descriptor
static void setup_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(my_module, enabled, "Enable module"),
    PARAM_INT(my_module, rate, "Processing rate", 1, 100),
    PARAM_ENUM(my_module, mode, "Operating mode", 2, s_mode_names, 3)
  };
  
  s_my_module_descriptor.param_count = 3;
  memcpy(s_my_module_descriptor.params, params, sizeof(params));
}

// Register in init function
void my_module_init(void) {
  // ... existing init code ...
  
  setup_parameters();
  MODULE_REGISTER(my_module);
}
#endif

#ifdef __cplusplus
}
#endif
