#include "Services/patch/patch_state.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include "Services/fs/fs_atomic.h"

#if __has_include("ff.h")
  #include "ff.h"
  #define STATE_HAS_FATFS 1
#else
  #define STATE_HAS_FATFS 0
#endif

static void trim(char* s) {
  while (*s && isspace((unsigned char)*s)) memmove(s, s+1, strlen(s));
  size_t n = strlen(s);
  while (n && isspace((unsigned char)s[n-1])) s[--n] = 0;
}

static int parse_u32(const char* v, uint32_t* out) {
  if (!v || !out) return -1;
  char* end=0;
  unsigned long x = strtoul(v,&end,0);
  if (end==v) return -2;
  *out = (uint32_t)x;
  return 0;
}

static int startswith_i(const char* s, const char* pfx) {
  while (*pfx) {
    char a=(char)toupper((unsigned char)*s++);
    char b=(char)toupper((unsigned char)*pfx++);
    if (a!=b) return 0;
  }
  return 1;
}

void patch_state_set_defaults(patch_state_t* st) {
  memset(st,0,sizeof(*st));
  strncpy(st->bank_path, "0:/patch/banks/bank_01.ngb", sizeof(st->bank_path)-1);
  st->patch_index = 0;
}

int patch_state_load(patch_state_t* st, const char* path) {
  if (!st || !path) return -1;
  patch_state_set_defaults(st);

#if !STATE_HAS_FATFS
  (void)path;
  return -10;
#else
  FIL fp;
  if (f_open(&fp, path, FA_READ) != FR_OK) return -2;

  char line[160];
  while (f_gets(line, sizeof(line), &fp)) {
    for (size_t i=0;i<sizeof(line) && line[i]; i++) {
      if (line[i]=='\r' || line[i]=='\n') { line[i]=0; break; }
    }
    trim(line);
    if (!line[0] || line[0]=='#' || line[0]=='[') continue;

    char* eq=strchr(line,'=');
    if (!eq) continue;
    *eq=0;
    char* k=line;
    char* v=eq+1;
    trim(k); trim(v);

    if (startswith_i(k,"BANK")) strncpy(st->bank_path, v, sizeof(st->bank_path)-1);
    else if (startswith_i(k,"PATCH_INDEX")) {
      uint32_t u=0;
      if (!parse_u32(v,&u)) st->patch_index = (uint16_t)u;
    }
  }
  f_close(&fp);
  return 0;
#endif
}

int patch_state_save(const patch_state_t* st, const char* path) {
  if (!st || !path) return -1;
#if !STATE_HAS_FATFS
  (void)st; (void)path;
  return -10;
#else
  char out[256];
int n = snprintf(out, sizeof(out),
                 "[STATE]"
                 "BANK=%s"
                 "PATCH_INDEX=%u"
                 "",
                 st->bank_path, (unsigned)st->patch_index);
if (n <= 0) return -3;
if ((size_t)n >= sizeof(out)) n = (int)(sizeof(out)-1);
return fs_atomic_write_text(path, out, (size_t)n);
#endif
}
