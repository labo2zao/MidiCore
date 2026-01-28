/**
 * @file patch_cli.c
 * @brief CLI integration for patch system
 * 
 * Patch/preset management with SD card storage
 */

#include "Services/patch/patch.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static char s_current_patch[64] = "";
static char s_current_bank[32] = "";

static int patch_param_get_current_patch(uint8_t track, param_value_t* out) {
  (void)track;
  out->string_val = s_current_patch;
  return 0;
}

static int patch_param_get_current_bank(uint8_t track, param_value_t* out) {
  (void)track;
  out->string_val = s_current_bank;
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int patch_cli_enable(uint8_t track) {
  (void)track;
  return 0; // Always enabled
}

static int patch_cli_disable(uint8_t track) {
  (void)track;
  return 0;
}

static int patch_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_patch_descriptor = {
  .name = "patch",
  .description = "Patch/preset system with SD card storage",
  .category = MODULE_CATEGORY_SYSTEM,
  .init = patch_init,
  .enable = patch_cli_enable,
  .disable = patch_cli_disable,
  .get_status = patch_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_patch_parameters(void) {
  module_param_t params[] = {
    {
      .name = "current_patch",
      .description = "Currently loaded patch name",
      .type = PARAM_TYPE_STRING,
      .read_only = 1,
      .get_value = patch_param_get_current_patch,
      .set_value = NULL
    },
    {
      .name = "current_bank",
      .description = "Currently loaded bank name",
      .type = PARAM_TYPE_STRING,
      .read_only = 1,
      .get_value = patch_param_get_current_bank,
      .set_value = NULL
    }
  };
  
  s_patch_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_patch_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int patch_register_cli(void) {
  setup_patch_parameters();
  return module_registry_register(&s_patch_descriptor);
}
