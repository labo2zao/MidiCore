/**
 * @file channelizer_cli.c
 * @brief CLI integration for channelizer module
 * 
 * Intelligent channel mapping and voice management
 */

#include "Services/channelizer/channelizer.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(channelizer, enabled, channelizer_get_enabled, channelizer_set_enabled)

static int channelizer_param_get_mode(uint8_t track, param_value_t* out) {
  
  out->int_val = channelizer_get_mode(track);
  return 0;
}

static int channelizer_param_set_mode(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 5) return -1;
  channelizer_set_mode(track, (uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_INT_TRACK(channelizer, force_channel, channelizer_get_force_channel, channelizer_set_force_channel)

DEFINE_PARAM_INT_TRACK(channelizer, voice_limit, channelizer_get_voice_limit, channelizer_set_voice_limit)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(channelizer, channelizer_set_enabled, channelizer_is_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_mode_names[] = {
  "BYPASS",
  "FORCE",
  "REMAP",
  "ROTATE",
  "ZONE",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static int channelizer_cli_init(void) { 
  channelizer_init(); 
  return 0; 
}

static module_descriptor_t s_channelizer_descriptor = {
  .name = "channelizer",
  .description = "Intelligent channel mapping and voice management",
  .category = MODULE_CATEGORY_EFFECT,
  .init = channelizer_cli_init,
  .enable = channelizer_cli_enable,
  .disable = channelizer_cli_disable,
  .get_status = channelizer_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_channelizer_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(channelizer, enabled, "Enable channelizer"),
    {
      .name = "mode",
      .description = "Operating mode",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 4,
      .enum_values = s_mode_names,
      .enum_count = 5,
      .read_only = 0,
      .get_value = channelizer_param_get_mode,
      .set_value = channelizer_param_set_mode
    },
    PARAM_INT(channelizer, force_channel, "Force to channel (0-15)", 0, 15),
    PARAM_INT(channelizer, voice_limit, "Max voices (1-16)", 1, 16),
  };
  
  s_channelizer_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_channelizer_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int channelizer_register_cli(void) {
  setup_channelizer_parameters();
  return module_registry_register(&s_channelizer_descriptor);
}
