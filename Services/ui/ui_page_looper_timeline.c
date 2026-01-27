#include "Services/ui/ui_page_looper_timeline.h"
#include "Services/ui/ui_gfx.h"
#include "Services/looper/looper.h"
#include <stdio.h>

static inline uint32_t wrap_tick_i32(uint32_t cur, int32_t d, uint32_t L){
  if(L==0) return 0;
  int32_t m = (int32_t)(cur % L);
  int32_t x = m + d;
  x %= (int32_t)L;
  if(x < 0) x += (int32_t)L;
  return (uint32_t)x;
}

#define MAX_SNAP 512  // Event snapshot buffer for timeline display

// shared state (kept simple)
static uint8_t g_track = 0;
static uint32_t g_cursor_tick = 0;

// zoom: ticks per screen width (256px)
static const uint32_t zoom_ticks[] = { 96u, 192u, 384u, 768u, 1536u, 3072u };
static uint8_t g_zoom = 2; // 384 ticks -> 1 bar in 4/4 with PPQN=96
static uint32_t g_sel_idx = 0;
static uint8_t g_in_edit = 0;
static uint8_t g_edit_field = 0; // 0 tick, 1 note, 2 vel

static looper_event_view_t snap[MAX_SNAP];
static uint32_t snap_n = 0;

static uint8_t is_note_on(const looper_event_view_t* e) {
  return (e->len == 3) && ((e->b0 & 0xF0) == 0x90) && (e->b2 != 0);
}

static void refresh_snapshot(void) {
  snap_n = looper_export_events(g_track, snap, MAX_SNAP);
  if (snap_n == 0) g_sel_idx = 0;
  else if (g_sel_idx >= snap_n) g_sel_idx = snap_n - 1;
}

static uint32_t loop_len(void) {
  uint32_t L = looper_get_loop_len_ticks(g_track);
  if (L == 0) L = 96u * 4u;
  return L;
}

static uint32_t tick_to_x(uint32_t tick, uint32_t base, uint32_t span) {
  uint32_t dt = (tick + loop_len() - base) % loop_len();
  if (dt >= span) return 0xFFFFFFFFu;
  // map 0..span -> 0..255
  return (dt * 255u) / (span ? span : 1u);
}

static int note_to_y(uint8_t note) {
  // map note 36..96 into 10..60
  if (note < 24) note = 24;
  if (note > 108) note = 108;
  int y0 = 12;
  int h = 48;
  return y0 + (int)((108 - note) * h / (108 - 24));
}

static void draw_cursor(uint32_t base, uint32_t span) {
  uint32_t x = tick_to_x(g_cursor_tick, base, span);
  if (x != 0xFFFFFFFFu) {
    ui_gfx_rect((int)x, 10, 1, 54, 6);
  }
}

static void draw_loop_region(uint32_t base, uint32_t span) {
  uint32_t L = loop_len();
  if (L == 0) return;
  
  // Draw loop start marker (left edge)
  uint32_t start_x = tick_to_x(0, base, span);
  if (start_x != 0xFFFFFFFFu && start_x < 256) {
    // Vertical line for loop start
    ui_gfx_rect((int)start_x, 10, 2, 54, 10);
    // Small triangle at top
    ui_gfx_rect((int)start_x, 8, 4, 2, 10);
  }
  
  // Draw loop end marker (right edge)
  uint32_t end_x = tick_to_x(L - 1, base, span);
  if (end_x != 0xFFFFFFFFu && end_x < 256) {
    // Vertical line for loop end
    ui_gfx_rect((int)end_x, 10, 2, 54, 10);
    // Small triangle at top
    ui_gfx_rect((int)end_x - 3, 8, 4, 2, 10);
  }
  
  // Draw loop region shading if both markers visible
  if (start_x != 0xFFFFFFFFu && end_x != 0xFFFFFFFFu && 
      start_x < 256 && end_x < 256 && end_x > start_x) {
    // Subtle horizontal lines to show loop region
    for (int y = 10; y < 64; y += 8) {
      for (int x = (int)start_x; x < (int)end_x && x < 256; x += 4) {
        ui_gfx_rect(x, y, 1, 1, 3);
      }
    }
  }
}

static void draw_playhead(uint32_t base, uint32_t span) {
  // Get real-time playhead position from looper
  looper_state_t state = looper_get_state(g_track);
  if (state != LOOPER_STATE_PLAY && state != LOOPER_STATE_OVERDUB && state != LOOPER_STATE_REC) {
    return; // Only show playhead during playback
  }
  
  uint32_t playhead_tick = looper_get_cursor_position(g_track);
  uint32_t x = tick_to_x(playhead_tick, base, span);
  
  if (x != 0xFFFFFFFFu && x < 256) {
    // Draw bright playhead line (different from edit cursor)
    ui_gfx_rect((int)x, 10, 2, 54, 15);
    // Draw playhead triangle at top
    if (x >= 2) ui_gfx_rect((int)x - 2, 10, 2, 2, 15);
    if (x + 2 < 256) ui_gfx_rect((int)x + 2, 10, 2, 2, 15);
  }
}

static void draw_events(uint32_t base, uint32_t span) {
  for (uint32_t i=0; i<snap_n; i++) {
    if (!is_note_on(&snap[i])) continue;
    uint32_t x = tick_to_x(snap[i].tick, base, span);
    if (x == 0xFFFFFFFFu) continue;
    int y = note_to_y(snap[i].b1);
    
    // LoopA-style: Show velocity through brightness
    uint8_t vel = snap[i].b2;
    uint8_t g;
    if (i == g_sel_idx) {
      g = 15; // Selected event is brightest
    } else {
      // Map velocity to brightness (6-12 range)
      g = 6 + ((uint32_t)vel * 6) / 127;
    }
    
    // LoopA-style: Larger event markers (3x3 instead of 2x2)
    ui_gfx_fill_rect((int)x-1, y-1, 3, 3, g);
    
    // Add border to selected events
    if (i == g_sel_idx) {
      ui_gfx_rect((int)x-1, y-1, 3, 3, 15);
    }
  }
}

static void draw_header(void) {
  char line[64];
  looper_transport_t tp; looper_get_transport(&tp);
  looper_state_t state = looper_get_state(g_track);
  uint32_t L = loop_len();
  uint32_t loop_bars = L / (96 * tp.ts_num / (tp.ts_den / 4));
  
  const char* state_str = (state == LOOPER_STATE_PLAY) ? "PLAY" :
                          (state == LOOPER_STATE_REC) ? "REC" :
                          (state == LOOPER_STATE_OVERDUB) ? "OVDUB" : 
                          g_in_edit ? "EDIT" : "NAV";
  
  // Use 8×8 font for header
  ui_gfx_set_font(UI_FONT_8X8);
  snprintf(line, sizeof(line), "TIME T%u BPM:%u Z:%u L:%ub %s",
           (unsigned)(g_track+1), (unsigned)tp.bpm, (unsigned)g_zoom,
           (unsigned)loop_bars, state_str);
  ui_gfx_text(0, 0, line, 15);
  ui_gfx_hline(0, 11, 256, 8);
}

static void apply_zoom(void) {
  // g_zoom is a simple 0..2 index into zoom_ticks[]
  if (g_zoom > 2) g_zoom = 2;
}

static void draw_footer(void) {
  ui_gfx_set_font(UI_FONT_5X7);
  if (!g_in_edit) {
    ui_gfx_text(0, 56, "ENC:scroll B1:trk B2:zoom B3:sel B4:edit", 10);
  } else {
    ui_gfx_text(0, 56, "ENC:chg B3:field B4:apply B2:cancel", 10);
  }
}

static void select_nearest(void) {
  if (snap_n == 0) return;
  uint32_t base = g_cursor_tick;
  uint32_t span = zoom_ticks[g_zoom];
  uint32_t best = 0;
  uint32_t best_dx = 0xFFFFFFFFu;
  for (uint32_t i=0;i<snap_n;i++) {
    if (!is_note_on(&snap[i])) continue;
    uint32_t x = tick_to_x(snap[i].tick, base, span);
    if (x == 0xFFFFFFFFu) continue;
    uint32_t dt = (snap[i].tick + loop_len() - g_cursor_tick) % loop_len();
    if (dt < best_dx) { best_dx = dt; best = i; }
  }
  g_sel_idx = best;
}

void ui_page_looper_timeline_render(uint32_t now_ms) {
  (void)now_ms;
  ui_gfx_clear(0);

  refresh_snapshot();

  draw_header();

  uint32_t span = zoom_ticks[g_zoom];
  uint32_t base = g_cursor_tick;

  // simple grid: 4 vertical lines
  for (int i=0;i<4;i++) {
    int x = i * 64;
    ui_gfx_rect(x, 10, 1, 54, 2);
  }

  draw_loop_region(base, span);  // Draw loop bounds first
  draw_events(base, span);
  draw_playhead(base, span);     // Draw playhead on top
  draw_cursor(base, span);       // Draw edit cursor last

  // selected event info
  if (snap_n) {
    char inf[64];
    looper_event_view_t* e = &snap[g_sel_idx];
    snprintf(inf, sizeof(inf), "idx:%lu tick:%lu note:%u vel:%u",
             (unsigned long)g_sel_idx, (unsigned long)e->tick, (unsigned)e->b1, (unsigned)e->b2);
    ui_gfx_text(0, 46, inf, 10);
    if (g_in_edit) {
      const char* f = (g_edit_field==0)?"TICK":(g_edit_field==1)?"NOTE":"VEL";
      char ed[32];
      snprintf(ed, sizeof(ed), "EDIT %s", f);
      ui_gfx_text(200, 46, ed, 15);
    }
  }

  draw_footer();
}

void ui_page_looper_timeline_on_button(uint8_t id, uint8_t pressed) {
  if (!pressed) return;
  if (!g_in_edit) {
    switch (id) {
      case 1: g_track = (uint8_t)((g_track + 1) % LOOPER_TRACKS); g_cursor_tick = 0; g_sel_idx = 0; break;
      case 2: g_zoom = (uint8_t)((g_zoom + 1) % (sizeof(zoom_ticks)/sizeof(zoom_ticks[0]))); break;
      case 3: select_nearest(); break;
      case 4: g_in_edit = 1; g_edit_field = 0; break;
      default: break;
    }
  } else {
    switch (id) {
      case 2: g_in_edit = 0; break; // cancel
      case 3: g_edit_field = (uint8_t)((g_edit_field + 1) % 3); break;
      case 4: {
        if (snap_n == 0) { g_in_edit = 0; break; }
        looper_event_view_t* e = &snap[g_sel_idx];
        (void)looper_edit_event(g_track, e->idx, e->tick, e->len, e->b0, e->b1, e->b2);
        g_in_edit = 0;
      } break;
      default: break;
    }
  }
}

void ui_page_looper_timeline_on_encoder(int8_t delta) {
  if (!delta) return;
  refresh_snapshot();
  if (!g_in_edit) {
    // scroll cursor
    uint32_t span = zoom_ticks[g_zoom];
    int32_t step = (int32_t)(span / 64u);
    if (step < 1) step = 1;
    int32_t dt = (int32_t)delta * step;
    uint32_t L = loop_len();
    g_cursor_tick = wrap_tick_i32(g_cursor_tick, dt, L);
  } else {
    if (snap_n == 0) return;
    looper_event_view_t* e = &snap[g_sel_idx];
    if (g_edit_field == 0) {
      // tick fine adjust
      int32_t dt = (int32_t)delta * 4;
      uint32_t L = loop_len();
      e->tick = wrap_tick_i32(e->tick, dt, L);
    } else if (g_edit_field == 1) {
      int32_t n = (int32_t)e->b1 + (int32_t)delta;
      if (n < 0) n = 0;
      if (n > 127) n = 127;
      e->b1 = (uint8_t)n;
    } else {
      int32_t v = (int32_t)e->b2 + (int32_t)delta*2;
      if (v < 1) v = 1;
      if (v > 127) v = 127;
      e->b2 = (uint8_t)v;
    }
  }
}

uint8_t ui_page_looper_timeline_get_track(void) { return g_track; }

void ui_page_looper_timeline_zoom_in(void) {
  if (g_zoom < 2) g_zoom++;
  apply_zoom();
}
void ui_page_looper_timeline_zoom_out(void) {
  if (g_zoom > 0) g_zoom--;
  apply_zoom();
}
