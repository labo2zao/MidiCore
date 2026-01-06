#include "Services/ui/ui_actions.h"
#include "Services/ui/ui_nav.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#if __has_include("ff.h")
  #include "ff.h"
  #define ACT_HAS_FATFS 1
#else
  #define ACT_HAS_FATFS 0
#endif

// Optional UI hooks
#if __has_include("Services/ui/ui.h")
  #include "Services/ui/ui.h"
  #define ACT_HAS_UI 1
#else
  #define ACT_HAS_UI 0
#endif

#if __has_include("Services/patch/patch_system.h")
  #include "Services/patch/patch_system.h"
  #define ACT_HAS_PATCHSYS 1
#else
  #define ACT_HAS_PATCHSYS 0
#endif

static void trim(char* s) {
  while (*s && isspace((unsigned char)*s)) memmove(s, s+1, strlen(s));
  size_t n=strlen(s);
  while (n && isspace((unsigned char)s[n-1])) s[--n]=0;
}
static void upcase(char* s){ for(;*s;s++) *s=(char)toupper((unsigned char)*s); }

static ui_action_t parse_action(const char* v) {
  if (!v) return UI_ACT_NONE;
  char tmp[64];
  size_t L=strlen(v); if (L>=sizeof(tmp)) L=sizeof(tmp)-1;
  memcpy(tmp,v,L); tmp[L]=0;
  trim(tmp); upcase(tmp);

  #define M(name, val) if(!strcmp(tmp,(name))) return (val)
  M("NONE", UI_ACT_NONE);

  M("PATCH_PREV", UI_ACT_PATCH_PREV);
  M("PATCH_NEXT", UI_ACT_PATCH_NEXT);
  M("BANK_PREV", UI_ACT_BANK_PREV);
  M("BANK_NEXT", UI_ACT_BANK_NEXT);
  M("LOAD_APPLY", UI_ACT_LOAD_APPLY);

  M("UI_PREV_PAGE", UI_ACT_UI_PREV_PAGE);
  M("UI_NEXT_PAGE", UI_ACT_UI_NEXT_PAGE);
  M("CURSOR_LEFT", UI_ACT_CURSOR_LEFT);
  M("CURSOR_RIGHT", UI_ACT_CURSOR_RIGHT);
  M("ZOOM_OUT", UI_ACT_ZOOM_OUT);
  M("ZOOM_IN", UI_ACT_ZOOM_IN);
  M("QUANTIZE", UI_ACT_QUANTIZE);
  M("DELETE", UI_ACT_DELETE);
  M("TOGGLE_CHORD_MODE", UI_ACT_TOGGLE_CHORD_MODE);
  M("TOGGLE_AUTO_LOOP", UI_ACT_TOGGLE_AUTO_LOOP);
  #undef M

  return UI_ACT_NONE;
}

void ui_actions_defaults(ui_actions_cfg_t* c) {
  if (!c) return;
  memset(c,0,sizeof(*c));

  // Encoder 0: NAV (same behavior as before)
  c->enc_cw[0] = UI_ACT_PATCH_NEXT;
  c->enc_ccw[0] = UI_ACT_PATCH_PREV;
  c->enc_shift_cw[0] = UI_ACT_BANK_NEXT;
  c->enc_shift_ccw[0] = UI_ACT_BANK_PREV;
  c->enc_btn[0] = UI_ACT_LOAD_APPLY;
  c->enc_shift_btn[0] = UI_ACT_LOAD_APPLY;

  // Encoder 1: UI edit defaults
  c->enc_cw[1] = UI_ACT_CURSOR_RIGHT;
  c->enc_ccw[1] = UI_ACT_CURSOR_LEFT;
  c->enc_shift_cw[1] = UI_ACT_ZOOM_IN;
  c->enc_shift_ccw[1] = UI_ACT_ZOOM_OUT;
  c->enc_btn[1] = UI_ACT_QUANTIZE;
  c->enc_shift_btn[1] = UI_ACT_DELETE;
}

static void apply_action(ui_action_t a) {
  if (a == UI_ACT_NONE) return;

#if ACT_HAS_PATCHSYS
  switch (a) {
    case UI_ACT_PATCH_PREV: (void)patch_system_patch_prev(); return;
    case UI_ACT_PATCH_NEXT: (void)patch_system_patch_next(); return;
    case UI_ACT_BANK_PREV:  (void)patch_system_bank_prev();  return;
    case UI_ACT_BANK_NEXT:  (void)patch_system_bank_next();  return;
    case UI_ACT_LOAD_APPLY: (void)patch_system_apply();      return;
    default: break;
  }
#endif

#if ACT_HAS_UI
  // Generic UI dispatcher (compile-safe). If these functions don't exist in your ui.c, nothing happens.
  switch (a) {
    case UI_ACT_UI_PREV_PAGE: ui_prev_page(); return;
    case UI_ACT_UI_NEXT_PAGE: ui_next_page(); return;
    case UI_ACT_CURSOR_LEFT:  ui_cursor_move(-1); return;
    case UI_ACT_CURSOR_RIGHT: ui_cursor_move(+1); return;
    case UI_ACT_ZOOM_OUT:     ui_zoom(-1); return;
    case UI_ACT_ZOOM_IN:      ui_zoom(+1); return;
    case UI_ACT_QUANTIZE:     ui_quantize(); return;
    case UI_ACT_DELETE:       ui_delete(); return;
    case UI_ACT_TOGGLE_CHORD_MODE: ui_toggle_chord_mode(); return;
    case UI_ACT_TOGGLE_AUTO_LOOP:  ui_toggle_auto_loop(); return;
    default: break;
  }
#endif

  // If not handled, do nothing.
}

static void set_key(ui_actions_cfg_t* c, const char* key_in, const char* v) {
  char key[64];
  size_t L=strlen(key_in); if (L>=sizeof(key)) L=sizeof(key)-1;
  memcpy(key,key_in,L); key[L]=0;
  trim(key); upcase(key);

  // ENC0_CW, ENC1_SHIFT_CCW, ENC0_BTN, ENC1_SHIFT_BTN
  if (strncmp(key,"ENC0_",5) && strncmp(key,"ENC1_",5)) return;
  uint8_t enc = (key[3]=='0') ? 0 : 1;
  const char* k = key+5;

  ui_action_t act = parse_action(v);
  if (!strcmp(k,"CW")) c->enc_cw[enc]=act;
  else if (!strcmp(k,"CCW")) c->enc_ccw[enc]=act;
  else if (!strcmp(k,"SHIFT_CW")) c->enc_shift_cw[enc]=act;
  else if (!strcmp(k,"SHIFT_CCW")) c->enc_shift_ccw[enc]=act;
  else if (!strcmp(k,"BTN")) c->enc_btn[enc]=act;
  else if (!strcmp(k,"SHIFT_BTN")) c->enc_shift_btn[enc]=act;
}

int ui_actions_load(ui_actions_cfg_t* c, const char* path) {
  if (!c || !path) return -1;
  ui_actions_defaults(c);

#if !ACT_HAS_FATFS
  (void)path;
  return -10;
#else
  FIL fp;
  if (f_open(&fp, path, FA_READ) != FR_OK) return -2;

  char section[16]={0};
  char line[160];

  while (f_gets(line, sizeof(line), &fp)) {
    for (size_t i=0;i<sizeof(line) && line[i]; i++) {
      if (line[i]=='\r' || line[i]=='\n') { line[i]=0; break; }
    }
    trim(line);
    if (!line[0] || line[0]=='#') continue;

    if (line[0]=='[') {
      char* end=strchr(line,']');
      if (!end) continue;
      *end=0;
      strncpy(section, line+1, sizeof(section)-1);
      section[sizeof(section)-1]=0;
      trim(section); upcase(section);
      continue;
    }

    char* eq=strchr(line,'=');
    if (!eq) continue;
    *eq=0;
    char* k=line; char* v=eq+1;
    trim(k); trim(v);
    if (!k[0]) continue;

    // Accept only [ACTIONS] or no section
    if (section[0] && strcmp(section,"ACTIONS")!=0) continue;
    set_key(c, k, v);
  }

  f_close(&fp);
  return 0;
#endif
}

void ui_actions_on_encoder(const ui_actions_cfg_t* c, uint8_t enc, int8_t step, uint8_t shift) {
  if (!c || enc >= 2) return;
  if (step > 0) apply_action(shift ? c->enc_shift_cw[enc] : c->enc_cw[enc]);
  else if (step < 0) apply_action(shift ? c->enc_shift_ccw[enc] : c->enc_ccw[enc]);
}

void ui_actions_on_button(const ui_actions_cfg_t* c, uint8_t enc, uint8_t shift) {
  if (!c || enc >= 2) return;
  apply_action(shift ? c->enc_shift_btn[enc] : c->enc_btn[enc]);
}
