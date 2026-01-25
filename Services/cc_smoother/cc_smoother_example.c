/**
 * @file cc_smoother_example.c
 * @brief Example usage of the CC Smoother module
 * 
 * This file demonstrates how to use the CC Smoother module in various scenarios.
 * Compile and integrate with your MidiCore application.
 */

#include "Services/cc_smoother/cc_smoother.h"
#include <stdio.h>

// Example MIDI output callback
static void midi_output_callback(uint8_t track, uint8_t cc_number, uint8_t value, uint8_t channel) {
    printf("Track %u, CC %u = %u (Ch %u)\n", track, cc_number, value, channel);
    
    // In a real application, send to MIDI output:
    // midi_send_cc(channel, cc_number, value);
}

/**
 * @brief Example 1: Basic setup and processing
 */
void example_basic_usage(void) {
    printf("\n=== Example 1: Basic Usage ===\n");
    
    // Initialize
    cc_smoother_init();
    
    // Enable smoothing for track 0
    cc_smoother_set_enabled(0, 1);
    cc_smoother_set_mode(0, CC_SMOOTH_MODE_MEDIUM);
    
    printf("Smoothing enabled: %u\n", cc_smoother_is_enabled(0));
    printf("Smoothing mode: %s\n", cc_smoother_get_mode_name(cc_smoother_get_mode(0)));
    
    // Process some CC messages
    uint8_t cc_number = 74;  // Filter cutoff
    
    // Simulate incoming CC values
    uint8_t input_values[] = {0, 30, 60, 90, 127, 90, 60, 30, 0};
    
    printf("\nProcessing CC %u:\n", cc_number);
    for (int i = 0; i < 9; i++) {
        uint8_t smoothed = cc_smoother_process(0, cc_number, input_values[i]);
        printf("  Input: %3u -> Smoothed: %3u\n", input_values[i], smoothed);
        
        // Simulate time passing (in real app, this is done by timer)
        for (int t = 0; t < 10; t++) {
            cc_smoother_tick_1ms();
        }
    }
}

/**
 * @brief Example 2: Custom configuration
 */
void example_custom_config(void) {
    printf("\n=== Example 2: Custom Configuration ===\n");
    
    cc_smoother_init();
    
    // Configure track 1 with custom settings
    uint8_t track = 1;
    cc_smoother_set_enabled(track, 1);
    cc_smoother_set_mode(track, CC_SMOOTH_MODE_CUSTOM);
    cc_smoother_set_amount(track, 80);        // 80% smoothing
    cc_smoother_set_attack(track, 40);        // 40ms attack
    cc_smoother_set_release(track, 120);      // 120ms release
    cc_smoother_set_slew_limit(track, 60);    // Max 60 units/ms change
    
    printf("Track %u configuration:\n", track);
    printf("  Mode: %s\n", cc_smoother_get_mode_name(cc_smoother_get_mode(track)));
    printf("  Amount: %u%%\n", cc_smoother_get_amount(track));
    printf("  Attack: %u ms\n", cc_smoother_get_attack(track));
    printf("  Release: %u ms\n", cc_smoother_get_release(track));
    printf("  Slew Limit: %u\n", cc_smoother_get_slew_limit(track));
    
    // Process a rapid CC change
    uint8_t cc_number = 7;  // Volume
    printf("\nRapid volume change (0 -> 127):\n");
    
    uint8_t smoothed = cc_smoother_process(track, cc_number, 127);
    printf("  Immediate: %u\n", smoothed);
    
    // Update over time to see smoothing
    for (int i = 0; i < 5; i++) {
        for (int t = 0; t < 20; t++) {
            cc_smoother_tick_1ms();
        }
        smoothed = cc_smoother_get_current_value(track, cc_number);
        printf("  After %d ms: %u\n", (i + 1) * 20, smoothed);
    }
}

/**
 * @brief Example 3: Selective CC smoothing
 */
void example_selective_smoothing(void) {
    printf("\n=== Example 3: Selective CC Smoothing ===\n");
    
    cc_smoother_init();
    
    uint8_t track = 0;
    cc_smoother_set_enabled(track, 1);
    cc_smoother_set_mode(track, CC_SMOOTH_MODE_LIGHT);
    
    // Disable smoothing for sustain pedal (CC 64) - it's binary
    cc_smoother_set_cc_enabled(track, 64, 0);
    
    // Keep smoothing enabled for modulation wheel (CC 1)
    cc_smoother_set_cc_enabled(track, 1, 1);
    
    printf("CC 1 (Mod Wheel) smoothing: %s\n", 
           cc_smoother_is_cc_enabled(track, 1) ? "Enabled" : "Disabled");
    printf("CC 64 (Sustain) smoothing: %s\n", 
           cc_smoother_is_cc_enabled(track, 64) ? "Enabled" : "Disabled");
    
    // Process both CCs
    printf("\nProcessing CC 1 (Mod Wheel): 0 -> 127\n");
    uint8_t mod_smoothed = cc_smoother_process(track, 1, 127);
    printf("  Result: %u (smoothed)\n", mod_smoothed);
    
    printf("Processing CC 64 (Sustain): 0 -> 127\n");
    uint8_t sustain_smoothed = cc_smoother_process(track, 64, 127);
    printf("  Result: %u (pass-through, no smoothing)\n", sustain_smoothed);
}

/**
 * @brief Example 4: Multiple tracks with different modes
 */
void example_multiple_tracks(void) {
    printf("\n=== Example 4: Multiple Tracks ===\n");
    
    cc_smoother_init();
    
    // Track 0: Lead synth - light smoothing
    cc_smoother_set_enabled(0, 1);
    cc_smoother_set_mode(0, CC_SMOOTH_MODE_LIGHT);
    
    // Track 1: Pad - heavy smoothing
    cc_smoother_set_enabled(1, 1);
    cc_smoother_set_mode(1, CC_SMOOTH_MODE_HEAVY);
    
    // Track 2: Bass - medium smoothing
    cc_smoother_set_enabled(2, 1);
    cc_smoother_set_mode(2, CC_SMOOTH_MODE_MEDIUM);
    
    // Track 3: Drums - no smoothing
    cc_smoother_set_enabled(3, 0);
    
    printf("Track configurations:\n");
    for (uint8_t t = 0; t < CC_SMOOTHER_MAX_TRACKS; t++) {
        printf("  Track %u: %s, Mode: %s\n", 
               t,
               cc_smoother_is_enabled(t) ? "Enabled" : "Disabled",
               cc_smoother_get_mode_name(cc_smoother_get_mode(t)));
    }
    
    // Process same CC value on different tracks
    uint8_t cc_number = 74;  // Filter cutoff
    uint8_t input_value = 100;
    
    printf("\nProcessing CC %u = %u on all tracks:\n", cc_number, input_value);
    for (uint8_t t = 0; t < CC_SMOOTHER_MAX_TRACKS; t++) {
        uint8_t smoothed = cc_smoother_process(t, cc_number, input_value);
        printf("  Track %u: %u\n", t, smoothed);
    }
}

/**
 * @brief Example 5: Using output callback
 */
void example_output_callback(void) {
    printf("\n=== Example 5: Output Callback ===\n");
    
    cc_smoother_init();
    
    // Set output callback
    cc_smoother_set_output_callback(midi_output_callback);
    
    // Enable smoothing
    cc_smoother_set_enabled(0, 1);
    cc_smoother_set_mode(0, CC_SMOOTH_MODE_MEDIUM);
    
    printf("Processing CC with automatic output:\n");
    
    // Process CC value
    cc_smoother_process(0, 74, 80);
    
    // Update tick - callback will be called when value changes
    printf("Updating...\n");
    for (int i = 0; i < 10; i++) {
        cc_smoother_tick_1ms();
    }
}

/**
 * @brief Example 6: Attack vs. Release behavior
 */
void example_attack_release(void) {
    printf("\n=== Example 6: Attack vs. Release ===\n");
    
    cc_smoother_init();
    
    uint8_t track = 0;
    cc_smoother_set_enabled(track, 1);
    cc_smoother_set_mode(track, CC_SMOOTH_MODE_CUSTOM);
    
    // Fast attack, slow release (good for volume)
    cc_smoother_set_attack(track, 20);     // Fast increase
    cc_smoother_set_release(track, 200);   // Slow decrease
    
    printf("Configuration: Attack=%u ms, Release=%u ms\n",
           cc_smoother_get_attack(track),
           cc_smoother_get_release(track));
    
    uint8_t cc_number = 7;  // Volume
    
    // Test attack (0 -> 127)
    printf("\nAttack phase (0 -> 127):\n");
    cc_smoother_reset_cc(track, cc_number);  // Start from 0
    cc_smoother_process(track, cc_number, 127);
    
    for (int i = 1; i <= 5; i++) {
        for (int t = 0; t < 10; t++) cc_smoother_tick_1ms();
        printf("  After %d ms: %u\n", i * 10, 
               cc_smoother_get_current_value(track, cc_number));
    }
    
    // Test release (127 -> 0)
    printf("\nRelease phase (127 -> 0):\n");
    cc_smoother_process(track, cc_number, 0);
    
    for (int i = 1; i <= 5; i++) {
        for (int t = 0; t < 40; t++) cc_smoother_tick_1ms();
        printf("  After %d ms: %u\n", i * 40, 
               cc_smoother_get_current_value(track, cc_number));
    }
}

/**
 * @brief Example 7: Slew rate limiting
 */
void example_slew_limiting(void) {
    printf("\n=== Example 7: Slew Rate Limiting ===\n");
    
    cc_smoother_init();
    
    uint8_t track = 0;
    cc_smoother_set_enabled(track, 1);
    cc_smoother_set_mode(track, CC_SMOOTH_MODE_CUSTOM);
    cc_smoother_set_amount(track, 50);
    cc_smoother_set_slew_limit(track, 10);  // Max 10 units per ms
    
    printf("Slew limit: %u units/ms\n", cc_smoother_get_slew_limit(track));
    
    uint8_t cc_number = 74;
    
    // Process large jump
    printf("\nLarge jump (0 -> 127) with slew limiting:\n");
    cc_smoother_reset_cc(track, cc_number);
    cc_smoother_process(track, cc_number, 127);
    
    for (int i = 1; i <= 10; i++) {
        for (int t = 0; t < 5; t++) cc_smoother_tick_1ms();
        uint8_t current = cc_smoother_get_current_value(track, cc_number);
        printf("  After %d ms: %u (change limited by slew rate)\n", i * 5, current);
        if (current >= 125) break;
    }
}

/**
 * @brief Main example runner
 */
int main(void) {
    printf("=================================================\n");
    printf("     CC Smoother Module - Usage Examples\n");
    printf("=================================================\n");
    
    example_basic_usage();
    example_custom_config();
    example_selective_smoothing();
    example_multiple_tracks();
    example_output_callback();
    example_attack_release();
    example_slew_limiting();
    
    printf("\n=================================================\n");
    printf("Examples complete!\n");
    printf("=================================================\n");
    
    return 0;
}
