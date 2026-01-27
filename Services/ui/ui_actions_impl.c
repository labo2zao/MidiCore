#include <stdint.h>
#include "Services/ui/ui.h"
#include "Services/looper/looper.h"
#include "Services/ui/ui_page_looper.h"
#include "Services/ui/ui_page_looper_timeline.h"
#if MODULE_ENABLE_UI_PAGE_PIANOROLL
#include "Services/ui/ui_page_looper_pianoroll.h"
#endif

void ui_prev_page(void) {
  ui_page_t p = ui_get_page();
#if MODULE_ENABLE_UI_PAGE_PIANOROLL
  if (p == UI_PAGE_LOOPER) ui_set_page(UI_PAGE_LOOPER_PR);
  else if (p == UI_PAGE_LOOPER_TL) ui_set_page(UI_PAGE_LOOPER);
  else if (p == UI_PAGE_LOOPER_PR) ui_set_page(UI_PAGE_LOOPER_TL);
  else ui_set_page(UI_PAGE_LOOPER);
#else
  if (p == UI_PAGE_LOOPER) ui_set_page(UI_PAGE_LOOPER_TL);
  else if (p == UI_PAGE_LOOPER_TL) ui_set_page(UI_PAGE_LOOPER);
  else ui_set_page(UI_PAGE_LOOPER);
#endif
}

void ui_next_page(void) {
  ui_page_t p = ui_get_page();
#if MODULE_ENABLE_UI_PAGE_PIANOROLL
  if (p == UI_PAGE_LOOPER) ui_set_page(UI_PAGE_LOOPER_TL);
  else if (p == UI_PAGE_LOOPER_TL) ui_set_page(UI_PAGE_LOOPER_PR);
  else if (p == UI_PAGE_LOOPER_PR) ui_set_page(UI_PAGE_LOOPER);
  else ui_set_page(UI_PAGE_LOOPER);
#else
  if (p == UI_PAGE_LOOPER) ui_set_page(UI_PAGE_LOOPER_TL);
  else if (p == UI_PAGE_LOOPER_TL) ui_set_page(UI_PAGE_LOOPER);
  else ui_set_page(UI_PAGE_LOOPER);
#endif
}

void ui_cursor_move(int8_t delta) {
  ui_on_encoder(delta);
}

void ui_zoom(int8_t delta) {
  ui_page_t p = ui_get_page();
  if (p == UI_PAGE_LOOPER_TL) {
    if (delta > 0) ui_page_looper_timeline_zoom_in();
    else if (delta < 0) ui_page_looper_timeline_zoom_out();
  }
#if MODULE_ENABLE_UI_PAGE_PIANOROLL
  else if (p == UI_PAGE_LOOPER_PR) {
    if (delta > 0) ui_page_looper_pianoroll_zoom_in();
    else if (delta < 0) ui_page_looper_pianoroll_zoom_out();
  }
#endif
}

void ui_quantize(void) {
  uint8_t tr = 0;
  ui_page_t p = ui_get_page();
  if (p == UI_PAGE_LOOPER) tr = ui_page_looper_get_track();
  else if (p == UI_PAGE_LOOPER_TL) tr = ui_page_looper_timeline_get_track();
#if MODULE_ENABLE_UI_PAGE_PIANOROLL
  else if (p == UI_PAGE_LOOPER_PR) tr = ui_page_looper_pianoroll_get_track();
#endif

  looper_quant_t q = looper_get_quant(tr);
  q = (looper_quant_t)((q + 1) % LOOPER_QUANT_COUNT);
  looper_set_quant(tr, q);
}

void ui_delete(void) {
  ui_page_t p = ui_get_page();
#if MODULE_ENABLE_UI_PAGE_PIANOROLL
  if (p == UI_PAGE_LOOPER_PR) {
    ui_page_looper_pianoroll_on_button(1, 1);
  }
#else
  (void)p;  // Suppress unused variable warning
#endif
}


void ui_toggle_chord_mode(void) {
  uint8_t current = ui_get_chord_mode();
  ui_set_chord_mode(!current);
}

void ui_toggle_auto_loop(void) {
  looper_transport_t tp;
  looper_get_transport(&tp);
  tp.auto_loop = (uint8_t)!tp.auto_loop;
  looper_set_transport(&tp);
}
