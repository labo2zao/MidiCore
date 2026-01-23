#include "Services/ui/ui_gfx.h"
#include <string.h>
#include <stdlib.h>  // For abs()

static uint8_t* g_fb = 0;
static uint16_t g_w = 0, g_h = 0;

static const uint8_t font5x7[96][5] = {
  {0,0,0,0,0},{0,0,0x5F,0,0},{0,0x07,0,0x07,0},{0x14,0x7F,0x14,0x7F,0x14},
  {0x24,0x2A,0x7F,0x2A,0x12},{0x23,0x13,0x08,0x64,0x62},{0x36,0x49,0x55,0x22,0x50},{0,0x05,0x03,0,0},
  {0,0x1C,0x22,0x41,0},{0,0x41,0x22,0x1C,0},{0x14,0x08,0x3E,0x08,0x14},{0x08,0x08,0x3E,0x08,0x08},
  {0,0x50,0x30,0,0},{0x08,0x08,0x08,0x08,0x08},{0,0x60,0x60,0,0},{0x20,0x10,0x08,0x04,0x02},
  {0x3E,0x51,0x49,0x45,0x3E},{0,0x42,0x7F,0x40,0},{0x42,0x61,0x51,0x49,0x46},{0x21,0x41,0x45,0x4B,0x31},
  {0x18,0x14,0x12,0x7F,0x10},{0x27,0x45,0x45,0x45,0x39},{0x3C,0x4A,0x49,0x49,0x30},{0x01,0x71,0x09,0x05,0x03},
  {0x36,0x49,0x49,0x49,0x36},{0x06,0x49,0x49,0x29,0x1E},{0,0x36,0x36,0,0},{0,0x56,0x36,0,0},
  {0x08,0x14,0x22,0x41,0},{0x14,0x14,0x14,0x14,0x14},{0,0x41,0x22,0x14,0x08},{0x02,0x01,0x51,0x09,0x06},
  {0x32,0x49,0x79,0x41,0x3E},{0x7E,0x11,0x11,0x11,0x7E},{0x7F,0x49,0x49,0x49,0x36},{0x3E,0x41,0x41,0x41,0x22},
  {0x7F,0x41,0x41,0x22,0x1C},{0x7F,0x49,0x49,0x49,0x41},{0x7F,0x09,0x09,0x09,0x01},{0x3E,0x41,0x49,0x49,0x7A},
  {0x7F,0x08,0x08,0x08,0x7F},{0,0x41,0x7F,0x41,0},{0x20,0x40,0x41,0x3F,0x01},{0x7F,0x08,0x14,0x22,0x41},
  {0x7F,0x40,0x40,0x40,0x40},{0x7F,0x02,0x0C,0x02,0x7F},{0x7F,0x04,0x08,0x10,0x7F},{0x3E,0x41,0x41,0x41,0x3E},
  {0x7F,0x09,0x09,0x09,0x06},{0x3E,0x41,0x51,0x21,0x5E},{0x7F,0x09,0x19,0x29,0x46},{0x46,0x49,0x49,0x49,0x31},
  {0x01,0x01,0x7F,0x01,0x01},{0x3F,0x40,0x40,0x40,0x3F},{0x1F,0x20,0x40,0x20,0x1F},{0x7F,0x20,0x18,0x20,0x7F},
  {0x63,0x14,0x08,0x14,0x63},{0x03,0x04,0x78,0x04,0x03},{0x61,0x51,0x49,0x45,0x43},{0,0x7F,0x41,0x41,0},
  {0x02,0x04,0x08,0x10,0x20},{0,0x41,0x41,0x7F,0},{0x04,0x02,0x01,0x02,0x04},{0x40,0x40,0x40,0x40,0x40},
  {0,0x01,0x02,0x04,0},{0x20,0x54,0x54,0x54,0x78},{0x7F,0x48,0x44,0x44,0x38},{0x38,0x44,0x44,0x44,0x20},
  {0x38,0x44,0x44,0x48,0x7F},{0x38,0x54,0x54,0x54,0x18},{0x08,0x7E,0x09,0x01,0x02},{0x0C,0x52,0x52,0x52,0x3E},
  {0x7F,0x08,0x04,0x04,0x78},{0,0x44,0x7D,0x40,0},{0x20,0x40,0x44,0x3D,0},{0x7F,0x10,0x28,0x44,0},
  {0,0x41,0x7F,0x40,0},{0x7C,0x04,0x18,0x04,0x78},{0x7C,0x08,0x04,0x04,0x78},{0x38,0x44,0x44,0x44,0x38},
  {0x7C,0x14,0x14,0x14,0x08},{0x08,0x14,0x14,0x18,0x7C},{0x7C,0x08,0x04,0x04,0x08},{0x48,0x54,0x54,0x54,0x20},
  {0x04,0x3F,0x44,0x40,0x20},{0x3C,0x40,0x40,0x20,0x7C},{0x1C,0x20,0x40,0x20,0x1C},{0x3C,0x40,0x30,0x40,0x3C},
  {0x44,0x28,0x10,0x28,0x44},{0x0C,0x50,0x50,0x50,0x3C},{0x44,0x64,0x54,0x4C,0x44},{0,0x08,0x36,0x41,0},
  {0,0,0x7F,0,0},{0,0x41,0x36,0x08,0},{0x08,0x04,0x08,0x10,0x08},
};

void ui_gfx_set_fb(uint8_t* fb, uint16_t w, uint16_t h) { g_fb = fb; g_w = w; g_h = h; }

void ui_gfx_clear(uint8_t gray) {
  if (!g_fb) return;
  uint8_t v = (uint8_t)(((gray & 0x0F) << 4) | (gray & 0x0F));
  memset(g_fb, v, (g_w * g_h) / 2);
}

void ui_gfx_pixel(int x, int y, uint8_t gray) {
  if (!g_fb) return;
  if (x < 0 || y < 0 || x >= (int)g_w || y >= (int)g_h) return;
  uint32_t idx = (uint32_t)y * (uint32_t)g_w + (uint32_t)x;
  uint32_t b = idx >> 1;
  if ((idx & 1u) == 0) g_fb[b] = (uint8_t)((g_fb[b] & 0x0F) | ((gray & 0x0F) << 4));
  else                g_fb[b] = (uint8_t)((g_fb[b] & 0xF0) |  (gray & 0x0F));
}

void ui_gfx_rect(int x, int y, int w, int h, uint8_t gray) {
  for (int yy=0; yy<h; yy++) for (int xx=0; xx<w; xx++) ui_gfx_pixel(x+xx, y+yy, gray);
}

static void draw_char(int x, int y, char c, uint8_t gray) {
  if (c < 32 || c > 127) c = '?';
  const uint8_t* col = font5x7[(int)c - 32];
  for (int cx=0; cx<5; cx++) {
    uint8_t bits = col[cx];
    for (int cy=0; cy<7; cy++) if (bits & (1u<<cy)) ui_gfx_pixel(x+cx, y+cy, gray);
  }
}

void ui_gfx_text(int x, int y, const char* s, uint8_t gray) {
  if (!s) return;
  int cx = x;
  while (*s) {
    if (*s == '\n') { cx = x; y += 8; s++; continue; }
    draw_char(cx, y, *s, gray);
    cx += 6;
    s++;
  }
}

void ui_gfx_fill_rect(int x, int y, int w, int h, uint8_t gray) {
  ui_gfx_rect(x, y, w, h, gray);
}

void ui_gfx_hline(int x, int y, int w, uint8_t gray) {
  for (int xx = 0; xx < w; xx++) {
    ui_gfx_pixel(x + xx, y, gray);
  }
}

void ui_gfx_vline(int x, int y, int h, uint8_t gray) {
  for (int yy = 0; yy < h; yy++) {
    ui_gfx_pixel(x, y + yy, gray);
  }
}

// Bresenham's line algorithm
void ui_gfx_line(int x0, int y0, int x1, int y1, uint8_t gray) {
  int dx = x1 - x0;
  int dy = y1 - y0;
  
  // Handle negative deltas - determine step direction and get absolute values
  int sx = (dx > 0) ? 1 : -1;
  int sy = (dy > 0) ? 1 : -1;
  dx = abs(dx);
  dy = abs(dy);
  
  int err = dx - dy;
  
  while (1) {
    ui_gfx_pixel(x0, y0, gray);
    
    if (x0 == x1 && y0 == y1) break;
    
    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (e2 < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

// Midpoint circle algorithm
void ui_gfx_circle(int cx, int cy, int radius, uint8_t gray) {
  int x = radius;
  int y = 0;
  int err = 0;
  
  while (x >= y) {
    // Draw 8 octants
    ui_gfx_pixel(cx + x, cy + y, gray);
    ui_gfx_pixel(cx + y, cy + x, gray);
    ui_gfx_pixel(cx - y, cy + x, gray);
    ui_gfx_pixel(cx - x, cy + y, gray);
    ui_gfx_pixel(cx - x, cy - y, gray);
    ui_gfx_pixel(cx - y, cy - x, gray);
    ui_gfx_pixel(cx + y, cy - x, gray);
    ui_gfx_pixel(cx + x, cy - y, gray);
    
    if (err <= 0) {
      y++;
      err += 2 * y + 1;
    }
    
    if (err > 0) {
      x--;
      err -= 2 * x + 1;
    }
  }
}

// Filled circle using horizontal lines
void ui_gfx_filled_circle(int cx, int cy, int radius, uint8_t gray) {
  int x = radius;
  int y = 0;
  int err = 0;
  
  while (x >= y) {
    // Draw horizontal lines for all 8 octants
    ui_gfx_hline(cx - x, cy + y, 2 * x + 1, gray);
    ui_gfx_hline(cx - y, cy + x, 2 * y + 1, gray);
    ui_gfx_hline(cx - y, cy - x, 2 * y + 1, gray);
    ui_gfx_hline(cx - x, cy - y, 2 * x + 1, gray);
    
    if (err <= 0) {
      y++;
      err += 2 * y + 1;
    }
    
    if (err > 0) {
      x--;
      err -= 2 * x + 1;
    }
  }
}

// Triangle using three lines
void ui_gfx_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint8_t gray) {
  ui_gfx_line(x0, y0, x1, y1, gray);
  ui_gfx_line(x1, y1, x2, y2, gray);
  ui_gfx_line(x2, y2, x0, y0, gray);
}

// Filled triangle using scanline algorithm
void ui_gfx_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint8_t gray) {
  // Sort vertices by y-coordinate (y0 <= y1 <= y2)
  if (y0 > y1) { int tx = x0; x0 = x1; x1 = tx; int ty = y0; y0 = y1; y1 = ty; }
  if (y0 > y2) { int tx = x0; x0 = x2; x2 = tx; int ty = y0; y0 = y2; y2 = ty; }
  if (y1 > y2) { int tx = x1; x1 = x2; x2 = tx; int ty = y1; y1 = y2; y2 = ty; }
  
  // Special case: all vertices on same line
  if (y0 == y2) {
    int minx = (x0 < x1) ? x0 : x1;
    minx = (minx < x2) ? minx : x2;
    int maxx = (x0 > x1) ? x0 : x1;
    maxx = (maxx > x2) ? maxx : x2;
    ui_gfx_hline(minx, y0, maxx - minx + 1, gray);
    return;
  }
  
  // Rasterize
  for (int y = y0; y <= y2; y++) {
    int xa, xb;
    
    if (y < y1) {
      // Upper part
      if (y1 - y0 != 0) xa = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
      else xa = x0;
    } else {
      // Lower part
      if (y2 - y1 != 0) xa = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
      else xa = x1;
    }
    
    // Long edge
    if (y2 - y0 != 0) xb = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
    else xb = x0;
    
    if (xa > xb) { int tmp = xa; xa = xb; xb = tmp; }
    ui_gfx_hline(xa, y, xb - xa + 1, gray);
  }
}

// Arc using parametric circle equation with integer approximation
void ui_gfx_arc(int cx, int cy, int radius, int start_angle, int end_angle, uint8_t gray) {
  // Normalize angles to 0-360 range
  while (start_angle < 0) start_angle += 360;
  while (end_angle < 0) end_angle += 360;
  start_angle %= 360;
  end_angle %= 360;
  
  // Draw arc using small angle steps (every 5 degrees for smoothness)
  int step = 5;
  for (int angle = start_angle; ; angle += step) {
    if (angle > 360) angle -= 360;
    
    // Simple integer-based approximation without trig
    // Use 16-point lookup table
    int idx = (angle * 16) / 360;
    int x, y;
    
    // 16-point circle lookup (approximation)
    switch(idx % 16) {
      case 0:  x = radius; y = 0; break;
      case 1:  x = (radius * 15) / 16; y = (radius * 4) / 16; break;
      case 2:  x = (radius * 14) / 16; y = (radius * 7) / 16; break;
      case 3:  x = (radius * 11) / 16; y = (radius * 11) / 16; break;
      case 4:  x = (radius * 7) / 16; y = (radius * 14) / 16; break;
      case 5:  x = (radius * 4) / 16; y = (radius * 15) / 16; break;
      case 6:  x = 0; y = radius; break;
      case 7:  x = -(radius * 4) / 16; y = (radius * 15) / 16; break;
      case 8:  x = -(radius * 7) / 16; y = (radius * 14) / 16; break;
      case 9:  x = -(radius * 11) / 16; y = (radius * 11) / 16; break;
      case 10: x = -(radius * 14) / 16; y = (radius * 7) / 16; break;
      case 11: x = -(radius * 15) / 16; y = (radius * 4) / 16; break;
      case 12: x = -radius; y = 0; break;
      case 13: x = -(radius * 15) / 16; y = -(radius * 4) / 16; break;
      case 14: x = -(radius * 14) / 16; y = -(radius * 7) / 16; break;
      case 15: x = -(radius * 11) / 16; y = -(radius * 11) / 16; break;
      default: x = radius; y = 0; break;
    }
    
    ui_gfx_pixel(cx + x, cy + y, gray);
    
    // Check if we've completed the arc
    if (start_angle < end_angle) {
      if (angle >= end_angle) break;
    } else {
      if (angle >= end_angle && angle > start_angle) break;
    }
    
    // Prevent infinite loop
    if (angle == start_angle + 360) break;
  }
}
