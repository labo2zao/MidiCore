/**
 * @file config_cli.c
 * @brief CLI integration for config module
 * 
 * Global system configuration
 * 
 * NOTE: This CLI module is currently disabled because the config module
 * does not have a runtime API. Configuration is loaded once at boot from
 * SD card (global.ngc file) and used to initialize other modules.
 * 
 * To enable CLI access to config:
 * 1. Add getter/setter API to Services/config/config.c
 * 2. Add a global config instance or access functions
 * 3. Re-enable this CLI module and update wrappers
 * 
 * For now, configuration must be edited on SD card and requires reboot.
 */

#include "Services/config/config.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// TODO: Config module needs runtime API before CLI can work
// Currently config is loaded once at boot in app_init.c as a local variable

#if 0  // Disabled until config module has proper runtime API

// =============================================================================
// PARAMETER WRAPPERS - DISABLED (see note above)
// =============================================================================

// NOTE: These wrappers reference a global config that doesn't exist yet
// The config module currently only has config_load_from_sd() and config_set_defaults()
// No runtime getter/setter API exists

static config_t* get_global_config(void) {
  // TODO: Implement in config.c
  // For now, return NULL to indicate not available
  return NULL;
}

static uint8_t config_get_srio_enable(void) {
  config_t* cfg = get_global_config();
  return cfg ? cfg->srio_enable : 0;
}

static void config_set_srio_enable(uint8_t val) {
  config_t* cfg = get_global_config();
  if (cfg) cfg->srio_enable = val;
}

static uint8_t config_get_srio_din_enable(void) {
  config_t* cfg = get_global_config();
  return cfg ? cfg->srio_din_enable : 0;
}

static void config_set_srio_din_enable(uint8_t val) {
  config_t* cfg = get_global_config();
  if (cfg) cfg->srio_din_enable = val;
}

static uint8_t config_get_srio_dout_enable(void) {
  config_t* cfg = get_global_config();
  return cfg ? cfg->srio_dout_enable : 0;
}

static void config_set_srio_dout_enable(uint8_t val) {
  config_t* cfg = get_global_config();
  if (cfg) cfg->srio_dout_enable = val;
}

DEFINE_PARAM_BOOL(config, srio_enable, config_get_srio_enable, config_set_srio_enable)

DEFINE_PARAM_BOOL(config, srio_din_enable, config_get_srio_din_enable, config_set_srio_din_enable)

DEFINE_PARAM_BOOL(config, srio_dout_enable, config_get_srio_dout_enable, config_set_srio_dout_enable)
// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int config_cli_enable(uint8_t track) {
  (void)track;
  // Config is always "enabled" (loaded at boot)
  return 0;
}

static int config_cli_disable(uint8_t track) {
  (void)track;
  // Config cannot be disabled
  return -1;
}

static int config_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_config_descriptor = {
  .name = "config",
  .description = "Global system configuration (SD card only)",
  .category = MODULE_CATEGORY_SYSTEM,
  .init = NULL, // Config loaded at boot, no runtime init
  .enable = config_cli_enable,
  .disable = config_cli_disable,
  .get_status = config_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1,
  .param_count = 0  // No parameters until runtime API exists
};

// =============================================================================
// PARAMETER SETUP - DISABLED
// =============================================================================

static void setup_config_parameters(void) {
  // TODO: Enable when config module has runtime API
  // Currently no parameters are registered
  s_config_descriptor.param_count = 0;
}

#endif  // Disabled code block

// =============================================================================
// REGISTRATION - Always available (even if parameters disabled)
// =============================================================================

int config_register_cli(void) {
  // Register module descriptor (but with no parameters)
  // This allows "module info config" to work and show that
  // config exists but must be edited on SD card
  
  #if 0  // Keep disabled for now - uncomment when ready
  setup_config_parameters();
  return module_registry_register(&s_config_descriptor);
  #else
  // Not registered - config is SD-only for now
  return 0;
  #endif
}
