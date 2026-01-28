/**
 * @file zones_cli.c
 * @brief CLI integration for zone configuration
 * 
 * Keyboard zone mapping with layers and transposition
 */

#include "Services/zones/zones_cfg.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int zones_param_get_zone_count(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = ZONES_MAX;
  return 0;
}

static int zones_param_get_enabled(uint8_t track, param_value_t* out) {
  if (track >= ZONES_MAX) return -1;
  const zones_cfg_t* cfg = zones_cfg_get();
  out->bool_val = cfg->zone[track].enable;
  return 0;
}

static int zones_param_set_enabled(uint8_t track, const param_value_t* val) {
  if (track >= ZONES_MAX) return -1;
  zones_cfg_t cfg = *zones_cfg_get();
  cfg.zone[track].enable = val->bool_val;
  zones_cfg_set(&cfg);
  return 0;
}

static int zones_param_get_key_min(uint8_t track, param_value_t* out) {
  if (track >= ZONES_MAX) return -1;
  const zones_cfg_t* cfg = zones_cfg_get();
  out->int_val = cfg->zone[track].key_min;
  return 0;
}

static int zones_param_set_key_min(uint8_t track, const param_value_t* val) {
  if (track >= ZONES_MAX || val->int_val < 0 || val->int_val > 127) return -1;
  zones_cfg_t cfg = *zones_cfg_get();
  cfg.zone[track].key_min = (uint8_t)val->int_val;
  zones_cfg_set(&cfg);
  return 0;
}

static int zones_param_get_key_max(uint8_t track, param_value_t* out) {
  if (track >= ZONES_MAX) return -1;
  const zones_cfg_t* cfg = zones_cfg_get();
  out->int_val = cfg->zone[track].key_max;
  return 0;
}

static int zones_param_set_key_max(uint8_t track, const param_value_t* val) {
  if (track >= ZONES_MAX || val->int_val < 0 || val->int_val > 127) return -1;
  zones_cfg_t cfg = *zones_cfg_get();
  cfg.zone[track].key_max = (uint8_t)val->int_val;
  zones_cfg_set(&cfg);
  return 0;
}

static int zones_param_get_channel_l1(uint8_t track, param_value_t* out) {
  if (track >= ZONES_MAX) return -1;
  const zones_cfg_t* cfg = zones_cfg_get();
  out->int_val = cfg->zone[track].ch[0];
  return 0;
}

static int zones_param_set_channel_l1(uint8_t track, const param_value_t* val) {
  if (track >= ZONES_MAX || val->int_val < 0 || val->int_val > 15) return -1;
  zones_cfg_t cfg = *zones_cfg_get();
  cfg.zone[track].ch[0] = (uint8_t)val->int_val;
  zones_cfg_set(&cfg);
  return 0;
}

static int zones_param_get_transpose_l1(uint8_t track, param_value_t* out) {
  if (track >= ZONES_MAX) return -1;
  const zones_cfg_t* cfg = zones_cfg_get();
  out->int_val = cfg->zone[track].transpose[0];
  return 0;
}

static int zones_param_set_transpose_l1(uint8_t track, const param_value_t* val) {
  if (track >= ZONES_MAX || val->int_val < -24 || val->int_val > 24) return -1;
  zones_cfg_t cfg = *zones_cfg_get();
  cfg.zone[track].transpose[0] = (int8_t)val->int_val;
  zones_cfg_set(&cfg);
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int zones_cli_enable(uint8_t track) {
  if (track >= ZONES_MAX) return -1;
  zones_cfg_t cfg = *zones_cfg_get();
  cfg.zone[track].enable = 1;
  zones_cfg_set(&cfg);
  return 0;
}

static int zones_cli_disable(uint8_t track) {
  if (track >= ZONES_MAX) return -1;
  zones_cfg_t cfg = *zones_cfg_get();
  cfg.zone[track].enable = 0;
  zones_cfg_set(&cfg);
  return 0;
}

static int zones_cli_get_status(uint8_t track) {
  if (track >= ZONES_MAX) return MODULE_STATUS_ERROR;
  const zones_cfg_t* cfg = zones_cfg_get();
  return cfg->zone[track].enable ? MODULE_STATUS_ENABLED : MODULE_STATUS_DISABLED;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_zones_descriptor = {
  .name = "zones",
  .description = "Keyboard zone mapping with layers",
  .category = MODULE_CATEGORY_EFFECT,
  .init = NULL, // Initialized via config system
  .enable = zones_cli_enable,
  .disable = zones_cli_disable,
  .get_status = zones_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0,
  .max_tracks = ZONES_MAX
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_zones_parameters(void) {
  module_param_t params[] = {
    {
      .name = "zone_count",
      .description = "Total number of zones",
      .type = PARAM_TYPE_INT,
      .min = ZONES_MAX,
      .max = ZONES_MAX,
      .read_only = 1,
      .get_value = zones_param_get_zone_count,
      .set_value = NULL
    },
    {
      .name = "enabled",
      .description = "Zone enabled",
      .type = PARAM_TYPE_BOOL,
      .read_only = 0,
      .get_value = zones_param_get_enabled,
      .set_value = zones_param_set_enabled
    },
    {
      .name = "key_min",
      .description = "Minimum key (0-127)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 127,
      .read_only = 0,
      .get_value = zones_param_get_key_min,
      .set_value = zones_param_set_key_min
    },
    {
      .name = "key_max",
      .description = "Maximum key (0-127)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 127,
      .read_only = 0,
      .get_value = zones_param_get_key_max,
      .set_value = zones_param_set_key_max
    },
    {
      .name = "channel_l1",
      .description = "Layer 1 MIDI channel (0-15)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 15,
      .read_only = 0,
      .get_value = zones_param_get_channel_l1,
      .set_value = zones_param_set_channel_l1
    },
    {
      .name = "transpose_l1",
      .description = "Layer 1 transpose (semitones, -24 to +24)",
      .type = PARAM_TYPE_INT,
      .min = -24,
      .max = 24,
      .read_only = 0,
      .get_value = zones_param_get_transpose_l1,
      .set_value = zones_param_set_transpose_l1
    }
  };
  
  s_zones_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_zones_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int zones_register_cli(void) {
  setup_zones_parameters();
  return module_registry_register(&s_zones_descriptor);
}
