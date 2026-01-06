#include "Services/patch/patch_manager.h"
#include "Services/ui/ui.h"
#include "Services/dream/dream_sysex.h"
#include "Services/safe/safe_mode.h"
#include "Services/patch/patch.h"
#include "Services/patch/patch_router.h"
#include "Services/patch/patch_sd_mount.h"

#include <string.h>

static const char* k_state_path = "0:/patch/state.ngs";
static const char* k_router_default = "0:/cfg/router_default.ngc";

void patch_manager_init(patch_manager_t* pm) {
  memset(pm, 0, sizeof(*pm));
  patch_state_set_defaults(&pm->state);
}

int patch_manager_boot(patch_manager_t* pm) {
  if (!pm) return -1;

  (void)patch_sd_mount_init();
  (void)patch_state_load(&pm->state, k_state_path);

  int br = patch_bank_load(&pm->bank, pm->state.bank_path);
  if (br < 0) return br;

  if (pm->bank.patch_count == 0) return -20;
  if (pm->state.patch_index >= pm->bank.patch_count) pm->state.patch_index = 0;

  strncpy(pm->current_patch_path, pm->bank.patches[pm->state.patch_index].file, sizeof(pm->current_patch_path)-1);
  return 0;
}

int patch_manager_select_patch(patch_manager_t* pm, uint16_t patch_index) {
  if (!pm) return -1;
  if (pm->bank.patch_count == 0) return -20;
  if (patch_index >= pm->bank.patch_count) return -2;
  pm->state.patch_index = patch_index;
  strncpy(pm->current_patch_path, pm->bank.patches[patch_index].file, sizeof(pm->current_patch_path)-1);
  return 0;
}

#if __has_include("ff.h")
#include "ff.h"
#define PM_HAS_FATFS 1
#else
#define PM_HAS_FATFS 0
#endif

static void pm_trim(char* s) {
  while (*s && (*s==' '||*s=='\t'||*s=='\r'||*s=='\n')) memmove(s, s+1, strlen(s));
  size_t n=strlen(s);
  while (n && (s[n-1]==' '||s[n-1]=='\t'||s[n-1]=='\r'||s[n-1]=='\n')) s[--n]=0;
}
static void pm_upcase(char* s){ for(;*s;s++) if(*s>='a'&&*s<='z') *s=(char)(*s-32); }

static int find_patch_chord_bank(const char* patch_path, char* out, size_t out_sz) {
  if (!patch_path || !out || out_sz < 4) return -1;
  out[0]=0;
#if !PM_HAS_FATFS
  (void)patch_path;
  return -10;
#else
  FIL fp;
  if (f_open(&fp, patch_path, FA_READ) != FR_OK) return -2;
  char section[16]={0};
  char line[160];
  while (f_gets(line, sizeof(line), &fp)) {
    for (size_t i=0;i<sizeof(line) && line[i]; i++) {
      if (line[i]=='\r' || line[i]=='\n') { line[i]=0; break; }
    }
    pm_trim(line);
    if (!line[0] || line[0]=='#') continue;

    if (line[0]=='[') {
      char* end=strchr(line,']');
      if (!end) continue;
      *end=0;
      strncpy(section, line+1, sizeof(section)-1);
      section[sizeof(section)-1]=0;
      pm_trim(section); pm_upcase(section);
      continue;
    }

    char* eq=strchr(line,'=');
    if (!eq) continue;
    *eq=0;
    char* k=line; char* v=eq+1;
    pm_trim(k); pm_trim(v);
    if (!k[0]) continue;
    pm_upcase(k);

    if (section[0] && strcmp(section,"PATCH")!=0) continue;
    if (!strcmp(k,"CHORD_BANK")) {
      strncpy(out, v, out_sz-1);
      out[out_sz-1]=0;
      break;
    }
  }
  f_close(&fp);
  return out[0] ? 1 : 0;
#endif
}

int patch_manager_apply(patch_manager_t* pm) {
  if (!pm) return -1;

  // Apply router defaults first
  (void)patch_load(k_router_default);
  patch_ctx_t pctx = { .midi_ch = 1, .in_node = 0 };
  (void)patch_router_apply(&pctx);

  // Load patch file
  int pr = patch_load(pm->current_patch_path);
  if (pr < 0) return pr;

  // Chord bank selection priority:
  // 1) per-patch override: CHORD_BANK=...
  // 2) per-bank default:   pm->bank.chord_bank_path
  // 3) global default:     /cfg/chord_bank.ngc
  char chord_path[96] = {0};
  int fr = find_patch_chord_bank(pm->current_patch_path, chord_path, sizeof(chord_path));
  if (fr > 0) {
    (void)ui_reload_chord_bank(chord_path);
  } else if (pm->bank.chord_bank_path[0]) {
    (void)ui_reload_chord_bank(pm->bank.chord_bank_path);
  } else {
    (void)ui_reload_chord_bank(0);
  }

  // DREAM init (optional)
  if (!safe_mode_is_enabled()) {
    (void)dream_apply_from_patch(pm->current_patch_path);
  }

  // Apply routing and save state
  (void)patch_router_apply(&pctx);
  if (!safe_mode_is_enabled()) {
    (void)patch_state_save(&pm->state, k_state_path);
  }
  return 0;
}
