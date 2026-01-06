#include "Services/pressure/pressure_i2c.h"
#include "Hal/i2c_hal.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

#if __has_include("ff.h")
  #include "ff.h"
  #define PC_HAS_FATFS 1
#else
  #define PC_HAS_FATFS 0
#endif

static pressure_cfg_t g_cfg;

void pressure_defaults(pressure_cfg_t* c){
  if(!c) return;
  memset(c,0,sizeof(*c));
  c->enable=0;
  c->i2c_bus=2;      // J4A on MBHP = I2C2
  c->addr7=0x58;
  c->reg=0x00;
  c->type=PRESS_TYPE_XGZP6847D_24B;
  c->map_mode=PRESS_MAP_CENTER_0PA;
  c->interval_ms=5;

  c->offset=0;
  c->scale=1.0f;
  c->clamp_min=0;
  c->clamp_max=4095;

  c->pmin_pa=-40000;
  c->pmax_pa= 40000;
  c->atm0_pa= 0;
}

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
static int32_t s32(const char* v){ long x=strtol(v,0,0); return (int32_t)x; }
static float f32(const char* v){ return (float)strtod(v,0); }

int pressure_load_sd(pressure_cfg_t* c, const char* path){
#if !PC_HAS_FATFS
  (void)c;(void)path;
  return -10;
#else
  if(!c||!path) return -1;
  FIL f;
  if(f_open(&f,path,FA_READ)!=FR_OK) return -2;
  char line[180];
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
    else if(keyeq(k,"I2C_BUS")) c->i2c_bus = (u8(v)==1)?1:2;
    else if(keyeq(k,"ADDR")) c->addr7 = u8(v);
    else if(keyeq(k,"REG")) c->reg = u8(v);
    else if(keyeq(k,"TYPE")) c->type = u8(v);
    else if(keyeq(k,"MAP_MODE")) c->map_mode = u8(v);
    else if(keyeq(k,"INTERVAL_MS")) c->interval_ms = u8(v);

    else if(keyeq(k,"OFFSET")) c->offset = s32(v);
    else if(keyeq(k,"SCALE")) c->scale = f32(v);
    else if(keyeq(k,"CLAMP_MIN")) c->clamp_min = (uint16_t)u8(v);
    else if(keyeq(k,"CLAMP_MAX")) c->clamp_max = (uint16_t)u8(v);

    else if(keyeq(k,"PMIN_PA")) c->pmin_pa = s32(v);
    else if(keyeq(k,"PMAX_PA")) c->pmax_pa = s32(v);
    else if(keyeq(k,"ATM0_PA")) c->atm0_pa = s32(v);
  }
  f_close(&f);

  if(c->interval_ms<2) c->interval_ms=2;
  if(c->scale < 0.00001f) c->scale = 1.0f;
  if(c->clamp_min>4095) c->clamp_min=4095;
  if(c->clamp_max>4095) c->clamp_max=4095;
  if(c->clamp_min>c->clamp_max){ uint16_t t=c->clamp_min; c->clamp_min=c->clamp_max; c->clamp_max=t; }
  if(c->pmin_pa==c->pmax_pa){ c->pmax_pa=c->pmin_pa+1; }
  return 0;
#endif
}

void pressure_set_cfg(const pressure_cfg_t* c){ if(c) g_cfg=*c; else pressure_defaults(&g_cfg); }
const pressure_cfg_t* pressure_get_cfg(void){ return &g_cfg; }

// XGZP6847D absolute decode (Pa) from datasheet equation
static int xgzp_read_pa_abs(const pressure_cfg_t* c, int32_t* out_pa_abs){
  uint8_t b[3]={0,0,0};
  int r = i2c_hal_read(c->i2c_bus, c->addr7, 0x04, b, 3, 10);
  if(r!=0) return r;

  uint32_t sum = ((uint32_t)b[0]<<16) | ((uint32_t)b[1]<<8) | (uint32_t)b[2];
  int32_t ssum = (sum < 8388608UL) ? (int32_t)sum : (int32_t)(sum - 16777216UL);

  const int32_t span = (c->pmax_pa - c->pmin_pa);
  // Here we interpret PMIN/PMAX as the *configured physical range*, used only for scaling.
  // For absolute reading, we still use PMIN/PMAX because the chip's transfer function expects them.
  double p = ((double)ssum / (double)(1<<21)) * (double)span + (double)c->pmin_pa;
  *out_pa_abs = (int32_t)llround(p);
  return 0;
}

int pressure_read_pa_abs(int32_t* out_pa_abs){
  if(!out_pa_abs) return -1;
  const pressure_cfg_t* c=&g_cfg;
  if(!c->enable) return -2;
  if(c->type != PRESS_TYPE_XGZP6847D_24B) return -3;
  return xgzp_read_pa_abs(c, out_pa_abs);
}

int pressure_read_pa(int32_t* out_pa_signed){
  if(!out_pa_signed) return -1;
  const pressure_cfg_t* c=&g_cfg;
  if(!c->enable) return -2;

  if(c->type == PRESS_TYPE_XGZP6847D_24B){
    int32_t abs=0;
    int r = xgzp_read_pa_abs(c, &abs);
    if(r!=0) return r;
    *out_pa_signed = (int32_t)(abs - c->atm0_pa);
    return 0;
  }

  // Generic
  int32_t raw=0;
  int r = pressure_read_once(&raw);
  if(r!=0) return r;
  double p = ((double)(raw - c->offset)) * (double)c->scale;
  *out_pa_signed = (int32_t)llround(p);
  return 0;
}

int pressure_read_once(int32_t* out_value){
  if(!out_value) return -1;
  const pressure_cfg_t* c=&g_cfg;
  if(!c->enable) return -2;

  if(c->type == PRESS_TYPE_XGZP6847D_24B){
    return pressure_read_pa(out_value); // Pa signed
  }

  uint8_t b[2]={0,0};
  int r = i2c_hal_read(c->i2c_bus, c->addr7, c->reg, b, 2, 10);
  if(r!=0) return r;

  int32_t raw=0;
  if(c->type==PRESS_TYPE_GENERIC_S16BE){
    int16_t s = (int16_t)((b[0]<<8)|b[1]);
    raw = (int32_t)s;
  } else {
    uint16_t u = (uint16_t)((b[0]<<8)|b[1]);
    raw = (int32_t)u;
  }
  *out_value = raw;
  return 0;
}

static uint16_t clamp_u12(int32_t y, uint16_t mn, uint16_t mx){
  if(y < (int32_t)mn) y = mn;
  if(y > (int32_t)mx) y = mx;
  if(y<0) y=0;
  if(y>4095) y=4095;
  return (uint16_t)y;
}

uint16_t pressure_mid_raw(void){
  const pressure_cfg_t* c=&g_cfg;
  if(c->type != PRESS_TYPE_XGZP6847D_24B) return 2048;
  double pmin = (double)c->pmin_pa;
  double pmax = (double)c->pmax_pa;
  if(pmax == pmin) return 2048;
  double t = (0.0 - pmin) / (pmax - pmin);
  double u = t * 4095.0;
  int32_t mid = (int32_t)llround(u);
  if(mid < 0) mid = 0;
  if(mid > 4095) mid = 4095;
  return (uint16_t)mid;
}

uint16_t pressure_to_12b(int32_t value){
  const pressure_cfg_t* c=&g_cfg;

  if(c->type == PRESS_TYPE_XGZP6847D_24B && c->map_mode == PRESS_MAP_CENTER_0PA){
    double p = (double)value;
    double pmin = (double)c->pmin_pa;
    double pmax = (double)c->pmax_pa;
    if(pmax == pmin) pmax = pmin + 1.0;
    double t = (p - pmin) / (pmax - pmin);
    double u = t * 4095.0;
    return clamp_u12((int32_t)llround(u), 0, 4095);
  }

  double x = ((double)(value - c->offset)) * (double)c->scale;
  return clamp_u12((int32_t)llround(x), c->clamp_min, c->clamp_max);
}
