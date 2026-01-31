#pragma once
#include <stdint.h>

#define OLED_W 256
#define OLED_H 64

// ============================================================================
// Initialization Functions
// ============================================================================
void oled_init_newhaven(void);           // Complete Newhaven NHD-3.12 init (LoopA production) - USE THIS IN PRODUCTION

// ============================================================================
// Test/Debug Functions - Only available when MODULE_TEST_OLED=1
// These functions are for hardware testing and debugging only
// NOT COMPILED IN PRODUCTION BUILDS
// ============================================================================
#ifdef MODULE_TEST_OLED

// Test/Debug Init Functions
void oled_init(void);                    // Simple MidiCore test init (basic, working) - FOR TESTING ONLY
void oled_init_progressive(uint8_t max_step);  // For debugging (step-by-step) - FOR TESTING ONLY

#endif // MODULE_TEST_OLED

// ============================================================================
// Framebuffer Functions
// ============================================================================
uint8_t* oled_framebuffer(void);         // Get framebuffer pointer
void oled_flush(void);                   // Transfer framebuffer to display
void oled_clear(void);                   // Clear framebuffer

// ============================================================================
// OLED Test Functions - Only available when MODULE_TEST_OLED=1
// Visual verification of display capabilities
// NOT COMPILED IN PRODUCTION BUILDS
// ============================================================================
#ifdef MODULE_TEST_OLED

void oled_test_mios32_pattern(void);     // MIOS32-compatible test pattern (gradient + white)
void oled_test_checkerboard(void);       // Checkerboard pattern for pixel uniformity
void oled_test_h_gradient(void);         // Horizontal gradient (left=black, right=white)
void oled_test_v_gradient(void);         // Vertical gradient (top=black, bottom=white)
void oled_test_rectangles(void);         // Concentric rectangles pattern
void oled_test_stripes(void);            // Diagonal stripe pattern
void oled_test_voxel_landscape(void);    // Simple 3D voxel terrain visualization
void oled_test_gray_levels(void);        // All 16 grayscale levels as vertical bars
void oled_test_text_pattern(void);       // Simulated text pattern

#endif // MODULE_TEST_OLED
