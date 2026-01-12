#include "Services/dream/dream_sysex.h"
#include "Services/router/router_send.h"
#include "Config/router_config.h"
#include "Services/usb_midi/usb_midi_sysex.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#if __has_include("ff.h")
#include "ff.h"
#define DREAM_HAS_FATFS 1
#else
#define DREAM_HAS_FATFS 0
#endif

static void trim(char* s) {
  while (*s && isspace((unsigned char)*s)) memmove(s, s+1, strlen(s));
  size_t n=strlen(s);
  while (n && isspace((unsigned char)s[n-1])) s[--n]=0;
}
static void upcase(char* s){ for(;*s;s++) *s=(char)toupper((unsigned char)*s); }

static int parse_out_node(const char* v, uint8_t* out_node) {
  if (!v || !out_node) return -1;
  char tmp[24]; strncpy(tmp, v, sizeof(tmp)-1); tmp[sizeof(tmp)-1]=0;
  trim(tmp); upcase(tmp);

  if (!strcmp(tmp,"USB") || !strcmp(tmp,"USB_OUT")) { *out_node = ROUTER_NODE_USB_OUT; return 0; }
  if (!strcmp(tmp,"DIN1") || !strcmp(tmp,"DIN_OUT1")) { *out_node = ROUTER_NODE_DIN_OUT1; return 0; }
  if (!strcmp(tmp,"DIN2") || !strcmp(tmp,"DIN_OUT2")) { *out_node = ROUTER_NODE_DIN_OUT2; return 0; }
  if (!strcmp(tmp,"DIN3") || !strcmp(tmp,"DIN_OUT3")) { *out_node = ROUTER_NODE_DIN_OUT3; return 0; }
  if (!strcmp(tmp,"DIN4") || !strcmp(tmp,"DIN_OUT4")) { *out_node = ROUTER_NODE_DIN_OUT4; return 0; }

  // numeric
  int n = atoi(tmp);
  if (n >= 0 && n <= 255) { *out_node = (uint8_t)n; return 0; }
  return -2;
}

// Parse a text .syx-like file: hex bytes separated by spaces, comments with '#'
static int load_sysex_text(const char* path, uint8_t* buf, size_t cap, size_t* out_len) {
#if !DREAM_HAS_FATFS
  (void)path; (void)buf; (void)cap; (void)out_len;
  return -10;
#else
  FIL fp;
  if (f_open(&fp, path, FA_READ) != FR_OK) return -2;

  size_t n = 0;
  char line[160];
  while (f_gets(line, sizeof(line), &fp)) {
    for (size_t i=0;i<sizeof(line) && line[i]; i++) {
      if (line[i]=='\r' || line[i]=='\n') { line[i]=0; break; }
    }
    char* hash = strchr(line,'#');
    if (hash) *hash = 0;
    trim(line);
    if (!line[0]) continue;

    char* p = line;
    while (*p) {
      while (*p && isspace((unsigned char)*p)) p++;
      if (!*p) break;

      char tok[8]={0};
      size_t tl=0;
      while (*p && !isspace((unsigned char)*p) && tl<sizeof(tok)-1) tok[tl++]=*p++;
      tok[tl]=0;

      unsigned long val = strtoul(tok, 0, 16);
      if (n < cap) buf[n++] = (uint8_t)(val & 0xFF);
      else { f_close(&fp); return -3; }
    }
  }
  f_close(&fp);
  if (out_len) *out_len = n;
  return 0;
#endif
}

static int load_sysex_bin(const char* path, uint8_t* buf, size_t cap, size_t* out_len) {
#if !DREAM_HAS_FATFS
  (void)path; (void)buf; (void)cap; (void)out_len;
  return -10;
#else
  FIL fp;
  if (f_open(&fp, path, FA_READ) != FR_OK) return -2;
  UINT br=0;
  if (cap > 0xFFFFu) cap = 0xFFFFu; // FATFS UINT limit safety
  FRESULT fr = f_read(&fp, buf, (UINT)cap, &br);
  f_close(&fp);
  if (fr != FR_OK) return -4;
  if (out_len) *out_len = (size_t)br;
  return 0;
#endif
}

static int is_probably_text(const uint8_t* h, size_t n) {
  // allow ASCII hex digits, whitespace, '#', ';', ',', '[', ']', '=', letters
  for (size_t i=0;i<n;i++) {
    uint8_t c=h[i];
    if (c==0) continue;
    if (c==0xF0 || c==0xF7) return 0; // looks binary
    if (c=='#' || c==';' || c==',' || c=='[' || c==']' || c=='=' ) continue;
    if (c == '\r' || c == '\n' || c == '\t' || c == ' ') continue;
    if ((c>='0'&&c<='9')||(c>='A'&&c<='F')||(c>='a'&&c<='f')) continue;
    if ((c>='A'&&c<='Z')||(c>='a'&&c<='z')||c=='_'||c=='/'||c==':'||c=='.') continue;
    return 0;
  }
  return 1;
}

static int load_sysex_auto(const char* path, uint8_t* buf, size_t cap, size_t* out_len) {
#if !DREAM_HAS_FATFS
  (void)path; (void)buf; (void)cap; (void)out_len;
  return -10;
#else
  // peek header
  FIL fp;
  if (f_open(&fp, path, FA_READ) != FR_OK) return -2;
  uint8_t hdr[16]={0};
  UINT br=0;
  (void)f_read(&fp, hdr, sizeof(hdr), &br);
  f_close(&fp);

  if (br >= 1 && hdr[0] == 0xF0) {
    return load_sysex_bin(path, buf, cap, out_len);
  }
  if (is_probably_text(hdr, br)) {
    return load_sysex_text(path, buf, cap, out_len);
  }
  // fallback: binary
  return load_sysex_bin(path, buf, cap, out_len);
#endif
}

static uint32_t dj_hash(const char* s) {
  uint32_t h=2166136261u;
  while (s && *s) { h ^= (uint8_t)*s++; h *= 16777619u; }
  return h ? h : 1u;
}
static uint32_t g_sent_hashes[8] = {0};
static uint8_t g_sent_wr = 0;

static uint8_t was_sent(uint32_t h) {
  for (uint8_t i=0;i<8;i++) if (g_sent_hashes[i] == h) return 1;
  return 0;
}
static void mark_sent(uint32_t h) {
  g_sent_hashes[g_sent_wr++ & 7u] = h;
}

static int send_sysex(uint8_t out_node, const uint8_t* data, size_t len) {

  if (!data || len == 0) return -1;

  if (out_node == ROUTER_NODE_USB_OUT) {
    usb_midi_send_sysex(data, len);
    return 0;
  }

  // UART DIN: send raw bytes
  // DIN out nodes are contiguous in router_send_default, but we can send ourselves:
  router_msg_t m = {0};
  m.type = ROUTER_MSG_1B;
  for (size_t i=0;i<len;i++) {
    m.b0 = data[i];
    (void)router_send_default(out_node, &m);
  }
  return 0;
}

int dream_apply_from_patch(const char* patch_path) {
#if !DREAM_HAS_FATFS
  (void)patch_path;
  return 0;
#else
  if (!patch_path) return -1;

  // Parse [DREAM] section keys:
  // SYSEX_FILE=<path>
  // SYSEX_LIST=<path1;path2;path3>
  // OUT_NODE=USB_OUT|DIN_OUT1|...
  // SEND_ONCE=0|1
  // KEY=<string> (optional: identifies once-per-boot)
  char syx_path[96] = {0};
  char syx_list[192] = {0};
  char key[64] = {0};
  uint8_t send_once = 0;
  uint8_t out_node = ROUTER_NODE_USB_OUT;

  FIL fp;
  if (f_open(&fp, patch_path, FA_READ) != FR_OK) return 0;

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

    if (strcmp(section,"DREAM")!=0) continue;

    char* eq=strchr(line,'=');
    if (!eq) continue;
    *eq=0;
    char* k=line; char* v=eq+1;
    trim(k); trim(v); upcase(k);

    if (!strcmp(k,"SYSEX_FILE")) {
      strncpy(syx_path, v, sizeof(syx_path)-1);
      syx_path[sizeof(syx_path)-1]=0;
    } else if (!strcmp(k,"OUT_NODE")) {
      (void)parse_out_node(v, &out_node);
    }
  }

  f_close(&fp);// Determine once-per-boot key
if (send_once) {
  const char* ksrc = key[0] ? key : patch_path;
  uint32_t h = dj_hash(ksrc);
  if (was_sent(h)) return 0;
  mark_sent(h);
}

// Send one or multiple SysEx files
uint8_t buf[256];
size_t n=0;

if (syx_list[0]) {
  // split by ';' or ','
  char list[192];
  strncpy(list, syx_list, sizeof(list)-1);
  list[sizeof(list)-1]=0;

  char* p = list;
  while (p && *p) {
    char* sep = strpbrk(p, ";,");
    if (sep) *sep = 0;
    trim(p);
    if (p[0]) {
      n = 0;
      int lr = load_sysex_auto(p, buf, sizeof(buf), &n);
      if (lr >= 0 && n > 0) (void)send_sysex(out_node, buf, n);
    }
    if (!sep) break;
    p = sep + 1;
  }
  return 0;
}

if (!syx_path[0]) return 0;
int lr = load_sysex_auto(syx_path, buf, sizeof(buf), &n);
if (lr < 0 || n == 0) return lr;
return send_sysex(out_node, buf, n);
#endif
}
