/**
 * @file quantizer_example.c
 * @brief Example usage of the Timing Quantizer module
 * 
 * This file demonstrates various quantizer configurations and use cases.
 * Compile with: gcc -I../.. quantizer_example.c quantizer.c -o quantizer_example
 */

#include "Services/quantizer/quantizer.h"
#include <stdio.h>

// Example 1: Basic quantization
void example_basic_quantization(void) {
    printf("\n=== Example 1: Basic Quantization ===\n");
    
    quantizer_init(120, 96);
    quantizer_set_enabled(0, 1);
    quantizer_set_resolution(0, QUANTIZER_RES_16TH);
    quantizer_set_strength(0, 100);
    
    // Test times slightly off grid
    uint32_t test_times[] = {1003, 1247, 1512, 1789};
    
    for (int i = 0; i < 4; i++) {
        uint32_t quantized = quantizer_calculate_time(0, test_times[i]);
        int32_t offset = quantizer_get_offset(0, test_times[i]);
        printf("Original: %u ms -> Quantized: %u ms (offset: %d ms)\n",
               test_times[i], quantized, offset);
    }
}

// Example 2: Soft quantization with groove
void example_soft_quantization(void) {
    printf("\n=== Example 2: Soft Quantization (70%% strength) ===\n");
    
    quantizer_set_enabled(1, 1);
    quantizer_set_resolution(1, QUANTIZER_RES_16TH);
    quantizer_set_strength(1, 70);  // Softer quantization
    quantizer_set_swing(1, 60);     // Add swing
    
    uint32_t test_times[] = {1003, 1247, 1512, 1789};
    
    for (int i = 0; i < 4; i++) {
        uint32_t quantized = quantizer_calculate_time(1, test_times[i]);
        int32_t offset = quantizer_get_offset(1, test_times[i]);
        printf("Original: %u ms -> Quantized: %u ms (offset: %d ms)\n",
               test_times[i], quantized, offset);
    }
}

// Example 3: Triplet feel
void example_triplet_feel(void) {
    printf("\n=== Example 3: 16th Note Triplet Feel ===\n");
    
    quantizer_set_enabled(2, 1);
    quantizer_set_resolution(2, QUANTIZER_RES_16TH_TRIPLET);
    quantizer_set_strength(2, 100);
    
    uint32_t interval = quantizer_get_grid_interval_ms(2);
    printf("Grid interval: %u ms\n", interval);
    
    uint32_t test_times[] = {1003, 1100, 1200, 1300};
    
    for (int i = 0; i < 4; i++) {
        uint32_t quantized = quantizer_calculate_time(2, test_times[i]);
        printf("Original: %u ms -> Quantized: %u ms\n",
               test_times[i], quantized);
    }
}

// Example 4: Late note handling modes
void example_late_note_modes(void) {
    printf("\n=== Example 4: Late Note Handling Modes ===\n");
    
    quantizer_set_enabled(0, 1);
    quantizer_set_resolution(0, QUANTIZER_RES_16TH);
    quantizer_set_strength(0, 100);
    
    uint32_t test_time = 1137;  // Slightly late
    uint32_t next_grid = quantizer_get_next_grid(0, test_time);
    uint32_t prev_grid = quantizer_get_prev_grid(0, test_time);
    
    printf("Test time: %u ms\n", test_time);
    printf("Previous grid: %u ms\n", prev_grid);
    printf("Next grid: %u ms\n", next_grid);
    
    // Test each mode
    quantizer_set_late_mode(0, QUANTIZER_LATE_SNAP_NEAREST);
    uint32_t nearest = quantizer_calculate_time(0, test_time);
    printf("Nearest mode: %u ms\n", nearest);
    
    quantizer_set_late_mode(0, QUANTIZER_LATE_SNAP_FORWARD);
    uint32_t forward = quantizer_calculate_time(0, test_time);
    printf("Forward mode: %u ms\n", forward);
    
    quantizer_set_late_mode(0, QUANTIZER_LATE_SNAP_BACKWARD);
    uint32_t backward = quantizer_calculate_time(0, test_time);
    printf("Backward mode: %u ms\n", backward);
}

// Example 5: Note buffering
void example_note_buffering(void) {
    printf("\n=== Example 5: Note Buffering ===\n");
    
    quantizer_init(120, 96);
    quantizer_set_enabled(0, 1);
    quantizer_set_resolution(0, QUANTIZER_RES_16TH);
    quantizer_set_strength(0, 100);
    
    // Buffer some notes
    quantizer_process_note_on(0, 60, 100, 0, 1003);
    quantizer_process_note_on(0, 64, 90, 0, 1248);
    quantizer_process_note_on(0, 67, 95, 0, 1513);
    
    uint8_t buffered;
    uint32_t total;
    int32_t avg_offset;
    quantizer_get_stats(0, &buffered, &total, &avg_offset);
    printf("Notes buffered: %u\n", buffered);
    printf("Total quantized: %u\n", total);
    printf("Average offset: %d ms\n", avg_offset);
    
    // Get ready notes
    quantizer_note_t ready_notes[QUANTIZER_MAX_NOTES_PER_TRACK];
    uint8_t ready_count = quantizer_get_ready_notes(0, 2000, ready_notes);
    
    printf("\nReady notes: %u\n", ready_count);
    for (uint8_t i = 0; i < ready_count; i++) {
        printf("  Note %u: vel=%u, orig=%u ms, quant=%u ms\n",
               ready_notes[i].note,
               ready_notes[i].velocity,
               ready_notes[i].original_time_ms,
               ready_notes[i].quantized_time_ms);
    }
}

// Example 6: Tick-based quantization
void example_tick_based(void) {
    printf("\n=== Example 6: Tick-Based Quantization ===\n");
    
    quantizer_init(120, 96);
    quantizer_set_enabled(0, 1);
    quantizer_set_resolution(0, QUANTIZER_RES_16TH);
    quantizer_set_strength(0, 100);
    
    uint32_t ticks_per_grid = quantizer_get_grid_interval_ticks(0);
    printf("Ticks per 16th note: %u\n", ticks_per_grid);
    
    // Test tick positions slightly off grid
    uint32_t test_ticks[] = {97, 143, 197, 241};
    
    for (int i = 0; i < 4; i++) {
        uint32_t quantized = quantizer_calculate_ticks(0, test_ticks[i]);
        printf("Original: %u ticks -> Quantized: %u ticks\n",
               test_ticks[i], quantized);
    }
}

// Example 7: Multi-track configuration
void example_multi_track(void) {
    printf("\n=== Example 7: Multi-Track Configuration ===\n");
    
    quantizer_init(120, 96);
    
    // Track 0: Tight drums (16th notes, hard snap)
    quantizer_set_enabled(0, 1);
    quantizer_set_resolution(0, QUANTIZER_RES_16TH);
    quantizer_set_strength(0, 100);
    quantizer_set_late_mode(0, QUANTIZER_LATE_SNAP_NEAREST);
    printf("Track 0 (Drums): %s, 100%% strength, %s\n",
           quantizer_get_resolution_name(QUANTIZER_RES_16TH),
           quantizer_get_late_mode_name(QUANTIZER_LATE_SNAP_NEAREST));
    
    // Track 1: Groovy bass (8th notes, soft snap, swing)
    quantizer_set_enabled(1, 1);
    quantizer_set_resolution(1, QUANTIZER_RES_8TH);
    quantizer_set_strength(1, 75);
    quantizer_set_swing(1, 60);
    printf("Track 1 (Bass): %s, 75%% strength, swing=%u\n",
           quantizer_get_resolution_name(QUANTIZER_RES_8TH),
           quantizer_get_swing(1));
    
    // Track 2: Triplet hi-hats
    quantizer_set_enabled(2, 1);
    quantizer_set_resolution(2, QUANTIZER_RES_16TH_TRIPLET);
    quantizer_set_strength(2, 90);
    printf("Track 2 (Hi-Hats): %s, 90%% strength\n",
           quantizer_get_resolution_name(QUANTIZER_RES_16TH_TRIPLET));
    
    // Track 3: Manual timing (disabled)
    quantizer_set_enabled(3, 0);
    printf("Track 3 (Lead): Disabled (manual timing)\n");
}

// Example 8: Grid checking
void example_grid_checking(void) {
    printf("\n=== Example 8: Grid Checking ===\n");
    
    quantizer_set_enabled(0, 1);
    quantizer_set_resolution(0, QUANTIZER_RES_16TH);
    
    uint32_t test_times[] = {1000, 1003, 1125, 1250, 1258};
    uint16_t tolerance = 5;  // 5ms tolerance
    
    for (int i = 0; i < 5; i++) {
        uint8_t on_grid = quantizer_is_on_grid(0, test_times[i], tolerance);
        printf("Time %u ms: %s\n", test_times[i], 
               on_grid ? "ON GRID" : "OFF GRID");
    }
}

int main(void) {
    printf("Timing Quantizer Module - Example Usage\n");
    printf("========================================\n");
    
    example_basic_quantization();
    example_soft_quantization();
    example_triplet_feel();
    example_late_note_modes();
    example_note_buffering();
    example_tick_based();
    example_multi_track();
    example_grid_checking();
    
    printf("\n=== All Examples Complete ===\n");
    return 0;
}
