/**
 * @file scale_cli.c
 * @brief CLI integration for scale module
 * 
 * Scale quantization
 */

#include "Services/scale/scale.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int scale_param_get_scale_type(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = scale_get_scale_type();
  return 0;
}

static int scale_param_set_scale_type(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val >= 14) return -1;
  scale_set_scale_type((uint8_t)val->int_val);
  return 0;
}

DEFINE_PARAM_INT(scale, root_note, scale_get_root_note, scale_set_root_note)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int scale_cli_enable(uint8_t track) {
  (void)track;
  return 0;
}

static int scale_cli_disable(uint8_t track) {
  (void)track;
  return 0;
}

static int scale_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_scale_type_names[] = {
  "CHROMATIC",
  "MAJOR",
  "MINOR_NAT",
  "MINOR_HAR",
  "MINOR_MEL",
  "DORIAN",
  "PHRYGIAN",
  "LYDIAN",
  "MIXOLYDIAN",
  "LOCRIAN",
  "PENTATONIC_MAJ",
  "PENTATONIC_MIN",
  "BLUES",
  "WHOLE_TONE",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_scale_descriptor = {
  .name = "scale",
  .description = "Scale quantization",
  .category = MODULE_CATEGORY_EFFECT,
  .init = scale_init,
  .enable = scale_cli_enable,
  .disable = scale_cli_disable,
  .get_status = scale_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_scale_parameters(void) {
  module_param_t params[] = {
    {
      .name = "scale_type",
      .description = "Scale type",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 13,
      .enum_values = s_scale_type_names,
      .enum_count = 14,
      .read_only = 0,
      .get_value = scale_param_get_scale_type,
      .set_value = scale_param_set_scale_type
    },
    PARAM_INT(scale, root_note, "Root note (0-11, C=0)", 0, 11),
  };
  
  s_scale_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_scale_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int scale_register_cli(void) {
  setup_scale_parameters();
  return module_registry_register(&s_scale_descriptor);
}
