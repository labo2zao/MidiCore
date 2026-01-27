/**
 * @file metronome_cli.c
 * @brief CLI integration for metronome module
 * 
 * This file adds CLI support to the metronome module using the helper macros.
 * Add this file to your build and call metronome_register_cli() from metronome_init().
 */

#include "Services/metronome/metronome.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

// Simple boolean parameter
DEFINE_PARAM_BOOL(metronome, enabled, metronome_get_enabled, metronome_set_enabled)

// Integer parameters that require config access
static int metronome_param_get_midi_channel(uint8_t track, param_value_t* out) {
  (void)track;
  metronome_config_t config;
  metronome_get_config(&config);
  out->int_val = config.midi_channel;
  return 0;
}

static int metronome_param_set_midi_channel(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val > 15) return -1;
  
  metronome_config_t config;
  metronome_get_config(&config);
  config.midi_channel = (uint8_t)val->int_val;
  metronome_set_config(&config);
  return 0;
}

static int metronome_param_get_accent_note(uint8_t track, param_value_t* out) {
  (void)track;
  metronome_config_t config;
  metronome_get_config(&config);
  out->int_val = config.accent_note;
  return 0;
}

static int metronome_param_set_accent_note(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val > 127) return -1;
  
  metronome_config_t config;
  metronome_get_config(&config);
  config.accent_note = (uint8_t)val->int_val;
  metronome_set_config(&config);
  return 0;
}

static int metronome_param_get_regular_note(uint8_t track, param_value_t* out) {
  (void)track;
  metronome_config_t config;
  metronome_get_config(&config);
  out->int_val = config.regular_note;
  return 0;
}

static int metronome_param_set_regular_note(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val > 127) return -1;
  
  metronome_config_t config;
  metronome_get_config(&config);
  config.regular_note = (uint8_t)val->int_val;
  metronome_set_config(&config);
  return 0;
}

static int metronome_param_get_accent_velocity(uint8_t track, param_value_t* out) {
  (void)track;
  metronome_config_t config;
  metronome_get_config(&config);
  out->int_val = config.accent_velocity;
  return 0;
}

static int metronome_param_set_accent_velocity(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 1 || val->int_val > 127) return -1;
  
  metronome_config_t config;
  metronome_get_config(&config);
  config.accent_velocity = (uint8_t)val->int_val;
  metronome_set_config(&config);
  return 0;
}

static int metronome_param_get_regular_velocity(uint8_t track, param_value_t* out) {
  (void)track;
  metronome_config_t config;
  metronome_get_config(&config);
  out->int_val = config.regular_velocity;
  return 0;
}

static int metronome_param_set_regular_velocity(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 1 || val->int_val > 127) return -1;
  
  metronome_config_t config;
  metronome_get_config(&config);
  config.regular_velocity = (uint8_t)val->int_val;
  metronome_set_config(&config);
  return 0;
}

static int metronome_param_get_mode(uint8_t track, param_value_t* out) {
  (void)track;
  metronome_config_t config;
  metronome_get_config(&config);
  out->int_val = config.mode;
  return 0;
}

static int metronome_param_set_mode(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val > METRONOME_MODE_AUDIO) return -1;
  
  metronome_config_t config;
  metronome_get_config(&config);
  config.mode = (uint8_t)val->int_val;
  metronome_set_config(&config);
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_GLOBAL(metronome, metronome_set_enabled, metronome_get_enabled)

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_metronome_mode_names[] = {
  "OFF",
  "MIDI",
  "AUDIO"
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_metronome_descriptor = {
  .name = "metronome",
  .description = "Metronome synchronized to looper BPM",
  .category = MODULE_CATEGORY_GENERATOR,
  .init = metronome_init,
  .enable = metronome_cli_enable,
  .disable = metronome_cli_disable,
  .get_status = metronome_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_metronome_parameters(void) {
  module_param_t params[] = {
    {
      .name = "enabled",
      .description = "Enable metronome",
      .type = PARAM_TYPE_BOOL,
      .min = 0,
      .max = 1,
      .read_only = 0,
      .get_value = metronome_param_get_enabled,
      .set_value = metronome_param_set_enabled
    },
    {
      .name = "mode",
      .description = "Output mode (0=OFF, 1=MIDI, 2=AUDIO)",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = METRONOME_MODE_AUDIO,
      .enum_values = s_metronome_mode_names,
      .enum_count = 3,
      .read_only = 0,
      .get_value = metronome_param_get_mode,
      .set_value = metronome_param_set_mode
    },
    {
      .name = "midi_channel",
      .description = "MIDI output channel (0-15)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 15,
      .read_only = 0,
      .get_value = metronome_param_get_midi_channel,
      .set_value = metronome_param_set_midi_channel
    },
    {
      .name = "accent_note",
      .description = "MIDI note for accent (downbeat) (0-127)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 127,
      .read_only = 0,
      .get_value = metronome_param_get_accent_note,
      .set_value = metronome_param_set_accent_note
    },
    {
      .name = "regular_note",
      .description = "MIDI note for regular beat (0-127)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 127,
      .read_only = 0,
      .get_value = metronome_param_get_regular_note,
      .set_value = metronome_param_set_regular_note
    },
    {
      .name = "accent_velocity",
      .description = "Velocity for accent beat (1-127)",
      .type = PARAM_TYPE_INT,
      .min = 1,
      .max = 127,
      .read_only = 0,
      .get_value = metronome_param_get_accent_velocity,
      .set_value = metronome_param_set_accent_velocity
    },
    {
      .name = "regular_velocity",
      .description = "Velocity for regular beat (1-127)",
      .type = PARAM_TYPE_INT,
      .min = 1,
      .max = 127,
      .read_only = 0,
      .get_value = metronome_param_get_regular_velocity,
      .set_value = metronome_param_set_regular_velocity
    }
  };
  
  s_metronome_descriptor.param_count = 7;
  memcpy(s_metronome_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

/**
 * @brief Register metronome module with CLI
 * 
 * Call this from metronome_init() or app initialization.
 * 
 * Example in metronome.c:
 * 
 *   void metronome_init(void) {
 *     // ... existing init code ...
 *     
 *     #ifdef MODULE_ENABLE_CLI
 *     metronome_register_cli();
 *     #endif
 *   }
 */
int metronome_register_cli(void) {
  setup_metronome_parameters();
  return module_registry_register(&s_metronome_descriptor);
}

// =============================================================================
// CLI USAGE EXAMPLES
// =============================================================================

/*
 * Once registered, these commands become available:
 * 
 * # Enable metronome
 * module enable metronome
 * 
 * # Disable metronome
 * module disable metronome
 * 
 * # Check status
 * module status metronome
 * 
 * # List parameters
 * module params metronome
 * 
 * # Get parameter values
 * module get metronome enabled
 * module get metronome midi_channel
 * module get metronome accent_note
 * 
 * # Set parameter values
 * module set metronome enabled true
 * module set metronome mode 1           # MIDI
 * module set metronome midi_channel 9   # Drum channel
 * module set metronome accent_note 76   # High wood block
 * module set metronome regular_note 77  # Low wood block
 * module set metronome accent_velocity 100
 * module set metronome regular_velocity 80
 * 
 * # Save configuration
 * config save 0:/metronome.ini
 * 
 * # Configuration file format (metronome.ini):
 * [metronome]
 * enabled = true
 * mode = 1
 * midi_channel = 9
 * accent_note = 76
 * regular_note = 77
 * accent_velocity = 100
 * regular_velocity = 80
 */
