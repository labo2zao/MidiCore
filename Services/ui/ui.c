#include "Services/ui/ui.h"
#include "Services/safe/safe_mode.h"
#include "Services/fs/sd_guard.h"
#include "Services/ui/ui_gfx.h"
#include "Services/ui/ui_page_looper.h"
#include "Services/ui/ui_page_looper_timeline.h"
#if MODULE_ENABLE_UI_PAGE_PIANOROLL
#include "Services/ui/ui_page_looper_pianoroll.h"
#endif
#include "Services/ui/ui_page_song.h"
#include "Services/ui/ui_page_midi_monitor.h"
#include "Services/ui/ui_page_sysex.h"
#include "Services/ui/ui_page_config.h"
#include "Services/ui/ui_page_livefx.h"
#include "Services/ui/ui_page_rhythm.h"
#include "Services/ui/ui_page_automation.h"
#if MODULE_TEST_OLED
#include "Services/ui/ui_page_oled_test.h"
#endif
#if MODULE_ENABLE_LFO && MODULE_ENABLE_HUMANIZER
#include "Services/ui/ui_page_humanizer.h"
#endif
#include "Services/ui/ui_state.h"
#include "Services/ui/chord_cfg.h"
#include "Hal/oled_ssd1322/oled_ssd1322.h"
#include <string.h>
#include <stdio.h>
static char s_status_line[22] = {0};
void ui_set_status_line(const char* line) {
  if (!line) { s_status_line[0]=0; return; }
  strncpy(s_status_line, line, sizeof(s_status_line)-1);
  s_status_line[sizeof(s_status_line)-1]=0;
}


static ui_page_t g_page = UI_PAGE_LOOPER;
static uint32_t g_ms = 0;
static uint32_t g_last_flush = 0;
static uint8_t g_chord_mode = 0;
static chord_bank_t g_chord_bank;

// Button state tracking for combined key navigation (LoopA-style)
static uint8_t g_button_state[10] = {0}; // Track pressed state for buttons 0-9
static uint8_t g_combo_active = 0; // Visual feedback: 1 when B5 held for combinations

static char g_bank_label[24] = "Bank";
static char g_patch_label[24] = "Patch";

void ui_set_patch_status(const char* bank, const char* patch) {
  if (bank && bank[0]) {
    strncpy(g_bank_label, bank, sizeof(g_bank_label)-1);
    g_bank_label[sizeof(g_bank_label)-1] = 0;
  }
  if (patch && patch[0]) {
    strncpy(g_patch_label, patch, sizeof(g_patch_label)-1);
    g_patch_label[sizeof(g_patch_label)-1] = 0;
  }
}

void ui_init(void) {
  ui_gfx_set_fb(oled_framebuffer(), OLED_W, OLED_H);
  oled_clear();
  oled_flush();
}

void ui_set_page(ui_page_t p) { if (p < UI_PAGE_COUNT) { g_page = p; ui_state_mark_dirty(); } }
ui_page_t ui_get_page(void) { return g_page; }

uint8_t ui_get_chord_mode(void) { return g_chord_mode; }
const chord_bank_t* ui_get_chord_bank(void) { return &g_chord_bank; }

int ui_reload_chord_bank(const char* path) {
  const char* p = (path && path[0]) ? path : "/cfg/chord_bank.ngc";
  return chord_bank_load(&g_chord_bank, p);
}
void ui_set_chord_mode(uint8_t en) { g_chord_mode = en ? 1 : 0; ui_state_mark_dirty(); }

void ui_on_button(uint8_t id, uint8_t pressed) {
  // Update button state for combined key detection
  if (id < 10) {
    g_button_state[id] = pressed ? 1 : 0;
  }
  
  // Update combo active flag for visual feedback
  g_combo_active = g_button_state[5];
  
  // Combined key navigation (LoopA-inspired)
  // Only trigger on button press (not release) and when another button is held
  if (pressed) {
    // B5 + B1 -> Piano Roll
    if (id == 1 && g_button_state[5]) {
      g_page = UI_PAGE_LOOPER_PR;
      ui_state_mark_dirty();
      return;
    }
    // B5 + B2 -> Timeline
    if (id == 2 && g_button_state[5]) {
      g_page = UI_PAGE_LOOPER_TL;
      ui_state_mark_dirty();
      return;
    }
    // B5 + B3 -> Rhythm Trainer
    if (id == 3 && g_button_state[5]) {
      g_page = UI_PAGE_RHYTHM;
      ui_state_mark_dirty();
      return;
    }
    // B5 + B4 -> LiveFX
    if (id == 4 && g_button_state[5]) {
      g_page = UI_PAGE_LIVEFX;
      ui_state_mark_dirty();
      return;
    }
    // B5 + B6 -> Song Mode
    if (id == 6 && g_button_state[5]) {
      g_page = UI_PAGE_SONG;
      ui_state_mark_dirty();
      return;
    }
    // B5 + B7 -> Config
    if (id == 7 && g_button_state[5]) {
      g_page = UI_PAGE_CONFIG;
      ui_state_mark_dirty();
      return;
    }
    // B5 + B8 -> Automation
    if (id == 8 && g_button_state[5]) {
      g_page = UI_PAGE_AUTOMATION;
      ui_state_mark_dirty();
      return;
    }
#if MODULE_ENABLE_LFO && MODULE_ENABLE_HUMANIZER
    // B5 + B9 -> Humanizer
    if (id == 9 && g_button_state[5]) {
      g_page = UI_PAGE_HUMANIZER;
      ui_state_mark_dirty();
      return;
    }
#endif
  }
  
  // Button 5 alone cycles through pages
  if (pressed && id == 5 && !g_button_state[1] && !g_button_state[2] && 
      !g_button_state[3] && !g_button_state[4] && !g_button_state[6] &&
      !g_button_state[7] && !g_button_state[8] && !g_button_state[9]) {
    // cycle pages: LOOPER -> TIMELINE -> PIANOROLL -> SONG -> MIDI_MONITOR -> SYSEX -> CONFIG -> LIVEFX -> RHYTHM -> AUTOMATION -> [HUMANIZER] -> OLED_TEST -> LOOPER
    if (g_page == UI_PAGE_LOOPER) g_page = UI_PAGE_LOOPER_TL;
    else if (g_page == UI_PAGE_LOOPER_TL) g_page = UI_PAGE_LOOPER_PR;
    else if (g_page == UI_PAGE_LOOPER_PR) g_page = UI_PAGE_SONG;
    else if (g_page == UI_PAGE_SONG) g_page = UI_PAGE_MIDI_MONITOR;
    else if (g_page == UI_PAGE_MIDI_MONITOR) g_page = UI_PAGE_SYSEX;
    else if (g_page == UI_PAGE_SYSEX) g_page = UI_PAGE_CONFIG;
    else if (g_page == UI_PAGE_CONFIG) g_page = UI_PAGE_LIVEFX;
    else if (g_page == UI_PAGE_LIVEFX) g_page = UI_PAGE_RHYTHM;
    else if (g_page == UI_PAGE_RHYTHM) g_page = UI_PAGE_AUTOMATION;
#if MODULE_ENABLE_LFO && MODULE_ENABLE_HUMANIZER
    else if (g_page == UI_PAGE_AUTOMATION) g_page = UI_PAGE_HUMANIZER;
    else if (g_page == UI_PAGE_HUMANIZER) g_page = UI_PAGE_OLED_TEST;
#else
    else if (g_page == UI_PAGE_AUTOMATION) g_page = UI_PAGE_OLED_TEST;
#endif
    else g_page = UI_PAGE_LOOPER;
    ui_state_mark_dirty();
    return;
  }


// Header (bank/patch + page)
ui_gfx_rect(0, 0, OLED_W, 12, 0); // clear header band (black)
char line1[64];
const char* page = (g_page == UI_PAGE_LOOPER) ? "LOOP" :
                   (g_page == UI_PAGE_LOOPER_TL) ? "TIME" :
                   (g_page == UI_PAGE_LOOPER_PR) ? "PIANO" :
                   (g_page == UI_PAGE_SONG) ? "SONG" :
                   (g_page == UI_PAGE_MIDI_MONITOR) ? "MMON" :
                   (g_page == UI_PAGE_SYSEX) ? "SYSX" :
                   (g_page == UI_PAGE_CONFIG) ? "CONF" :
                   (g_page == UI_PAGE_LIVEFX) ? "LFXC" :
                   (g_page == UI_PAGE_RHYTHM) ? "RHYT" :
                   (g_page == UI_PAGE_AUTOMATION) ? "AUTO" :
#if MODULE_ENABLE_LFO && MODULE_ENABLE_HUMANIZER
                   (g_page == UI_PAGE_HUMANIZER) ? "HUMN" :
#endif
                   (g_page == UI_PAGE_OLED_TEST) ? "TEST" :
                   "UI";
// Bank | Patch | Page (with combo indicator)
if (g_combo_active) {
  snprintf(line1, sizeof(line1), "%s:%s  %s [B5]", g_bank_label, g_patch_label, page);
} else {
  snprintf(line1, sizeof(line1), "%s:%s  %s", g_bank_label, g_patch_label, page);
}
ui_gfx_text(0, 2, line1, 15);

  switch (g_page) {
    case UI_PAGE_LOOPER: ui_page_looper_on_button(id, pressed); break;
    case UI_PAGE_LOOPER_TL: ui_page_looper_timeline_on_button(id, pressed); break;
#if MODULE_ENABLE_UI_PAGE_PIANOROLL
    case UI_PAGE_LOOPER_PR: ui_page_looper_pianoroll_on_button(id, pressed); break;
#endif
    case UI_PAGE_SONG: ui_page_song_on_button(id, pressed); break;
    case UI_PAGE_MIDI_MONITOR: ui_page_midi_monitor_on_button(id, pressed); break;
    case UI_PAGE_SYSEX: ui_page_sysex_on_button(id, pressed); break;
    case UI_PAGE_CONFIG: ui_page_config_on_button(id, pressed); break;
    case UI_PAGE_LIVEFX: ui_page_livefx_on_button(id, pressed); break;
    case UI_PAGE_RHYTHM: ui_page_rhythm_button(id); break;
    case UI_PAGE_AUTOMATION: ui_page_automation_on_button(id, pressed); break;
#if MODULE_ENABLE_LFO && MODULE_ENABLE_HUMANIZER
    case UI_PAGE_HUMANIZER: ui_page_humanizer_on_button(id, pressed); break;
#endif
#if MODULE_TEST_OLED
    case UI_PAGE_OLED_TEST: ui_page_oled_test_on_button(id, pressed); break;
#endif
    default: break;
  }
}
void ui_on_encoder(int8_t delta) {

// Header (bank/patch + page)
ui_gfx_rect(0, 0, OLED_W, 12, 0); // clear header band (black)
char line1[64];
const char* page = (g_page == UI_PAGE_LOOPER) ? "LOOP" :
                   (g_page == UI_PAGE_LOOPER_TL) ? "TIME" :
                   (g_page == UI_PAGE_LOOPER_PR) ? "PIANO" :
                   (g_page == UI_PAGE_SONG) ? "SONG" :
                   (g_page == UI_PAGE_MIDI_MONITOR) ? "MMON" :
                   (g_page == UI_PAGE_SYSEX) ? "SYSX" :
                   (g_page == UI_PAGE_CONFIG) ? "CONF" :
                   (g_page == UI_PAGE_LIVEFX) ? "LFXC" :
                   (g_page == UI_PAGE_RHYTHM) ? "RHYT" :
#if MODULE_ENABLE_LFO && MODULE_ENABLE_HUMANIZER
                   (g_page == UI_PAGE_HUMANIZER) ? "HUMN" :
#endif
                   (g_page == UI_PAGE_OLED_TEST) ? "TEST" :
                   "UI";
// Bank | Patch | Page
snprintf(line1, sizeof(line1), "%s:%s  %s", g_bank_label, g_patch_label, page);
ui_gfx_text(0, 2, line1, 15);

  switch (g_page) {
    case UI_PAGE_LOOPER: ui_page_looper_on_encoder(delta); break;
    case UI_PAGE_LOOPER_TL: ui_page_looper_timeline_on_encoder(delta); break;
#if MODULE_ENABLE_UI_PAGE_PIANOROLL
    case UI_PAGE_LOOPER_PR: ui_page_looper_pianoroll_on_encoder(delta); break;
#endif
    case UI_PAGE_SONG: ui_page_song_on_encoder(delta); break;
    case UI_PAGE_MIDI_MONITOR: ui_page_midi_monitor_on_encoder(delta); break;
    case UI_PAGE_SYSEX: ui_page_sysex_on_encoder(delta); break;
    case UI_PAGE_CONFIG: ui_page_config_on_encoder(delta); break;
    case UI_PAGE_LIVEFX: ui_page_livefx_on_encoder(delta); break;
    case UI_PAGE_RHYTHM: ui_page_rhythm_encoder(delta); break;
    case UI_PAGE_AUTOMATION: ui_page_automation_on_encoder(delta); break;
#if MODULE_ENABLE_LFO && MODULE_ENABLE_HUMANIZER
    case UI_PAGE_HUMANIZER: ui_page_humanizer_on_encoder(delta); break;
#endif
    case UI_PAGE_OLED_TEST: ui_page_oled_test_on_encoder(delta); break;
    default: break;
  }
}

void ui_tick_20ms(void) {
  g_ms += 20;
  ui_state_tick_20ms();


// Header (bank/patch + page)
ui_gfx_rect(0, 0, OLED_W, 12, 0); // clear header band (black)
char line1[64];
const char* page = (g_page == UI_PAGE_LOOPER) ? "LOOP" :
                   (g_page == UI_PAGE_LOOPER_TL) ? "TIME" :
                   (g_page == UI_PAGE_LOOPER_PR) ? "PIANO" :
                   (g_page == UI_PAGE_SONG) ? "SONG" :
                   (g_page == UI_PAGE_MIDI_MONITOR) ? "MMON" :
                   (g_page == UI_PAGE_SYSEX) ? "SYSX" :
                   (g_page == UI_PAGE_CONFIG) ? "CONF" :
                   (g_page == UI_PAGE_LIVEFX) ? "LFXC" :
                   (g_page == UI_PAGE_RHYTHM) ? "RHYT" :
#if MODULE_ENABLE_LFO && MODULE_ENABLE_HUMANIZER
                   (g_page == UI_PAGE_HUMANIZER) ? "HUMN" :
#endif
                   (g_page == UI_PAGE_OLED_TEST) ? "TEST" :
                   "UI";
// Bank | Patch | Page
snprintf(line1, sizeof(line1), "%s:%s  %s", g_bank_label, g_patch_label, page);
ui_gfx_text(0, 2, line1, 15);

  switch (g_page) {
    case UI_PAGE_LOOPER: ui_page_looper_render(g_ms); break;
    case UI_PAGE_LOOPER_TL: ui_page_looper_timeline_render(g_ms); break;
#if MODULE_ENABLE_UI_PAGE_PIANOROLL
    case UI_PAGE_LOOPER_PR: ui_page_looper_pianoroll_render(g_ms); break;
#endif
    case UI_PAGE_SONG: ui_page_song_render(g_ms); break;
    case UI_PAGE_MIDI_MONITOR: ui_page_midi_monitor_render(g_ms); break;
    case UI_PAGE_SYSEX: ui_page_sysex_render(g_ms); break;
    case UI_PAGE_CONFIG: ui_page_config_render(g_ms); break;
    case UI_PAGE_LIVEFX: ui_page_livefx_render(g_ms); break;
    case UI_PAGE_RHYTHM: ui_page_rhythm_update(0); break;
    case UI_PAGE_AUTOMATION: ui_page_automation_render(g_ms); break;
#if MODULE_ENABLE_LFO && MODULE_ENABLE_HUMANIZER
    case UI_PAGE_HUMANIZER: ui_page_humanizer_render(g_ms); break;
#endif
#if MODULE_TEST_OLED
    case UI_PAGE_OLED_TEST: ui_page_oled_test_render(g_ms); break;
#endif
    default: break;
  }

  if ((g_ms - g_last_flush) >= 100) {
    oled_flush();
    g_last_flush = g_ms;
  }
}
