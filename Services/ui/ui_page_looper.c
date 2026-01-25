#include "Services/ui/ui_page_looper.h"
#include "Services/ui/ui_gfx.h"
#include "Services/looper/looper.h"
#include <stdio.h>

static uint8_t sel_track = 0;

static const char* st_name(looper_state_t st) {
  switch (st) {
    case LOOPER_STATE_STOP: return "STOP";
    case LOOPER_STATE_REC: return "REC";
    case LOOPER_STATE_PLAY: return "PLAY";
    case LOOPER_STATE_OVERDUB: return "ODUB";
    default: return "?";
  }
}

void ui_page_looper_render(uint32_t now_ms) {
  (void)now_ms;
  char line[64];

  looper_transport_t tp;
  looper_get_transport(&tp);

  ui_gfx_clear(0);

  // Use 8x8 font for header - more readable
  ui_gfx_set_font(UI_FONT_8X8);
  snprintf(line, sizeof(line), "LOOPER BPM:%3u TS:%u/%u", tp.bpm, tp.ts_num, tp.ts_den);
  ui_gfx_text(0, 0, line, 15);
  ui_gfx_hline(0, 11, 256, 8);

  // Track info with better spacing (14px per track instead of 12px)
  for (uint8_t t=0; t<LOOPER_TRACKS; t++) {
    looper_state_t st = looper_get_state(t);
    uint16_t beats = looper_get_loop_beats(t);
    looper_quant_t q = looper_get_quant(t);
    uint8_t mute = looper_is_track_muted(t);

    snprintf(line, sizeof(line), "%cT%u %-4s L:%u Q:%s M:%u",
             (t==sel_track)?'>':' ', (unsigned)(t+1), st_name(st),
             (unsigned)beats, looper_get_quant_name(q), (unsigned)mute);
    ui_gfx_text(0, 14 + (int)t*13, line, (t==sel_track)?15:12);
  }

  // Footer with button hints - use smaller 5x7 font
  ui_gfx_hline(0, 54, 256, 6);
  ui_gfx_set_font(UI_FONT_5X7);
  ui_gfx_text(0, 56, "B1:REC B2:PLAY B3:STOP B4:MUTE ENC:sel", 10);
}

void ui_page_looper_on_button(uint8_t id, uint8_t pressed) {
  if (!pressed) return;
  switch (id) {
    case 1: looper_set_state(sel_track, LOOPER_STATE_REC); break;
    case 2: looper_set_state(sel_track, LOOPER_STATE_PLAY); break;
    case 3: looper_set_state(sel_track, LOOPER_STATE_STOP); break;
    case 4: looper_set_track_muted(sel_track, (uint8_t)!looper_is_track_muted(sel_track)); break;
    default: break;
  }
}

void ui_page_looper_on_encoder(int8_t delta) {
  if (delta > 0) sel_track = (uint8_t)((sel_track + 1) % LOOPER_TRACKS);
  else if (delta < 0) sel_track = (uint8_t)((sel_track + LOOPER_TRACKS - 1) % LOOPER_TRACKS);
}

uint8_t ui_page_looper_get_track(void) { return sel_track; }
