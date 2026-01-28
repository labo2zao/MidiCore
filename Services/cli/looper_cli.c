/**
 * @file looper_cli.c
 * @brief CLI integration for looper module
 * 
 * Comprehensive CLI support for the looper/sequencer system.
 * Provides access to all transport, track, scene, and recording functions.
 */

#include "Services/looper/looper.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS - Transport
// =============================================================================

static int looper_param_get_bpm(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = looper_get_tempo();
  return 0;
}

static int looper_param_set_bpm(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 20 || val->int_val > 300) return -1;
  looper_set_tempo((uint16_t)val->int_val);
  return 0;
}

static int looper_param_get_time_sig_num(uint8_t track, param_value_t* out) {
  (void)track;
  looper_transport_t t;
  looper_get_transport(&t);
  out->int_val = t.ts_num;
  return 0;
}

static int looper_param_set_time_sig_num(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 1 || val->int_val > 16) return -1;
  looper_transport_t t;
  looper_get_transport(&t);
  t.ts_num = (uint8_t)val->int_val;
  looper_set_transport(&t);
  return 0;
}

static int looper_param_get_time_sig_den(uint8_t track, param_value_t* out) {
  (void)track;
  looper_transport_t t;
  looper_get_transport(&t);
  out->int_val = t.ts_den;
  return 0;
}

static int looper_param_set_time_sig_den(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val != 2 && val->int_val != 4 && val->int_val != 8 && val->int_val != 16) return -1;
  looper_transport_t t;
  looper_get_transport(&t);
  t.ts_den = (uint8_t)val->int_val;
  looper_set_transport(&t);
  return 0;
}

static int looper_param_get_auto_loop(uint8_t track, param_value_t* out) {
  (void)track;
  looper_transport_t t;
  looper_get_transport(&t);
  out->bool_val = t.auto_loop;
  return 0;
}

static int looper_param_set_auto_loop(uint8_t track, const param_value_t* val) {
  (void)track;
  looper_transport_t t;
  looper_get_transport(&t);
  t.auto_loop = val->bool_val ? 1 : 0;
  looper_set_transport(&t);
  return 0;
}

// =============================================================================
// PARAMETER WRAPPERS - Per-Track State
// =============================================================================

static int looper_param_get_state(uint8_t track, param_value_t* out) {
  out->int_val = looper_get_state(track);
  return 0;
}

static int looper_param_set_state(uint8_t track, const param_value_t* val) {
  if (val->int_val < 0 || val->int_val > LOOPER_STATE_OVERDUB_NOTES_ONLY) return -1;
  looper_set_state(track, (looper_state_t)val->int_val);
  return 0;
}

static int looper_param_get_mute(uint8_t track, param_value_t* out) {
  out->bool_val = looper_get_mute(track);
  return 0;
}

static int looper_param_set_mute(uint8_t track, const param_value_t* val) {
  looper_set_mute(track, val->bool_val);
  return 0;
}

static int looper_param_get_solo(uint8_t track, param_value_t* out) {
  out->bool_val = looper_get_solo(track);
  return 0;
}

static int looper_param_set_solo(uint8_t track, const param_value_t* val) {
  looper_set_solo(track, val->bool_val);
  return 0;
}

static int looper_param_get_quantize(uint8_t track, param_value_t* out) {
  out->int_val = looper_get_quantize(track);
  return 0;
}

static int looper_param_set_quantize(uint8_t track, const param_value_t* val) {
  if (val->int_val < 0 || val->int_val >= LOOPER_QUANT_COUNT) return -1;
  looper_set_quantize(track, (looper_quant_t)val->int_val);
  return 0;
}

static int looper_param_get_midi_channel(uint8_t track, param_value_t* out) {
  out->int_val = looper_get_midi_channel(track);
  return 0;
}

static int looper_param_set_midi_channel(uint8_t track, const param_value_t* val) {
  if (val->int_val < 0 || val->int_val > 15) return -1;
  looper_set_midi_channel(track, (uint8_t)val->int_val);
  return 0;
}

static int looper_param_get_transpose(uint8_t track, param_value_t* out) {
  out->int_val = looper_get_transpose(track);
  return 0;
}

static int looper_param_set_transpose(uint8_t track, const param_value_t* val) {
  if (val->int_val < -127 || val->int_val > 127) return -1;
  looper_set_transpose(track, (int8_t)val->int_val);
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int looper_cli_enable(uint8_t track) {
  looper_set_state(track, LOOPER_STATE_PLAY);
  return 0;
}

static int looper_cli_disable(uint8_t track) {
  looper_set_state(track, LOOPER_STATE_STOP);
  return 0;
}

static int looper_cli_get_status(uint8_t track) {
  looper_state_t state = looper_get_state(track);
  return (state != LOOPER_STATE_STOP) ? MODULE_STATUS_ENABLED : MODULE_STATUS_DISABLED;
}

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_looper_state_names[] = {
  "STOP",
  "REC",
  "PLAY",
  "OVERDUB",
  "OVERDUB_CC_ONLY",
  "OVERDUB_NOTES_ONLY"
};

static const char* s_looper_quant_names[] = {
  "OFF",
  "1_16",
  "1_8",
  "1_4",
  "1_32T",
  "1_16T",
  "1_8T",
  "1_2T",
  "1_32Q",
  "1_16Q",
  "1_8Q",
  "1_16S",
  "1_8S",
  "1_16SEPT",
  "1_8SEPT",
  "1_16_DOT",
  "1_8_DOT",
  "1_4_DOT"
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_looper_descriptor = {
  .name = "looper",
  .description = "Multi-track looper/sequencer (LoopA-inspired)",
  .category = MODULE_CATEGORY_LOOPER,
  .init = looper_init,
  .enable = looper_cli_enable,
  .disable = looper_cli_disable,
  .get_status = looper_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_looper_parameters(void) {
  module_param_t params[] = {
    // Global transport parameters
    {
      .name = "bpm",
      .description = "Tempo (20-300 BPM)",
      .type = PARAM_TYPE_INT,
      .min = 20,
      .max = 300,
      .read_only = 0,
      .get_value = looper_param_get_bpm,
      .set_value = looper_param_set_bpm
    },
    {
      .name = "time_sig_num",
      .description = "Time signature numerator (1-16)",
      .type = PARAM_TYPE_INT,
      .min = 1,
      .max = 16,
      .read_only = 0,
      .get_value = looper_param_get_time_sig_num,
      .set_value = looper_param_set_time_sig_num
    },
    {
      .name = "time_sig_den",
      .description = "Time signature denominator (2,4,8,16)",
      .type = PARAM_TYPE_INT,
      .min = 2,
      .max = 16,
      .read_only = 0,
      .get_value = looper_param_get_time_sig_den,
      .set_value = looper_param_set_time_sig_den
    },
    {
      .name = "auto_loop",
      .description = "Auto-stop recording at loop length",
      .type = PARAM_TYPE_BOOL,
      .min = 0,
      .max = 1,
      .read_only = 0,
      .get_value = looper_param_get_auto_loop,
      .set_value = looper_param_set_auto_loop
    },
    // Per-track parameters
    {
      .name = "state",
      .description = "Track state (STOP/REC/PLAY/OVERDUB/...)",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = LOOPER_STATE_OVERDUB_NOTES_ONLY,
      .enum_values = s_looper_state_names,
      .enum_count = 6,
      .read_only = 0,
      .get_value = looper_param_get_state,
      .set_value = looper_param_set_state
    },
    {
      .name = "mute",
      .description = "Mute track",
      .type = PARAM_TYPE_BOOL,
      .min = 0,
      .max = 1,
      .read_only = 0,
      .get_value = looper_param_get_mute,
      .set_value = looper_param_set_mute
    },
    {
      .name = "solo",
      .description = "Solo track (mute others)",
      .type = PARAM_TYPE_BOOL,
      .min = 0,
      .max = 1,
      .read_only = 0,
      .get_value = looper_param_get_solo,
      .set_value = looper_param_set_solo
    },
    {
      .name = "quantize",
      .description = "Quantization grid",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = LOOPER_QUANT_COUNT - 1,
      .enum_values = s_looper_quant_names,
      .enum_count = LOOPER_QUANT_COUNT,
      .read_only = 0,
      .get_value = looper_param_get_quantize,
      .set_value = looper_param_set_quantize
    },
    {
      .name = "midi_channel",
      .description = "MIDI output channel (0-15)",
      .type = PARAM_TYPE_INT,
      .min = 0,
      .max = 15,
      .read_only = 0,
      .get_value = looper_param_get_midi_channel,
      .set_value = looper_param_set_midi_channel
    },
    {
      .name = "transpose",
      .description = "Transpose semitones (-127 to +127)",
      .type = PARAM_TYPE_INT,
      .min = -127,
      .max = 127,
      .read_only = 0,
      .get_value = looper_param_get_transpose,
      .set_value = looper_param_set_transpose
    }
  };
  
  s_looper_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_looper_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

/**
 * @brief Register looper module with CLI
 * Call this from looper_init() or app initialization
 */
int looper_register_cli(void) {
  setup_looper_parameters();
  return module_registry_register(&s_looper_descriptor);
}

// =============================================================================
// CLI USAGE EXAMPLES
// =============================================================================

/*
 * Global Transport Commands:
 * 
 * module set looper bpm 120
 * module set looper time_sig_num 4
 * module set looper time_sig_den 4
 * module set looper auto_loop true
 * module get looper bpm
 * 
 * Per-Track Commands (use track index 0-3):
 * 
 * # Transport control
 * module set looper state 0 REC          # Start recording track 0
 * module set looper state 0 PLAY         # Play track 0
 * module set looper state 0 OVERDUB      # Overdub track 0
 * module set looper state 0 STOP         # Stop track 0
 * 
 * # Track control
 * module set looper mute 0 true
 * module set looper solo 1 true
 * module set looper midi_channel 0 5
 * module set looper transpose 1 -12
 * 
 * # Quantization
 * module set looper quantize 0 1_16      # 1/16 note quantization
 * module set looper quantize 1 1_8T      # 1/8 triplet
 * module set looper quantize 2 OFF       # No quantization
 * 
 * # Quick enable/disable
 * module enable looper 0     # Set track 0 to PLAY
 * module disable looper 0    # Set track 0 to STOP
 * module status looper 0     # Check if playing
 * 
 * # List all parameters
 * module params looper
 * 
 * # Get parameter values
 * module get looper state 0
 * module get looper mute 1
 * module get looper quantize 2
 */
