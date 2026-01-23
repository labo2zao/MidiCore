#include "Services/ui/ui_page_oled_test.h"
#include "Services/ui/ui_gfx.h"
#include "Hal/oled_ssd1322/oled_ssd1322.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static uint8_t test_mode = 0; // 0=patterns, 1=grayscale, 2=pixels, 3=text, 4=animations, 5=hardware info, 6=framebuffer, 7=scrolling text, 8=bouncing ball, 9=performance, 10=circles
static uint32_t last_update = 0;
static uint8_t anim_frame = 0;
static int scroll_offset = 0;
static int ball_x = OLED_W / 2;  // Center horizontally for better maintainability
static int ball_y = 32;
static int ball_dx = 2;
static int ball_dy = 1;
static uint32_t fps_counter = 0;
static uint32_t fps_last_time = 0;
static uint32_t fps_value = 0;

void ui_page_oled_test_render(uint32_t ms) {
  uint8_t* fb = oled_framebuffer();
  
  // FPS calculation
  fps_counter++;
  if (ms - fps_last_time >= 1000) {
    fps_value = fps_counter;
    fps_counter = 0;
    fps_last_time = ms;
  }
  
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
    
    case 7: { // Scrolling text test
      ui_gfx_text(0, 26, "Scrolling Text", 15);
      
      if (ms - last_update > 50) {
        scroll_offset += 2;
        if (scroll_offset > 300) scroll_offset = -OLED_W;
        last_update = ms;
      }
      
      const char* scroll_text = "MidiCore OLED SSD1322 Driver Test - Smooth Scrolling";
      ui_gfx_text(scroll_offset, 40, scroll_text, 12);
      
      // Show speed
      char speed_info[32];
      snprintf(speed_info, sizeof(speed_info), "Speed: 2px/50ms");
      ui_gfx_text(0, 55, speed_info, 8);
      break;
    }
    
    case 8: { // Bouncing ball test
      ui_gfx_text(0, 26, "Bouncing Ball", 15);
      
      if (ms - last_update > 30) {
        // Update ball position
        ball_x += ball_dx;
        ball_y += ball_dy;
        
        // Bounce off walls
        if (ball_x <= 0 || ball_x >= OLED_W - 6) ball_dx = -ball_dx;
        if (ball_y <= 38 || ball_y >= OLED_H - 6) ball_dy = -ball_dy;
        
        last_update = ms;
      }
      
      // Draw ball (6x6 square with gradient)
      for (int by = 0; by < 6; by++) {
        for (int bx = 0; bx < 6; bx++) {
          int dist_from_center = (bx - 3) * (bx - 3) + (by - 3) * (by - 3);
          uint8_t brightness;
          if (dist_from_center / 2 > 15) {
            brightness = 0;  // Too far from center, make it black
          } else {
            brightness = (uint8_t)(15 - (dist_from_center / 2));
          }
          ui_gfx_pixel(ball_x + bx, ball_y + by, brightness);
        }
      }
      
      // Show position
      char pos_info[32];
      snprintf(pos_info, sizeof(pos_info), "X:%d Y:%d", ball_x, ball_y);
      ui_gfx_text(OLED_W - 70, 38, pos_info, 8);
      break;
    }
    
    case 9: { // Performance test with FPS
      ui_gfx_text(0, 26, "Performance Test", 15);
      
      // Animate multiple elements
      if (ms - last_update > 20) {
        anim_frame++;
        last_update = ms;
      }
      
      // Draw multiple moving elements
      for (int i = 0; i < 5; i++) {
        int x = ((anim_frame * (i + 1)) % OLED_W);
        int y = 40 + i * 4;
        ui_gfx_rect(x, y, 10, 3, 10 + i);
      }
      
      // Show FPS
      char fps_info[32];
      snprintf(fps_info, sizeof(fps_info), "FPS: %lu", fps_value);
      ui_gfx_text(0, 38, fps_info, 15);
      
      // Show frame counter
      char frame_info[32];
      snprintf(frame_info, sizeof(frame_info), "Frame: %d", anim_frame);
      ui_gfx_text(OLED_W - 80, 38, frame_info, 10);
      break;
    }
    
    case 10: { // Circle/geometry test
      ui_gfx_text(0, 26, "Circles & Lines", 15);
      
      if (ms - last_update > 100) {
        anim_frame++;
        last_update = ms;
      }
      
      // Draw expanding circles using the new circle function
      int center_x = OLED_W / 2;
      int center_y = 48;
      
      for (int r = 0; r < 3; r++) {
        int radius = 5 + ((anim_frame + r * 7) % 15);
        uint8_t brightness = (uint8_t)(13 - r * 3);
        ui_gfx_circle(center_x, center_y, radius, brightness);
      }
      
      // Draw diagonal lines using line function
      int offset = (anim_frame * 2) % 60;
      ui_gfx_line(0, 38 + offset % 20, OLED_W - 1, 38 + (offset + 10) % 20, 10);
      ui_gfx_line(0, 50 + offset % 14, OLED_W - 1, 50 + (offset + 7) % 14, 8);
      
      // Draw a rotating line from center (simplified approximation)
      // Using simplified angle calculation for 4 cardinal directions
      int angle_step = (anim_frame / 10) % 8;  // 8 steps for smoother rotation
      int line_len = 20;
      int line_x = center_x;
      int line_y = center_y;
      
      // Simple 8-direction rotation using lookup
      switch (angle_step) {
        case 0: line_x += line_len; break;                    // East
        case 1: line_x += line_len; line_y -= line_len; break; // NE
        case 2: line_y -= line_len; break;                    // North
        case 3: line_x -= line_len; line_y -= line_len; break; // NW
        case 4: line_x -= line_len; break;                    // West
        case 5: line_x -= line_len; line_y += line_len; break; // SW
        case 6: line_y += line_len; break;                    // South
        case 7: line_x += line_len; line_y += line_len; break; // SE
      }
      
      ui_gfx_line(center_x, center_y, line_x, line_y, 15);
      
      break;
    }
    
    default:
      test_mode = 0;
      break;
  }
  
  // Always show current milliseconds and FPS
  char ms_info[64];
  snprintf(ms_info, sizeof(ms_info), "MS:%lu FPS:%lu", ms, fps_value);
  ui_gfx_text(OLED_W - 110, 2, ms_info, 10);
}

void ui_page_oled_test_on_button(uint8_t id, uint8_t pressed) {
  if (!pressed) return;
  
  if (id == 0) {
    // Button 0: Previous test
    if (test_mode > 0) {
      test_mode--;
    } else {
      test_mode = 10; // Updated max test mode
    }
    // Reset animation state
    anim_frame = 0;
    scroll_offset = 0;
    ball_x = OLED_W / 2; 
    ball_y = 32;
  } else if (id == 1) {
    // Button 1: Next test
    test_mode = (test_mode + 1) % 11; // Updated max test mode
    // Reset animation state
    anim_frame = 0;
    scroll_offset = 0;
    ball_x = OLED_W / 2; 
    ball_y = 32;
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
  } else if (id == 5) {
    // Button 5: Reset FPS counter
    fps_counter = 0;
    fps_last_time = 0;
    fps_value = 0;
  }
}

void ui_page_oled_test_on_encoder(int8_t delta) {
  if (delta > 0) {
    test_mode = (test_mode + 1) % 11; // Updated max test mode
    // Reset animation state
    anim_frame = 0;
    scroll_offset = 0;
    ball_x = OLED_W / 2; 
    ball_y = 32;
  } else if (delta < 0) {
    if (test_mode > 0) {
      test_mode--;
    } else {
      test_mode = 10; // Updated max test mode
    }
    // Reset animation state
    anim_frame = 0;
    scroll_offset = 0;
    ball_x = OLED_W / 2; 
    ball_y = 32;
  }
}
