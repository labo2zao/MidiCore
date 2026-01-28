/**
 * @file midi_monitor_cli.c
 * @brief CLI integration for MIDI message monitoring
 * 
 * MIDI message capture, decode, and filtering
 */

#include "Services/midi_monitor/midi_monitor.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static uint8_t s_enabled = 0;
static uint8_t s_filter_channel = 0xFF; // All channels
static uint8_t s_filter_type = 0xFF;    // All types

static int midi_monitor_param_get_enabled(uint8_t track, param_value_t* out) {
  (void)track;
  out->bool_val = s_enabled;
  return 0;
}

static int midi_monitor_param_set_enabled(uint8_t track, const param_value_t* val) {
  (void)track;
  s_enabled = val->bool_val;
  return 0;
}

static int midi_monitor_param_get_count(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = midi_monitor_get_count();
  return 0;
}

static int midi_monitor_param_get_filter_channel(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = s_filter_channel;
  return 0;
}

static int midi_monitor_param_set_filter_channel(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < -1 || val->int_val > 15) return -1;
  s_filter_channel = (val->int_val == -1) ? 0xFF : (uint8_t)val->int_val;
  return 0;
}

static int midi_monitor_param_get_filter_type(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = s_filter_type;
  return 0;
}

static int midi_monitor_param_set_filter_type(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < -1 || val->int_val > 7) return -1;
  s_filter_type = (val->int_val == -1) ? 0xFF : (uint8_t)val->int_val;
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int midi_monitor_cli_enable(uint8_t track) {
  (void)track;
  s_enabled = 1;
  return 0;
}

static int midi_monitor_cli_disable(uint8_t track) {
  (void)track;
  s_enabled = 0;
  return 0;
}

static int midi_monitor_cli_get_status(uint8_t track) {
  (void)track;
  return s_enabled ? MODULE_STATUS_ENABLED : MODULE_STATUS_DISABLED;
}

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_filter_type_names[] = {
  "NOTE_OFF",
  "NOTE_ON",
  "POLY_PRESSURE",
  "CC",
  "PROGRAM_CHANGE",
  "CHANNEL_PRESSURE",
  "PITCH_BEND",
  "SYSEX",
  "ALL"
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_midi_monitor_descriptor = {
  .name = "midi_monitor",
  .description = "MIDI message capture and decode",
  .category = MODULE_CATEGORY_MIDI,
  .init = midi_monitor_init,
  .enable = midi_monitor_cli_enable,
  .disable = midi_monitor_cli_disable,
  .get_status = midi_monitor_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_midi_monitor_parameters(void) {
  module_param_t params[] = {
    {
      .name = "enabled",
      .description = "Enable MIDI monitoring",
      .type = PARAM_TYPE_BOOL,
      .read_only = 0,
      .get_value = midi_monitor_param_get_enabled,
      .set_value = midi_monitor_param_set_enabled
    },
    {
      .name = "count",
      .description = "Captured message count",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 0x7FFFFFFF,
      .read_only = 1,
      .get_value = midi_monitor_param_get_count,
      .set_value = NULL
    },
    {
      .name = "filter_channel",
      .description = "Filter by MIDI channel (0-15, -1=all)",
      .type = PARAM_TYPE_INT,
      .min = -1,
      .max = 15,
      .read_only = 0,
      .get_value = midi_monitor_param_get_filter_channel,
      .set_value = midi_monitor_param_set_filter_channel
    },
    {
      .name = "filter_type",
      .description = "Filter by message type (-1=all)",
      .type = PARAM_TYPE_ENUM,
      .min = -1,
      .max = 8,
      .enum_values = s_filter_type_names,
      .enum_count = 9,
      .read_only = 0,
      .get_value = midi_monitor_param_get_filter_type,
      .set_value = midi_monitor_param_set_filter_type
    }
  };
  
  s_midi_monitor_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_midi_monitor_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int midi_monitor_register_cli(void) {
  setup_midi_monitor_parameters();
  return module_registry_register(&s_midi_monitor_descriptor);
}
