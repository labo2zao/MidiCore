#pragma once
#include <stdint.h>

#define OLED_W 256
#define OLED_H 64

// ============================================================================
// Initialization Functions
// ============================================================================
void oled_init(void);                    // Simple MIOS32 test init (basic, working)
void oled_init_newhaven(void);           // Complete Newhaven NHD-3.12 init (LoopA production)
void oled_init_progressive(uint8_t max_step);  // For debugging (step-by-step)

// ============================================================================
// Framebuffer Functions
// ============================================================================
uint8_t* oled_framebuffer(void);         // Get framebuffer pointer
void oled_flush(void);                   // Transfer framebuffer to display
void oled_clear(void);                   // Clear framebuffer

// ============================================================================
// OLED Test Functions - Visual verification of display capabilities
// ============================================================================
void oled_test_mios32_pattern(void);     // MIOS32-compatible test pattern (gradient + white)
void oled_test_checkerboard(void);       // Checkerboard pattern for pixel uniformity
void oled_test_h_gradient(void);         // Horizontal gradient (left=black, right=white)
void oled_test_v_gradient(void);         // Vertical gradient (top=black, bottom=white)
void oled_test_rectangles(void);         // Concentric rectangles pattern
void oled_test_stripes(void);            // Diagonal stripe pattern
void oled_test_voxel_landscape(void);    // Simple 3D voxel terrain visualization
void oled_test_gray_levels(void);        // All 16 grayscale levels as vertical bars
void oled_test_text_pattern(void);       // Simulated text pattern
