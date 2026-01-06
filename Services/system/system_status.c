#include "Services/system/system_status.h"

static uint8_t g_sd_required = 0;
static uint8_t g_sd_ok = 1;

void system_set_sd_required(uint8_t required) { g_sd_required = required ? 1u : 0u; }
void system_set_sd_ok(uint8_t ok) { g_sd_ok = ok ? 1u : 0u; }
uint8_t system_is_sd_required(void) { return g_sd_required; }
uint8_t system_is_sd_ok(void) { return g_sd_ok; }

uint8_t system_is_fatal(void) {
  return (g_sd_required && !g_sd_ok) ? 1u : 0u;
}
