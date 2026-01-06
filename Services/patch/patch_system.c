#include "Services/patch/patch_system.h"
#include "Services/ui/ui.h"
#include "Services/safe/safe_mode.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

static patch_manager_t g_pm;

static int extract_bank_number(const char* path) {
  if (!path) return -1;
  const char* p = strrchr(path, '/');
  p = p ? (p+1) : path;
  int n = -1;
  for (const char* s=p; *s; s++) {
    if (isdigit((unsigned char)*s)) { n = atoi(s); break; }
  }
  return n;
}

static void set_bank_number(char* out, size_t out_sz, int n) {
  snprintf(out, out_sz, "0:/patch/banks/bank_%02d.ngb", n);
}

void patch_system_init(void) {
  patch_manager_init(&g_pm);
  int br = patch_manager_boot(&g_pm);
  if (br < 0) {
    strcpy(g_pm.bank.bank_name, "NO_SD");
    strcpy(g_pm.bank.bank_id, "SD?");
    g_pm.bank.patch_count = 1;
    strcpy(g_pm.bank.patches[0].label, "Init");
    g_pm.state.patch_index = 0;
    g_pm.current_patch_path[0] = 0;
    safe_mode_set_sd_ok(0);
    return;
  }
  (void)patch_manager_apply(&g_pm);
}
int patch_system_apply(void) {
  (void)ui_reload_chord_bank(g_pm.bank.chord_bank_path);
  return patch_manager_apply(&g_pm);
}

int patch_system_patch_next(void) {
  if (g_pm.bank.patch_count == 0) return -20;
  uint16_t idx = g_pm.state.patch_index;
  idx = (uint16_t)((idx + 1u) % g_pm.bank.patch_count);
  patch_manager_select_patch(&g_pm, idx);
  return 0;
}

int patch_system_patch_prev(void) {
  if (g_pm.bank.patch_count == 0) return -20;
  uint16_t idx = g_pm.state.patch_index;
  idx = (uint16_t)((idx == 0u) ? (g_pm.bank.patch_count - 1u) : (idx - 1u));
  patch_manager_select_patch(&g_pm, idx);
  return 0;
}

static int bank_step(int delta) {
  int cur = extract_bank_number(g_pm.state.bank_path);
  if (cur < 0) cur = 1;
  int next = cur + delta;
  if (next < 1) next = 1;

  patch_state_t st = g_pm.state;
  set_bank_number(st.bank_path, sizeof(st.bank_path), next);

  patch_bank_t bk;
  int br = patch_bank_load(&bk, st.bank_path);
  if (br < 0) return br;

  g_pm.state = st;
  g_pm.bank = bk;
  (void)ui_reload_chord_bank(g_pm.bank.chord_bank_path);
  if (g_pm.bank.patch_count == 0) g_pm.state.patch_index = 0;
  else if (g_pm.state.patch_index >= g_pm.bank.patch_count) g_pm.state.patch_index = 0;

  strncpy(g_pm.current_patch_path,
          g_pm.bank.patches[g_pm.state.patch_index].file,
          sizeof(g_pm.current_patch_path)-1);
  return 0;
}

int patch_system_bank_next(void) { return bank_step(+1); }
int patch_system_bank_prev(void) { return bank_step(-1); }

const patch_manager_t* patch_system_get(void) { return &g_pm; }
