#include "Services/ui/ui_bindings.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#if __has_include("ff.h")
  #include "ff.h"
  #define BIND_HAS_FATFS 1
#else
  #define BIND_HAS_FATFS 0
#endif

static void trim(char* s) {
  while (*s && isspace((unsigned char)*s)) memmove(s, s+1, strlen(s));
  size_t n = strlen(s);
  while (n && isspace((unsigned char)s[n-1])) s[--n] = 0;
}

static void upcase(char* s) { for (; *s; s++) *s = (char)toupper((unsigned char)*s); }

static int parse_u32(const char* v, uint32_t* out) {
  if (!v || !out) return -1;
  char* end=0;
  unsigned long x=strtoul(v,&end,0);
  if (end==v) return -2;
  *out=(uint32_t)x;
  return 0;
}

void ui_bindings_defaults(ui_bindings_t* b) {
  if (!b) return;
  b->din_patch_prev = 0;
  b->din_patch_next = 1;
  b->din_load_apply = 2;
  b->din_bank_prev  = 3;
  b->din_bank_next  = 4;
}

static void set_key(ui_bindings_t* b, const char* key_in, const char* v) {
  char key[48];
  size_t L=strlen(key_in);
  if (L>=sizeof(key)) L=sizeof(key)-1;
  memcpy(key,key_in,L); key[L]=0;
  upcase(key);

  uint32_t u=0;
  if (parse_u32(v,&u)) return;

  if (!strcmp(key,"DIN_PATCH_PREV") || !strcmp(key,"PATCH_PREV")) b->din_patch_prev = (uint16_t)u;
  else if (!strcmp(key,"DIN_PATCH_NEXT") || !strcmp(key,"PATCH_NEXT")) b->din_patch_next = (uint16_t)u;
  else if (!strcmp(key,"DIN_LOAD_APPLY") || !strcmp(key,"LOAD_APPLY") || !strcmp(key,"LOAD")) b->din_load_apply = (uint16_t)u;
  else if (!strcmp(key,"DIN_BANK_PREV") || !strcmp(key,"BANK_PREV")) b->din_bank_prev = (uint16_t)u;
  else if (!strcmp(key,"DIN_BANK_NEXT") || !strcmp(key,"BANK_NEXT")) b->din_bank_next = (uint16_t)u;
}

int ui_bindings_load(ui_bindings_t* b, const char* path) {
  if (!b || !path) return -1;
  ui_bindings_defaults(b);

#if !BIND_HAS_FATFS
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
      trim(section);
      upcase(section);
      continue;
    }

    char* eq=strchr(line,'=');
    if (!eq) continue;
    *eq=0;
    char* k=line;
    char* v=eq+1;
    trim(k); trim(v);
    if (!k[0]) continue;

    if (section[0] && strcmp(section,"BINDINGS")!=0) continue;
    set_key(b, k, v);
  }

  f_close(&fp);
  return 0;
#endif
}
