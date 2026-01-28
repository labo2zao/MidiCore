/**
 * @file din_map_cli.c
 * @brief CLI integration for DIN (digital input) mapping
 * 
 * Digital input button to MIDI note/CC mapping
 */

#include "Services/din/din_map.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int din_map_param_get_button_count(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = 128; // Maximum DIN inputs
  return 0;
}

static int din_map_param_get_note(uint8_t track, param_value_t* out) {
  const DIN_MapEntry* table = din_map_get_table();
  if (track >= 128) return -1;
  out->int_val = table[track].note;
  return 0;
}

static int din_map_param_set_note(uint8_t track, const param_value_t* val) {
  DIN_MapEntry* table = (DIN_MapEntry*)din_map_get_table();
  if (track >= 128 || val->int_val < 0 || val->int_val > 127) return -1;
  table[track].note = (uint8_t)val->int_val;
  return 0;
}

static int din_map_param_get_cc(uint8_t track, param_value_t* out) {
  const DIN_MapEntry* table = din_map_get_table();
  if (track >= 128) return -1;
  out->int_val = table[track].cc;
  return 0;
}

static int din_map_param_set_cc(uint8_t track, const param_value_t* val) {
  DIN_MapEntry* table = (DIN_MapEntry*)din_map_get_table();
  if (track >= 128 || val->int_val < 0 || val->int_val > 127) return -1;
  table[track].cc = (uint8_t)val->int_val;
  return 0;
}

static int din_map_param_get_channel(uint8_t track, param_value_t* out) {
  const DIN_MapEntry* table = din_map_get_table();
  if (track >= 128) return -1;
  out->int_val = table[track].channel;
  return 0;
}

static int din_map_param_set_channel(uint8_t track, const param_value_t* val) {
  DIN_MapEntry* table = (DIN_MapEntry*)din_map_get_table();
  if (track >= 128 || val->int_val < 0 || val->int_val > 15) return -1;
  table[track].channel = (uint8_t)val->int_val;
  return 0;
}

static int din_map_param_get_mode(uint8_t track, param_value_t* out) {
  const DIN_MapEntry* table = din_map_get_table();
  if (track >= 128) return -1;
  out->int_val = table[track].mode;
  return 0;
}

static int din_map_param_set_mode(uint8_t track, const param_value_t* val) {
  DIN_MapEntry* table = (DIN_MapEntry*)din_map_get_table();
  if (track >= 128 || val->int_val < 0 || val->int_val >= 3) return -1;
  table[track].mode = (uint8_t)val->int_val;
  return 0;
}

static int din_map_param_get_velocity(uint8_t track, param_value_t* out) {
  const DIN_MapEntry* table = din_map_get_table();
  if (track >= 128) return -1;
  out->int_val = table[track].velocity;
  return 0;
}

static int din_map_param_set_velocity(uint8_t track, const param_value_t* val) {
  DIN_MapEntry* table = (DIN_MapEntry*)din_map_get_table();
  if (track >= 128 || val->int_val < 0 || val->int_val > 127) return -1;
  table[track].velocity = (uint8_t)val->int_val;
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int din_map_cli_enable(uint8_t track) {
  (void)track;
  return 0; // Always enabled
}

static int din_map_cli_disable(uint8_t track) {
  (void)track;
  return -1; // Cannot disable hardware input
}

static int din_map_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_mode_names[] = {
  "NOTE",      // Send note on/off
  "CC_TOGGLE", // Toggle CC 0/127
  "CC_GATE",   // CC 127 on press, CC 0 on release
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_din_map_descriptor = {
  .name = "din",
  .description = "Digital input (button) to MIDI mapping",
  .category = MODULE_CATEGORY_INPUT,
  .init = din_map_init_defaults,
  .enable = din_map_cli_enable,
  .disable = din_map_cli_disable,
  .get_status = din_map_cli_get_status,
  .has_per_track_state = 1,  // Per-button configuration
  .is_global = 0,
  .max_tracks = 128  // Maximum DIN inputs
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_din_map_parameters(void) {
  module_param_t params[] = {
    {
      .name = "button_count",
      .description = "Total number of DIN buttons",
      .type = PARAM_TYPE_INT,
      .min = 128,
      .max = 128,
      .read_only = 1,
      .get_value = din_map_param_get_button_count,
      .set_value = NULL
    },
    {
      .name = "note",
      .description = "MIDI note number (0-127, for NOTE mode)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 127,
      .read_only = 0,
      .get_value = din_map_param_get_note,
      .set_value = din_map_param_set_note
    },
    {
      .name = "cc",
      .description = "MIDI CC number (0-127, for CC modes)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 127,
      .read_only = 0,
      .get_value = din_map_param_get_cc,
      .set_value = din_map_param_set_cc
    },
    {
      .name = "channel",
      .description = "MIDI channel (0-15)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 15,
      .read_only = 0,
      .get_value = din_map_param_get_channel,
      .set_value = din_map_param_set_channel
    },
    {
      .name = "mode",
      .description = "Button mode (NOTE, CC_TOGGLE, CC_GATE)",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = 2,
      .enum_values = s_mode_names,
      .enum_count = 3,
      .read_only = 0,
      .get_value = din_map_param_get_mode,
      .set_value = din_map_param_set_mode
    },
    {
      .name = "velocity",
      .description = "Note velocity (0-127, for NOTE mode)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 127,
      .read_only = 0,
      .get_value = din_map_param_get_velocity,
      .set_value = din_map_param_set_velocity
    }
  };
  
  s_din_map_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_din_map_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int din_map_register_cli(void) {
  setup_din_map_parameters();
  return module_registry_register(&s_din_map_descriptor);
}
