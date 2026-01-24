/**
 * @file swing_example.c
 * @brief Example usage of the Swing/Groove MIDI FX module
 * 
 * This file demonstrates how to use the swing module in various scenarios.
 */

#include "Services/swing/swing.h"
#include <stdio.h>

/**
 * @brief Example 1: Basic swing setup
 */
void example_basic_swing(void) {
    printf("Example 1: Basic Swing Setup\n");
    printf("=============================\n\n");
    
    // Initialize with 120 BPM
    swing_init(120);
    
    // Enable swing on track 0
    swing_set_enabled(0, 1);
    
    // Set classic swing feel
    swing_set_groove(0, SWING_GROOVE_SWING);
    swing_set_amount(0, 66);
    swing_set_resolution(0, SWING_RESOLUTION_16TH);
    swing_set_depth(0, 100);
    
    printf("Track 0: Enabled=%d, Amount=%d%%, Groove=%s, Resolution=%s\n",
           swing_is_enabled(0),
           swing_get_amount(0),
           swing_get_groove_name(swing_get_groove(0)),
           swing_get_resolution_name(swing_get_resolution(0)));
    
    // Calculate timing offsets for first few beats
    uint16_t ppqn = 96;
    printf("\nTiming offsets at 120 BPM:\n");
    for (uint32_t tick = 0; tick < ppqn * 2; tick += (ppqn / 4)) {
        int16_t offset = swing_calculate_offset(0, tick, ppqn);
        printf("  Tick %4u: offset = %+4d ms\n", tick, offset);
    }
    printf("\n");
}

/**
 * @brief Example 2: Multiple tracks with different grooves
 */
void example_multi_track(void) {
    printf("Example 2: Multiple Tracks\n");
    printf("===========================\n\n");
    
    swing_init(128);
    
    // Track 0: Straight (drums)
    swing_set_enabled(0, 1);
    swing_set_groove(0, SWING_GROOVE_STRAIGHT);
    swing_set_amount(0, 50);
    
    // Track 1: Swing (bass)
    swing_set_enabled(1, 1);
    swing_set_groove(1, SWING_GROOVE_SWING);
    swing_set_amount(1, 66);
    swing_set_resolution(1, SWING_RESOLUTION_8TH);
    
    // Track 2: Shuffle (hi-hats)
    swing_set_enabled(2, 1);
    swing_set_groove(2, SWING_GROOVE_SHUFFLE);
    swing_set_amount(2, 75);
    swing_set_resolution(2, SWING_RESOLUTION_16TH);
    
    // Track 3: Subtle humanization (melody)
    swing_set_enabled(3, 1);
    swing_set_groove(3, SWING_GROOVE_SWING);
    swing_set_amount(3, 55);
    swing_set_depth(3, 70);
    
    for (uint8_t track = 0; track < 4; track++) {
        printf("Track %d: Enabled=%d, Groove=%s, Amount=%d%%, Depth=%d%%\n",
               track,
               swing_is_enabled(track),
               swing_get_groove_name(swing_get_groove(track)),
               swing_get_amount(track),
               swing_get_depth(track));
    }
    printf("\n");
}

/**
 * @brief Example 3: Custom groove pattern
 */
void example_custom_groove(void) {
    printf("Example 3: Custom Groove Pattern\n");
    printf("=================================\n\n");
    
    swing_init(110);
    swing_set_enabled(0, 1);
    
    // Create a custom groove with varied timing
    uint8_t custom_pattern[16] = {
        50, 60, 50, 70,  // First beat: straight, delay, straight, heavy delay
        50, 55, 50, 65,  // Second beat: similar but lighter
        50, 58, 50, 68,  // Third beat: medium variation
        50, 62, 50, 72   // Fourth beat: stronger variation
    };
    
    swing_set_custom_pattern(0, custom_pattern, 16);
    swing_set_groove(0, SWING_GROOVE_CUSTOM);
    swing_set_amount(0, 60);
    swing_set_resolution(0, SWING_RESOLUTION_16TH);
    
    printf("Custom pattern set with 16 steps\n");
    printf("Pattern values: ");
    
    uint8_t retrieved_pattern[16];
    uint8_t length;
    swing_get_custom_pattern(0, retrieved_pattern, &length);
    
    for (uint8_t i = 0; i < length; i++) {
        printf("%d ", retrieved_pattern[i]);
        if ((i + 1) % 4 == 0) printf("| ");
    }
    printf("\n\n");
}

/**
 * @brief Example 4: Time-based swing calculation
 */
void example_time_based(void) {
    printf("Example 4: Time-Based Swing\n");
    printf("============================\n\n");
    
    swing_init(140);
    swing_set_enabled(0, 1);
    swing_set_groove(0, SWING_GROOVE_SWING);
    swing_set_amount(0, 66);
    swing_set_resolution(0, SWING_RESOLUTION_16TH);
    
    printf("Calculating offsets using time (ms) at 140 BPM:\n");
    
    // Calculate timing for first few subdivisions
    uint32_t ms_per_quarter = 60000 / 140;  // ~428ms
    uint32_t ms_per_16th = ms_per_quarter / 4;
    
    for (uint8_t i = 0; i < 8; i++) {
        uint32_t time_ms = i * ms_per_16th;
        int16_t offset = swing_calculate_offset_ms(0, time_ms);
        printf("  Time %4u ms (16th #%d): offset = %+4d ms\n", 
               time_ms, i, offset);
    }
    printf("\n");
}

/**
 * @brief Example 5: Tempo changes
 */
void example_tempo_change(void) {
    printf("Example 5: Tempo Changes\n");
    printf("========================\n\n");
    
    swing_init(120);
    swing_set_enabled(0, 1);
    swing_set_groove(0, SWING_GROOVE_SWING);
    swing_set_amount(0, 66);
    swing_set_resolution(0, SWING_RESOLUTION_8TH);
    
    uint16_t tempos[] = {80, 120, 160, 200};
    uint32_t tick = 96;  // One quarter note
    uint16_t ppqn = 96;
    
    printf("Offset for same tick position at different tempos:\n");
    for (uint8_t i = 0; i < 4; i++) {
        swing_set_tempo(tempos[i]);
        int16_t offset = swing_calculate_offset(0, tick, ppqn);
        printf("  Tempo %3d BPM: offset = %+4d ms\n", tempos[i], offset);
    }
    printf("\n");
}

/**
 * @brief Example 6: Different resolutions
 */
void example_resolutions(void) {
    printf("Example 6: Different Resolutions\n");
    printf("=================================\n\n");
    
    swing_init(120);
    swing_set_enabled(0, 1);
    swing_set_groove(0, SWING_GROOVE_SWING);
    swing_set_amount(0, 66);
    
    swing_resolution_t resolutions[] = {
        SWING_RESOLUTION_8TH,
        SWING_RESOLUTION_16TH,
        SWING_RESOLUTION_32ND
    };
    
    uint16_t ppqn = 96;
    
    for (uint8_t res = 0; res < 3; res++) {
        swing_set_resolution(0, resolutions[res]);
        printf("Resolution: %s\n", 
               swing_get_resolution_name(resolutions[res]));
        
        // Show offsets for one quarter note
        uint16_t tick_increment = ppqn / (1 << (res + 1));  // 48, 24, 12
        for (uint32_t tick = 0; tick < ppqn; tick += tick_increment) {
            int16_t offset = swing_calculate_offset(0, tick, ppqn);
            printf("  Tick %3u: offset = %+3d ms\n", tick, offset);
        }
        printf("\n");
    }
}

/**
 * @brief Example 7: All groove types
 */
void example_all_grooves(void) {
    printf("Example 7: All Groove Types\n");
    printf("============================\n\n");
    
    swing_init(120);
    swing_set_enabled(0, 1);
    swing_set_amount(0, 66);
    swing_set_resolution(0, SWING_RESOLUTION_16TH);
    
    uint16_t ppqn = 96;
    
    for (uint8_t groove = 0; groove < SWING_GROOVE_CUSTOM; groove++) {
        swing_set_groove(0, (swing_groove_t)groove);
        printf("Groove: %s\n", swing_get_groove_name((swing_groove_t)groove));
        
        // Show first few offsets
        for (uint32_t tick = 0; tick < ppqn; tick += (ppqn / 8)) {
            int16_t offset = swing_calculate_offset(0, tick, ppqn);
            printf("  Tick %3u: %+4d ms\n", tick, offset);
        }
        printf("\n");
    }
}

/**
 * @brief Main example runner
 */
int main(void) {
    printf("=========================================\n");
    printf("Swing/Groove MIDI FX Module Examples\n");
    printf("=========================================\n\n");
    
    example_basic_swing();
    example_multi_track();
    example_custom_groove();
    example_time_based();
    example_tempo_change();
    example_resolutions();
    example_all_grooves();
    
    printf("All examples completed!\n");
    
    return 0;
}
