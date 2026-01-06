#include <stdint.h>
#include "Services/ui/ui.h"
#include "Services/looper/looper.h"
#include "Services/ui/ui_page_looper.h"
#include "Services/ui/ui_page_looper_timeline.h"
#include "Services/ui/ui_page_looper_pianoroll.h"

void ui_prev_page(void) {
  ui_page_t p = ui_get_page();
  if (p == UI_PAGE_LOOPER) ui_set_page(UI_PAGE_LOOPER_PR);
  else if (p == UI_PAGE_LOOPER_TL) ui_set_page(UI_PAGE_LOOPER);
  else if (p == UI_PAGE_LOOPER_PR) ui_set_page(UI_PAGE_LOOPER_TL);
  else ui_set_page(UI_PAGE_LOOPER);
}

void ui_next_page(void) {
  ui_page_t p = ui_get_page();
  if (p == UI_PAGE_LOOPER) ui_set_page(UI_PAGE_LOOPER_TL);
  else if (p == UI_PAGE_LOOPER_TL) ui_set_page(UI_PAGE_LOOPER_PR);
  else if (p == UI_PAGE_LOOPER_PR) ui_set_page(UI_PAGE_LOOPER);
  else ui_set_page(UI_PAGE_LOOPER);
}

void ui_cursor_move(int8_t delta) {
  ui_on_encoder(delta);
}

void ui_zoom(int8_t delta) {
  ui_page_t p = ui_get_page();
  if (p == UI_PAGE_LOOPER_TL) {
    if (delta > 0) ui_page_looper_timeline_zoom_in();
    else if (delta < 0) ui_page_looper_timeline_zoom_out();
  } else if (p == UI_PAGE_LOOPER_PR) {
    if (delta > 0) ui_page_looper_pianoroll_zoom_in();
    else if (delta < 0) ui_page_looper_pianoroll_zoom_out();
  }
}

void ui_quantize(void) {
  uint8_t tr = 0;
  ui_page_t p = ui_get_page();
  if (p == UI_PAGE_LOOPER) tr = ui_page_looper_get_track();
  else if (p == UI_PAGE_LOOPER_TL) tr = ui_page_looper_timeline_get_track();
  else if (p == UI_PAGE_LOOPER_PR) tr = ui_page_looper_pianoroll_get_track();

  looper_quant_t q = looper_get_quant(tr);
  q = (looper_quant_t)((q + 1) % 4);
  looper_set_quant(tr, q);
}

void ui_delete(void) {
  ui_page_t p = ui_get_page();
  if (p == UI_PAGE_LOOPER_PR) {
    ui_page_looper_pianoroll_on_button(1, 1);
  }
}


void ui_toggle_auto_loop(void) {
  looper_transport_t tp;
  looper_get_transport(&tp);
  tp.auto_loop = (uint8_t)!tp.auto_loop;
  looper_set_transport(&tp);
}
