#include "Services/safe/safe_mode.h"

static uint8_t g_forced = 0;
static uint8_t g_cfg = 0;
static uint8_t g_sd_ok = 1;

void safe_mode_set_forced(uint8_t forced) { g_forced = forced ? 1u : 0u; }
void safe_mode_set_cfg(uint8_t enabled) { g_cfg = enabled ? 1u : 0u; }
void safe_mode_set_sd_ok(uint8_t sd_ok) { g_sd_ok = sd_ok ? 1u : 0u; }

uint8_t safe_mode_is_enabled(void) {
  return (g_forced || g_cfg || !g_sd_ok) ? 1u : 0u;
}

safe_reason_t safe_mode_reason(void) {
  if (g_forced) return SAFE_REASON_FORCED_SHIFT;
  if (!g_sd_ok) return SAFE_REASON_NO_SD;
  if (g_cfg) return SAFE_REASON_CFG;
  return SAFE_REASON_NONE;
}

const char* safe_mode_reason_str(void) {
  switch (safe_mode_reason()) {
    case SAFE_REASON_FORCED_SHIFT: return "SHIFT";
    case SAFE_REASON_NO_SD: return "NO_SD";
    case SAFE_REASON_CFG: return "CFG";
    default: return "";
  }
}
