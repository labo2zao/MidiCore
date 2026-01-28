/**
 * @file rhythm_trainer_cli.c
 * @brief CLI integration for rhythm trainer
 * 
 * Pedagogical tool for timing practice with real-time feedback
 */

#include "Services/rhythm_trainer/rhythm_trainer.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static rhythm_cfg_t s_cfg = {0};

static int rhythm_trainer_param_get_enabled(uint8_t track, param_value_t* out) {
  (void)track;
  out->bool_val = s_cfg.enabled;
  return 0;
}

static int rhythm_trainer_param_set_enabled(uint8_t track, const param_value_t* val) {
  (void)track;
  s_cfg.enabled = val->bool_val;
  return 0;
}

static int rhythm_trainer_param_get_subdivision(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = s_cfg.subdivision;
  return 0;
}

static int rhythm_trainer_param_set_subdivision(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val >= RHYTHM_SUBDIV_COUNT) return -1;
  s_cfg.subdivision = (uint8_t)val->int_val;
  return 0;
}

static int rhythm_trainer_param_get_perfect_window(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = s_cfg.perfect_window;
  return 0;
}

static int rhythm_trainer_param_set_perfect_window(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val > 96) return -1;
  s_cfg.perfect_window = (uint16_t)val->int_val;
  return 0;
}

static int rhythm_trainer_param_get_good_window(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = s_cfg.good_window;
  return 0;
}

static int rhythm_trainer_param_set_good_window(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val > 96) return -1;
  s_cfg.good_window = (uint16_t)val->int_val;
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int rhythm_trainer_cli_enable(uint8_t track) {
  (void)track;
  s_cfg.enabled = 1;
  return 0;
}

static int rhythm_trainer_cli_disable(uint8_t track) {
  (void)track;
  s_cfg.enabled = 0;
  return 0;
}

static int rhythm_trainer_cli_get_status(uint8_t track) {
  (void)track;
  return s_cfg.enabled ? MODULE_STATUS_ENABLED : MODULE_STATUS_DISABLED;
}

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_subdivision_names[] = {
  "1/4", "1/8", "1/16", "1/32",
  "1/8T", "1/16T",
  "1/4.", "1/8.", "1/16.",
  "5-TUPLET", "7-TUPLET", "8-TUPLET",
  "11-TUPLET", "13-TUPLET"
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_rhythm_trainer_descriptor = {
  .name = "rhythm_trainer",
  .description = "Rhythm training with timing feedback",
  .category = MODULE_CATEGORY_EFFECT,
  .init = NULL, // No init function needed
  .enable = rhythm_trainer_cli_enable,
  .disable = rhythm_trainer_cli_disable,
  .get_status = rhythm_trainer_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_rhythm_trainer_parameters(void) {
  module_param_t params[] = {
    {
      .name = "enabled",
      .description = "Enable rhythm training",
      .type = PARAM_TYPE_BOOL,
      .read_only = 0,
      .get_value = rhythm_trainer_param_get_enabled,
      .set_value = rhythm_trainer_param_set_enabled
    },
    {
      .name = "subdivision",
      .description = "Target grid subdivision",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = RHYTHM_SUBDIV_COUNT - 1,
      .enum_values = s_subdivision_names,
      .enum_count = RHYTHM_SUBDIV_COUNT,
      .read_only = 0,
      .get_value = rhythm_trainer_param_get_subdivision,
      .set_value = rhythm_trainer_param_set_subdivision
    },
    {
      .name = "perfect_window",
      .description = "Perfect timing window (±ticks)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 96,
      .read_only = 0,
      .get_value = rhythm_trainer_param_get_perfect_window,
      .set_value = rhythm_trainer_param_set_perfect_window
    },
    {
      .name = "good_window",
      .description = "Good timing window (±ticks)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 96,
      .read_only = 0,
      .get_value = rhythm_trainer_param_get_good_window,
      .set_value = rhythm_trainer_param_set_good_window
    }
  };
  
  s_rhythm_trainer_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_rhythm_trainer_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int rhythm_trainer_register_cli(void) {
  // Initialize default configuration
  s_cfg.enabled = 0;
  s_cfg.perfect_window = 4;   // ~10ms @ 120bpm
  s_cfg.good_window = 12;     // ~30ms @ 120bpm
  s_cfg.off_window = 48;      // 1/8 note
  s_cfg.subdivision = RHYTHM_SUBDIV_1_16;
  
  setup_rhythm_trainer_parameters();
  return module_registry_register(&s_rhythm_trainer_descriptor);
}
