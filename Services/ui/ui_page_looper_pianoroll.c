#include "Services/ui/ui_page_looper_pianoroll.h"
#include "Services/ui/ui_gfx.h"
#include "Services/looper/looper.h"
#include <stdio.h>
#include <string.h>

static inline uint32_t wrap_tick_i32(uint32_t cur, int32_t d, uint32_t L){
  if(L==0) return 0;
  int32_t m = (int32_t)(cur % L);
  int32_t x = m + d;
  x %= (int32_t)L;
  if(x < 0) x += (int32_t)L;
  return (uint32_t)x;
}

// --- quantize helpers (PPQN=96 as in looper) ---
// Use the looper's quantization calculation to avoid duplication
static uint32_t quant_step_ticks(looper_quant_t q) {
  return looper_get_quant_step_ticks(q);
}
static uint32_t quantize_tick_u32(uint32_t t, uint32_t step) {
  if (!step) return t;
  uint32_t r = t % step;
  uint32_t down = t - r;
  uint32_t up = down + step;
  return (r < (step/2u)) ? down : up;
}

static uint32_t rng_state = 0x12345678u;
static uint32_t xorshift32(void) {
  uint32_t x = rng_state;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  rng_state = x;
  return x;
}

#define MAX_EVT 768
#define MAX_NOTES 256

typedef struct {
  uint32_t on_idx;
  uint32_t off_idx;     // 0xFFFFFFFF if implicit
  uint32_t start;
  uint32_t end;
  uint8_t  ch;
  uint8_t  note;
  uint8_t  vel;
} note_span_t;

static uint8_t g_track = 0;
static uint32_t g_cursor = 0;

static const uint32_t zoom_ticks[] = { 96u, 192u, 384u, 768u, 1536u, 3072u, 6144u };
static uint8_t g_zoom = 2;

static uint32_t g_sel = 0;
static uint8_t g_edit = 0;
static uint8_t g_field = 0; // 0 start, 1 len, 2 note, 3 vel

static looper_event_view_t ev[MAX_EVT];
static uint32_t ev_n = 0;

static note_span_t notes[MAX_NOTES];
static uint32_t notes_n = 0;

// simple active map for pairing within one loop
typedef struct { uint32_t on_idx; uint32_t start; uint8_t vel; uint8_t valid; } active_t;
// Large array (32KB) - only used in pianoroll UI
static active_t active[16][128];

static uint8_t is_note_on(const looper_event_view_t* e) {
  return (e->len==3) && ((e->b0 & 0xF0)==0x90) && e->b2!=0;
}
static uint8_t is_note_off(const looper_event_view_t* e) {
  return (e->len==3) && ( ((e->b0 & 0xF0)==0x80) || (((e->b0 & 0xF0)==0x90) && e->b2==0) );
}

static uint32_t loop_len(void) {
  uint32_t L = looper_get_loop_len_ticks(g_track);
  if (L == 0) L = 96u * 4u;
  return L;
}

static void refresh_snapshot(void) {
  ev_n = looper_export_events(g_track, ev, MAX_EVT);

  memset(active, 0, sizeof(active));
  notes_n = 0;

  uint32_t L = loop_len();

  for (uint32_t i=0; i<ev_n && notes_n<MAX_NOTES; i++) {
    looper_event_view_t* e = &ev[i];
    if (e->len != 3) continue;
    uint8_t ch = e->b0 & 0x0F;
    uint8_t note = e->b1;

    if (is_note_on(e)) {
      active[ch][note].on_idx = e->idx;
      active[ch][note].start = e->tick;
      active[ch][note].vel = e->b2;
      active[ch][note].valid = 1;
    } else if (is_note_off(e)) {
      if (active[ch][note].valid) {
        note_span_t* n = &notes[notes_n++];
        n->on_idx = active[ch][note].on_idx;
        n->off_idx = e->idx;
        n->start = active[ch][note].start;
        n->end = e->tick;
        n->ch = ch;
        n->note = note;
        n->vel = active[ch][note].vel;
        active[ch][note].valid = 0;

        // handle wrap: if end < start, assume wrap to next loop
        if (n->end <= n->start) n->end += L;
      }
    }
  }

  // Any notes still active: implicit end at loop end
  for (uint8_t ch=0; ch<16 && notes_n<MAX_NOTES; ch++) {
    for (uint16_t note=0; note<128 && notes_n<MAX_NOTES; note++) {
      if (active[ch][note].valid) {
        note_span_t* n = &notes[notes_n++];
        n->on_idx = active[ch][note].on_idx;
        n->off_idx = 0xFFFFFFFFu;
        n->start = active[ch][note].start;
        n->end = L;
        n->ch = ch;
        n->note = (uint8_t)note;
        n->vel = active[ch][note].vel;
        if (n->end <= n->start) n->end = n->start + 1;
      }
    }
  }

  if (notes_n == 0) g_sel = 0;
  else if (g_sel >= notes_n) g_sel = notes_n-1;
}

static uint32_t tick_to_x(uint32_t tick, uint32_t base, uint32_t span) {
  uint32_t L = loop_len();
  uint32_t dt = (tick + L - base) % L;
  if (dt >= span) return 0xFFFFFFFFu;
  return (dt * 255u) / (span ? span : 1u);
}

static int note_to_y(uint8_t note) {
  // 10..58
  if (note < 24) note = 24;
  if (note > 108) note = 108;
  int y0 = 12;
  int h = 46;
  return y0 + (int)((108 - note) * h / (108 - 24));
}

static void draw_header(void) {
  char line[64];
  looper_transport_t tp; looper_get_transport(&tp);
  
  // Use 8×8 font for header
  ui_gfx_set_font(UI_FONT_8X8);
  snprintf(line, sizeof(line), "PIANO T%u BPM:%u Z:%u %s",
           (unsigned)(g_track+1), (unsigned)tp.bpm, (unsigned)g_zoom, g_edit?"EDIT":"NAV");
  ui_gfx_text(0, 0, line, 15);
  ui_gfx_hline(0, 11, 256, 8);
}

static void draw_grid(uint32_t base, uint32_t span) {
  // LoopA-style: Subtle vertical grid lines at quarter divisions
  for (int i=0;i<4;i++) {
    int x = i * 64;
    ui_gfx_vline(x, 10, 54, 2);
  }
  
  // LoopA-style: Horizontal octave lines (more prominent)
  for (int y=16; y<60; y+=12) {
    ui_gfx_hline(0, y, 256, 3);
  }

  // Quantization grid (if enabled) - more subtle than before
  looper_quant_t q = looper_get_quant(g_track);
  uint32_t step = quant_step_ticks(q);
  if (!step) return;

  uint32_t L = loop_len();
  uint32_t nlines = (span / step);
  if (nlines > 64) nlines = 64; // limit
  for (uint32_t i=0; i<=nlines; i++) {
    uint32_t t = (base + i*step) % L;
    uint32_t x = tick_to_x(t, base, span);
    if (x != 0xFFFFFFFFu) ui_gfx_vline((int)x, 10, 54, 3);
  }
}

static void draw_cursor(uint32_t base, uint32_t span) {
  uint32_t x = tick_to_x(g_cursor, base, span);
  // LoopA-style: Thicker, brighter cursor for better visibility
  if (x != 0xFFFFFFFFu) {
    ui_gfx_vline((int)x, 10, 54, 12);
    if (x > 0) ui_gfx_vline((int)x-1, 10, 54, 6);
    if (x < 255) ui_gfx_vline((int)x+1, 10, 54, 6);
  }
}

static void draw_notes(uint32_t base, uint32_t span) {
  uint32_t L = loop_len();
  for (uint32_t i=0;i<notes_n;i++) {
    note_span_t* n = &notes[i];
    uint32_t s = n->start;
    uint32_t e = n->end;
    // map end possibly beyond L (wrap) by modulo for display near start
    uint32_t sx = tick_to_x(s % L, base, span);
    if (sx == 0xFFFFFFFFu) continue;
    uint32_t ex = tick_to_x((e % L), base, span);
    if (ex == 0xFFFFFFFFu) ex = 255;
    int y = note_to_y(n->note);
    int w = (int)ex - (int)sx;
    if (w < 2) w = 2;
    
    // LoopA-style: Use velocity for brightness (more visual feedback)
    // Selected notes get max brightness, others scale with velocity
    uint8_t g;
    if (i == g_sel) {
      g = 15; // Selected note is always brightest
    } else {
      // Map velocity (0-127) to grayscale (6-13) for better contrast
      g = 6 + ((uint32_t)n->vel * 7) / 127;
    }
    
    // LoopA-style: Taller note bars (4px instead of 3px)
    ui_gfx_fill_rect((int)sx, y, w, 4, g);
    
    // Draw note border for better definition (selected notes only)
    if (i == g_sel) {
      ui_gfx_hline((int)sx, y, w, 15);
      ui_gfx_hline((int)sx, y+3, w, 15);
    }
  }
}

static void apply_zoom(void) {
  // g_zoom is a simple 0..2 index into zoom_ticks[]
  if (g_zoom > 2) g_zoom = 2;
}

static void draw_footer(void) {
  ui_gfx_set_font(UI_FONT_5X7);
  if (!g_edit) ui_gfx_text(0, 56, "ENC:scroll B1:trk B2:zoom B3:sel B4:edit B6:dup B7:^ B8:v B9:hum", 10);
  else         ui_gfx_text(0, 56, "ENC:chg B3:field B4:apply B2:cancel B1:del", 10);
}

static void select_nearest(void) {
  if (!notes_n) return;
  uint32_t L = loop_len();
  uint32_t best = 0;
  uint32_t best_dt = 0xFFFFFFFFu;
  for (uint32_t i=0;i<notes_n;i++) {
    uint32_t dt = (notes[i].start + L - g_cursor) % L;
    if (dt < best_dt) { best_dt = dt; best = i; }
  }
  g_sel = best;
}

static void apply_edit(note_span_t* n) {
  uint32_t L = loop_len();
  looper_quant_t q = looper_get_quant(g_track);
  uint32_t step = quant_step_ticks(q);
  uint32_t start = n->start % L;
  if (step) start = quantize_tick_u32(start, step);
  uint32_t end = n->end;
  if (end <= start) end = start + 1;
  uint32_t len = end - n->start;
  if (len < 1) len = 1;
  uint32_t end_tick = (start + (len % L)) % L;
  if (step) end_tick = quantize_tick_u32(end_tick, step);

  // edit NOTE ON
  uint8_t on_status = 0x90 | (n->ch & 0x0F);
  (void)looper_edit_event(g_track, n->on_idx, start, 3, on_status, n->note, n->vel);

  // edit or create NOTE OFF
  uint8_t off_status = 0x80 | (n->ch & 0x0F);
  if (n->off_idx != 0xFFFFFFFFu) {
    (void)looper_edit_event(g_track, n->off_idx, end_tick, 3, off_status, n->note, 0);
  } else {
    (void)looper_add_event(g_track, end_tick, 3, off_status, n->note, 0);
  }
}

static void delete_note(note_span_t* n) {
  // delete note off first (if exists), then note on
  if (n->off_idx != 0xFFFFFFFFu) (void)looper_delete_event(g_track, n->off_idx);
  (void)looper_delete_event(g_track, n->on_idx);
}

void ui_page_looper_pianoroll_render(uint32_t now_ms) {
  (void)now_ms;
  ui_gfx_clear(0);

  refresh_snapshot();

  draw_header();

  uint32_t span = zoom_ticks[g_zoom];
  uint32_t base = g_cursor;

  draw_grid(base, span);
  draw_notes(base, span);
  draw_cursor(base, span);

  if (notes_n) {
    char inf[64];
    note_span_t* n = &notes[g_sel];
    uint32_t L = loop_len();
    uint32_t dur = (n->end > n->start) ? (n->end - n->start) : 1;
    snprintf(inf, sizeof(inf), "idx:%lu st:%lu dur:%lu n:%u v:%u",
             (unsigned long)g_sel, (unsigned long)(n->start%L), (unsigned long)dur,
             (unsigned)n->note, (unsigned)n->vel);
    ui_gfx_text(0, 46, inf, 10);
    if (g_edit) {
      const char* f = (g_field==0)?"START":(g_field==1)?"LEN":(g_field==2)?"NOTE":"VEL";
      char ed[24]; snprintf(ed, sizeof(ed), "EDIT %s", f);
      ui_gfx_text(200, 46, ed, 15);
    }
  }

  draw_footer();
}

void ui_page_looper_pianoroll_on_button(uint8_t id, uint8_t pressed) {
  if (!pressed) return;
  refresh_snapshot();

  if (!g_edit) {
    switch (id) {
      case 1: g_track = (uint8_t)((g_track + 1) % LOOPER_TRACKS); g_cursor = 0; g_sel = 0; break;
      case 2: g_zoom = (uint8_t)((g_zoom + 1) % (sizeof(zoom_ticks)/sizeof(zoom_ticks[0]))); break;
      case 3: select_nearest(); break;
      case 4: g_edit = 1; g_field = 0; break;
      case 6: { // duplicate selected note forward by 1 quant step (or 1/16)
        if (!notes_n) break;
        uint32_t L = loop_len();
        looper_quant_t q = looper_get_quant(g_track);
        uint32_t step = quant_step_ticks(q);
        if (!step) step = 96u/4u;
        note_span_t n = notes[g_sel];
        n.start = (n.start + step) % L;
        n.end = n.start + (notes[g_sel].end - notes[g_sel].start);
        if (n.end <= n.start) n.end = n.start + step;
        // create ON
        uint8_t on_status = 0x90 | (n.ch & 0x0F);
        uint8_t off_status = 0x80 | (n.ch & 0x0F);
        (void)looper_add_event(g_track, n.start % L, 3, on_status, n.note, n.vel);
        (void)looper_add_event(g_track, (n.end % L), 3, off_status, n.note, 0);
      } break;
      case 7: { // transpose +1
        if (!notes_n) break;
        note_span_t n = notes[g_sel];
        if (n.note < 127) n.note++;
        apply_edit(&n);
      } break;
      case 8: { // transpose -1
        if (!notes_n) break;
        note_span_t n = notes[g_sel];
        if (n.note > 0) n.note--;
        apply_edit(&n);
      } break;
      case 9: { // humanize (small random)
        if (!notes_n) break;
        note_span_t n = notes[g_sel];
        rng_state ^= (uint32_t)(n.start + (n.note<<8) + n.vel);
        int32_t dt = (int32_t)(xorshift32() % 9u) - 4; // -4..+4 ticks
        int32_t dv = (int32_t)(xorshift32() % 9u) - 4; // -4..+4 vel
        uint32_t L = loop_len();
        n.start = wrap_tick_i32(n.start, dt, L);
        int32_t vv = (int32_t)n.vel + dv;
        if (vv < 1) vv = 1;
        if (vv > 127) vv = 127;
        n.vel = (uint8_t)vv;
        apply_edit(&n);
      } break;
      default: break;
    }
  } else {
    switch (id) {
      case 1:
        if (notes_n) delete_note(&notes[g_sel]);
        g_edit = 0;
        break;
      case 2: g_edit = 0; break; // cancel
      case 3: g_field = (uint8_t)((g_field + 1) % 4); break;
      case 4:
        if (notes_n) apply_edit(&notes[g_sel]);
        g_edit = 0;
        break;
      default: break;
    }
  }
}

void ui_page_looper_pianoroll_on_encoder(int8_t delta) {
  if (!delta) return;
  refresh_snapshot();
  uint32_t L = loop_len();

  if (!g_edit) {
    uint32_t span = zoom_ticks[g_zoom];
    int32_t step = (int32_t)(span / 64u);
    if (step < 1) step = 1;
    int32_t dt = (int32_t)delta * step;
    g_cursor = wrap_tick_i32(g_cursor, dt, L);
  } else {
    if (!notes_n) return;
    note_span_t* n = &notes[g_sel];
    uint32_t qstep = quant_step_ticks(looper_get_quant(g_track));
    if (g_field == 0) {
      int32_t dt = (int32_t)delta * 4;
      n->start = wrap_tick_i32(n->start, dt, L);
      if (qstep) n->start = quantize_tick_u32(n->start, qstep);
      if (n->end <= n->start) n->end = n->start + 1;
    } else if (g_field == 1) {
      int32_t d = (int32_t)delta * 4;
      int32_t cur = (int32_t)(n->end - n->start);
      if (cur < 1) cur = 1;
      cur += d;
      if (cur < 1) cur = 1;
      if (qstep) {
        // snap length to quant step
        uint32_t u = (uint32_t)cur;
        u = quantize_tick_u32(u, qstep);
        if (u < qstep) u = qstep;
        cur = (int32_t)u;
      }
      if (cur > (int32_t)L) cur = (int32_t)L;
      n->end = n->start + (uint32_t)cur;
    } else if (g_field == 2) {
      int32_t nn = (int32_t)n->note + (int32_t)delta;
      if (nn < 0) nn = 0;
      if (nn > 127) nn = 127;
      n->note = (uint8_t)nn;
    } else {
      int32_t vv = (int32_t)n->vel + (int32_t)delta*2;
      if (vv < 1) vv = 1;
      if (vv > 127) vv = 127;
      n->vel = (uint8_t)vv;
    }
  }
}

uint8_t ui_page_looper_pianoroll_get_track(void) { return g_track; }

void ui_page_looper_pianoroll_zoom_in(void) {
  if (g_zoom < 2) g_zoom++;
  apply_zoom();
}
void ui_page_looper_pianoroll_zoom_out(void) {
  if (g_zoom > 0) g_zoom--;
  apply_zoom();
}
