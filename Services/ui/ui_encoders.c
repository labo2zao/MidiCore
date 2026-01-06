#include "Services/ui/ui_encoders.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#if __has_include("ff.h")
  #include "ff.h"
  #define ENC_HAS_FATFS 1
#else
  #define ENC_HAS_FATFS 0
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

void ui_encoders_defaults(ui_encoders_cfg_t* c) {
  if (!c) return;
  memset(c, 0, sizeof(*c));
  c->shift_din = 5;
  c->shift_long_ms = 700;
  c->shift_latch = 1;

  c->enc_a[0] = 6;
  c->enc_b[0] = 7;
  c->enc_btn[0] = 8;
  c->enc_mode[0] = UI_ENC_MODE_NAV;

  // ENC1 disabled by default
  c->enc_a[1] = 0xFFFFu;
  c->enc_b[1] = 0xFFFFu;
  c->enc_btn[1] = 0xFFFFu;
  c->enc_mode[1] = UI_ENC_MODE_UI;
}

static ui_enc_mode_t parse_mode(const char* v) {
  if (!v) return UI_ENC_MODE_NAV;
  char tmp[12];
  size_t L=strlen(v);
  if (L>=sizeof(tmp)) L=sizeof(tmp)-1;
  memcpy(tmp,v,L); tmp[L]=0;
  trim(tmp); upcase(tmp);
  if (!strcmp(tmp,"UI")) return UI_ENC_MODE_UI;
  return UI_ENC_MODE_NAV;
}

static void set_key(ui_encoders_cfg_t* c, const char* key_in, const char* v) {
  char key[48];
  size_t L=strlen(key_in);
  if (L>=sizeof(key)) L=sizeof(key)-1;
  memcpy(key,key_in,L); key[L]=0;
  upcase(key);

  if (!strcmp(key,"SHIFT_DIN")) {
    uint32_t u=0; if (!parse_u32(v,&u)) c->shift_din=(uint16_t)u;
    return;
  }
  if (!strcmp(key,"SHIFT_LONG_MS")) {
    uint32_t u=0; if (!parse_u32(v,&u)) c->shift_long_ms=(uint16_t)u;
    return;
  }
  if (!strcmp(key,"SHIFT_LATCH")) {
    uint32_t u=0; if (!parse_u32(v,&u)) c->shift_latch=(uint8_t)(u?1:0);
    return;
  }

  if (!strncmp(key,"ENC0_",5) || !strncmp(key,"ENC1_",5)) {

uint8_t enc = (key[3] == '0') ? 0 : 1; // ENC0_ or ENC1_
const char* k = key+5;
if (!strcmp(k,"MODE")) { c->enc_mode[enc]=parse_mode(v); return; }
uint32_t u=0;
if (!strcmp(k,"A")) { if (!parse_u32(v,&u)) c->enc_a[enc]=(uint16_t)u; return; }
if (!strcmp(k,"B")) { if (!parse_u32(v,&u)) c->enc_b[enc]=(uint16_t)u; return; }
if (!strcmp(k,"BTN")) { if (!parse_u32(v,&u)) c->enc_btn[enc]=(uint16_t)u; return; }
return;
}

}
int ui_encoders_load(ui_encoders_cfg_t* c, const char* path) {
  if (!c || !path) return -1;
  ui_encoders_defaults(c);

#if !ENC_HAS_FATFS
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

    if (section[0] && strcmp(section,"ENCODERS")!=0) continue;
    set_key(c,k,v);
  }

  f_close(&fp);
  return 0;
#endif
}
