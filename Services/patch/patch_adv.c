#include "Services/patch/patch_adv.h"
#include "ff.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef PATCH_ADV_MAX_ENTRIES
// Memory optimization: Reduce entries to balance functionality and RAM usage
// Each entry is 160 bytes (24+24+64+48)
// Production: 192 entries = 30.7 KB (reduced from 256 to save ~10 KB)
// Test: 128 entries = 20 KB (saves additional 10 KB)
#if defined(MODULE_TEST_LOOPER) || defined(MODULE_TEST_OLED_SSD1322) || defined(MODULE_TEST_ALL) || \
    defined(MODULE_TEST_UI) || defined(MODULE_TEST_GDB_DEBUG) || defined(MODULE_TEST_AINSER64) || \
    defined(MODULE_TEST_SRIO) || defined(MODULE_TEST_SRIO_DOUT) || defined(MODULE_TEST_MIDI_DIN) || \
    defined(MODULE_TEST_ROUTER) || defined(MODULE_TEST_LFO) || defined(MODULE_TEST_HUMANIZER) || \
    defined(MODULE_TEST_UI_PAGE_SONG) || defined(MODULE_TEST_UI_PAGE_MIDI_MONITOR) || \
    defined(MODULE_TEST_UI_PAGE_SYSEX) || defined(MODULE_TEST_UI_PAGE_CONFIG) || \
    defined(MODULE_TEST_UI_PAGE_LIVEFX) || defined(MODULE_TEST_UI_PAGE_RHYTHM) || \
    defined(MODULE_TEST_UI_PAGE_HUMANIZER) || defined(MODULE_TEST_PATCH_SD) || \
    defined(MODULE_TEST_PRESSURE) || defined(MODULE_TEST_BREATH) || \
    defined(MODULE_TEST_USB_HOST_MIDI) || defined(MODULE_TEST_USB_DEVICE_MIDI) || \
    defined(MODULE_TEST_FOOTSWITCH) || defined(APP_TEST_DIN_MIDI)
  #define PATCH_ADV_MAX_ENTRIES 128  // Test mode: 128 entries (20 KB)
#else
  #define PATCH_ADV_MAX_ENTRIES 192  // Production: 192 entries (30.7 KB, reduced from 256 to save ~10 KB)
#endif
#endif

static patch_entry_t g_entries[PATCH_ADV_MAX_ENTRIES];
static uint32_t g_count = 0;

static void str_trim(char* s) {
  char* p = s;
  while (*p && isspace((unsigned char)*p)) p++;
  if (p != s) memmove(s, p, strlen(p)+1);
  size_t n = strlen(s);
  while (n && isspace((unsigned char)s[n-1])) { s[n-1] = 0; n--; }
}

static inline uint8_t starts_with(const char* s, const char* pfx) {
  size_t pfx_len = strlen(pfx);
  return strncmp(s, pfx, pfx_len) == 0;
}

static void clear_all(void) {
  memset(g_entries, 0, sizeof(g_entries));
  g_count = 0;
}

void patch_adv_init(void) { 
  g_count = 0;
  // No need to clear entire array; only clear entries as they're reused
}

static int find_entry(const char* section, const char* key, const char* cond) {
  if (!cond) cond = "";
  for (uint32_t i=0;i<g_count;i++) {
    if (strcmp(g_entries[i].section, section) != 0) continue;
    if (strcmp(g_entries[i].key, key) != 0) continue;
    if (strcmp(g_entries[i].cond, cond) != 0) continue;
    return (int)i;
  }
  return -1;
}

int patch_adv_set(const char* section, const char* key, const char* value, const char* cond) {
  if (!section) section = "";
  if (!key) key = "";
  if (!value) value = "";
  if (!cond) cond = "";

  int idx = find_entry(section, key, cond);
  if (idx < 0) {
    if (g_count >= PATCH_ADV_MAX_ENTRIES) return -1;
    idx = (int)g_count++;
    strncpy(g_entries[idx].section, section, sizeof(g_entries[idx].section)-1);
    strncpy(g_entries[idx].key, key, sizeof(g_entries[idx].key)-1);
    strncpy(g_entries[idx].cond, cond, sizeof(g_entries[idx].cond)-1);
  }
  strncpy(g_entries[idx].value, value, sizeof(g_entries[idx].value)-1);
  return 0;
}

int patch_adv_get(const char* section, const char* key, char* out, uint32_t out_max) {
  if (!out || out_max == 0) return -1;
  if (!section) section = "";
  if (!key) key = "";

  // First try unconditional entries
  for (uint32_t i=0;i<g_count;i++) {
    if (strcmp(g_entries[i].section, section) != 0) continue;
    if (strcmp(g_entries[i].key, key) != 0) continue;
    if (g_entries[i].cond[0]) continue;
    strncpy(out, g_entries[i].value, out_max-1);
    out[out_max-1]=0;
    return 0;
  }
  // Fallback: first conditional entry (caller can evaluate cond)
  for (uint32_t i=0;i<g_count;i++) {
    if (strcmp(g_entries[i].section, section) != 0) continue;
    if (strcmp(g_entries[i].key, key) != 0) continue;
    strncpy(out, g_entries[i].value, out_max-1);
    out[out_max-1]=0;
    return 0;
  }
  return -1;
}

uint32_t patch_adv_count(void) { return g_count; }
const patch_entry_t* patch_adv_at(uint32_t idx) { return (idx<g_count) ? &g_entries[idx] : 0; }

static void parse_section(char* line, char* current, size_t maxlen) {
  char* end = strchr(line, ']');
  if (!end) return;
  *end = 0;
  char* name = line+1;
  str_trim(name);
  strncpy(current, name, maxlen-1);
  current[maxlen-1]=0;
}

int patch_adv_load(const char* filename) {
  FIL f;
  if (f_open(&f, filename, FA_READ) != FR_OK) return -1;

  char current_section[24] = "";
  char line[160];

  while (f_gets(line, sizeof(line), &f)) {
    line[strcspn(line, "\r\n")] = 0;
    str_trim(line);
    if (!line[0]) continue;
    if (line[0] == '#' || line[0] == ';') continue;

    if (line[0] == '[') {
      parse_section(line, current_section, sizeof(current_section));
      continue;
    }

    char* eq = strchr(line, '=');
    if (!eq) continue;
    *eq = 0;
    char* key = line;
    char* val = eq+1;
    str_trim(key);
    str_trim(val);

    char cond[48] = "";
    char* q = strchr(key, '?');
    if (q) {
      *q = 0;
      strncpy(cond, q+1, sizeof(cond)-1);
      str_trim(key);
      str_trim(cond);
    }

    (void)patch_adv_set(current_section, key, val, cond);
  }

  f_close(&f);
  return 0;
}

int patch_adv_save(const char* filename) {
  FIL f;
  if (f_open(&f, filename, FA_CREATE_ALWAYS | FA_WRITE) != FR_OK) return -1;

  char last_section[24] = "";
  for (uint32_t i=0;i<g_count;i++) {
    if (strcmp(last_section, g_entries[i].section) != 0) {
      strncpy(last_section, g_entries[i].section, sizeof(last_section)-1);
      last_section[sizeof(last_section)-1]=0;
      if (last_section[0]) f_printf(&f, "\n[%s]\n", last_section);
    }
    if (g_entries[i].cond[0]) {
      f_printf(&f, "%s?%s=%s\n", g_entries[i].key, g_entries[i].cond, g_entries[i].value);
    } else {
      f_printf(&f, "%s=%s\n", g_entries[i].key, g_entries[i].value);
    }
  }

  f_close(&f);
  return 0;
}

int patch_adv_load_bank(uint8_t bank) {
  char path[64];
  snprintf(path, sizeof(path), "0:/patches/bank%02u/patch.txt", (unsigned)bank);
  return patch_adv_load(path);
}


static uint8_t parse_uint(const char* s, uint32_t* out) {
  if (!s || !*s) return 0;
  char* end=0;
  long v = strtol(s, &end, 10);
  if (end == s) return 0;
  if (v < 0) return 0;
  *out = (uint32_t)v;
  return 1;
}

// ---------------- Condition expression evaluator ----------------
// Grammar (whitespace ignored):
//   expr    := or_expr
//   or_expr := and_expr ( ( "||" | "or" ) and_expr )*
//   and_expr:= unary ( ( "&&" | "and" ) unary )*
//   unary   := ( "!" | "not" ) unary | primary
//   primary := "(" expr ")" | comparison | literal
//   comparison := ident op value
//   op := "==" "!=" ">=" "<=" ">" "<"
//   value := number | range | ident
//   range := number ".." number
//
// Supported identifiers: ch, node
// Supported literals: true, false, always
//
// Examples:
//   ch==1
//   ch>=1 && ch<=4
//   ch in 1..4   (written as: ch==1..4  OR ch=1..4 is also accepted)
//   (node==0 || node==1) && ch!=10
// ----------------------------------------------------------------

typedef struct {
  const char* s;
} cond_lex_t;

static void skip_ws(cond_lex_t* lx) {
  while (*lx->s && isspace((unsigned char)*lx->s)) lx->s++;
}

static uint8_t match_kw(cond_lex_t* lx, const char* kw) {
  skip_ws(lx);
  size_t n = strlen(kw);
  if (strncasecmp(lx->s, kw, n) == 0) {
    // ensure keyword boundary
    if (isalnum((unsigned char)kw[n-1]) || kw[n-1]=='_') {
      if (isalnum((unsigned char)lx->s[n]) || lx->s[n]=='_') return 0;
    }
    lx->s += n;
    return 1;
  }
  return 0;
}

static uint8_t match_sym(cond_lex_t* lx, const char* sym) {
  skip_ws(lx);
  size_t n = strlen(sym);
  if (strncmp(lx->s, sym, n) == 0) {
    lx->s += n;
    return 1;
  }
  return 0;
}

static uint8_t parse_ident(cond_lex_t* lx, char* out, size_t outsz) {
  skip_ws(lx);
  const char* p = lx->s;
  if (!(isalpha((unsigned char)*p) || *p=='_')) return 0;
  size_t i=0;
  while (*p && (isalnum((unsigned char)*p) || *p=='_')) {
    if (i+1 < outsz) out[i++] = *p;
    p++;
  }
  out[i]=0;
  lx->s = p;
  return 1;
}

static uint8_t parse_number(cond_lex_t* lx, uint32_t* out) {
  skip_ws(lx);
  const char* p = lx->s;
  if (!isdigit((unsigned char)*p)) return 0;
  char buf[16]; size_t i=0;
  while (*p && isdigit((unsigned char)*p) && i+1<sizeof(buf)) buf[i++]=*p++;
  buf[i]=0;
  lx->s = p;
  return parse_uint(buf, out);
}

static uint8_t parse_value(cond_lex_t* lx, uint32_t* a, uint8_t* is_range, uint32_t* b, const patch_ctx_t* ctx) {
  // value can be number, range, or ident (ch/node)
  *is_range = 0;
  uint32_t v1=0;
  const char* save = lx->s;
  if (parse_number(lx, &v1)) {
    skip_ws(lx);
    if (match_sym(lx, "..")) {
      uint32_t v2=0;
      if (!parse_number(lx, &v2)) { lx->s = save; return 0; }
      *a = v1; *b = v2; *is_range = 1;
      return 1;
    }
    *a = v1; return 1;
  }
  lx->s = save;
  char id[16];
  if (parse_ident(lx, id, sizeof(id))) {
    uint32_t vv=0;
    if (ctx) {
      if (strcasecmp(id,"ch")==0) vv = ctx->midi_ch;
      else if (strcasecmp(id,"node")==0) vv = ctx->in_node;
      else if (strcasecmp(id,"true")==0) vv = 1;
      else if (strcasecmp(id,"false")==0) vv = 0;
      else { lx->s = save; return 0; }
      *a = vv;
      return 1;
    }
  }
  lx->s = save;
  return 0;
}

static uint8_t eval_comparison(const char* ident, const char* op, uint32_t valA, uint8_t is_range, uint32_t valB, const patch_ctx_t* ctx) {
  uint32_t lhs=0;
  if (!ctx) return 0;
  if (strcasecmp(ident,"ch")==0) lhs = ctx->midi_ch;
  else if (strcasecmp(ident,"node")==0) lhs = ctx->in_node;
  else return 0;

  if (is_range) {
    // treat as "lhs in [A..B]"
    uint32_t lo = valA <= valB ? valA : valB;
    uint32_t hi = valA <= valB ? valB : valA;
    uint8_t inside = (lhs >= lo && lhs <= hi);
    if (strcmp(op,"==")==0) return inside;
    if (strcmp(op,"!=")==0) return !inside;
    // other ops on range not supported => false
    return 0;
  }

  if (strcmp(op,"==")==0) return lhs == valA;
  if (strcmp(op,"!=")==0) return lhs != valA;
  if (strcmp(op,">=")==0) return lhs >= valA;
  if (strcmp(op,"<=")==0) return lhs <= valA;
  if (strcmp(op,">")==0)  return lhs >  valA;
  if (strcmp(op,"<")==0)  return lhs <  valA;
  return 0;
}

static uint8_t parse_primary(cond_lex_t* lx, const patch_ctx_t* ctx);

static uint8_t parse_unary(cond_lex_t* lx, const patch_ctx_t* ctx) {
  if (match_sym(lx, "!") || match_kw(lx, "not")) {
    return (uint8_t)!parse_unary(lx, ctx);
  }
  return parse_primary(lx, ctx);
}

static uint8_t parse_and(cond_lex_t* lx, const patch_ctx_t* ctx) {
  uint8_t v = parse_unary(lx, ctx);
  for (;;) {
    const char* save = lx->s;
    if (match_sym(lx, "&&") || match_kw(lx, "and")) {
      uint8_t rhs = parse_unary(lx, ctx);
      v = (uint8_t)(v && rhs);
      continue;
    }
    lx->s = save;
    break;
  }
  return v;
}

static uint8_t parse_or(cond_lex_t* lx, const patch_ctx_t* ctx) {
  uint8_t v = parse_and(lx, ctx);
  for (;;) {
    const char* save = lx->s;
    if (match_sym(lx, "||") || match_kw(lx, "or")) {
      uint8_t rhs = parse_and(lx, ctx);
      v = (uint8_t)(v || rhs);
      continue;
    }
    lx->s = save;
    break;
  }
  return v;
}

static uint8_t parse_primary(cond_lex_t* lx, const patch_ctx_t* ctx) {
  skip_ws(lx);
  if (match_sym(lx, "(")) {
    uint8_t v = parse_or(lx, ctx);
    (void)match_sym(lx, ")");
    return v;
  }

  // literals
  if (match_kw(lx, "true") || match_kw(lx, "always")) return 1;
  if (match_kw(lx, "false")) return 0;

  // comparison: ident op value
  char ident[16];
  const char* save = lx->s;
  if (!parse_ident(lx, ident, sizeof(ident))) { lx->s = save; return 0; }

  // Accept shorthand "ch=1..4" as "== range"
  char op[3] = {0,0,0};
  skip_ws(lx);
  if (match_sym(lx, "==")) { op[0]='='; op[1]='='; }
  else if (match_sym(lx, "!=")) { op[0]='!'; op[1]='='; }
  else if (match_sym(lx, ">=")) { op[0]='>'; op[1]='='; }
  else if (match_sym(lx, "<=")) { op[0]='<'; op[1]='='; }
  else if (match_sym(lx, ">"))  { op[0]='>'; }
  else if (match_sym(lx, "<"))  { op[0]='<'; }
  else if (match_sym(lx, "="))  { op[0]='='; op[1]='='; } // '=' treated as '=='
  else { lx->s = save; return 0; }

  uint32_t a=0,b=0; uint8_t is_range=0;
  if (!parse_value(lx, &a, &is_range, &b, ctx)) { lx->s = save; return 0; }

  return eval_comparison(ident, op, a, is_range, b, ctx);
}

uint8_t patch_adv_cond_eval(const char* cond, const patch_ctx_t* ctx) {
  if (!cond || !cond[0]) return 1;
  cond_lex_t lx = { .s = cond };
  uint8_t v = parse_or(&lx, ctx);
  // If unparsed garbage remains, treat as false (safe)
  skip_ws(&lx);
  if (*lx.s != 0) return 0;
  return v;
}
