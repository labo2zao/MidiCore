/**
 * @file arpeggiator_cli_integration.c
 * @brief Example: Adding CLI support to the arpeggiator module
 * 
 * This file demonstrates how to integrate an existing module with the
 * CLI and module registry systems.
 * 
 * Steps:
 * 1. Include required headers
 * 2. Create parameter wrapper functions
 * 3. Define module descriptor
 * 4. Register module at init time
 */

#include "Services/arpeggiator/arpeggiator.h"
#include "Services/module_registry/module_registry.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPER FUNCTIONS
// =============================================================================

/**
 * @brief Wrapper to get 'enabled' parameter
 */
static int arp_param_get_enabled(uint8_t track, param_value_t* out)
{
  (void)track; // Arpeggiator is global, not per-track
  out->bool_val = arp_get_enabled();
  return 0;
}

/**
 * @brief Wrapper to set 'enabled' parameter
 */
static int arp_param_set_enabled(uint8_t track, const param_value_t* val)
{
  (void)track;
  arp_set_enabled(val->bool_val);
  return 0;
}

/**
 * @brief Wrapper to get 'pattern' parameter
 */
static int arp_param_get_pattern(uint8_t track, param_value_t* out)
{
  (void)track;
  out->int_val = (int32_t)arp_get_pattern();
  return 0;
}

/**
 * @brief Wrapper to set 'pattern' parameter
 */
static int arp_param_set_pattern(uint8_t track, const param_value_t* val)
{
  (void)track;
  
  // Validate range
  if (val->int_val < 0 || val->int_val >= ARP_PATTERN_COUNT) {
    return -1; // Invalid value
  }
  
  arp_set_pattern((arp_pattern_t)val->int_val);
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPER FUNCTIONS
// =============================================================================

/**
 * @brief Enable wrapper (maps to arp_set_enabled)
 */
static int arp_enable(uint8_t track)
{
  (void)track;
  arp_set_enabled(1);
  return 0;
}

/**
 * @brief Disable wrapper (maps to arp_set_enabled)
 */
static int arp_disable(uint8_t track)
{
  (void)track;
  arp_set_enabled(0);
  return 0;
}

/**
 * @brief Status wrapper
 */
static int arp_get_status(uint8_t track)
{
  (void)track;
  return arp_get_enabled() ? MODULE_STATUS_ENABLED : MODULE_STATUS_DISABLED;
}

// =============================================================================
// ENUM VALUE STRINGS
// =============================================================================

static const char* s_arp_pattern_names[] = {
  "UP",
  "DOWN",
  "UP_DOWN",
  "RANDOM",
  "AS_PLAYED"
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_arp_descriptor = {
  .name = "arpeggiator",
  .description = "MIDI arpeggiator with multiple patterns",
  .category = MODULE_CATEGORY_EFFECT,
  
  // Control functions
  .init = arp_init,
  .enable = arp_enable,
  .disable = arp_disable,
  .get_status = arp_get_status,
  
  // Parameters
  .params = {
    {
      .name = "enabled",
      .description = "Enable arpeggiator",
      .type = PARAM_TYPE_BOOL,
      .min = 0,
      .max = 1,
      .read_only = 0,
      .get_value = arp_param_get_enabled,
      .set_value = arp_param_set_enabled
    },
    {
      .name = "pattern",
      .description = "Arpeggio pattern (0=UP, 1=DOWN, 2=UP_DOWN, 3=RANDOM, 4=AS_PLAYED)",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = ARP_PATTERN_COUNT - 1,
      .enum_values = s_arp_pattern_names,
      .enum_count = ARP_PATTERN_COUNT,
      .read_only = 0,
      .get_value = arp_param_get_pattern,
      .set_value = arp_param_set_pattern
    }
  },
  .param_count = 2,
  
  // Flags
  .has_per_track_state = 0,  // Arpeggiator is global
  .is_global = 1
};

// =============================================================================
// REGISTRATION
// =============================================================================

/**
 * @brief Register arpeggiator with module registry
 * 
 * Call this from arp_init() or during application initialization
 */
int arp_register_cli(void)
{
  return module_registry_register(&s_arp_descriptor);
}

// =============================================================================
// USAGE EXAMPLES (via CLI)
// =============================================================================

/*
 * Once registered, the following CLI commands become available:
 * 
 * # List all modules
 * module list
 * 
 * # Get arpeggiator information
 * module info arpeggiator
 * 
 * # Enable arpeggiator
 * module enable arpeggiator
 * 
 * # Disable arpeggiator
 * module disable arpeggiator
 * 
 * # Get status
 * module status arpeggiator
 * 
 * # List parameters
 * module params arpeggiator
 * 
 * # Get parameter values
 * module get arpeggiator enabled
 * module get arpeggiator pattern
 * 
 * # Set parameter values
 * module set arpeggiator enabled true
 * module set arpeggiator pattern 0      # UP
 * module set arpeggiator pattern 1      # DOWN
 * module set arpeggiator pattern 2      # UP_DOWN
 * 
 * # Save configuration
 * config save 0:/arpeggiator.ini
 * 
 * # Load configuration
 * config load 0:/arpeggiator.ini
 */

// =============================================================================
// INTEGRATION INTO EXISTING CODE
// =============================================================================

/*
 * To integrate this into the existing arpeggiator.c file:
 * 
 * 1. Add to arp_init():
 * 
 *    void arp_init(void) {
 *      // ... existing initialization ...
 *      
 *      // Register with CLI system
 *      arp_register_cli();
 *    }
 * 
 * 2. Or call from app_init.c:
 * 
 *    void app_init(void) {
 *      // Initialize module registry and CLI
 *      module_registry_init();
 *      cli_init();
 *      cli_module_commands_init();
 *      
 *      // Initialize modules
 *      arp_init();
 *      
 *      // Register modules with CLI
 *      arp_register_cli();
 *      looper_register_cli();
 *      // ... etc for all modules
 *    }
 */
