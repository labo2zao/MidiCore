#include "Services/config/config.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#if __has_include("ff.h")
  #include "ff.h"
  #define CONFIG_HAS_FATFS 1
#else
  #define CONFIG_HAS_FATFS 0
#endif

static inline void trim(char* s) {
  char* p = s;
  while (*p && isspace((unsigned char)*p)) p++;
  if (p != s) memmove(s, p, strlen(p)+1);
  size_t n = strlen(s);
  while (n && isspace((unsigned char)s[n-1])) s[--n] = 0;
}

static inline int parse_u32(const char* v, uint32_t* out) {
  if (!v || !out) return -1;
  char* end = 0;
  unsigned long x = strtoul(v, &end, 0);
  if (end == v) return -2;
  *out = (uint32_t)x;
  return 0;
}

static inline void upcase(char* s) {
  for (; *s; s++) *s = (char)toupper((unsigned char)*s);
}

void config_set_defaults(config_t* c) {
  memset(c, 0, sizeof(*c));

  c->srio_enable = 1;
  c->srio_din_enable = 1;
  c->srio_dout_enable = 1;
  c->srio_din_bytes = 0;
  c->srio_dout_bytes = 0;
  c->srio_scan_ms = 5;

  c->dout_invert_default = 0;
  c->din_invert_default = 0;

  c->rgb_r_invert = 0;
  c->rgb_g_invert = 0;
  c->rgb_b_invert = 0;
  for (int i=0; i<16; i++) {
    c->rgb_map_r[i] = 0xFF;
    c->rgb_map_g[i] = 0xFF;
    c->rgb_map_b[i] = 0xFF;
  }

  c->ui_shift_hold_ms = 0;
  c->instrument_auto_loop = 0;

  c->ainser_enable = 0;
  c->ainser_scan_ms = 5;
}

static void set_key(config_t* c, const char* key_in, const char* v) {
  char key[96];
  size_t L = strlen(key_in);
  if (L >= sizeof(key)) L = sizeof(key)-1;
  memcpy(key, key_in, L);
  key[L] = 0;
  upcase(key);

  uint32_t u=0;

  // SRIO
  if (!strcmp(key,"SRIO_ENABLE")) { if (!parse_u32(v,&u)) c->srio_enable = u?1:0; return; }
  if (!strcmp(key,"SRIO_DIN_ENABLE")) { if (!parse_u32(v,&u)) c->srio_din_enable = u?1:0; return; }
  if (!strcmp(key,"SRIO_DOUT_ENABLE")) { if (!parse_u32(v,&u)) c->srio_dout_enable = u?1:0; return; }
  if (!strcmp(key,"SRIO_DIN_BYTES")) { if (!parse_u32(v,&u)) c->srio_din_bytes = (uint16_t)u; return; }
  if (!strcmp(key,"SRIO_DOUT_BYTES")) { if (!parse_u32(v,&u)) c->srio_dout_bytes = (uint16_t)u; return; }
  if (!strcmp(key,"SRIO_SCAN_MS")) { if (!parse_u32(v,&u)) c->srio_scan_ms = (uint16_t)u; return; }

  // Polarity
  if (!strcmp(key,"DOUT_INVERT_DEFAULT") || !strcmp(key,"DOUT_INVERT")) { if (!parse_u32(v,&u)) c->dout_invert_default = u?1:0; return; }
  if (!strcmp(key,"DIN_INVERT_DEFAULT") || !strcmp(key,"DIN_INVERT")) { if (!parse_u32(v,&u)) c->din_invert_default = u?1:0; return; }

  // BIT_INV_n
  if (!strncmp(key,"BIT_INV_",8)) {
    if (!parse_u32(key+8, &u) && u < 64) {
      uint32_t b=0;
      if (!parse_u32(v,&b)) c->bit_inv[u] = b?1:0;
    }
    return;
  }

  // RGB
  if (!strcmp(key,"RGB_R_INVERT")) { if (!parse_u32(v,&u)) c->rgb_r_invert = u?1:0; return; }
  if (!strcmp(key,"RGB_G_INVERT")) { if (!parse_u32(v,&u)) c->rgb_g_invert = u?1:0; return; }
  if (!strcmp(key,"RGB_B_INVERT")) { if (!parse_u32(v,&u)) c->rgb_b_invert = u?1:0; return; }

  if (!strncmp(key,"RGB_LED_",8)) {
    const char* p = key + 8;
    if (parse_u32(p, &u) || u >= 16) return;
    const char* us = strrchr(key, '_');
    if (!us || !us[1]) return;
    uint32_t bit=0;
    if (parse_u32(v, &bit) || bit > 63) return;
    char ch = us[1];
    if (ch=='R') c->rgb_map_r[u] = (uint8_t)bit;
    else if (ch=='G') c->rgb_map_g[u] = (uint8_t)bit;
    else if (ch=='B') c->rgb_map_b[u] = (uint8_t)bit;
    return;
  }

  // UI
  if (!strcmp(key,"UI_SHIFT_HOLD_MS") || !strcmp(key,"SHIFT_HOLD_MS")) { if (!parse_u32(v,&u)) c->ui_shift_hold_ms = (uint16_t)u; return; }

  // INSTRUMENT
  if (!strcmp(key,"INSTRUMENT_AUTO_LOOP") || !strcmp(key,"AUTO_LOOP")) { if (!parse_u32(v,&u)) c->instrument_auto_loop = u?1:0; return; }

  // AINSER placeholders
  if (!strcmp(key,"AINSER_ENABLE")) { if (!parse_u32(v,&u)) c->ainser_enable = u?1:0; return; }
  if (!strcmp(key,"AINSER_SCAN_MS")) { if (!parse_u32(v,&u)) c->ainser_scan_ms = (uint16_t)u; return; }
// --- Global / Safety ---
if (!strcmp(key, "GLOBAL_SAFE_MODE") || !strcmp(key, "SAFE_MODE")) {
  c->global_safe_mode = (uint8_t)(atoi(v) ? 1 : 0);
  return;
}
if (!strcmp(key, "GLOBAL_SD_REQUIRED") || !strcmp(key, "SD_REQUIRED")) {
  c->global_sd_required = (uint8_t)(atoi(v) ? 1 : 0);
  return;
}
if (!strcmp(key, "GLOBAL_SHIFT_ACTIVE_LOW") || !strcmp(key, "SHIFT_ACTIVE_LOW")) {
  c->global_shift_active_low = (uint8_t)(atoi(v) ? 1 : 0);
  return;
}
}


static void make_prefixed_key(char* out, size_t out_sz, const char* section, const char* key) {
  if (!section[0]) {
    strncpy(out, key, out_sz-1);
    out[out_sz-1]=0;
    return;
  }
  snprintf(out, out_sz, "%s_%s", section, key);
}

int config_load_from_sd(config_t* c, const char* path) {
  if (!c || !path) return -1;
  config_set_defaults(c);

#if !CONFIG_HAS_FATFS
  (void)path;
  return -10;
#else
  FIL fp;
  FRESULT fr = f_open(&fp, path, FA_READ);
  if (fr != FR_OK) return -2;

  char section[16] = {0};
  char line[160];

  while (f_gets(line, sizeof(line), &fp)) {
    for (size_t i=0; i<sizeof(line) && line[i]; i++) {
      if (line[i] == '\r' || line[i] == '\n') { line[i]=0; break; }
    }
    trim(line);
    if (!line[0] || line[0] == '#') continue;

    if (line[0] == '[') {
      char* end = strchr(line, ']');
      if (!end) continue;
      *end = 0;
      strncpy(section, line+1, sizeof(section)-1);
      section[sizeof(section)-1]=0;
      trim(section);
      upcase(section);
      // Alias sections
      if (!strcmp(section,"DOUT_RGB")) strcpy(section, "RGB");
      continue;
    }

    char* eq = strchr(line, '=');
    if (!eq) continue;
    *eq = 0;
    char* k = line;
    char* v = eq+1;
    trim(k); trim(v);
    if (!k[0]) continue;

    char keybuf[96];
    make_prefixed_key(keybuf, sizeof(keybuf), section, k);
    trim(keybuf);

    // allow [DOUT] INVERT_DEFAULT=1 to map to DOUT_INVERT_DEFAULT
    // This is handled by set_key via DOUT_INVERT alias
    set_key(c, keybuf, v);

    // additionally: if section is DOUT and key is INVERT_DEFAULT, also call DOUT_INVERT_DEFAULT
    // (covered by alias DOUT_INVERT)
  }

  f_close(&fp);
  return 0;
#endif
}
