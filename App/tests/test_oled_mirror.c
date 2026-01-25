/**
 * @file test_oled_mirror.c
 * @brief Debug output mirroring to OLED implementation
 */

#include "test_oled_mirror.h"
#include "Services/ui/ui_gfx.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

// OLED mirror state
static uint8_t mirror_enabled = 0;
static char mirror_lines[OLED_MIRROR_LINES][OLED_MIRROR_LINE_LEN + 1];
static uint8_t mirror_line_count = 0;
static uint8_t mirror_current_col = 0;

// External framebuffer (assuming 256x64 OLED)
extern uint8_t oled_fb[256 * 64 / 2];  // 4-bit grayscale

void oled_mirror_init(void) {
  mirror_enabled = 0;
  mirror_line_count = 0;
  mirror_current_col = 0;
  
  // Clear all lines
  for (uint8_t i = 0; i < OLED_MIRROR_LINES; i++) {
    memset(mirror_lines[i], 0, OLED_MIRROR_LINE_LEN + 1);
  }
}

void oled_mirror_set_enabled(uint8_t enabled) {
  mirror_enabled = enabled ? 1 : 0;
  if (mirror_enabled) {
    oled_mirror_clear();
  }
}

uint8_t oled_mirror_is_enabled(void) {
  return mirror_enabled;
}

void oled_mirror_clear(void) {
  mirror_line_count = 0;
  mirror_current_col = 0;
  for (uint8_t i = 0; i < OLED_MIRROR_LINES; i++) {
    memset(mirror_lines[i], 0, OLED_MIRROR_LINE_LEN + 1);
  }
}

static void mirror_add_char(char c) {
  if (!mirror_enabled) return;
  
  if (c == '\n' || c == '\r') {
    // Move to next line
    if (mirror_line_count < OLED_MIRROR_LINES) {
      mirror_line_count++;
    } else {
      // Scroll up - shift all lines
      for (uint8_t i = 0; i < OLED_MIRROR_LINES - 1; i++) {
        memcpy(mirror_lines[i], mirror_lines[i + 1], OLED_MIRROR_LINE_LEN + 1);
      }
      memset(mirror_lines[OLED_MIRROR_LINES - 1], 0, OLED_MIRROR_LINE_LEN + 1);
    }
    mirror_current_col = 0;
    return;
  }
  
  // Add character to current line
  uint8_t line_idx = (mirror_line_count > 0) ? mirror_line_count - 1 : 0;
  if (line_idx >= OLED_MIRROR_LINES) line_idx = OLED_MIRROR_LINES - 1;
  
  if (mirror_current_col < OLED_MIRROR_LINE_LEN) {
    mirror_lines[line_idx][mirror_current_col++] = c;
  }
  
  // Initialize line count if this is first char
  if (mirror_line_count == 0) {
    mirror_line_count = 1;
  }
}

void oled_mirror_print(const char* str) {
  if (!mirror_enabled || !str) return;
  
  while (*str) {
    mirror_add_char(*str++);
  }
}

void oled_mirror_printf(const char* format, ...) {
  if (!mirror_enabled) return;
  
  char buffer[128];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  
  oled_mirror_print(buffer);
}

void oled_mirror_update(void) {
  if (!mirror_enabled) return;
  
#if MODULE_ENABLE_UI && MODULE_ENABLE_OLED
  // Set framebuffer
  ui_gfx_set_fb(oled_fb, 256, 64);
  
  // Clear screen
  ui_gfx_clear(0);
  
  // Draw title bar
  ui_gfx_fill_rect(0, 0, 256, 10, 15);
  ui_gfx_text(2, 1, "TEST DEBUG MIRROR", 0);
  
  // Draw lines of text (8x8 font, ~32 chars per line)
  int y = 12;
  for (uint8_t i = 0; i < mirror_line_count && i < OLED_MIRROR_LINES; i++) {
    if (strlen(mirror_lines[i]) > 0) {
      ui_gfx_text(2, y, mirror_lines[i], 15);
    }
    y += 8;
  }
  
  // Draw status line at bottom
  char status[32];
  snprintf(status, sizeof(status), "Lines: %u/%u", 
           mirror_line_count, OLED_MIRROR_LINES);
  ui_gfx_text(2, 56, status, 10);
#endif
}

uint8_t oled_mirror_get_line_count(void) {
  return mirror_line_count;
}
