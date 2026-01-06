#include "Services/ui/ui_state.h"
#include "Services/ui/ui.h"
#include "Services/fs/fs_atomic.h"
#include "Services/safe/safe_mode.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#if __has_include("ff.h")
  #include "ff.h"
  #define UI_STATE_HAS_FATFS 1
#else
  #define UI_STATE_HAS_FATFS 0
#endif

static uint8_t s_loaded = 0;
static uint8_t s_dirty = 0;
static uint32_t s_ms = 0;
static uint32_t s_last_save_ms = 0;

void ui_state_mark_dirty(void) { s_dirty = 1; }

static void trim(char* s) {
  while (*s && isspace((unsigned char)*s)) memmove(s, s+1, strlen(s));
  size_t n = strlen(s);
  while (n && isspace((unsigned char)s[n-1])) s[--n]=0;
}

static void upcase(char* s){ for(;*s;s++) *s=(char)toupper((unsigned char)*s); }

#if UI_STATE_HAS_FATFS
static void try_load_once(void) {
  if (s_loaded) return;
  FIL fp;
  if (f_open(&fp, "/cfg/ui_state.ngs", FA_READ) != FR_OK) { s_loaded = 1; return; }

  char line[128];
  while (f_gets(line, sizeof(line), &fp)) {
    for (size_t i=0;i<sizeof(line) && line[i]; i++) {
      if (line[i]=='\r' || line[i]=='\n') { line[i]=0; break; }
    }
    trim(line);
    if (!line[0] || line[0]=='#') continue;
    char* eq = strchr(line,'=');
    if (!eq) continue;
    *eq=0;
    char* k=line;
    char* v=eq+1;
    trim(k); trim(v);
    upcase(k);
    if (!strcmp(k,"PAGE")) {
      int p = atoi(v);
      if (p >= 0 && p < (int)UI_PAGE_COUNT) ui_set_page((ui_page_t)p);
    } else if (!strcmp(k,"CHORD_MODE")) {
      int m = atoi(v);
      ui_set_chord_mode((uint8_t)(m?1:0));
    }
  }
  f_close(&fp);
  s_loaded = 1;
}

static void save_now(void) {
  char buf[96];
  int n = snprintf(buf, sizeof(buf), "# ui state (auto)\n");
(void)n;
if (n <= 0) return;
  if ((size_t)n >= sizeof(buf)) n = (int)(sizeof(buf)-1);
  if (fs_atomic_write_text("/cfg/ui_state.ngs", buf, (size_t)n) == 0) {
    s_dirty = 0;
    s_last_save_ms = s_ms;
  }
}
#endif

void ui_state_tick_20ms(void) {
  s_ms += 20;
#if UI_STATE_HAS_FATFS
  if (!s_loaded) try_load_once();
  if (s_dirty) {
    if (!safe_mode_is_enabled() && (s_ms - s_last_save_ms) >= 500) save_now();
  }
#endif
}
