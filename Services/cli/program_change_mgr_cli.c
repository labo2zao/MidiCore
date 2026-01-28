/**
 * @file program_change_mgr_cli.c
 * @brief CLI integration for program change manager
 * 
 * Program change and bank select preset management
 */

#include "Services/program_change_mgr/program_change_mgr.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int pc_mgr_param_get_slot_count(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = PROGRAM_CHANGE_MAX_SLOTS;
  return 0;
}

static int pc_mgr_param_get_program(uint8_t track, param_value_t* out) {
  if (track >= PROGRAM_CHANGE_MAX_SLOTS) return -1;
  program_preset_t preset;
  if (!program_change_mgr_get_preset(track, &preset) || !preset.valid) {
    return -1;
  }
  out->int_val = preset.program;
  return 0;
}

static int pc_mgr_param_get_bank_msb(uint8_t track, param_value_t* out) {
  if (track >= PROGRAM_CHANGE_MAX_SLOTS) return -1;
  program_preset_t preset;
  if (!program_change_mgr_get_preset(track, &preset) || !preset.valid) {
    return -1;
  }
  out->int_val = preset.bank_msb;
  return 0;
}

static int pc_mgr_param_get_bank_lsb(uint8_t track, param_value_t* out) {
  if (track >= PROGRAM_CHANGE_MAX_SLOTS) return -1;
  program_preset_t preset;
  if (!program_change_mgr_get_preset(track, &preset) || !preset.valid) {
    return -1;
  }
  out->int_val = preset.bank_lsb;
  return 0;
}

static int pc_mgr_param_get_channel(uint8_t track, param_value_t* out) {
  if (track >= PROGRAM_CHANGE_MAX_SLOTS) return -1;
  program_preset_t preset;
  if (!program_change_mgr_get_preset(track, &preset) || !preset.valid) {
    return -1;
  }
  out->int_val = preset.channel;
  return 0;
}

static int pc_mgr_param_get_name(uint8_t track, param_value_t* out) {
  if (track >= PROGRAM_CHANGE_MAX_SLOTS) return -1;
  program_preset_t preset;
  if (!program_change_mgr_get_preset(track, &preset) || !preset.valid) {
    out->str_val = "(empty)";
    return 0;
  }
  out->str_val = preset.name;
  return 0;
}

static int pc_mgr_param_get_valid(uint8_t track, param_value_t* out) {
  if (track >= PROGRAM_CHANGE_MAX_SLOTS) return -1;
  program_preset_t preset;
  out->bool_val = (program_change_mgr_get_preset(track, &preset) && preset.valid);
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int pc_mgr_cli_enable(uint8_t track) {
  (void)track;
  return 0; // Always enabled
}

static int pc_mgr_cli_disable(uint8_t track) {
  (void)track;
  return 0;
}

static int pc_mgr_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_pc_mgr_descriptor = {
  .name = "program_change_mgr",
  .description = "Program change/bank select manager",
  .category = MODULE_CATEGORY_MIDI,
  .init = program_change_mgr_init,
  .enable = pc_mgr_cli_enable,
  .disable = pc_mgr_cli_disable,
  .get_status = pc_mgr_cli_get_status,
  .has_per_track_state = 1,  // Per-slot configuration
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_pc_mgr_parameters(void) {
  module_param_t params[] = {
    {
      .name = "slot_count",
      .description = "Total preset slots",
      .type = PARAM_TYPE_INT,
      .min = PROGRAM_CHANGE_MAX_SLOTS,
      .max = PROGRAM_CHANGE_MAX_SLOTS,
      .read_only = 1,
      .get_value = pc_mgr_param_get_slot_count,
      .set_value = NULL
    },
    {
      .name = "program",
      .description = "Program number (0-127)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 127,
      .read_only = 1,
      .get_value = pc_mgr_param_get_program,
      .set_value = NULL
    },
    {
      .name = "bank_msb",
      .description = "Bank MSB (CC 0, 0-127)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 127,
      .read_only = 1,
      .get_value = pc_mgr_param_get_bank_msb,
      .set_value = NULL
    },
    {
      .name = "bank_lsb",
      .description = "Bank LSB (CC 32, 0-127)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 127,
      .read_only = 1,
      .get_value = pc_mgr_param_get_bank_lsb,
      .set_value = NULL
    },
    {
      .name = "channel",
      .description = "MIDI channel (0-15)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 15,
      .read_only = 1,
      .get_value = pc_mgr_param_get_channel,
      .set_value = NULL
    },
    {
      .name = "name",
      .description = "Preset name",
      .type = PARAM_TYPE_STRING,
      .read_only = 1,
      .get_value = pc_mgr_param_get_name,
      .set_value = NULL
    },
    {
      .name = "valid",
      .description = "Slot contains valid data",
      .type = PARAM_TYPE_BOOL,
      .read_only = 1,
      .get_value = pc_mgr_param_get_valid,
      .set_value = NULL
    }
  };
  
  s_pc_mgr_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_pc_mgr_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int program_change_mgr_register_cli(void) {
  setup_pc_mgr_parameters();
  return module_registry_register(&s_pc_mgr_descriptor);
}
