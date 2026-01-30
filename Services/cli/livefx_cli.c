/**
 * @file livefx_cli.c
 * @brief CLI integration for livefx module
 * 
 * Live FX system for real-time control
 */

#include "Services/livefx/livefx.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static uint8_t livefx_stub_is_enabled(uint8_t track) {
  return livefx_get_enabled(track);
}

DEFINE_PARAM_BOOL_TRACK(livefx, enabled, livefx_stub_is_enabled, livefx_set_enabled)

DEFINE_PARAM_INT_TRACK(livefx, transpose, livefx_get_transpose, livefx_set_transpose)

DEFINE_PARAM_INT_TRACK(livefx, velocity_scale, livefx_get_velocity_scale, livefx_set_velocity_scale)

// Note: livefx_get_force_scale() has 4 params (track, scale_type*, root*, enable*)
// which doesn't fit DEFINE_PARAM_BOOL_TRACK pattern. Need custom wrappers.
static uint8_t livefx_stub_get_force_scale_enabled(uint8_t track) {
  uint8_t scale_type, root, enable;
  livefx_get_force_scale(track, &scale_type, &root, &enable);
  return enable;
}

static void livefx_stub_set_force_scale_enabled(uint8_t track, uint8_t value) {
  uint8_t scale_type, root, enable;
  livefx_get_force_scale(track, &scale_type, &root, &enable);
  livefx_set_force_scale(track, scale_type, root, value);
}

DEFINE_PARAM_BOOL_TRACK(livefx, force_scale, livefx_stub_get_force_scale_enabled, livefx_stub_set_force_scale_enabled)

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static uint8_t livefx_stub_is_enabled_for_control(uint8_t track) {
  return livefx_get_enabled(track);
}

DEFINE_MODULE_CONTROL_TRACK(livefx, livefx_set_enabled, livefx_stub_is_enabled_for_control)

// =============================================================================
// INIT WRAPPER
// =============================================================================

static int livefx_cli_init(void) {
  livefx_init();
  return 0;
}

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_livefx_descriptor = {
  .name = "livefx",
  .description = "Live FX system for real-time control",
  .category = MODULE_CATEGORY_EFFECT,
  .init = livefx_cli_init,
  .enable = livefx_cli_enable,
  .disable = livefx_cli_disable,
  .get_status = livefx_cli_get_status,
  .has_per_track_state = 1,
  .is_global = 0
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_livefx_parameters(void) {
  module_param_t params[] = {
    PARAM_BOOL(livefx, enabled, "Enable live FX"),
    PARAM_INT(livefx, transpose, "Transpose semitones (-12 to +12)", -12, 12),
    PARAM_INT(livefx, velocity_scale, "Velocity scale (0-200%)", 0, 200),
    PARAM_BOOL(livefx, force_scale, "Force to scale"),
  };
  
  s_livefx_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_livefx_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int livefx_register_cli(void) {
  setup_livefx_parameters();
  return module_registry_register(&s_livefx_descriptor);
}
