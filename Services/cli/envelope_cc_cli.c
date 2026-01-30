/**
 * @file envelope_cc_cli.c
 * @brief CLI integration for envelope_cc module
 * 
 * ADSR envelope to CC output
 */

#include "Services/envelope_cc/envelope_cc.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

DEFINE_PARAM_BOOL_TRACK(envelope_cc, enabled, envelope_cc_is_enabled, envelope_cc_set_enabled)

DEFINE_PARAM_INT_TRACK(envelope_cc, channel, envelope_cc_get_channel, envelope_cc_set_channel)

DEFINE_PARAM_INT_TRACK(envelope_cc, cc_number, envelope_cc_get_cc_number, envelope_cc_set_cc_number)

DEFINE_PARAM_INT_TRACK(envelope_cc, attack, envelope_cc_get_attack, envelope_cc_set_attack)

DEFINE_PARAM_INT_TRACK(envelope_cc, decay, envelope_cc_get_decay, envelope_cc_set_decay)

DEFINE_PARAM_INT_TRACK(envelope_cc, sustain, envelope_cc_get_sustain, envelope_cc_set_sustain)

DEFINE_PARAM_INT_TRACK(envelope_cc, release, envelope_cc_get_release, envelope_cc_set_release)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

DEFINE_MODULE_CONTROL_TRACK(envelope_cc, envelope_cc_set_enabled, envelope_cc_is_enabled)

// =============================================================================
// INIT WRAPPER
// =============================================================================

static int envelope_cc_cli_init(void) {
  envelope_cc_init();
  return 0;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_envelope_cc_descriptor = {
  .name = "envelope_cc",
  .description = "ADSR envelope to CC output",
  .category = MODULE_CATEGORY_EFFECT,
  .init = envelope_cc_cli_init,
  .enable = envelope_cc_cli_enable,
  .disable = envelope_cc_cli_disable,
  .get_status = envelope_cc_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_envelope_cc_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(envelope_cc, enabled, "Enable envelope"),
    PARAM_INT(envelope_cc, channel, "Output channel (0-15)", 0, 15),
    PARAM_INT(envelope_cc, cc_number, "CC to modulate (0-127)", 0, 127),
    PARAM_INT(envelope_cc, attack, "Attack time (0-5000ms)", 0, 5000),
    PARAM_INT(envelope_cc, decay, "Decay time (0-5000ms)", 0, 5000),
    PARAM_INT(envelope_cc, sustain, "Sustain level (0-127)", 0, 127),
    PARAM_INT(envelope_cc, release, "Release time (0-5000ms)", 0, 5000),
  };
  
  s_envelope_cc_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_envelope_cc_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int envelope_cc_register_cli(void) {
  setup_envelope_cc_parameters();
  return module_registry_register(&s_envelope_cc_descriptor);
}
