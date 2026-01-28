/**
 * @file ainser_map_cli.c
 * @brief CLI integration for AINSER64 analog input mapping
 * 
 * AINSER64 (SPI ADC) analog input to MIDI CC mapper with 64 channels
 */

#include "Services/ainser/ainser_map.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int ainser_map_param_get_channel_count(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = 64; // AINSER64 has 64 channels
  return 0;
}

static int ainser_map_param_get_cc(uint8_t track, param_value_t* out) {
  const AINSER_MapEntry* table = ainser_map_get_table();
  if (track >= 64) return -1;
  out->int_val = table[track].cc;
  return 0;
}

static int ainser_map_param_set_cc(uint8_t track, const param_value_t* val) {
  AINSER_MapEntry* table = (AINSER_MapEntry*)ainser_map_get_table();
  if (track >= 64 || val->int_val < 0 || val->int_val > 127) return -1;
  table[track].cc = (uint8_t)val->int_val;
  return 0;
}

static int ainser_map_param_get_curve(uint8_t track, param_value_t* out) {
  const AINSER_MapEntry* table = ainser_map_get_table();
  if (track >= 64) return -1;
  out->int_val = table[track].curve;
  return 0;
}

static int ainser_map_param_set_curve(uint8_t track, const param_value_t* val) {
  AINSER_MapEntry* table = (AINSER_MapEntry*)ainser_map_get_table();
  if (track >= 64 || val->int_val < 0 || val->int_val >= 4) return -1;
  table[track].curve = (uint8_t)val->int_val;
  return 0;
}

static int ainser_map_param_get_deadband(uint8_t track, param_value_t* out) {
  const AINSER_MapEntry* table = ainser_map_get_table();
  if (track >= 64) return -1;
  out->int_val = table[track].deadband;
  return 0;
}

static int ainser_map_param_set_deadband(uint8_t track, const param_value_t* val) {
  AINSER_MapEntry* table = (AINSER_MapEntry*)ainser_map_get_table();
  if (track >= 64 || val->int_val < 0 || val->int_val > 255) return -1;
  table[track].deadband = (uint8_t)val->int_val;
  return 0;
}

static int ainser_map_param_get_min(uint8_t track, param_value_t* out) {
  const AINSER_MapEntry* table = ainser_map_get_table();
  if (track >= 64) return -1;
  out->int_val = table[track].min;
  return 0;
}

static int ainser_map_param_set_min(uint8_t track, const param_value_t* val) {
  AINSER_MapEntry* table = (AINSER_MapEntry*)ainser_map_get_table();
  if (track >= 64 || val->int_val < 0 || val->int_val > 4095) return -1;
  table[track].min = (uint16_t)val->int_val;
  return 0;
}

static int ainser_map_param_get_max(uint8_t track, param_value_t* out) {
  const AINSER_MapEntry* table = ainser_map_get_table();
  if (track >= 64) return -1;
  out->int_val = table[track].max;
  return 0;
}

static int ainser_map_param_set_max(uint8_t track, const param_value_t* val) {
  AINSER_MapEntry* table = (AINSER_MapEntry*)ainser_map_get_table();
  if (track >= 64 || val->int_val < 0 || val->int_val > 4095) return -1;
  table[track].max = (uint16_t)val->int_val;
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int ainser_map_cli_enable(uint8_t track) {
  (void)track;
  return 0; // Always enabled
}

static int ainser_map_cli_disable(uint8_t track) {
  (void)track;
  return -1; // Cannot disable hardware input
}

static int ainser_map_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_curve_names[] = {
  "LINEAR",
  "EXPONENTIAL",
  "LOGARITHMIC",
  "S_CURVE",
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_ainser_map_descriptor = {
  .name = "ainser",
  .description = "AINSER64 analog input mapping (64 channels, 12-bit ADC)",
  .category = MODULE_CATEGORY_INPUT,
  .init = ainser_map_init_defaults,
  .enable = ainser_map_cli_enable,
  .disable = ainser_map_cli_disable,
  .get_status = ainser_map_cli_get_status,
  .has_per_track_state = 1,  // Per-channel configuration
  .is_global = 0,
  .max_tracks = 64  // 64 analog channels
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_ainser_map_parameters(void) {
  module_param_t params[] = {
    {
      .name = "channel_count",
      .description = "Total number of AINSER channels",
      .type = PARAM_TYPE_INT,
      .min = 64,
      .max = 64,
      .read_only = 1,
      .get_value = ainser_map_param_get_channel_count,
      .set_value = NULL
    },
    {
      .name = "cc",
      .description = "MIDI CC number for this channel",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 127,
      .read_only = 0,
      .get_value = ainser_map_param_get_cc,
      .set_value = ainser_map_param_set_cc
    },
    {
      .name = "curve",
      .description = "Response curve",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 3,
      .enum_values = s_curve_names,
      .enum_count = 4,
      .read_only = 0,
      .get_value = ainser_map_param_get_curve,
      .set_value = ainser_map_param_set_curve
    },
    {
      .name = "deadband",
      .description = "Noise deadband (0-255)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 255,
      .read_only = 0,
      .get_value = ainser_map_param_get_deadband,
      .set_value = ainser_map_param_set_deadband
    },
    {
      .name = "min",
      .description = "Minimum ADC value (0-4095)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 4095,
      .read_only = 0,
      .get_value = ainser_map_param_get_min,
      .set_value = ainser_map_param_set_min
    },
    {
      .name = "max",
      .description = "Maximum ADC value (0-4095)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 4095,
      .read_only = 0,
      .get_value = ainser_map_param_get_max,
      .set_value = ainser_map_param_set_max
    }
  };
  
  s_ainser_map_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_ainser_map_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int ainser_map_register_cli(void) {
  setup_ainser_map_parameters();
  return module_registry_register(&s_ainser_map_descriptor);
}
