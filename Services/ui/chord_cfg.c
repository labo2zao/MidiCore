#include "Services/ui/chord_cfg.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#if __has_include("ff.h")
  #include "ff.h"
  #define CH_HAS_FATFS 1
#else
  #define CH_HAS_FATFS 0
#endif

static void trim(char* s) {
  while (*s && isspace((unsigned char)*s)) memmove(s, s+1, strlen(s));
  size_t n=strlen(s);
  while (n && isspace((unsigned char)s[n-1])) s[--n]=0;
}
static void upcase(char* s){ for(;*s;s++) *s=(char)toupper((unsigned char)*s); }

static int parse_i32(const char* v, int32_t* out) {
  if (!v || !out) return -1;
  char* end=0;
  long x=strtol(v,&end,0);
  if (end==v) return -2;
  *out=(int32_t)x;
  return 0;
}

static void preset_defaults(chord_preset_t* c) {
  memset(c,0,sizeof(*c));
  c->count = 3;
  c->intervals[0]=0; c->intervals[1]=4; c->intervals[2]=7; c->intervals[3]=12;
  c->transpose = 0;
  c->vel_scale[0]=100; c->vel_scale[1]=95; c->vel_scale[2]=90; c->vel_scale[3]=80;
}

void chord_bank_defaults(chord_bank_t* b) {
  if (!b) return;
  memset(b,0,sizeof(*b));
  b->preset_count = 1;
  preset_defaults(&b->preset[0]);
  for (int i=0;i<12;i++) b->map_noteclass[i]=0;
}

static void set_preset_key(chord_preset_t* c, const char* key_in, const char* v) {
  char key[16];
  size_t L=strlen(key_in); if (L>=sizeof(key)) L=sizeof(key)-1;
  memcpy(key,key_in,L); key[L]=0;
  trim(key); upcase(key);

  int32_t x=0;
  if (!strcmp(key,"COUNT")) {
    if (!parse_i32(v,&x)) { if (x<1) x=1; if (x>4) x=4; c->count=(uint8_t)x; }
    return;
  }
  if (!strcmp(key,"TRANSPOSE")) {
    if (!parse_i32(v,&x)) { if (x<-24) x=-24; if (x>24) x=24; c->transpose=(int8_t)x; }
    return;
  }
  if (key[0]=='I' && isdigit((unsigned char)key[1])) {
    uint8_t idx=(uint8_t)(key[1]-'0');
    if (idx<4 && !parse_i32(v,&x)) { if (x<-48) x=-48; if (x>48) x=48; c->intervals[idx]=(int8_t)x; }
    return;
  }
  if (key[0]=='V' && isdigit((unsigned char)key[1])) {
    uint8_t idx=(uint8_t)(key[1]-'0');
    if (idx<4 && !parse_i32(v,&x)) { if (x<0) x=0; if (x>200) x=200; c->vel_scale[idx]=(uint8_t)x; }
    return;
  }
}

static int noteclass_from_name(const char* k) {
  // Accept NOTECLASS0..11 or C,C#,DB,D,...,B
  if (!k) return -1;
  if (!strncmp(k,"NOTECLASS",9) && isdigit((unsigned char)k[9])) {
    int n = atoi(k+9);
    if (n>=0 && n<12) return n;
  }
  if (!strcmp(k,"C")) return 0;
  if (!strcmp(k,"C#") || !strcmp(k,"DB")) return 1;
  if (!strcmp(k,"D")) return 2;
  if (!strcmp(k,"D#") || !strcmp(k,"EB")) return 3;
  if (!strcmp(k,"E")) return 4;
  if (!strcmp(k,"F")) return 5;
  if (!strcmp(k,"F#") || !strcmp(k,"GB")) return 6;
  if (!strcmp(k,"G")) return 7;
  if (!strcmp(k,"G#") || !strcmp(k,"AB")) return 8;
  if (!strcmp(k,"A")) return 9;
  if (!strcmp(k,"A#") || !strcmp(k,"BB")) return 10;
  if (!strcmp(k,"B")) return 11;
  return -1;
}

int chord_bank_load(chord_bank_t* b, const char* path) {
  if (!b || !path) return -1;
  chord_bank_defaults(b);

#if !CH_HAS_FATFS
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
    upcase(k);

    if (!strcmp(section,"MAP")) {
      int nc = noteclass_from_name(k);
      int32_t idx=0;
      if (nc>=0 && !parse_i32(v,&idx)) {
        if (idx<0) idx=0;
        if (idx>=CHORD_MAX_PRESETS) idx=CHORD_MAX_PRESETS-1;
        b->map_noteclass[nc] = (uint8_t)idx;
        if ((uint8_t)(idx+1) > b->preset_count) b->preset_count = (uint8_t)(idx+1);
      }
      continue;
    }

    // CHORD0..CHORD7
    if (!strncmp(section,"CHORD",5) && isdigit((unsigned char)section[5])) {
      int pi = atoi(section+5);
      if (pi>=0 && pi<CHORD_MAX_PRESETS) {
        // ensure preset initialized
        static uint8_t init_mask[CHORD_MAX_PRESETS] = {0};
        if (!init_mask[pi]) { preset_defaults(&b->preset[pi]); init_mask[pi]=1; }
        set_preset_key(&b->preset[pi], k, v);
        if ((uint8_t)(pi+1) > b->preset_count) b->preset_count = (uint8_t)(pi+1);
      }
      continue;
    }
  }

  f_close(&fp);

  if (b->preset_count < 1) b->preset_count = 1;
  if (b->preset_count > CHORD_MAX_PRESETS) b->preset_count = CHORD_MAX_PRESETS;

  // clamp map to preset_count
  for (int i=0;i<12;i++) if (b->map_noteclass[i] >= b->preset_count) b->map_noteclass[i] = 0;

  return 0;
#endif
}

static uint8_t dedup(uint8_t* a, uint8_t n) {
  uint8_t out=0;
  for (uint8_t i=0;i<n;i++) {
    uint8_t x=a[i];
    uint8_t seen=0;
    for (uint8_t j=0;j<out;j++) if (a[j]==x) { seen=1; break; }
    if (!seen) a[out++]=x;
  }
  return out;
}

uint8_t chord_preset_scale_vel(const chord_preset_t* c, uint8_t idx, uint8_t vel) {
  if (!c) return vel;
  if (idx >= 4) return vel;
  uint32_t s = c->vel_scale[idx];
  uint32_t v = vel;
  v = (v * s + 50) / 100;
  if (v > 127) v = 127;
  return (uint8_t)v;
}

uint8_t chord_bank_expand(const chord_bank_t* b, uint8_t root, uint8_t notes_out[4], uint8_t* preset_used) {
  if (!b || !notes_out) return 0;
  uint8_t nc = (uint8_t)(root % 12u);
  uint8_t pi = b->map_noteclass[nc];
  if (pi >= b->preset_count) pi = 0;
  if (preset_used) *preset_used = pi;

  const chord_preset_t* c = &b->preset[pi];
  uint8_t n = c->count;
  if (n < 1) {
    n = 1;
  }
  if (n > 4) {
    n = 4;
  }

  for (uint8_t i=0;i<n;i++) {
    int32_t note = (int32_t)root + (int32_t)c->intervals[i] + (int32_t)c->transpose;
    if (note < 0) note = 0;
    if (note > 127) note = 127;
    notes_out[i] = (uint8_t)note;
  }
  n = dedup(notes_out, n);
  return n;
}
