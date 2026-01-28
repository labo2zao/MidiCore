/**
 * @file dream_cli.c
 * @brief CLI integration for Dream SAM5716 sampler control
 * 
 * Dream sampler control via MIDI/SysEx
 */

#include "Services/dream/dream_sysex.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static uint8_t s_enabled = 0;
static char s_patch_path[128] = "";

static int dream_param_get_enabled(uint8_t track, param_value_t* out) {
  (void)track;
  out->bool_val = s_enabled;
  return 0;
}

static int dream_param_set_enabled(uint8_t track, const param_value_t* val) {
  (void)track;
  s_enabled = val->bool_val;
  return 0;
}

static int dream_param_get_patch_path(uint8_t track, param_value_t* out) {
  (void)track;
  out->string_val = s_patch_path;
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int dream_cli_enable(uint8_t track) {
  (void)track;
  s_enabled = 1;
  return 0;
}

static int dream_cli_disable(uint8_t track) {
  (void)track;
  s_enabled = 0;
  return 0;
}

static int dream_cli_get_status(uint8_t track) {
  (void)track;
  return s_enabled ? MODULE_STATUS_ENABLED : MODULE_STATUS_DISABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_dream_descriptor = {
  .name = "dream",
  .description = "Dream SAM5716 sampler control",
  .category = MODULE_CATEGORY_GENERATOR,
  .init = NULL, // Initialized via patch system
  .enable = dream_cli_enable,
  .disable = dream_cli_disable,
  .get_status = dream_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_dream_parameters(void) {
  module_param_t params[] = {
    {
      .name = "enabled",
      .description = "Enable Dream sampler control",
      .type = PARAM_TYPE_BOOL,
      .read_only = 0,
      .get_value = dream_param_get_enabled,
      .set_value = dream_param_set_enabled
    },
    {
      .name = "patch_path",
      .description = "Current patch file path",
      .type = PARAM_TYPE_STRING,
      .read_only = 1,
      .get_value = dream_param_get_patch_path,
      .set_value = NULL
    }
  };
  
  s_dream_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_dream_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int dream_register_cli(void) {
  setup_dream_parameters();
  return module_registry_register(&s_dream_descriptor);
}
