#include "Services/fs/sd_guard.h"

static uint8_t g_err = 0;
static uint8_t g_ro = 0;

void sd_guard_reset(void) { g_err = 0; g_ro = 0; }
void sd_guard_note_write_error(void) {
  if (g_ro) return;
  if (g_err < 255) g_err++;
  if (g_err >= 3) g_ro = 1; // after 3 write errors -> readonly
}
uint8_t sd_guard_is_readonly(void) { return g_ro; }
uint8_t sd_guard_error_count(void) { return g_err; }
