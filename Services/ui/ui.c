#include "Services/ui/ui.h"
#include "Services/safe/safe_mode.h"
#include "Services/fs/sd_guard.h"
#include "Services/ui/ui_gfx.h"
#include "Services/ui/ui_page_looper.h"
#include "Services/ui/ui_page_looper_timeline.h"
#include "Services/ui/ui_page_looper_pianoroll.h"
#include "Services/ui/ui_page_song.h"
#include "Services/ui/ui_page_midi_monitor.h"
#include "Services/ui/ui_page_sysex.h"
#include "Services/ui/ui_page_config.h"
#include "Services/ui/ui_page_livefx.h"
#include "Services/ui/ui_page_rhythm.h"
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
  if (pressed && id == 5) {
    // cycle pages: OVERVIEW -> TIMELINE -> PIANOROLL -> SONG -> MIDI_MONITOR -> SYSEX -> CONFIG -> LIVEFX -> RHYTHM -> OVERVIEW
    if (g_page == UI_PAGE_LOOPER) g_page = UI_PAGE_LOOPER_TL;
    else if (g_page == UI_PAGE_LOOPER_TL) g_page = UI_PAGE_LOOPER_PR;
    else if (g_page == UI_PAGE_LOOPER_PR) g_page = UI_PAGE_SONG;
    else if (g_page == UI_PAGE_SONG) g_page = UI_PAGE_MIDI_MONITOR;
    else if (g_page == UI_PAGE_MIDI_MONITOR) g_page = UI_PAGE_SYSEX;
    else if (g_page == UI_PAGE_SYSEX) g_page = UI_PAGE_CONFIG;
    else if (g_page == UI_PAGE_CONFIG) g_page = UI_PAGE_LIVEFX;
    else if (g_page == UI_PAGE_LIVEFX) g_page = UI_PAGE_RHYTHM;
    else g_page = UI_PAGE_LOOPER;
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
                   (g_page == UI_PAGE_RHYTHM) ? "RHYT" : "UI";
// Bank | Patch | Page
snprintf(line1, sizeof(line1), "%s:%s  %s", g_bank_label, g_patch_label, page);
ui_gfx_text(0, 2, line1, 15);

  switch (g_page) {
    case UI_PAGE_LOOPER: ui_page_looper_on_button(id, pressed); break;
    case UI_PAGE_LOOPER_TL: ui_page_looper_timeline_on_button(id, pressed); break;
    case UI_PAGE_LOOPER_PR: ui_page_looper_pianoroll_on_button(id, pressed); break;
    case UI_PAGE_SONG: ui_page_song_on_button(id, pressed); break;
    case UI_PAGE_MIDI_MONITOR: ui_page_midi_monitor_on_button(id, pressed); break;
    case UI_PAGE_SYSEX: ui_page_sysex_on_button(id, pressed); break;
    case UI_PAGE_CONFIG: ui_page_config_on_button(id, pressed); break;
    case UI_PAGE_LIVEFX: ui_page_livefx_on_button(id, pressed); break;
    case UI_PAGE_RHYTHM: ui_page_rhythm_button(id); break;
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
                   (g_page == UI_PAGE_RHYTHM) ? "RHYT" : "UI";
// Bank | Patch | Page
snprintf(line1, sizeof(line1), "%s:%s  %s", g_bank_label, g_patch_label, page);
ui_gfx_text(0, 2, line1, 15);

  switch (g_page) {
    case UI_PAGE_LOOPER: ui_page_looper_on_encoder(delta); break;
    case UI_PAGE_LOOPER_TL: ui_page_looper_timeline_on_encoder(delta); break;
    case UI_PAGE_LOOPER_PR: ui_page_looper_pianoroll_on_encoder(delta); break;
    case UI_PAGE_SONG: ui_page_song_on_encoder(delta); break;
    case UI_PAGE_MIDI_MONITOR: ui_page_midi_monitor_on_encoder(delta); break;
    case UI_PAGE_SYSEX: ui_page_sysex_on_encoder(delta); break;
    case UI_PAGE_CONFIG: ui_page_config_on_encoder(delta); break;
    case UI_PAGE_LIVEFX: ui_page_livefx_on_encoder(delta); break;
    case UI_PAGE_RHYTHM: ui_page_rhythm_encoder(delta); break;
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
                   (g_page == UI_PAGE_RHYTHM) ? "RHYT" : "UI";
// Bank | Patch | Page
snprintf(line1, sizeof(line1), "%s:%s  %s", g_bank_label, g_patch_label, page);
ui_gfx_text(0, 2, line1, 15);

  switch (g_page) {
    case UI_PAGE_LOOPER: ui_page_looper_render(g_ms); break;
    case UI_PAGE_LOOPER_TL: ui_page_looper_timeline_render(g_ms); break;
    case UI_PAGE_LOOPER_PR: ui_page_looper_pianoroll_render(g_ms); break;
    case UI_PAGE_SONG: ui_page_song_render(g_ms); break;
    case UI_PAGE_MIDI_MONITOR: ui_page_midi_monitor_render(g_ms); break;
    case UI_PAGE_SYSEX: ui_page_sysex_render(g_ms); break;
    case UI_PAGE_CONFIG: ui_page_config_render(g_ms); break;
    case UI_PAGE_LIVEFX: ui_page_livefx_render(g_ms); break;
    case UI_PAGE_RHYTHM: ui_page_rhythm_update(0); break;
    default: break;
  }

  if ((g_ms - g_last_flush) >= 100) {
    oled_flush();
    g_last_flush = g_ms;
  }
}
