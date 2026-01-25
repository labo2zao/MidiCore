/**
 * @file midi_filter_example.c
 * @brief MIDI Filter usage examples and test cases
 * 
 * This file demonstrates various use cases of the MIDI Filter module.
 * Compile with: gcc -I../.. midi_filter.c midi_filter_example.c -o midi_filter_test
 */

#include "midi_filter.h"
#include <stdio.h>

// Helper function to print filter results
static void print_test_result(const char* test_name, midi_filter_result_t result) {
    printf("%-50s: %s\n", test_name, 
           result == MIDI_FILTER_RESULT_PASS ? "PASS" : "BLOCK");
}

// Helper function to create MIDI status byte
static uint8_t make_status(uint8_t msg_type, uint8_t channel) {
    return msg_type | (channel & 0x0F);
}

/**
 * @brief Example 1: Basic message type filtering
 */
void example_message_type_filtering(void) {
    printf("\n=== Example 1: Message Type Filtering ===\n");
    
    // Initialize and enable filter for track 0
    midi_filter_init();
    midi_filter_set_enabled(0, 1);
    
    // Only allow Note On and Note Off
    midi_filter_set_allowed_messages(0, 
        MIDI_FILTER_MSG_NOTE_ON | MIDI_FILTER_MSG_NOTE_OFF);
    
    // Test various message types
    print_test_result("Note On (should PASS)", 
        midi_filter_test_message(0, 0x90, 60, 100));
    print_test_result("Note Off (should PASS)", 
        midi_filter_test_message(0, 0x80, 60, 0));
    print_test_result("CC (should BLOCK)", 
        midi_filter_test_message(0, 0xB0, 7, 127));
    print_test_result("Program Change (should BLOCK)", 
        midi_filter_test_message(0, 0xC0, 42, 0));
    print_test_result("Pitch Bend (should BLOCK)", 
        midi_filter_test_message(0, 0xE0, 0, 64));
}

/**
 * @brief Example 2: Channel filtering
 */
void example_channel_filtering(void) {
    printf("\n=== Example 2: Channel Filtering ===\n");
    
    midi_filter_init();
    midi_filter_set_enabled(0, 1);
    
    // Allow all message types
    midi_filter_set_allowed_messages(0, MIDI_FILTER_MSG_ALL);
    
    // Only allow channels 1-4 (0-3 in 0-indexed)
    midi_filter_set_channel_mode(0, MIDI_FILTER_CHANNEL_MODE_ALLOW);
    midi_filter_set_channel_mask(0, 0x0000);  // Start with no channels
    for (uint8_t ch = 0; ch < 4; ch++) {
        midi_filter_set_channel_enabled(0, ch, 1);
    }
    
    // Test messages on different channels
    print_test_result("Note on Channel 1 (should PASS)", 
        midi_filter_test_message(0, make_status(0x90, 0), 60, 100));
    print_test_result("Note on Channel 4 (should PASS)", 
        midi_filter_test_message(0, make_status(0x90, 3), 60, 100));
    print_test_result("Note on Channel 5 (should BLOCK)", 
        midi_filter_test_message(0, make_status(0x90, 4), 60, 100));
    print_test_result("Note on Channel 16 (should BLOCK)", 
        midi_filter_test_message(0, make_status(0x90, 15), 60, 100));
}

/**
 * @brief Example 3: Note range filtering
 */
void example_note_range_filtering(void) {
    printf("\n=== Example 3: Note Range Filtering ===\n");
    
    midi_filter_init();
    midi_filter_set_enabled(0, 1);
    
    // Allow all message types
    midi_filter_set_allowed_messages(0, MIDI_FILTER_MSG_ALL);
    
    // Only allow middle octave (C4-B4: notes 60-71)
    midi_filter_set_note_range_enabled(0, 1);
    midi_filter_set_note_range(0, 60, 71);
    
    // Test notes at various ranges
    print_test_result("Note 59/B3 (should BLOCK)", 
        midi_filter_test_message(0, 0x90, 59, 100));
    print_test_result("Note 60/C4 (should PASS)", 
        midi_filter_test_message(0, 0x90, 60, 100));
    print_test_result("Note 65/F4 (should PASS)", 
        midi_filter_test_message(0, 0x90, 65, 100));
    print_test_result("Note 71/B4 (should PASS)", 
        midi_filter_test_message(0, 0x90, 71, 100));
    print_test_result("Note 72/C5 (should BLOCK)", 
        midi_filter_test_message(0, 0x90, 72, 100));
}

/**
 * @brief Example 4: Velocity filtering
 */
void example_velocity_filtering(void) {
    printf("\n=== Example 4: Velocity Filtering ===\n");
    
    midi_filter_init();
    midi_filter_set_enabled(0, 1);
    
    // Allow all message types
    midi_filter_set_allowed_messages(0, MIDI_FILTER_MSG_ALL);
    
    // Only allow soft notes (velocity 1-50)
    midi_filter_set_velocity_range_enabled(0, 1);
    midi_filter_set_velocity_range(0, 1, 50);
    
    // Test various velocities
    print_test_result("Velocity 1 (should PASS)", 
        midi_filter_test_message(0, 0x90, 60, 1));
    print_test_result("Velocity 25 (should PASS)", 
        midi_filter_test_message(0, 0x90, 60, 25));
    print_test_result("Velocity 50 (should PASS)", 
        midi_filter_test_message(0, 0x90, 60, 50));
    print_test_result("Velocity 51 (should BLOCK)", 
        midi_filter_test_message(0, 0x90, 60, 51));
    print_test_result("Velocity 127 (should BLOCK)", 
        midi_filter_test_message(0, 0x90, 60, 127));
}

/**
 * @brief Example 5: CC filtering
 */
void example_cc_filtering(void) {
    printf("\n=== Example 5: CC Filtering ===\n");
    
    midi_filter_init();
    midi_filter_set_enabled(0, 1);
    
    // Allow all message types
    midi_filter_set_allowed_messages(0, MIDI_FILTER_MSG_ALL);
    
    // Enable CC filtering and block specific CCs
    midi_filter_set_cc_filter_enabled(0, 1);
    
    // Block volume (CC#7) and expression (CC#11)
    midi_filter_set_cc_enabled(0, 7, 0);
    midi_filter_set_cc_enabled(0, 11, 0);
    
    // Test various CCs
    print_test_result("CC#1/Modulation (should PASS)", 
        midi_filter_test_message(0, 0xB0, 1, 64));
    print_test_result("CC#7/Volume (should BLOCK)", 
        midi_filter_test_message(0, 0xB0, 7, 127));
    print_test_result("CC#10/Pan (should PASS)", 
        midi_filter_test_message(0, 0xB0, 10, 64));
    print_test_result("CC#11/Expression (should BLOCK)", 
        midi_filter_test_message(0, 0xB0, 11, 127));
    print_test_result("CC#64/Sustain (should PASS)", 
        midi_filter_test_message(0, 0xB0, 64, 127));
}

/**
 * @brief Example 6: Combined filters
 */
void example_combined_filters(void) {
    printf("\n=== Example 6: Combined Filters ===\n");
    
    midi_filter_init();
    midi_filter_set_enabled(0, 1);
    
    // Only allow Note On/Off messages
    midi_filter_set_allowed_messages(0, 
        MIDI_FILTER_MSG_NOTE_ON | MIDI_FILTER_MSG_NOTE_OFF);
    
    // Only channel 1
    midi_filter_set_channel_mode(0, MIDI_FILTER_CHANNEL_MODE_ALLOW);
    midi_filter_set_channel_mask(0, 0x0000);  // Start with no channels
    midi_filter_set_channel_enabled(0, 0, 1);
    
    // Note range: C3-C5 (48-72)
    midi_filter_set_note_range_enabled(0, 1);
    midi_filter_set_note_range(0, 48, 72);
    
    // Velocity range: 40-120
    midi_filter_set_velocity_range_enabled(0, 1);
    midi_filter_set_velocity_range(0, 40, 120);
    
    // Test various combinations
    print_test_result("Ch1, Note 60, Vel 80 (should PASS)", 
        midi_filter_test_message(0, make_status(0x90, 0), 60, 80));
    print_test_result("Ch1, Note 60, Vel 30 (should BLOCK - velocity)", 
        midi_filter_test_message(0, make_status(0x90, 0), 60, 30));
    print_test_result("Ch1, Note 30, Vel 80 (should BLOCK - note range)", 
        midi_filter_test_message(0, make_status(0x90, 0), 30, 80));
    print_test_result("Ch2, Note 60, Vel 80 (should BLOCK - channel)", 
        midi_filter_test_message(0, make_status(0x90, 1), 60, 80));
    print_test_result("Ch1, CC#7, Val 127 (should BLOCK - msg type)", 
        midi_filter_test_message(0, make_status(0xB0, 0), 7, 127));
}

/**
 * @brief Example 7: Multi-track setup
 */
void example_multi_track(void) {
    printf("\n=== Example 7: Multi-Track Setup ===\n");
    
    midi_filter_init();
    
    // Track 0: Only notes on channels 1-4
    midi_filter_set_enabled(0, 1);
    midi_filter_set_allowed_messages(0, 
        MIDI_FILTER_MSG_NOTE_ON | MIDI_FILTER_MSG_NOTE_OFF);
    midi_filter_set_channel_mode(0, MIDI_FILTER_CHANNEL_MODE_ALLOW);
    midi_filter_set_channel_mask(0, 0x000F);  // Channels 0-3
    
    // Track 1: Only CCs on channel 10
    midi_filter_set_enabled(1, 1);
    midi_filter_set_allowed_messages(1, MIDI_FILTER_MSG_CONTROL_CHANGE);
    midi_filter_set_channel_mode(1, MIDI_FILTER_CHANNEL_MODE_ALLOW);
    midi_filter_set_channel_mask(1, 0x0000);  // Start with no channels
    midi_filter_set_channel_enabled(1, 9, 1);  // Channel 10 (index 9)
    
    // Track 2: Block realtime messages
    midi_filter_set_enabled(2, 1);
    midi_filter_set_allowed_messages(2, MIDI_FILTER_MSG_ALL);
    midi_filter_set_message_enabled(2, MIDI_FILTER_MSG_CLOCK, 0);
    
    // Test on different tracks
    printf("Track 0 (notes ch 1-4 only):\n");
    print_test_result("  Note on Ch1", 
        midi_filter_test_message(0, make_status(0x90, 0), 60, 100));
    print_test_result("  Note on Ch5", 
        midi_filter_test_message(0, make_status(0x90, 4), 60, 100));
    
    printf("Track 1 (CCs ch 10 only):\n");
    print_test_result("  CC on Ch10", 
        midi_filter_test_message(1, make_status(0xB0, 9), 7, 127));
    print_test_result("  CC on Ch1", 
        midi_filter_test_message(1, make_status(0xB0, 0), 7, 127));
    
    printf("Track 2 (no clock):\n");
    print_test_result("  Note On", 
        midi_filter_test_message(2, 0x90, 60, 100));
    print_test_result("  MIDI Clock", 
        midi_filter_test_message(2, 0xF8, 0, 0));
}

/**
 * @brief Example 8: Block channel mode
 */
void example_block_channel_mode(void) {
    printf("\n=== Example 8: Block Channel Mode ===\n");
    
    midi_filter_init();
    midi_filter_set_enabled(0, 1);
    
    // Allow all message types
    midi_filter_set_allowed_messages(0, MIDI_FILTER_MSG_ALL);
    
    // Block channels 10 (drums) and 16
    midi_filter_set_channel_mode(0, MIDI_FILTER_CHANNEL_MODE_BLOCK);
    midi_filter_set_channel_mask(0, 0x0000);  // Start with no channels blocked
    midi_filter_set_channel_enabled(0, 9, 1);   // Block channel 10 (index 9)
    midi_filter_set_channel_enabled(0, 15, 1);  // Block channel 16 (index 15)
    
    // Test messages on different channels
    print_test_result("Note on Channel 1 (should PASS)", 
        midi_filter_test_message(0, make_status(0x90, 0), 60, 100));
    print_test_result("Note on Channel 9 (should PASS)", 
        midi_filter_test_message(0, make_status(0x90, 8), 60, 100));
    print_test_result("Note on Channel 10 (should BLOCK)", 
        midi_filter_test_message(0, make_status(0x90, 9), 60, 100));
    print_test_result("Note on Channel 16 (should BLOCK)", 
        midi_filter_test_message(0, make_status(0x90, 15), 60, 100));
}

/**
 * @brief Example 9: System and realtime messages
 */
void example_system_messages(void) {
    printf("\n=== Example 9: System and Realtime Messages ===\n");
    
    midi_filter_init();
    midi_filter_set_enabled(0, 1);
    
    // Block all realtime messages
    midi_filter_set_allowed_messages(0, 
        MIDI_FILTER_MSG_NOTE_ON | 
        MIDI_FILTER_MSG_NOTE_OFF | 
        MIDI_FILTER_MSG_CONTROL_CHANGE);
    
    // Test realtime messages
    print_test_result("MIDI Clock (should BLOCK)", 
        midi_filter_test_message(0, 0xF8, 0, 0));
    print_test_result("MIDI Start (should BLOCK)", 
        midi_filter_test_message(0, 0xFA, 0, 0));
    print_test_result("MIDI Continue (should BLOCK)", 
        midi_filter_test_message(0, 0xFB, 0, 0));
    print_test_result("MIDI Stop (should BLOCK)", 
        midi_filter_test_message(0, 0xFC, 0, 0));
    print_test_result("Active Sensing (should BLOCK)", 
        midi_filter_test_message(0, 0xFE, 0, 0));
    print_test_result("Note On (should PASS)", 
        midi_filter_test_message(0, 0x90, 60, 100));
}

/**
 * @brief Main function - run all examples
 */
int main(void) {
    printf("MIDI Filter Module - Examples and Tests\n");
    printf("========================================\n");
    
    example_message_type_filtering();
    example_channel_filtering();
    example_note_range_filtering();
    example_velocity_filtering();
    example_cc_filtering();
    example_combined_filters();
    example_multi_track();
    example_block_channel_mode();
    example_system_messages();
    
    printf("\n========================================\n");
    printf("All examples completed!\n");
    
    return 0;
}
