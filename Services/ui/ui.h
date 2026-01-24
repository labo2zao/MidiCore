#pragma once
#include <stdint.h>
#include "Services/ui/chord_cfg.h"

typedef enum {
  UI_PAGE_LOOPER = 0,
  UI_PAGE_LOOPER_TL,
  UI_PAGE_LOOPER_PR,
  UI_PAGE_SONG,
  UI_PAGE_MIDI_MONITOR,
  UI_PAGE_SYSEX,
  UI_PAGE_CONFIG,
  UI_PAGE_LIVEFX,
  UI_PAGE_RHYTHM,
  UI_PAGE_HUMANIZER,
  UI_PAGE_AUTOMATION,
  UI_PAGE_ROUTER,
  UI_PAGE_PATCH,
  UI_PAGE_OLED_TEST,
  UI_PAGE_COUNT
} ui_page_t;

void ui_init(void);
void ui_set_status_line(const char* line);
void ui_tick_20ms(void);

void ui_set_page(ui_page_t p);
ui_page_t ui_get_page(void);

void ui_on_button(uint8_t id, uint8_t pressed);
void ui_on_encoder(int8_t delta);

// Status header
void ui_set_patch_status(const char* bank, const char* patch);

// Optional: chord mode flag (UI-only for now)
uint8_t ui_get_chord_mode(void);

void ui_set_chord_mode(uint8_t en);

const chord_bank_t* ui_get_chord_bank(void);

// Reload chord bank config (SD). If path is NULL/empty, loads default /cfg/chord_bank.ngc
int ui_reload_chord_bank(const char* path);
