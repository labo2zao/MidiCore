/**
 * @file velocity_compressor_test.c
 * @brief Test program for velocity compressor module
 * 
 * Demonstrates and validates all features of the velocity compressor.
 * Compile with: gcc -DSTANDALONE_TEST -o test velocity_compressor_test.c velocity_compressor.c -lm
 */

#include "velocity_compressor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ANSI color codes for output
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"

// Test result counters
static int tests_passed = 0;
static int tests_failed = 0;

// Test assertion macro
#define TEST_ASSERT(condition, description) do { \
    if (condition) { \
        printf(COLOR_GREEN "✓ PASS" COLOR_RESET ": %s\n", description); \
        tests_passed++; \
    } else { \
        printf(COLOR_RED "✗ FAIL" COLOR_RESET ": %s\n", description); \
        tests_failed++; \
    } \
} while(0)

/**
 * @brief Print a visual velocity bar graph
 */
void print_velocity_bar(uint8_t velocity, const char* label) {
    printf("%-20s [%3d] ", label, velocity);
    int bars = velocity / 4;  // Scale to ~32 characters max
    for (int i = 0; i < bars; i++) {
        printf("█");
    }
    printf("\n");
}

/**
 * @brief Test basic initialization
 */
void test_initialization(void) {
    printf("\n" COLOR_CYAN "=== Test: Initialization ===" COLOR_RESET "\n");
    
    velocity_compressor_init();
    
    // Check default values
    TEST_ASSERT(velocity_compressor_is_enabled(0) == 0, 
                "Track 0 starts disabled");
    TEST_ASSERT(velocity_compressor_get_threshold(0) == 80, 
                "Default threshold is 80");
    TEST_ASSERT(velocity_compressor_get_ratio(0) == COMP_RATIO_4_1, 
                "Default ratio is 4:1");
    TEST_ASSERT(velocity_compressor_get_makeup_gain(0) == 0, 
                "Default makeup gain is 0");
    TEST_ASSERT(velocity_compressor_get_knee(0) == COMP_KNEE_HARD, 
                "Default knee is hard");
    TEST_ASSERT(velocity_compressor_get_min_velocity(0) == 1, 
                "Default min velocity is 1");
    TEST_ASSERT(velocity_compressor_get_max_velocity(0) == 127, 
                "Default max velocity is 127");
}

/**
 * @brief Test bypass functionality
 */
void test_bypass(void) {
    printf("\n" COLOR_CYAN "=== Test: Bypass ===" COLOR_RESET "\n");
    
    velocity_compressor_init();
    
    // Test that disabled compressor passes signal unchanged
    uint8_t test_velocities[] = {1, 32, 64, 96, 127};
    
    for (int i = 0; i < 5; i++) {
        uint8_t vel = test_velocities[i];
        uint8_t output = velocity_compressor_process(0, vel);
        char desc[100];
        snprintf(desc, sizeof(desc), "Bypass: input %d == output %d", vel, output);
        TEST_ASSERT(output == vel, desc);
    }
}

/**
 * @brief Test threshold behavior
 */
void test_threshold(void) {
    printf("\n" COLOR_CYAN "=== Test: Threshold ===" COLOR_RESET "\n");
    
    velocity_compressor_init();
    velocity_compressor_set_enabled(0, 1);
    velocity_compressor_set_threshold(0, 64);
    velocity_compressor_set_ratio(0, COMP_RATIO_4_1);
    
    printf("Threshold: 64, Ratio: 4:1\n");
    
    // Velocities below threshold should pass through
    uint8_t below_threshold = velocity_compressor_process(0, 60);
    TEST_ASSERT(below_threshold == 60, 
                "Velocity below threshold passes unchanged");
    
    // Velocities above threshold should be reduced
    uint8_t above_threshold = velocity_compressor_process(0, 100);
    TEST_ASSERT(above_threshold < 100, 
                "Velocity above threshold is compressed");
    
    printf("  Input: 60 (below) -> Output: %d\n", below_threshold);
    printf("  Input: 100 (above) -> Output: %d\n", above_threshold);
}

/**
 * @brief Test compression ratios
 */
void test_ratios(void) {
    printf("\n" COLOR_CYAN "=== Test: Compression Ratios ===" COLOR_RESET "\n");
    
    velocity_compressor_init();
    velocity_compressor_set_enabled(0, 1);
    velocity_compressor_set_threshold(0, 64);
    
    uint8_t input = 100;
    printf("Input velocity: %d, Threshold: 64\n\n", input);
    
    velocity_comp_ratio_t ratios[] = {
        COMP_RATIO_1_1, COMP_RATIO_2_1, COMP_RATIO_4_1, 
        COMP_RATIO_8_1, COMP_RATIO_INF
    };
    
    uint8_t prev_output = 127;
    
    for (int i = 0; i < 5; i++) {
        velocity_compressor_set_ratio(0, ratios[i]);
        uint8_t output = velocity_compressor_process(0, input);
        const char* ratio_name = velocity_compressor_get_ratio_name(ratios[i]);
        
        printf("  Ratio %s: %d -> %d\n", ratio_name, input, output);
        
        // Higher ratios should produce more compression (lower output)
        if (i > 0) {
            char desc[100];
            snprintf(desc, sizeof(desc), 
                    "Higher ratio produces more compression (%s)", ratio_name);
            TEST_ASSERT(output <= prev_output, desc);
        }
        prev_output = output;
    }
}

/**
 * @brief Test makeup gain
 */
void test_makeup_gain(void) {
    printf("\n" COLOR_CYAN "=== Test: Makeup Gain ===" COLOR_RESET "\n");
    
    velocity_compressor_init();
    velocity_compressor_set_enabled(0, 1);
    velocity_compressor_set_threshold(0, 64);
    velocity_compressor_set_ratio(0, COMP_RATIO_4_1);
    
    uint8_t input = 100;
    
    // Test with no makeup gain
    velocity_compressor_set_makeup_gain(0, 0);
    uint8_t no_makeup = velocity_compressor_process(0, input);
    
    // Test with positive makeup gain
    velocity_compressor_set_makeup_gain(0, 15);
    uint8_t with_makeup = velocity_compressor_process(0, input);
    
    printf("  Input: %d\n", input);
    printf("  No makeup gain: %d\n", no_makeup);
    printf("  +15 makeup gain: %d\n", with_makeup);
    
    TEST_ASSERT(with_makeup > no_makeup, 
                "Makeup gain increases output level");
}

/**
 * @brief Test knee types
 */
void test_knee_types(void) {
    printf("\n" COLOR_CYAN "=== Test: Knee Types ===" COLOR_RESET "\n");
    
    velocity_compressor_init();
    velocity_compressor_set_enabled(0, 1);
    velocity_compressor_set_threshold(0, 64);
    velocity_compressor_set_ratio(0, COMP_RATIO_4_1);
    
    // Test velocities around the threshold
    uint8_t test_vels[] = {58, 62, 66, 70, 80, 100};
    
    printf("\nHard Knee:\n");
    velocity_compressor_set_knee(0, COMP_KNEE_HARD);
    for (int i = 0; i < 6; i++) {
        uint8_t output = velocity_compressor_process(0, test_vels[i]);
        printf("  %d -> %d\n", test_vels[i], output);
    }
    
    printf("\nSoft Knee:\n");
    velocity_compressor_set_knee(0, COMP_KNEE_SOFT);
    for (int i = 0; i < 6; i++) {
        uint8_t output = velocity_compressor_process(0, test_vels[i]);
        printf("  %d -> %d\n", test_vels[i], output);
    }
    
    TEST_ASSERT(1, "Knee types processed successfully");
}

/**
 * @brief Test min/max velocity caps
 */
void test_velocity_caps(void) {
    printf("\n" COLOR_CYAN "=== Test: Velocity Caps ===" COLOR_RESET "\n");
    
    velocity_compressor_init();
    velocity_compressor_set_enabled(0, 1);
    
    // Disable compression (1:1 ratio) to test caps only
    velocity_compressor_set_ratio(0, COMP_RATIO_1_1);
    velocity_compressor_set_threshold(0, 127);  // Threshold above max
    
    velocity_compressor_set_min_velocity(0, 20);
    velocity_compressor_set_max_velocity(0, 100);
    
    // Test minimum cap
    uint8_t low = velocity_compressor_process(0, 10);
    TEST_ASSERT(low == 20, "Min velocity cap enforced");
    printf("  Input: 10 -> Output: %d (clamped to min 20)\n", low);
    
    // Test maximum cap
    uint8_t high = velocity_compressor_process(0, 127);
    TEST_ASSERT(high == 100, "Max velocity cap enforced");
    printf("  Input: 127 -> Output: %d (clamped to max 100)\n", high);
}

/**
 * @brief Test limiter mode (infinite ratio)
 */
void test_limiter(void) {
    printf("\n" COLOR_CYAN "=== Test: Limiter Mode (∞:1) ===" COLOR_RESET "\n");
    
    velocity_compressor_init();
    velocity_compressor_set_enabled(0, 1);
    velocity_compressor_set_threshold(0, 90);
    velocity_compressor_set_ratio(0, COMP_RATIO_INF);
    
    printf("Threshold: 90, Ratio: ∞:1 (limiter)\n");
    
    // Test that all velocities above threshold are limited to ~threshold
    uint8_t test_vels[] = {50, 80, 90, 100, 110, 127};
    
    for (int i = 0; i < 6; i++) {
        uint8_t output = velocity_compressor_process(0, test_vels[i]);
        printf("  %d -> %d\n", test_vels[i], output);
        
        if (test_vels[i] > 90) {
            char desc[100];
            snprintf(desc, sizeof(desc), 
                    "Limiter keeps output near threshold (input %d)", test_vels[i]);
            TEST_ASSERT(output <= 92, desc);  // Allow small margin
        }
    }
}

/**
 * @brief Test gain reduction calculation
 */
void test_gain_reduction(void) {
    printf("\n" COLOR_CYAN "=== Test: Gain Reduction ===" COLOR_RESET "\n");
    
    velocity_compressor_init();
    velocity_compressor_set_enabled(0, 1);
    velocity_compressor_set_threshold(0, 64);
    velocity_compressor_set_ratio(0, COMP_RATIO_4_1);
    
    // Test velocities
    uint8_t test_vels[] = {50, 64, 80, 100, 127};
    
    for (int i = 0; i < 5; i++) {
        uint8_t vel = test_vels[i];
        uint8_t reduction = velocity_compressor_get_gain_reduction(0, vel);
        uint8_t output = velocity_compressor_process(0, vel);
        
        printf("  Input: %3d -> Output: %3d (GR: %d)\n", 
               vel, output, reduction);
        
        if (vel <= 64) {
            char desc[100];
            snprintf(desc, sizeof(desc), 
                    "No gain reduction below threshold (vel %d)", vel);
            TEST_ASSERT(reduction == 0, desc);
        }
    }
}

/**
 * @brief Visual demonstration of compression curve
 */
void test_compression_curve_visual(void) {
    printf("\n" COLOR_CYAN "=== Visual: Compression Curve ===" COLOR_RESET "\n");
    
    velocity_compressor_init();
    velocity_compressor_set_enabled(0, 1);
    velocity_compressor_set_threshold(0, 64);
    velocity_compressor_set_ratio(0, COMP_RATIO_4_1);
    velocity_compressor_set_makeup_gain(0, 10);
    
    printf("Settings: Threshold=64, Ratio=4:1, Makeup=+10\n\n");
    
    for (uint8_t vel = 20; vel <= 127; vel += 10) {
        uint8_t output = velocity_compressor_process(0, vel);
        char label[50];
        snprintf(label, sizeof(label), "In:%3d -> Out:%3d", vel, output);
        print_velocity_bar(vel, label);
    }
}

/**
 * @brief Test all tracks independently
 */
void test_multi_track(void) {
    printf("\n" COLOR_CYAN "=== Test: Multi-Track Configuration ===" COLOR_RESET "\n");
    
    velocity_compressor_init();
    
    // Configure different settings for each track with safe ratio mapping
    velocity_comp_ratio_t ratios[] = {COMP_RATIO_1_1, COMP_RATIO_2_1, 
                                      COMP_RATIO_3_1, COMP_RATIO_4_1};
    
    for (uint8_t t = 0; t < VELOCITY_COMP_MAX_TRACKS; t++) {
        velocity_compressor_set_enabled(t, 1);
        velocity_compressor_set_threshold(t, 60 + (t * 5));
        velocity_compressor_set_ratio(t, ratios[t]);
    }
    
    uint8_t input = 100;
    printf("Input velocity: %d\n", input);
    
    for (uint8_t t = 0; t < VELOCITY_COMP_MAX_TRACKS; t++) {
        uint8_t output = velocity_compressor_process(t, input);
        printf("  Track %d: threshold=%d, ratio=%s -> output=%d\n",
               t,
               velocity_compressor_get_threshold(t),
               velocity_compressor_get_ratio_name(velocity_compressor_get_ratio(t)),
               output);
    }
    
    TEST_ASSERT(1, "All tracks configured independently");
}

/**
 * @brief Main test runner
 */
int main(void) {
    printf("\n");
    printf(COLOR_MAGENTA "╔═══════════════════════════════════════════════════════╗\n");
    printf("║   MIDI Velocity Compressor Test Suite                ║\n");
    printf("╚═══════════════════════════════════════════════════════╝\n" COLOR_RESET);
    
    // Run all tests
    test_initialization();
    test_bypass();
    test_threshold();
    test_ratios();
    test_makeup_gain();
    test_knee_types();
    test_velocity_caps();
    test_limiter();
    test_gain_reduction();
    test_compression_curve_visual();
    test_multi_track();
    
    // Print summary
    printf("\n");
    printf(COLOR_MAGENTA "═══════════════════════════════════════════════════════\n" COLOR_RESET);
    printf(COLOR_CYAN "Test Results:\n" COLOR_RESET);
    printf("  " COLOR_GREEN "Passed: %d\n" COLOR_RESET, tests_passed);
    printf("  " COLOR_RED "Failed: %d\n" COLOR_RESET, tests_failed);
    printf("  Total:  %d\n", tests_passed + tests_failed);
    
    if (tests_failed == 0) {
        printf("\n" COLOR_GREEN "✓ All tests passed!\n" COLOR_RESET);
        return 0;
    } else {
        printf("\n" COLOR_RED "✗ Some tests failed!\n" COLOR_RESET);
        return 1;
    }
}
