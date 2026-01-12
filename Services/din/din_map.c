#include "din_map.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#if __has_include("ff.h")
  #include "ff.h"
  #define DM_HAS_FATFS 1
#else
  #define DM_HAS_FATFS 0
#endif

#define DIN_MAP_NUM_CHANNELS 64

static DIN_MapEntry s_din_map[DIN_MAP_NUM_CHANNELS];
static DIN_MapOutputFn s_out_cb = 0;

// defaults: all enabled, active-low, NOTE, ch1, base_note+idx, vel_on=100
void din_map_init_defaults(uint8_t base_note) {
  memset(s_din_map, 0, sizeof(s_din_map));
  for (uint8_t i = 0; i < DIN_MAP_NUM_CHANNELS; ++i) {
    DIN_MapEntry *e = &s_din_map[i];
    e->enabled = 1;
    e->invert  = 0;
    e->type    = DIN_MAP_TYPE_NOTE;
    e->channel = 0; // ch1
    e->number  = (uint8_t)(base_note + i);
    e->vel_on  = 100;
    e->vel_off = 0;
  }
}

DIN_MapEntry *din_map_get_table(void) {
  return s_din_map;
}

void din_map_set_output_cb(DIN_MapOutputFn cb) {
  s_out_cb = cb;
}

void din_map_process_event(uint8_t index, uint8_t pressed) {
  if (index >= DIN_MAP_NUM_CHANNELS) return;
  if (!s_out_cb) return;

  DIN_MapEntry *e = &s_din_map[index];
  if (!e->enabled) return;

  uint8_t state = pressed ? 1u : 0u;
  if (e->invert) {
    state = state ? 0u : 1u;
  }

  if (e->type == DIN_MAP_TYPE_NOTE) {
    if (state) {
      s_out_cb(DIN_MAP_TYPE_NOTE, e->channel, e->number, e->vel_on);
    } else {
      s_out_cb(DIN_MAP_TYPE_NOTE, e->channel, e->number, e->vel_off);
    }
  } else if (e->type == DIN_MAP_TYPE_CC) {
    uint8_t val = state ? 127u : 0u;
    s_out_cb(DIN_MAP_TYPE_CC, e->channel, e->number, val);
  }
}

// --- SD loading -----------------------------------------------------------
#if DM_HAS_FATFS

static void dm_trim(char* s) {
  if (!s) return;
  while (*s && isspace((unsigned char)*s)) memmove(s, s+1, strlen(s));
  size_t n = strlen(s);
  while (n && isspace((unsigned char)s[n-1])) s[--n] = 0;
}

static int dm_keyeq(const char* a, const char* b) {
  if (!a || !b) return 0;
  while (*a && *b) {
    char ca = (char)tolower((unsigned char)*a);
    char cb = (char)tolower((unsigned char)*b);
    if (ca != cb) return 0;
    ++a; ++b;
  }
  return *a == 0 && *b == 0;
}

static uint8_t dm_u8(const char* s) {
  if (!s) return 0;
  return (uint8_t)strtoul(s, 0, 0);
}

#endif // DM_HAS_FATFS

int din_map_load_sd(const char* path) {
#if !DM_HAS_FATFS
  (void)path;
  return -10;
#else
  if (!path) return -1;

  FIL f;
  FRESULT fr = f_open(&f, path, FA_READ);
  if (fr != FR_OK)
    return -2;

  char line[128];
  int cur = -1;

  while (f_gets(line, sizeof(line), &f)) {
    dm_trim(line);
    if (!line[0] || line[0] == '#' || line[0] == ';')
      continue;

    // [CHn]
    if (line[0] == '[') {
      char* end = strchr(line, ']');
      if (!end) continue;
      *end = 0;
      char* tag = line + 1;
      dm_trim(tag);
      cur = -1;
      if ((tag[0] == 'C' || tag[0] == 'c') && (tag[1] == 'H' || tag[1] == 'h')) {
        int idx = (int)strtol(tag + 2, 0, 10);
        if (idx >= 0 && idx < (int)DIN_MAP_NUM_CHANNELS) {
          cur = idx;
        }
      }
      continue;
    }

    if (cur < 0) continue;

    char* eq = strchr(line, '=');
    if (!eq) continue;
    *eq = 0;
    char* k = line;
    char* v = eq + 1;
    dm_trim(k);
    dm_trim(v);
    if (!k[0]) continue;

    DIN_MapEntry *e = &s_din_map[cur];

    if (dm_keyeq(k, "TYPE")) {
      if (v[0] >= '0' && v[0] <= '9') {
        e->type = dm_u8(v);
      } else {
        char c0 = (char)tolower((unsigned char)v[0]);
        if (c0 == 'n') e->type = DIN_MAP_TYPE_NOTE;
        else if (c0 == 'c') e->type = DIN_MAP_TYPE_CC;
      }
    } else if (dm_keyeq(k, "CHAN") || dm_keyeq(k, "CHANNEL")) {
      e->channel = (uint8_t)(dm_u8(v) & 0x0Fu);
    } else if (dm_keyeq(k, "NUMBER") || dm_keyeq(k, "NOTE") || dm_keyeq(k, "CC")) {
      uint8_t t = dm_u8(v);
      if (t > 127u) t = 127u;
      e->number = t;
    } else if (dm_keyeq(k, "VEL_ON") || dm_keyeq(k, "VELON")) {
      e->vel_on = dm_u8(v);
    } else if (dm_keyeq(k, "VEL_OFF") || dm_keyeq(k, "VELOFF")) {
      e->vel_off = dm_u8(v);
    } else if (dm_keyeq(k, "INVERT")) {
      e->invert = dm_u8(v) ? 1u : 0u;
    } else if (dm_keyeq(k, "ENABLED") || dm_keyeq(k, "ENABLE")) {
      e->enabled = dm_u8(v) ? 1u : 0u;
    }
  }

  f_close(&f);
  return 0;
#endif
}