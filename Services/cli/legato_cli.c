/**
 * @file legato_cli.c
 * @brief CLI integration for legato module
 * 
 * Legato/mono/priority handling
 */

#include "Services/legato/legato.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(legato, enabled, legato_is_enabled, legato_set_enabled)

static int legato_param_get_priority(uint8_t track, param_value_t* out) {
  
  out->int_val = legato_get_priority(track);
  return 0;
}

static int legato_param_set_priority(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 4) return -1;
  legato_set_priority(track, (uint8_t)val->int_val);
  return 0;
}

static int legato_param_get_retrigger(uint8_t track, param_value_t* out) {
  
  out->int_val = legato_get_retrigger(track);
  return 0;
}

static int legato_param_set_retrigger(uint8_t track, const param_value_t* val) {
  
  if (val->int_val < 0 || val->int_val >= 2) return -1;
  legato_set_retrigger(track, (uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_INT_TRACK(legato, glide_time, legato_get_glide_time, legato_set_glide_time)

DEFINE_PARAM_BOOL_TRACK(legato, mono_mode, legato_is_mono_mode, legato_set_mono_mode)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(legato, legato_set_enabled, legato_is_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_priority_names[] = {
  "LAST",
  "HIGHEST",
  "LOWEST",
  "FIRST",
};

static const char* s_retrigger_names[] = {
  "OFF",
  "ON",
};

// =============================================================================
// INIT WRAPPER
// =============================================================================

static int legato_cli_init(void) {
  legato_init();
  return 0;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_legato_descriptor = {
  .name = "legato",
  .description = "Legato/mono/priority handling",
  .category = MODULE_CATEGORY_EFFECT,
  .init = legato_cli_init,
  .enable = legato_cli_enable,
  .disable = legato_cli_disable,
  .get_status = legato_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_legato_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(legato, enabled, "Enable legato mode"),
    {
      .name = "priority",
      .description = "Note priority",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 3,
      .enum_values = s_priority_names,
      .enum_count = 4,
      .read_only = 0,
      .get_value = legato_param_get_priority,
      .set_value = legato_param_set_priority
    },
    {
      .name = "retrigger",
      .description = "Retrigger mode",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 1,
      .enum_values = s_retrigger_names,
      .enum_count = 2,
      .read_only = 0,
      .get_value = legato_param_get_retrigger,
      .set_value = legato_param_set_retrigger
    },
    PARAM_INT(legato, glide_time, "Portamento time (0-2000ms)", 0, 2000),
    PARAM_BOOL(legato, mono_mode, "Mono mode"),
  };
  
  s_legato_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_legato_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int legato_register_cli(void) {
  setup_legato_parameters();
  return module_registry_register(&s_legato_descriptor);
}
