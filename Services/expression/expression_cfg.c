#include "Services/expression/expression_cfg.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#if __has_include("ff.h")
  #include "ff.h"
  #define EC_HAS_FATFS 1
#else
  #define EC_HAS_FATFS 0
#endif

static void trim(char* s){
  while(*s && isspace((unsigned char)*s)) memmove(s,s+1,strlen(s));
  size_t n=strlen(s);
  while(n && isspace((unsigned char)s[n-1])) s[--n]=0;
}
static int keyeq(const char* a,const char* b){
  while(*a && *b){
    char ca=(char)toupper((unsigned char)*a++);
    char cb=(char)toupper((unsigned char)*b++);
    if(ca!=cb) return 0;
  }
  return *a==0 && *b==0;
}
static uint8_t u8(const char* v){ long x=strtol(v,0,0); if(x<0)x=0; if(x>255)x=255; return (uint8_t)x; }
static uint16_t u16(const char* v){ long x=strtol(v,0,0); if(x<0)x=0; if(x>65535)x=65535; return (uint16_t)x; }

void expression_cfg_defaults(expr_cfg_t* c){
  if(!c) return;
  memset(c,0,sizeof(*c));
  c->enable=0;
  c->midi_ch=0;
  c->cc_num=11;
  c->cc_push=11;
  c->cc_pull=2;
  c->bidir=EXPR_BIDIR_OFF;
  c->raw_min=0;
  c->raw_max=4095;
  c->zero_deadband_pa=500;
  c->out_min=0;
  c->out_max=127;
  c->rate_ms=20;
  c->smoothing=200;
  c->deadband_cc=2;
  c->hyst_cc=1;
  c->curve=EXPR_CURVE_EXPO;
  c->curve_param=180;
}

int expression_cfg_load_sd(expr_cfg_t* c, const char* path){
#if !EC_HAS_FATFS
  (void)c;(void)path;
  return -10;
#else
  if(!c||!path) return -1;
  FIL f;
  if(f_open(&f,path,FA_READ)!=FR_OK) return -2;
  char line[160];
  while(f_gets(line,sizeof(line),&f)){
    trim(line);
    if(!line[0]||line[0]=='#'||line[0]==';') continue;
    if(line[0]=='[') continue;
    char* eq=strchr(line,'=');
    if(!eq) continue;
    *eq=0;
    char* k=line; char* v=eq+1;
    trim(k); trim(v);

    if(keyeq(k,"ENABLE")) c->enable=u8(v)?1:0;
    else if(keyeq(k,"MIDI_CH")) { uint8_t t=u8(v); c->midi_ch = (t>15)?15:t; }

    else if(keyeq(k,"CC")) { uint8_t t=u8(v); c->cc_num = (t>127)?127:t; }
    else if(keyeq(k,"BIDIR")) c->bidir = u8(v);
    else if(keyeq(k,"CC_PUSH")) { uint8_t t=u8(v); c->cc_push = (t>127)?127:t; }
    else if(keyeq(k,"CC_PULL")) { uint8_t t=u8(v); c->cc_pull = (t>127)?127:t; }

    else if(keyeq(k,"RAW_MIN")) c->raw_min = u16(v);
    else if(keyeq(k,"RAW_MAX")) c->raw_max = u16(v);
    else if(keyeq(k,"ZERO_DEADBAND_PA")) c->zero_deadband_pa = u16(v);
    else if(keyeq(k,"OUT_MIN")) { uint8_t t=u8(v); c->out_min = (t>127)?127:t; }
    else if(keyeq(k,"OUT_MAX")) { uint8_t t=u8(v); c->out_max = (t>127)?127:t; }

    else if(keyeq(k,"RATE_MS")) c->rate_ms = u8(v);
    else if(keyeq(k,"SMOOTH")) c->smoothing = u8(v);

    else if(keyeq(k,"DEADBAND_CC")) c->deadband_cc = u8(v);
    else if(keyeq(k,"HYST_CC")) c->hyst_cc = u8(v);

    else if(keyeq(k,"CURVE")) c->curve = u8(v);
    else if(keyeq(k,"CURVE_PARAM")) c->curve_param = u16(v);
  }
  f_close(&f);

  if(c->rate_ms<5) c->rate_ms=5;
  if(c->out_min>c->out_max){ uint8_t t=c->out_min; c->out_min=c->out_max; c->out_max=t; }
  if(c->raw_min>c->raw_max){ uint16_t t=c->raw_min; c->raw_min=c->raw_max; c->raw_max=t; }

  if(c->deadband_cc==0) c->deadband_cc=1;
  if(c->curve>2) c->curve=0;
  if(c->bidir>1) c->bidir=0;
  if(c->curve_param==0) c->curve_param=180;

  return 0;
#endif
}
