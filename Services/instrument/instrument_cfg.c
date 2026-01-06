#include "Services/instrument/instrument_cfg.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#if __has_include("ff.h")
  #include "ff.h"
  #define IC_HAS_FATFS 1
#else
  #define IC_HAS_FATFS 0
#endif

static instrument_cfg_t g_cfg;

void instrument_cfg_defaults(instrument_cfg_t* c) {
  if (!c) return;
  memset(c,0,sizeof(*c));
  c->human_enable = 1;
  c->human_time_ms = 3;
  c->human_vel = 5;
  c->human_apply_mask = (1u<<0) | (1u<<1) | (1u<<2); // keys+chord+looper

  c->chord_cond_enable = 0;
  c->chord_vel_gt = 0;
  c->chord_vel_lt = 0;
  c->chord_need_hold = 0;
  c->chord_block_shift = 0;
  c->hold_phys_id = 4;

  c->strum_enable = 1;
  c->strum_spread_ms = 8;
  c->strum_dir = 0;

  c->vel_min = 10;
  c->vel_max = 120;
  c->vel_curve = 0;
  c->vel_gamma = 1.35f;
}

static void trim(char* s) {
  while (*s && isspace((unsigned char)*s)) memmove(s, s+1, strlen(s));
  size_t n=strlen(s);
  while (n && isspace((unsigned char)s[n-1])) s[--n]=0;
}

static int keyeq(const char* a, const char* b) {
  while (*a && *b) {
    char ca=(char)toupper((unsigned char)*a++);
    char cb=(char)toupper((unsigned char)*b++);
    if (ca!=cb) return 0;
  }
  return *a==0 && *b==0;
}

static uint8_t parse_u8(const char* v) { long x=strtol(v,0,10); if(x<0)x=0; if(x>255)x=255; return (uint8_t)x; }
static uint16_t parse_u16(const char* v) { long x=strtol(v,0,10); if(x<0)x=0; if(x>65535)x=65535; return (uint16_t)x; }
static float parse_f(const char* v) { return (float)strtod(v,0); }

static uint8_t parse_apply_mask(const char* v) {
  uint8_t m=0;
  if (!v) return 0;
  char tmp[64]; strncpy(tmp,v,sizeof(tmp)-1); tmp[sizeof(tmp)-1]=0;
  for (char* p=tmp; *p; ++p) if (*p=='|') *p=';';
  char* tok=strtok(tmp,";,\r\n");
  while(tok){
    trim(tok);
    for(char* p=tok; *p; ++p) *p=(char)toupper((unsigned char)*p);
    if (strcmp(tok,"KEYS")==0) m|=HUMAN_APPLY_KEYS;
    else if (strcmp(tok,"CHORD")==0) m|=HUMAN_APPLY_CHORD;
    else if (strcmp(tok,"LOOPER")==0) m|=HUMAN_APPLY_LOOPER;
    else if (strcmp(tok,"THRU")==0) m|=HUMAN_APPLY_THRU;
    tok=strtok(NULL,";,\r\n");
  }
  return m;
}

static uint8_t parse_curve(const char* v) {
  if (!v) return VCURVE_LINEAR;
  char t[24]; strncpy(t,v,sizeof(t)-1); t[sizeof(t)-1]=0;
  trim(t); for(char* p=t;*p;++p)*p=(char)toupper((unsigned char)*p);
  if (strcmp(t,"SOFT")==0) return VCURVE_SOFT;
  if (strcmp(t,"HARD")==0) return VCURVE_HARD;
  if (strcmp(t,"CUSTOM")==0) return VCURVE_CUSTOM;
  return VCURVE_LINEAR;
}

static uint8_t parse_strum_dir(const char* v) {
  if (!v) return STRUM_UP;
  char t[24]; strncpy(t,v,sizeof(t)-1); t[sizeof(t)-1]=0;
  trim(t); for(char* p=t;*p;++p)*p=(char)toupper((unsigned char)*p);
  if (strcmp(t,"DOWN")==0) return STRUM_DOWN;
  if (strcmp(t,"RANDOM")==0) return STRUM_RANDOM;
  return STRUM_UP;
}

static void set_key(instrument_cfg_t* c, const char* section, const char* key, const char* val) {
  if (!c||!section||!key||!val) return;
  if (keyeq(section,"HUMAN")) {
    if (keyeq(key,"ENABLE")) c->human_enable = parse_u8(val)?1:0;
    else if (keyeq(key,"TIME_MS")) c->human_time_ms = parse_u8(val);
    else if (keyeq(key,"VEL")) c->human_vel = parse_u8(val);
    else if (keyeq(key,"APPLY")) c->human_apply_mask = parse_apply_mask(val);
  } else if (keyeq(section,"CHORD_COND")) {
    if (keyeq(key,"ENABLE")) c->chord_cond_enable = parse_u8(val)?1:0;
    else if (keyeq(key,"VEL_GT")) c->chord_vel_gt = parse_u8(val);
    else if (keyeq(key,"VEL_LT")) c->chord_vel_lt = parse_u8(val);
    else if (keyeq(key,"NEED_HOLD")) c->chord_need_hold = parse_u8(val)?1:0;
    else if (keyeq(key,"BLOCK_SHIFT")) c->chord_block_shift = parse_u8(val)?1:0;
    else if (keyeq(key,"HOLD_PHYS")) c->hold_phys_id = parse_u16(val);
  } else if (keyeq(section,"CHORD_STRUM")) {
    if (keyeq(key,"ENABLE")) c->strum_enable = parse_u8(val)?1:0;
    else if (keyeq(key,"SPREAD_MS")) c->strum_spread_ms = parse_u8(val);
    else if (keyeq(key,"DIRECTION")) c->strum_dir = parse_strum_dir(val);
  } else if (keyeq(section,"VELOCITY")) {
    if (keyeq(key,"MIN")) c->vel_min = parse_u8(val);
    else if (keyeq(key,"MAX")) c->vel_max = parse_u8(val);
    else if (keyeq(key,"CURVE")) c->vel_curve = parse_curve(val);
    else if (keyeq(key,"GAMMA")) c->vel_gamma = parse_f(val);
  }
}

int instrument_cfg_load_sd(instrument_cfg_t* c, const char* path) {
#if !IC_HAS_FATFS
  (void)c; (void)path;
  return -10;
#else
  if (!c || !path) return -1;
  FIL f;
  if (f_open(&f, path, FA_READ) != FR_OK) return -2;

  char line[128];
  char section[32] = "GLOBAL";

  while (f_gets(line, sizeof(line), &f)) {
    trim(line);
    if (!line[0] || line[0]=='#' || line[0]==';') continue;
    if (line[0]=='[') {
      char* e=strchr(line,']'); if(!e) continue;
      *e=0;
      strncpy(section, line+1, sizeof(section)-1);
      section[sizeof(section)-1]=0;
      trim(section);
      continue;
    }
    char* eq=strchr(line,'=');
    if (!eq) continue;
    *eq=0;
    char* k=line;
    char* v=eq+1;
    trim(k); trim(v);
    set_key(c, section, k, v);
  }
  f_close(&f);
  // sanitize
  if (c->vel_min<1) c->vel_min=1;
  if (c->vel_max<1) c->vel_max=1;
  if (c->vel_min>127) c->vel_min=127;
  if (c->vel_max>127) c->vel_max=127;
  if (c->vel_min>c->vel_max) { uint8_t t=c->vel_min; c->vel_min=c->vel_max; c->vel_max=t; }
  return 0;
#endif
}

const instrument_cfg_t* instrument_cfg_get(void) { return &g_cfg; }
void instrument_cfg_set(const instrument_cfg_t* c) { if(c) g_cfg = *c; else instrument_cfg_defaults(&g_cfg); }
