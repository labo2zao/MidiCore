#include "Services/ui/ui_page_oled_test.h"
#include "Services/ui/ui_gfx.h"
#include "Hal/oled_ssd1322/oled_ssd1322.h"
#include <stdio.h>
#include <string.h>

static uint8_t test_mode = 0; // 0=patterns, 1=grayscale, 2=pixels, 3=text, 4=animations
static uint32_t last_update = 0;
static uint8_t anim_frame = 0;

void ui_page_oled_test_render(uint32_t ms) {
  uint8_t* fb = oled_framebuffer();
  
  // Clear screen
  ui_gfx_rect(0, 12, OLED_W, OLED_H - 12, 0);
  
  char info[64];
  snprintf(info, sizeof(info), "Test Mode %d - Use ENC", test_mode);
  ui_gfx_text(0, 14, info, 15);
  
  switch (test_mode) {
    case 0: { // Test patterns
      ui_gfx_text(0, 26, "Pattern Test", 15);
      
      // Horizontal stripes
      for (int y = 40; y < 48; y++) {
        ui_gfx_hline(0, y, OLED_W, 15);
      }
      
      // Vertical stripes
      for (int x = 0; x < OLED_W; x += 4) {
        ui_gfx_vline(x, 50, 8, 15);
      }
      
      // Checkerboard
      for (int y = 0; y < 6; y++) {
        for (int x = 0; x < 32; x++) {
          if ((x + y) % 2 == 0) {
            ui_gfx_pixel(x * 8, 60 + y, 15);
            ui_gfx_pixel(x * 8 + 1, 60 + y, 15);
          }
        }
      }
      break;
    }
    
    case 1: { // Grayscale test
      ui_gfx_text(0, 26, "Grayscale Levels", 15);
      
      // 16 levels of gray
      for (int i = 0; i < 16; i++) {
        ui_gfx_rect(i * 16, 40, 16, 20, i);
        
        char level[4];
        snprintf(level, sizeof(level), "%X", i);
        ui_gfx_text(i * 16 + 6, 62, level, 15 - i);
      }
      break;
    }
    
    case 2: { // Pixel test
      ui_gfx_text(0, 26, "Pixel Test", 15);
      
      // Draw individual pixels in a grid
      for (int y = 40; y < 64; y += 2) {
        for (int x = 0; x < OLED_W; x += 2) {
          ui_gfx_pixel(x, y, ((x + y) / 2) % 16);
        }
      }
      break;
    }
    
    case 3: { // Text test
      ui_gfx_text(0, 26, "Text Rendering Test", 15);
      ui_gfx_text(0, 38, "0123456789", 12);
      ui_gfx_text(0, 50, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", 10);
      ui_gfx_text(0, 62, "abcdefghijklmnopqrstuvwxyz", 8);
      break;
    }
    
    case 4: { // Animation test
      ui_gfx_text(0, 26, "Animation Test", 15);
      
      if (ms - last_update > 100) {
        anim_frame++;
        last_update = ms;
      }
      
      // Moving bar
      int bar_x = (anim_frame * 4) % OLED_W;
      ui_gfx_rect(bar_x, 40, 20, 10, 15);
      
      // Pulsing circle (square approximation)
      int size = 10 + ((anim_frame % 20) / 2);
      ui_gfx_rect(OLED_W / 2 - size / 2, 55 - size / 2, size, size, 12);
      
      char frame_info[32];
      snprintf(frame_info, sizeof(frame_info), "Frame: %d", anim_frame);
      ui_gfx_text(0, 62, frame_info, 10);
      break;
    }
    
    case 5: { // Hardware info test
      ui_gfx_text(0, 26, "Hardware Info", 15);
      ui_gfx_text(0, 38, "Display: SSD1322", 12);
      ui_gfx_text(0, 50, "Resolution: 256x64", 12);
      ui_gfx_text(0, 62, "Pins: PA8/PC8/PC11", 12);
      break;
    }
    
    case 6: { // Framebuffer direct test
      ui_gfx_text(0, 26, "Direct FB Test", 15);
      
      // Write directly to framebuffer
      // Fill bottom rows with alternating pattern
      for (int row = 40; row < 64; row++) {
        for (int col = 0; col < 128; col++) {
          fb[row * 128 + col] = (col + row) & 0xFF;
        }
      }
      
      ui_gfx_text(0, 38, "Raw framebuffer write", 10);
      break;
    }
    
    default:
      test_mode = 0;
      break;
  }
  
  // Always show current milliseconds
  char ms_info[32];
  snprintf(ms_info, sizeof(ms_info), "MS: %lu", ms);
  ui_gfx_text(OLED_W - 80, 2, ms_info, 10);
}

void ui_page_oled_test_on_button(uint8_t id, uint8_t pressed) {
  if (!pressed) return;
  
  if (id == 0) {
    // Button 0: Previous test
    if (test_mode > 0) {
      test_mode--;
    } else {
      test_mode = 6;
    }
  } else if (id == 1) {
    // Button 1: Next test
    test_mode = (test_mode + 1) % 7;
  } else if (id == 2) {
    // Button 2: Clear screen test
    oled_clear();
  } else if (id == 3) {
    // Button 3: Fill screen white
    uint8_t* fb = oled_framebuffer();
    memset(fb, 0xFF, OLED_W * OLED_H / 2);
  } else if (id == 4) {
    // Button 4: Fill screen black
    oled_clear();
  }
}

void ui_page_oled_test_on_encoder(int8_t delta) {
  if (delta > 0) {
    test_mode = (test_mode + 1) % 7;
  } else if (delta < 0) {
    if (test_mode > 0) {
      test_mode--;
    } else {
      test_mode = 6;
    }
  }
}
