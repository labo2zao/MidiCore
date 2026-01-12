#include "Services/zones/zones_cfg.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#if __has_include("ff.h")
  #include "ff.h"
  #define ZC_HAS_FATFS 1
#else
  #define ZC_HAS_FATFS 0
#endif

static zones_cfg_t g_z;

static inline void trim(char* s) {
  char* p = s;
  while (*p && isspace((unsigned char)*p)) p++;
  if (p != s) memmove(s, p, strlen(p)+1);
  size_t n=strlen(s);
  while (n && isspace((unsigned char)s[n-1])) s[--n]=0;
}

static inline int keyeq(const char* a, const char* b) {
  while (*a && *b) {
    char ca=(char)toupper((unsigned char)*a++);
    char cb=(char)toupper((unsigned char)*b++);
    if (ca!=cb) return 0;
  }
  return *a==0 && *b==0;
}

static inline uint8_t u8(const char* v){ 
  long x=strtol(v,0,10); 
  if(x<0)x=0; 
  if(x>255)x=255; 
  return (uint8_t)x; 
}

static inline int8_t s8(const char* v){ 
  long x=strtol(v,0,10); 
  if(x<-128)x=-128; 
  if(x>127)x=127; 
  return (int8_t)x; 
}

void zones_cfg_defaults(zones_cfg_t* z) {
  if(!z) return;
  memset(z,0,sizeof(*z));
  z->zone[0].enable=1;
  z->zone[0].key_min=0; z->zone[0].key_max=63;
  z->zone[0].ch[0]=0; z->zone[0].ch[1]=0;
  z->zone[0].l2_enable=0;
  z->zone[0].stack=0;
  z->zone[0].transpose[0]=0; z->zone[0].transpose[1]=0;
  z->zone[0].vel_mul_q7=128;
  z->zone[0].vel_add=0;
  z->zone[0].prio=1;
}

static void set_zone_key(zone_t* z, const char* key, const char* val) {
  if (keyeq(key,"ENABLE")) z->enable = u8(val)?1:0;
  else if (keyeq(key,"KEY_MIN")) z->key_min = u8(val);
  else if (keyeq(key,"KEY_MAX")) z->key_max = u8(val);
  else if (keyeq(key,"CH1")) z->ch[0] = (u8(val)>15)?15:u8(val);
  else if (keyeq(key,"CH2")) z->ch[1] = (u8(val)>15)?15:u8(val);
  else if (keyeq(key,"L2_ENABLE")) z->l2_enable = u8(val)?1:0;
  else if (keyeq(key,"STACK")) z->stack = u8(val)?1:0;
  else if (keyeq(key,"TR1")) z->transpose[0] = s8(val);
  else if (keyeq(key,"TR2")) z->transpose[1] = s8(val);
  else if (keyeq(key,"VEL_MUL")) z->vel_mul_q7 = u8(val);
  else if (keyeq(key,"VEL_ADD")) z->vel_add = s8(val);
  else if (keyeq(key,"PRIO")) z->prio = u8(val);
}

int zones_cfg_load_sd(zones_cfg_t* z, const char* path) {
#if !ZC_HAS_FATFS
  (void)z;(void)path;
  return -10;
#else
  if(!z||!path) return -1;
  FIL f;
  if (f_open(&f, path, FA_READ) != FR_OK) return -2;

  char line[128];
  int cur=-1;

  while (f_gets(line,sizeof(line),&f)) {
    trim(line);
    if(!line[0] || line[0]=='#' || line[0]==';') continue;
    if(line[0]=='[') {
      char* e=strchr(line,']'); if(!e) continue;
      *e=0;
      if (strncmp(line+1,"ZONE",4)==0) {
        cur = (int)strtol(line+5,0,10);
        if (cur<0||cur>=ZONES_MAX) cur=-1;
      } else cur=-1;
      continue;
    }
    if(cur<0) continue;
    char* eq=strchr(line,'=');
    if(!eq) continue;
    *eq=0;
    char* k=line; char* v=eq+1;
    trim(k); trim(v);
    set_zone_key(&z->zone[cur], k, v);
  }
  f_close(&f);

  for(int i=0;i<ZONES_MAX;i++){
    zone_t* zn=&z->zone[i];
    if(zn->key_min>63) zn->key_min=63;
    if(zn->key_max>63) zn->key_max=63;
    if(zn->key_min>zn->key_max){ uint8_t t=zn->key_min; zn->key_min=zn->key_max; zn->key_max=t; }
    if(zn->vel_mul_q7==0) zn->vel_mul_q7=1;
    if(zn->l2_enable>1) zn->l2_enable=1;
    if(zn->stack>1) zn->stack=1;
  }
  return 0;
#endif
}

const zones_cfg_t* zones_cfg_get(void){ return &g_z; }
void zones_cfg_set(const zones_cfg_t* z){ if(z) g_z=*z; else zones_cfg_defaults(&g_z); }

static int match_zone(const zone_t* z, uint8_t key) {
  if(!z->enable) return 0;
  return (key>=z->key_min && key<=z->key_max) ? 1 : 0;
}

uint8_t zones_map_note(uint8_t key, uint8_t in_note, uint8_t in_vel,
                       uint8_t out_ch[ZONE_LAYERS_MAX],
                       uint8_t out_note[ZONE_LAYERS_MAX],
                       uint8_t out_vel[ZONE_LAYERS_MAX]) {
  const zone_t* best = 0;
  for(int i=0;i<ZONES_MAX;i++){
    const zone_t* z=&g_z.zone[i];
    if(!match_zone(z,key)) continue;
    if(!best || z->prio > best->prio) best=z;
  }
  if(!best) return 0;

  int32_t v = (int32_t)in_vel;
  v = (v * (int32_t)best->vel_mul_q7) / 128;
  v += (int32_t)best->vel_add;
  if(v<1) v=1;
  if(v>127) v=127;

  uint8_t cnt=0;
  for(int layer=0; layer<ZONE_LAYERS_MAX; layer++){
    if(layer==1) {
      uint8_t implicit = (best->transpose[1]!=0 || best->ch[1]!=best->ch[0]) ? 1u : 0u;
      if(!best->stack && !best->l2_enable && !implicit) continue;
    }

    int32_t n = (int32_t)in_note + (int32_t)best->transpose[layer];
    if(n<0) n=0;
    if(n>127) n=127;
    out_ch[cnt]=best->ch[layer] & 0x0F;
    out_note[cnt]=(uint8_t)n;
    out_vel[cnt]=(uint8_t)v;
    cnt++;
  }
  return cnt;
}
