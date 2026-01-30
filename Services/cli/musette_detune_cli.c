/**
 * @file musette_detune_cli.c
 * @brief CLI integration for musette_detune module
 * 
 * Classic accordion musette/chorus
 */

#include "Services/musette_detune/musette_detune.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int musette_detune_param_get_style(uint8_t track, param_value_t* out) {
  
  out->int_val = musette_detune_get_style(track);
  return 0;
}

static int musette_detune_param_set_style(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 6) return -1;
  musette_detune_set_style(track, (uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_INT_TRACK(musette_detune, detune_cents, musette_detune_get_detune_cents, musette_detune_set_detune_cents)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int musette_detune_cli_enable(uint8_t track) {
  
  return 0;
}

static int musette_detune_cli_disable(uint8_t track) {
  
  return 0;
}

static int musette_detune_cli_get_status(uint8_t track) {
  
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_style_names[] = {
  "DRY",
  "SCOTTISH",
  "AMERICAN",
  "FRENCH",
  "ITALIAN",
  "CUSTOM",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

// Init wrapper (module init returns void, descriptor needs int)
static int musette_detune_cli_init(void) {
  musette_init();
  return 0;
}

static module_descriptor_t s_musette_detune_descriptor = {
  .name = "musette_detune",
  .description = "Classic accordion musette/chorus",
  .category = MODULE_CATEGORY_ACCORDION,
  .init = musette_detune_cli_init,
  .enable = musette_detune_cli_enable,
  .disable = musette_detune_cli_disable,
  .get_status = musette_detune_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_musette_detune_parameters(void) {
  module_param_t params[] = {
    {
      .name = "style",
      .description = "Tuning style",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 5,
      .enum_values = s_style_names,
      .enum_count = 6,
      .read_only = 0,
      .get_value = musette_detune_param_get_style,
      .set_value = musette_detune_param_set_style
    },
    PARAM_INT(musette_detune, detune_cents, "Detune amount (cents)", 0, 50),
  };
  
  s_musette_detune_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_musette_detune_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int musette_detune_register_cli(void) {
  setup_musette_detune_parameters();
  return module_registry_register(&s_musette_detune_descriptor);
}
