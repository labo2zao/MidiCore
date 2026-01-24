/**
 * @file channelizer_example.c
 * @brief Example usage of the MIDI Channelizer module
 * 
 * This file demonstrates various use cases and configuration patterns
 * for the Channelizer module.
 */

#include "Services/channelizer/channelizer.h"
#include <stdio.h>

// Mock MIDI send function for examples
static void midi_send(uint8_t status, uint8_t data1, uint8_t data2) {
    printf("MIDI Out: %02X %02X %02X\n", status, data1, data2);
}

/**
 * Example 1: Simple channel forcing
 * Force all incoming messages to channel 1
 */
void example_force_channel(void) {
    printf("\n=== Example 1: Force Channel ===\n");
    
    channelizer_init();
    
    // Configure track 0 to force all messages to channel 1
    channelizer_set_mode(0, CHANNELIZER_MODE_FORCE);
    channelizer_set_force_channel(0, 0); // Channel 1 (0-indexed)
    channelizer_set_enabled(0, 1);
    
    // Test with messages on different channels
    channelizer_output_t outputs[4];
    uint8_t count;
    
    printf("Input: Note On Ch 5, Note 60, Vel 100\n");
    count = channelizer_process(0, 0x94, 60, 100, outputs, 4);
    for (uint8_t i = 0; i < count; i++) {
        midi_send(outputs[i].status, outputs[i].data1, outputs[i].data2);
    }
    
    printf("Input: Note On Ch 10, Note 64, Vel 80\n");
    count = channelizer_process(0, 0x99, 64, 80, outputs, 4);
    for (uint8_t i = 0; i < count; i++) {
        midi_send(outputs[i].status, outputs[i].data1, outputs[i].data2);
    }
}

/**
 * Example 2: Channel remapping
 * Map specific input channels to different output channels
 */
void example_channel_remap(void) {
    printf("\n=== Example 2: Channel Remapping ===\n");
    
    channelizer_init();
    
    // Configure track 0 for channel remapping
    channelizer_set_mode(0, CHANNELIZER_MODE_REMAP);
    
    // Map input channels to output channels
    channelizer_set_channel_remap(0, 0, 5);  // Ch 1 -> Ch 6
    channelizer_set_channel_remap(0, 1, 6);  // Ch 2 -> Ch 7
    channelizer_set_channel_remap(0, 2, 7);  // Ch 3 -> Ch 8
    
    channelizer_set_enabled(0, 1);
    
    channelizer_output_t outputs[4];
    uint8_t count;
    
    printf("Input: Note On Ch 1, Note 60, Vel 100\n");
    count = channelizer_process(0, 0x90, 60, 100, outputs, 4);
    for (uint8_t i = 0; i < count; i++) {
        midi_send(outputs[i].status, outputs[i].data1, outputs[i].data2);
    }
    
    printf("Input: CC Ch 2, CC#7, Value 64\n");
    count = channelizer_process(0, 0xB1, 7, 64, outputs, 4);
    for (uint8_t i = 0; i < count; i++) {
        midi_send(outputs[i].status, outputs[i].data1, outputs[i].data2);
    }
}

/**
 * Example 3: Keyboard split with zones
 * Split keyboard into bass and treble sections
 */
void example_keyboard_split(void) {
    printf("\n=== Example 3: Keyboard Split (Zones) ===\n");
    
    channelizer_init();
    
    channelizer_zone_t zone;
    
    // Lower zone: C0-B3 (0-59) -> Channel 1 (bass)
    zone.enabled = 1;
    zone.note_min = 0;
    zone.note_max = 59;
    zone.output_channel = 0;
    zone.transpose = 0;
    channelizer_set_zone(0, 0, &zone);
    
    // Upper zone: C4-G9 (60-127) -> Channel 2 (lead), transpose up 1 octave
    zone.enabled = 1;
    zone.note_min = 60;
    zone.note_max = 127;
    zone.output_channel = 1;
    zone.transpose = 12;
    channelizer_set_zone(0, 1, &zone);
    
    channelizer_set_mode(0, CHANNELIZER_MODE_ZONE);
    channelizer_set_enabled(0, 1);
    
    channelizer_output_t outputs[4];
    uint8_t count;
    
    printf("Input: Note On Ch 1, Note 48 (C3 - lower zone)\n");
    count = channelizer_process(0, 0x90, 48, 100, outputs, 4);
    for (uint8_t i = 0; i < count; i++) {
        midi_send(outputs[i].status, outputs[i].data1, outputs[i].data2);
    }
    
    printf("Input: Note On Ch 1, Note 72 (C5 - upper zone)\n");
    count = channelizer_process(0, 0x90, 72, 100, outputs, 4);
    for (uint8_t i = 0; i < count; i++) {
        midi_send(outputs[i].status, outputs[i].data1, outputs[i].data2);
    }
}

/**
 * Example 4: Round-robin voice allocation
 * Distribute notes across multiple channels for polyphony
 */
void example_voice_rotation(void) {
    printf("\n=== Example 4: Voice Rotation ===\n");
    
    channelizer_init();
    
    // Rotate through 4 channels for polyphonic allocation
    uint8_t channels[] = {0, 1, 2, 3};
    channelizer_set_rotate_channels(0, channels, 4);
    
    // Configure voice stealing
    channelizer_set_voice_steal_mode(0, CHANNELIZER_VOICE_STEAL_OLDEST);
    channelizer_set_voice_limit(0, 4);
    
    channelizer_set_mode(0, CHANNELIZER_MODE_ROTATE);
    channelizer_set_enabled(0, 1);
    
    channelizer_output_t outputs[4];
    uint8_t count;
    
    // Play 4 notes - each should go to a different channel
    printf("Playing C, E, G, C (chord)\n");
    
    printf("Input: Note On Note 60 (C)\n");
    count = channelizer_process(0, 0x90, 60, 100, outputs, 4);
    for (uint8_t i = 0; i < count; i++) {
        midi_send(outputs[i].status, outputs[i].data1, outputs[i].data2);
    }
    
    printf("Input: Note On Note 64 (E)\n");
    count = channelizer_process(0, 0x90, 64, 100, outputs, 4);
    for (uint8_t i = 0; i < count; i++) {
        midi_send(outputs[i].status, outputs[i].data1, outputs[i].data2);
    }
    
    printf("Input: Note On Note 67 (G)\n");
    count = channelizer_process(0, 0x90, 67, 100, outputs, 4);
    for (uint8_t i = 0; i < count; i++) {
        midi_send(outputs[i].status, outputs[i].data1, outputs[i].data2);
    }
    
    printf("Input: Note On Note 72 (C)\n");
    count = channelizer_process(0, 0x90, 72, 100, outputs, 4);
    for (uint8_t i = 0; i < count; i++) {
        midi_send(outputs[i].status, outputs[i].data1, outputs[i].data2);
    }
    
    printf("\nActive voices: %d\n", channelizer_get_active_voice_count(0));
    
    // Release one note
    printf("\nInput: Note Off Note 60\n");
    count = channelizer_process(0, 0x80, 60, 0, outputs, 4);
    for (uint8_t i = 0; i < count; i++) {
        midi_send(outputs[i].status, outputs[i].data1, outputs[i].data2);
    }
    
    printf("Active voices: %d\n", channelizer_get_active_voice_count(0));
}

/**
 * Example 5: Input channel filtering
 * Only process specific input channels
 */
void example_input_filtering(void) {
    printf("\n=== Example 5: Input Channel Filtering ===\n");
    
    channelizer_init();
    
    // Only process channels 1, 2, 3 (mask: 0x0007)
    channelizer_set_input_channel_mask(0, 0x0007);
    
    channelizer_set_mode(0, CHANNELIZER_MODE_FORCE);
    channelizer_set_force_channel(0, 0);
    channelizer_set_enabled(0, 1);
    
    channelizer_output_t outputs[4];
    uint8_t count;
    
    printf("Input: Note On Ch 1, Note 60 (should pass)\n");
    count = channelizer_process(0, 0x90, 60, 100, outputs, 4);
    printf("Output count: %d\n", count);
    for (uint8_t i = 0; i < count; i++) {
        midi_send(outputs[i].status, outputs[i].data1, outputs[i].data2);
    }
    
    printf("\nInput: Note On Ch 5, Note 64 (should be filtered)\n");
    count = channelizer_process(0, 0x94, 64, 100, outputs, 4);
    printf("Output count: %d\n", count);
    for (uint8_t i = 0; i < count; i++) {
        midi_send(outputs[i].status, outputs[i].data1, outputs[i].data2);
    }
}

/**
 * Example 6: Multi-zone layering
 * Create overlapping zones for layered sounds
 */
void example_multi_zone_layering(void) {
    printf("\n=== Example 6: Multi-Zone Layering ===\n");
    
    channelizer_init();
    
    channelizer_zone_t zone;
    
    // Zone 1: Full keyboard -> Channel 1 (pad)
    zone.enabled = 1;
    zone.note_min = 0;
    zone.note_max = 127;
    zone.output_channel = 0;
    zone.transpose = 0;
    channelizer_set_zone(0, 0, &zone);
    
    // Note: Current implementation uses first matching zone only
    // This example shows the configuration pattern for potential multi-zone support
    
    channelizer_set_mode(0, CHANNELIZER_MODE_ZONE);
    channelizer_set_enabled(0, 1);
    
    printf("Zone configuration allows for complex routing patterns\n");
    printf("with independent transpose and channel assignments per zone.\n");
}

/**
 * Example 7: Voice stealing demonstration
 */
void example_voice_stealing(void) {
    printf("\n=== Example 7: Voice Stealing ===\n");
    
    channelizer_init();
    
    // Set up rotation with only 2 voices
    uint8_t channels[] = {0, 1};
    channelizer_set_rotate_channels(0, channels, 2);
    channelizer_set_voice_limit(0, 2);
    
    // Use "quietest" stealing algorithm
    channelizer_set_voice_steal_mode(0, CHANNELIZER_VOICE_STEAL_QUIETEST);
    
    channelizer_set_mode(0, CHANNELIZER_MODE_ROTATE);
    channelizer_set_enabled(0, 1);
    
    channelizer_output_t outputs[4];
    uint8_t count;
    
    printf("Play 2 notes (fills voice table)\n");
    count = channelizer_process(0, 0x90, 60, 100, outputs, 4); // Loud
    count = channelizer_process(0, 0x90, 64, 50, outputs, 4);  // Quiet
    
    printf("Active voices: %d\n", channelizer_get_active_voice_count(0));
    
    printf("\nPlay 3rd note (should steal quietest voice)\n");
    count = channelizer_process(0, 0x90, 67, 80, outputs, 4);
    printf("Outputs: %d (1 note off + 1 note on)\n", count);
    for (uint8_t i = 0; i < count; i++) {
        midi_send(outputs[i].status, outputs[i].data1, outputs[i].data2);
    }
}

/**
 * Main example runner
 */
int main(void) {
    printf("==============================================\n");
    printf("  MIDI Channelizer Module - Usage Examples\n");
    printf("==============================================\n");
    
    example_force_channel();
    example_channel_remap();
    example_keyboard_split();
    example_voice_rotation();
    example_input_filtering();
    example_multi_zone_layering();
    example_voice_stealing();
    
    printf("\n==============================================\n");
    printf("  Examples Complete\n");
    printf("==============================================\n");
    
    return 0;
}
