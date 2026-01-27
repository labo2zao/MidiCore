#include "Services/log/log.h"
#include "Services/safe/safe_mode.h"
#include "Services/fs/sd_guard.h"

#if __has_include("ff.h")
  #include "ff.h"
  #define LOG_HAS_FATFS 1
#else
  #define LOG_HAS_FATFS 0
#endif

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifndef LOG_BUFFER_LINES
#define LOG_BUFFER_LINES 24  // Reduced from 32 to save RAM (768 bytes saved)
#endif
#ifndef LOG_LINE_MAX
#define LOG_LINE_MAX 96
#endif

static char g_lines[LOG_BUFFER_LINES][LOG_LINE_MAX];
static uint8_t g_head = 0;
static uint8_t g_count = 0;

void log_init(void) {
  g_head = 0;
  g_count = 0;
  memset(g_lines, 0, sizeof(g_lines));
}

static void push_line(const char* s) {
  if (!s) return;
  uint8_t idx = (uint8_t)((g_head + g_count) % LOG_BUFFER_LINES);
  strncpy(g_lines[idx], s, LOG_LINE_MAX-1);
  g_lines[idx][LOG_LINE_MAX-1] = 0;
  if (g_count < LOG_BUFFER_LINES) g_count++;
  else g_head = (uint8_t)((g_head + 1) % LOG_BUFFER_LINES); // overwrite oldest
}

void log_printf(const char* tag, const char* fmt, ...) {
  char line[LOG_LINE_MAX];
  int n = 0;
  if (tag && tag[0]) n = snprintf(line, sizeof(line), "[%s] ", tag);
  if (n < 0) n = 0;
  if ((size_t)n >= sizeof(line)) n = (int)sizeof(line)-1;

  va_list ap;
  va_start(ap, fmt);
  vsnprintf(line + n, sizeof(line) - (size_t)n, fmt ? fmt : "", ap);
  va_end(ap);

  // ensure newline for file
  size_t L = strnlen(line, sizeof(line));
  if (L < sizeof(line)-2) { line[L++] = '\n'; line[L]=0; }
  push_line(line);
}

void log_flush(void) {
#if !LOG_HAS_FATFS
  return;
#else
  if (g_count == 0) return;
  if (safe_mode_is_enabled()) return;
  if (sd_guard_is_readonly()) return;

  FIL fp;
  if (f_open(&fp, "0:/log.txt", FA_OPEN_APPEND | FA_WRITE) != FR_OK) return;

  while (g_count) {
    const char* s = g_lines[g_head];
    UINT bw=0;
    (void)f_write(&fp, s, (UINT)strnlen(s, LOG_LINE_MAX), &bw);
    g_head = (uint8_t)((g_head + 1) % LOG_BUFFER_LINES);
    g_count--;
  }
  (void)f_sync(&fp);
  f_close(&fp);
#endif
}
