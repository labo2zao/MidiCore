/**
 * @file ui_cli.c
 * @brief CLI integration for UI control
 * 
 * OLED UI page navigation and status
 */

#include "Services/ui/ui.h"
#include "Services/cli/module_cli_helpers.h"
#include <string.h>
#include <stdio.h>

// =============================================================================
// PARAMETER WRAPPERS
// =============================================================================

static int ui_param_get_current_page(uint8_t track, param_value_t* out) {
  (void)track;
  out->int_val = (int32_t)ui_get_page();
  return 0;
}

static int ui_param_set_current_page(uint8_t track, const param_value_t* val) {
  (void)track;
  if (val->int_val < 0 || val->int_val >= UI_PAGE_COUNT) return -1;
  ui_set_page((ui_page_t)val->int_val);
  return 0;
}

static int ui_param_get_chord_mode(uint8_t track, param_value_t* out) {
  (void)track;
  out->bool_val = ui_get_chord_mode();
  return 0;
}

static int ui_param_set_chord_mode(uint8_t track, const param_value_t* val) {
  (void)track;
  ui_set_chord_mode(val->bool_val);
  return 0;
}

// =============================================================================
// MODULE CONTROL WRAPPERS
// =============================================================================

static int ui_cli_enable(uint8_t track) {
  (void)track;
  return 0; // Always enabled
}

static int ui_cli_disable(uint8_t track) {
  (void)track;
  return -1; // Cannot disable UI
}

static int ui_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

// =============================================================================
// ENUM STRINGS
// =============================================================================

static const char* s_page_names[] = {
  "LOOPER",
  "LOOPER_TL",
  "LOOPER_PR",
  "SONG",
  "MIDI_MONITOR",
  "SYSEX",
  "CONFIG",
  "LIVEFX",
  "RHYTHM",
  "HUMANIZER",
  "AUTOMATION",
  "ROUTER",
  "PATCH",
  "OLED_TEST"
};

// =============================================================================
// MODULE DESCRIPTOR
// =============================================================================

static module_descriptor_t s_ui_descriptor = {
  .name = "ui",
  .description = "OLED UI control and page navigation",
  .category = MODULE_CATEGORY_SYSTEM,
  .init = ui_init,
  .enable = ui_cli_enable,
  .disable = ui_cli_disable,
  .get_status = ui_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};

// =============================================================================
// PARAMETER SETUP
// =============================================================================

static void setup_ui_parameters(void) {
  module_param_t params[] = {
    {
      .name = "current_page",
      .description = "Current UI page",
      .type = PARAM_TYPE_ENUM,
      .min = 0,
      .max = UI_PAGE_COUNT - 1,
      .enum_values = s_page_names,
      .enum_count = UI_PAGE_COUNT,
      .read_only = 0,
      .get_value = ui_param_get_current_page,
      .set_value = ui_param_set_current_page
    },
    {
      .name = "chord_mode",
      .description = "Chord mode enabled",
      .type = PARAM_TYPE_BOOL,
      .read_only = 0,
      .get_value = ui_param_get_chord_mode,
      .set_value = ui_param_set_chord_mode
    }
  };
  
  s_ui_descriptor.param_count = sizeof(params) / sizeof(params[0]);
  memcpy(s_ui_descriptor.params, params, sizeof(params));
}

// =============================================================================
// REGISTRATION
// =============================================================================

int ui_register_cli(void) {
  setup_ui_parameters();
  return module_registry_register(&s_ui_descriptor);
}
