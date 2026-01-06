#include "Services/patch/patch_bank.h"
#include <string.h>
#include <ctype.h>

#if __has_include("ff.h")
  #include "ff.h"
  #define BANK_HAS_FATFS 1
#else
  #define BANK_HAS_FATFS 0
#endif

static void trim(char* s) {
  while (*s && isspace((unsigned char)*s)) memmove(s, s+1, strlen(s));
  size_t n = strlen(s);
  while (n && isspace((unsigned char)s[n-1])) s[--n] = 0;
}

static int startswith_i(const char* s, const char* pfx) {
  while (*pfx) {
    char a = (char)toupper((unsigned char)*s++);
    char b = (char)toupper((unsigned char)*pfx++);
    if (a != b) return 0;
  }
  return 1;
}

int patch_bank_load(patch_bank_t* bank, const char* path) {
  if (!bank || !path) return -1;
  memset(bank, 0, sizeof(*bank));

#if !BANK_HAS_FATFS
  (void)path;
  return -10;
#else
  FIL fp;
  if (f_open(&fp, path, FA_READ) != FR_OK) return -2;

  char line[160];
  enum { SEC_NONE=0, SEC_BANK, SEC_PATCH } sec = SEC_NONE;
  uint16_t patch_idx = (uint16_t)-1;

  while (f_gets(line, sizeof(line), &fp)) {
    for (size_t i=0; i<sizeof(line) && line[i]; i++) {
      if (line[i]=='\r' || line[i]=='\n') { line[i]=0; break; }
    }
    trim(line);
    if (!line[0] || line[0]=='#') continue;

    if (line[0]=='[') {
      if (startswith_i(line, "[BANK]")) { sec = SEC_BANK; continue; }
      if (startswith_i(line, "[PATCH]")) {
        sec = SEC_PATCH;
        if ((patch_idx+1u) < PATCH_BANK_MAX_PATCHES) patch_idx++;
        continue;
      }
      sec = SEC_NONE;
      continue;
    }

    char* eq = strchr(line, '=');
    if (!eq) continue;
    *eq = 0;
    char* k = line;
    char* v = eq+1;
    trim(k); trim(v);

    if (sec == SEC_BANK) {
      if (startswith_i(k,"NAME")) strncpy(bank->bank_name, v, sizeof(bank->bank_name)-1);
      else if (startswith_i(k,"ID")) strncpy(bank->bank_id, v, sizeof(bank->bank_id)-1);
      else if (startswith_i(k,"CHORD_BANK")) strncpy(bank->chord_bank_path, v, sizeof(bank->chord_bank_path)-1);
    } else if (sec == SEC_PATCH) {
      if (patch_idx >= PATCH_BANK_MAX_PATCHES) continue;
      if (startswith_i(k,"FILE")) strncpy(bank->patches[patch_idx].file, v, sizeof(bank->patches[patch_idx].file)-1);
      else if (startswith_i(k,"LABEL")) strncpy(bank->patches[patch_idx].label, v, sizeof(bank->patches[patch_idx].label)-1);
    }
  }

  // Count valid patches
  uint16_t cnt=0;
  for (uint16_t i=0;i<PATCH_BANK_MAX_PATCHES;i++) {
    if (bank->patches[i].file[0]) cnt++;
    else break;
  }
  bank->patch_count = cnt;

  f_close(&fp);
  return 0;
#endif
}
