#include "Services/ui/ui_page_oled_test.h"
#include "Services/ui/ui_gfx.h"
#include "Hal/oled_ssd1322/oled_ssd1322.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static uint8_t test_mode = 0; // 0=patterns, 1=grayscale, 2=pixels, 3=text, 4=animations, 5=hardware info, 6=framebuffer, 7=scrolling text, 8=bouncing ball, 9=performance, 10=circles, 11=bitmap, 12=fill patterns, 13=stress test, 14=auto-cycle, 15=burn-in prevention, 16=stats display, 17=3D wireframe
static uint32_t last_update = 0;
static uint8_t anim_frame = 0;
static int scroll_offset = 0;
static int ball_x = 128;  // OLED_W / 2 = 256 / 2 = 128 (center horizontally)
static int ball_y = 32;
static int ball_dx = 2;
static int ball_dy = 1;
static uint32_t fps_counter = 0;
static uint32_t fps_last_time = 0;
static uint32_t fps_value = 0;
static uint8_t auto_cycle_enabled = 0;  // Auto-cycle mode flag
static uint32_t auto_cycle_timer = 0;    // Timer for auto-cycling
static uint32_t fps_min = 999;           // Minimum FPS recorded
static uint32_t fps_max = 0;             // Maximum FPS recorded
static uint32_t frame_time_sum = 0;      // Sum of frame times for average
static uint32_t frame_time_count = 0;    // Count of frame times
static uint32_t last_frame_time = 0;     // Time of last frame

void ui_page_oled_test_render(uint32_t ms) {
  uint8_t* fb = oled_framebuffer();
  
  // Frame time tracking
  if (last_frame_time > 0) {
    uint32_t frame_time = ms - last_frame_time;
    if (frame_time > 0 && frame_time < 1000) {  // Sanity check
      frame_time_sum += frame_time;
      frame_time_count++;
    }
  }
  last_frame_time = ms;
  
  // FPS calculation with min/max tracking
  fps_counter++;
  if (ms - fps_last_time >= 1000) {
    fps_value = fps_counter;
    
    // Track min/max
    if (fps_value > 0) {
      if (fps_value < fps_min) fps_min = fps_value;
      if (fps_value > fps_max) fps_max = fps_value;
    }
    
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
          int brightness_calc = 15 - (dist_from_center / 2);
          
          // Ensure brightness stays within valid range (0-15)
          if (brightness_calc < 0) {
            brightness = 0;
          } else if (brightness_calc > 15) {
            brightness = 15;
          } else {
            brightness = (uint8_t)brightness_calc;
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
    
    case 11: { // Bitmap/Image test
      ui_gfx_text(0, 26, "Bitmap Test", 15);
      
      // Draw a simple smiley face bitmap
      int face_x = OLED_W / 2 - 16;
      int face_y = 40;
      
      // Face outline (circle)
      ui_gfx_circle(face_x + 16, face_y + 8, 15, 12);
      
      // Left eye
      ui_gfx_circle(face_x + 10, face_y + 5, 2, 15);
      
      // Right eye
      ui_gfx_circle(face_x + 22, face_y + 5, 2, 15);
      
      // Smile (arc approximation with line)
      for (int x = 0; x < 16; x++) {
        int y_offset = (x - 8) * (x - 8) / 16;
        ui_gfx_pixel(face_x + 8 + x, face_y + 12 + y_offset / 4, 15);
      }
      
      ui_gfx_text(0, 38, "Simple Graphics Demo", 10);
      break;
    }
    
    case 12: { // Fill patterns test
      ui_gfx_text(0, 26, "Fill Patterns", 15);
      
      if (ms - last_update > 500) {
        anim_frame++;
        last_update = ms;
      }
      
      int pattern = anim_frame % 4;
      
      switch (pattern) {
        case 0: // Dots pattern
          for (int y = 40; y < 64; y += 3) {
            for (int x = 0; x < OLED_W; x += 3) {
              ui_gfx_pixel(x, y, ((x + y) / 3) % 16);
            }
          }
          ui_gfx_text(0, 38, "Dots Pattern", 10);
          break;
          
        case 1: // Dither pattern
          for (int y = 40; y < 64; y++) {
            for (int x = 0; x < OLED_W; x++) {
              uint8_t val = ((x ^ y) & 1) ? 15 : 0;
              ui_gfx_pixel(x, y, val);
            }
          }
          ui_gfx_text(0, 38, "Dither Pattern", 10);
          break;
          
        case 2: // Waves pattern
          for (int y = 40; y < 64; y++) {
            for (int x = 0; x < OLED_W; x += 2) {
              int wave = ((x / 8) + (y / 4)) % 16;
              ui_gfx_pixel(x, y, wave);
            }
          }
          ui_gfx_text(0, 38, "Waves Pattern", 10);
          break;
          
        case 3: // Grid pattern
          for (int y = 40; y < 64; y++) {
            for (int x = 0; x < OLED_W; x++) {
              if (x % 8 == 0 || y % 8 == 40) {
                ui_gfx_pixel(x, y, 15);
              }
            }
          }
          ui_gfx_text(0, 38, "Grid Pattern", 10);
          break;
      }
      
      char pattern_info[32];
      snprintf(pattern_info, sizeof(pattern_info), "Pattern %d/4", pattern + 1);
      ui_gfx_text(OLED_W - 70, 38, pattern_info, 8);
      break;
    }
    
    case 13: { // Stress test - maximum graphics throughput
      ui_gfx_text(0, 26, "Stress Test", 15);
      
      if (ms - last_update > 16) {  // ~60 FPS target
        anim_frame++;
        last_update = ms;
      }
      
      // Draw multiple animated elements simultaneously
      for (int i = 0; i < 10; i++) {
        int x = ((anim_frame * (i + 1) * 3) % OLED_W);
        int y = 40 + (i * 2);
        ui_gfx_rect(x, y, 8, 2, 10 + (i % 6));
      }
      
      // Draw circles
      for (int i = 0; i < 3; i++) {
        int cx = 50 + i * 70;
        int cy = 50;
        int radius = 5 + ((anim_frame + i * 10) % 8);
        ui_gfx_circle(cx, cy, radius, 8 + i * 2);
      }
      
      // Draw lines
      for (int i = 0; i < 5; i++) {
        int offset = (anim_frame * 2 + i * 20) % OLED_W;
        ui_gfx_line(offset, 40, (offset + 30) % OLED_W, 60, 6 + i);
      }
      
      char stress_info[64];
      snprintf(stress_info, sizeof(stress_info), "Elements: 18 | Target: 60 FPS");
      ui_gfx_text(0, 38, stress_info, 12);
      break;
    }
    
    case 14: { // Auto-cycle mode
      ui_gfx_text(0, 26, "Auto-Cycle Demo", 15);
      
      // Auto-cycle through modes every 3 seconds
      if (ms - auto_cycle_timer > 3000) {
        // Cycle to next mode (skip auto-cycle and new modes)
        uint8_t next_mode = (test_mode + 1) % 14;
        test_mode = next_mode;
        auto_cycle_timer = ms;
        anim_frame = 0;
        scroll_offset = 0;
        ball_x = 128;
        ball_y = 32;
      }
      
      uint32_t time_remaining = 3000 - (ms - auto_cycle_timer);
      char cycle_info[64];
      snprintf(cycle_info, sizeof(cycle_info), "Next mode in: %lu ms", time_remaining);
      ui_gfx_text(0, 40, cycle_info, 12);
      
      ui_gfx_text(0, 52, "Press any button to exit", 10);
      
      // Draw progress bar
      int progress_width = (int)((OLED_W - 20) * (3000 - time_remaining) / 3000);
      ui_gfx_rect(10, 60, progress_width, 3, 15);
      ui_gfx_rect(10 + progress_width, 60, (OLED_W - 20) - progress_width, 3, 3);
      break;
    }
    
    case 15: { // Burn-in prevention mode
      ui_gfx_text(0, 26, "Burn-In Prevention", 15);
      
      if (ms - last_update > 100) {
        anim_frame++;
        last_update = ms;
      }
      
      // Moving box pattern to prevent burn-in
      int box_size = 40;
      int box_x = ((anim_frame * 3) % (OLED_W - box_size));
      int box_y = 38 + ((anim_frame / 2) % (OLED_H - 38 - box_size));
      
      // Draw moving box with gradient
      for (int y = 0; y < box_size; y++) {
        for (int x = 0; x < box_size; x++) {
          int dist_from_center = abs(x - box_size/2) + abs(y - box_size/2);
          uint8_t brightness = (uint8_t)(15 - (dist_from_center / 3));
          if (brightness > 15) brightness = 0;
          ui_gfx_pixel(box_x + x, box_y + y, brightness);
        }
      }
      
      // Moving lines
      for (int i = 0; i < 3; i++) {
        int line_offset = ((anim_frame * (i + 2)) % OLED_W);
        ui_gfx_vline(line_offset, 38, OLED_H - 38, 8 + i * 2);
      }
      
      ui_gfx_text(0, 38, "Prevents static image", 10);
      
      // Show elapsed time
      uint32_t elapsed_sec = ms / 1000;
      char time_info[32];
      snprintf(time_info, sizeof(time_info), "Running: %lu sec", elapsed_sec);
      ui_gfx_text(OLED_W - 110, 38, time_info, 8);
      break;
    }
    
    case 16: { // Statistics display
      ui_gfx_text(0, 26, "Performance Stats", 15);
      
      // Calculate average frame time
      uint32_t avg_frame_time = 0;
      if (frame_time_count > 0) {
        avg_frame_time = frame_time_sum / frame_time_count;
      }
      
      // Display detailed statistics
      char stat1[48];
      snprintf(stat1, sizeof(stat1), "Current FPS: %lu", fps_value);
      ui_gfx_text(0, 38, stat1, 12);
      
      char stat2[48];
      snprintf(stat2, sizeof(stat2), "Min FPS: %lu  Max FPS: %lu", fps_min, fps_max);
      ui_gfx_text(0, 46, stat2, 10);
      
      char stat3[48];
      snprintf(stat3, sizeof(stat3), "Avg Frame Time: %lu ms", avg_frame_time);
      ui_gfx_text(0, 54, stat3, 10);
      
      char stat4[48];
      uint32_t uptime_sec = ms / 1000;
      uint32_t uptime_min = uptime_sec / 60;
      snprintf(stat4, sizeof(stat4), "Uptime: %lu min %lu sec", uptime_min, uptime_sec % 60);
      ui_gfx_text(0, 62, stat4, 8);
      
      // Draw FPS history bar graph (simple representation)
      if (fps_value > 0) {
        int bar_width = (int)((fps_value * (OLED_W - 20)) / 60);  // Scale to 60 FPS max
        if (bar_width > OLED_W - 20) bar_width = OLED_W - 20;
        ui_gfx_rect(10, 58, bar_width, 2, 15);
      }
      
      break;
    }
    
    case 17: { // 3D Wireframe cube
      ui_gfx_text(0, 26, "3D Wireframe Cube", 15);
      
      if (ms - last_update > 50) {
        anim_frame++;
        last_update = ms;
      }
      
      // Cube center and size
      int cx = OLED_W / 2;
      int cy = 48;
      int size = 15;
      
      // Rotation angles (simplified using frame counter)
      int angle = (anim_frame * 2) % 360;
      
      // Simple 3D cube vertices (8 corners)
      // Using simplified rotation without trigonometry
      int vertices[8][2];
      
      // Calculate rotation using lookup table approach (8 steps = 45 degrees each)
      int step = (angle / 45) % 8;
      
      // Front face (z near)
      int offset_x = (step < 4) ? (size * (2 - step)) / 2 : (size * (step - 6)) / 2;
      int offset_y = (step < 2 || step > 6) ? -size / 2 : (step < 4) ? size / 2 : (step < 6) ? size / 2 : -size / 2;
      
      vertices[0][0] = cx - size + offset_x / 3;  // Top-left-front
      vertices[0][1] = cy - size + offset_y / 3;
      vertices[1][0] = cx + size + offset_x / 3;  // Top-right-front
      vertices[1][1] = cy - size + offset_y / 3;
      vertices[2][0] = cx + size + offset_x / 3;  // Bottom-right-front
      vertices[2][1] = cy + size + offset_y / 3;
      vertices[3][0] = cx - size + offset_x / 3;  // Bottom-left-front
      vertices[3][1] = cy + size + offset_y / 3;
      
      // Back face (z far) - slightly offset for depth
      int depth = 8;
      vertices[4][0] = cx - size / 2 - offset_x / 4;  // Top-left-back
      vertices[4][1] = cy - size / 2 - offset_y / 4;
      vertices[5][0] = cx + size / 2 - offset_x / 4;  // Top-right-back
      vertices[5][1] = cy - size / 2 - offset_y / 4;
      vertices[6][0] = cx + size / 2 - offset_x / 4;  // Bottom-right-back
      vertices[6][1] = cy + size / 2 - offset_y / 4;
      vertices[7][0] = cx - size / 2 - offset_x / 4;  // Bottom-left-back
      vertices[7][1] = cy + size / 2 - offset_y / 4;
      
      // Draw front face
      ui_gfx_line(vertices[0][0], vertices[0][1], vertices[1][0], vertices[1][1], 15);
      ui_gfx_line(vertices[1][0], vertices[1][1], vertices[2][0], vertices[2][1], 15);
      ui_gfx_line(vertices[2][0], vertices[2][1], vertices[3][0], vertices[3][1], 15);
      ui_gfx_line(vertices[3][0], vertices[3][1], vertices[0][0], vertices[0][1], 15);
      
      // Draw back face
      ui_gfx_line(vertices[4][0], vertices[4][1], vertices[5][0], vertices[5][1], 10);
      ui_gfx_line(vertices[5][0], vertices[5][1], vertices[6][0], vertices[6][1], 10);
      ui_gfx_line(vertices[6][0], vertices[6][1], vertices[7][0], vertices[7][1], 10);
      ui_gfx_line(vertices[7][0], vertices[7][1], vertices[4][0], vertices[4][1], 10);
      
      // Draw connecting lines (edges between front and back)
      ui_gfx_line(vertices[0][0], vertices[0][1], vertices[4][0], vertices[4][1], 12);
      ui_gfx_line(vertices[1][0], vertices[1][1], vertices[5][0], vertices[5][1], 12);
      ui_gfx_line(vertices[2][0], vertices[2][1], vertices[6][0], vertices[6][1], 12);
      ui_gfx_line(vertices[3][0], vertices[3][1], vertices[7][0], vertices[7][1], 12);
      
      // Show rotation angle
      char angle_info[32];
      snprintf(angle_info, sizeof(angle_info), "Angle: %d deg", angle);
      ui_gfx_text(0, 38, angle_info, 8);
      
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
  
  // Exit auto-cycle mode if any button pressed
  if (test_mode == 14) {
    test_mode = 0;
    auto_cycle_timer = 0;
    anim_frame = 0;
    return;
  }
  
  if (id == 0) {
    // Button 0: Previous test
    if (test_mode > 0) {
      test_mode--;
    } else {
      test_mode = 17; // Updated max test mode
    }
    // Reset animation state
    anim_frame = 0;
    scroll_offset = 0;
    ball_x = 128;  // OLED_W / 2
    ball_y = 32;
    auto_cycle_timer = 0;
  } else if (id == 1) {
    // Button 1: Next test
    test_mode = (test_mode + 1) % 18; // Updated max test mode
    // Reset animation state
    anim_frame = 0;
    scroll_offset = 0;
    ball_x = 128;  // OLED_W / 2
    ball_y = 32;
    auto_cycle_timer = 0;
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
    // Button 5: Reset statistics
    fps_counter = 0;
    fps_last_time = 0;
    fps_value = 0;
    fps_min = 999;
    fps_max = 0;
    frame_time_sum = 0;
    frame_time_count = 0;
  }
}

void ui_page_oled_test_on_encoder(int8_t delta) {
  // Exit auto-cycle mode if encoder used
  if (test_mode == 14) {
    test_mode = 0;
    auto_cycle_timer = 0;
    anim_frame = 0;
    return;
  }
  
  if (delta > 0) {
    test_mode = (test_mode + 1) % 18; // Updated max test mode
    // Reset animation state
    anim_frame = 0;
    scroll_offset = 0;
    ball_x = 128;  // OLED_W / 2
    ball_y = 32;
    auto_cycle_timer = 0;
  } else if (delta < 0) {
    if (test_mode > 0) {
      test_mode--;
    } else {
      test_mode = 17; // Updated max test mode
    }
    // Reset animation state
    anim_frame = 0;
    scroll_offset = 0;
    ball_x = 128;  // OLED_W / 2
    ball_y = 32;
    auto_cycle_timer = 0;
  }
}
