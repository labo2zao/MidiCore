/**
 * @file strum_example.c
 * @brief Example usage of the strum module
 * 
 * Demonstrates various strum configurations and use cases.
 * This is a reference implementation, not compiled into the main project.
 */

#include "strum.h"
#include <stdio.h>

void print_separator(void) {
    printf("----------------------------------------\n");
}

void example_basic_usage(void) {
    printf("Example 1: Basic Guitar Downstroke\n");
    print_separator();
    
    strum_init();
    
    strum_set_enabled(0, 1);
    strum_set_time(0, 30);
    strum_set_direction(0, STRUM_DIR_DOWN);
    
    uint8_t chord[] = {60, 64, 67};
    uint8_t chord_size = 3;
    
    printf("C Major Chord (C, E, G) with 30ms strum time:\n");
    for (uint8_t i = 0; i < chord_size; i++) {
        uint8_t delay, velocity;
        strum_process_note(0, chord[i], 100, chord, chord_size, &delay, &velocity);
        printf("  Note %3d: delay=%3dms, velocity=%3d\n", chord[i], delay, velocity);
    }
    printf("\n");
}

void example_velocity_ramping(void) {
    printf("Example 2: Velocity Ramping\n");
    print_separator();
    
    strum_init();
    
    strum_set_enabled(0, 1);
    strum_set_time(0, 40);
    strum_set_direction(0, STRUM_DIR_UP);
    strum_set_velocity_ramp(0, STRUM_RAMP_INCREASE);
    strum_set_ramp_amount(0, 30);
    
    uint8_t chord[] = {48, 52, 55, 60};
    uint8_t chord_size = 4;
    
    printf("C Minor 7 Chord with increasing velocity (30%% ramp):\n");
    for (uint8_t i = 0; i < chord_size; i++) {
        uint8_t delay, velocity;
        strum_process_note(0, chord[i], 100, chord, chord_size, &delay, &velocity);
        printf("  Note %3d: delay=%3dms, velocity=%3d\n", chord[i], delay, velocity);
    }
    printf("\n");
}

void example_alternating_direction(void) {
    printf("Example 3: Alternating Up-Down Direction\n");
    print_separator();
    
    strum_init();
    
    strum_set_enabled(0, 1);
    strum_set_time(0, 25);
    strum_set_direction(0, STRUM_DIR_UP_DOWN);
    
    uint8_t chord[] = {60, 64, 67};
    uint8_t chord_size = 3;
    
    printf("First strum (should go up):\n");
    for (uint8_t i = 0; i < chord_size; i++) {
        uint8_t delay, velocity;
        strum_process_note(0, chord[i], 100, chord, chord_size, &delay, &velocity);
        printf("  Note %3d: delay=%3dms\n", chord[i], delay);
    }
    
    printf("\nSecond strum (should go down):\n");
    strum_reset(0);
    for (uint8_t i = 0; i < chord_size; i++) {
        uint8_t delay, velocity;
        strum_process_note(0, chord[i], 100, chord, chord_size, &delay, &velocity);
        printf("  Note %3d: delay=%3dms\n", chord[i], delay);
    }
    printf("\n");
}

void example_multi_track(void) {
    printf("Example 4: Multi-Track Configuration\n");
    print_separator();
    
    strum_init();
    
    strum_set_enabled(0, 1);
    strum_set_time(0, 20);
    strum_set_direction(0, STRUM_DIR_DOWN);
    
    strum_set_enabled(1, 1);
    strum_set_time(1, 50);
    strum_set_direction(1, STRUM_DIR_UP);
    
    uint8_t chord[] = {60, 64, 67, 72};
    uint8_t chord_size = 4;
    
    printf("Track 0 (fast downstroke):\n");
    for (uint8_t i = 0; i < chord_size; i++) {
        uint8_t delay, velocity;
        strum_process_note(0, chord[i], 100, chord, chord_size, &delay, &velocity);
        printf("  Note %3d: delay=%3dms\n", chord[i], delay);
    }
    
    printf("\nTrack 1 (slow upstroke):\n");
    for (uint8_t i = 0; i < chord_size; i++) {
        uint8_t delay, velocity;
        strum_process_note(1, chord[i], 100, chord, chord_size, &delay, &velocity);
        printf("  Note %3d: delay=%3dms\n", chord[i], delay);
    }
    printf("\n");
}

void example_configuration_queries(void) {
    printf("Example 5: Configuration Queries\n");
    print_separator();
    
    strum_init();
    
    strum_set_enabled(0, 1);
    strum_set_time(0, 45);
    strum_set_direction(0, STRUM_DIR_UP_DOWN);
    strum_set_velocity_ramp(0, STRUM_RAMP_INCREASE);
    strum_set_ramp_amount(0, 25);
    
    printf("Track 0 Configuration:\n");
    printf("  Enabled: %s\n", strum_is_enabled(0) ? "Yes" : "No");
    printf("  Time: %dms\n", strum_get_time(0));
    printf("  Direction: %s\n", strum_get_direction_name(strum_get_direction(0)));
    printf("  Velocity Ramp: %s\n", strum_get_ramp_name(strum_get_velocity_ramp(0)));
    printf("  Ramp Amount: %d%%\n", strum_get_ramp_amount(0));
    printf("\n");
}

void example_edge_cases(void) {
    printf("Example 6: Edge Cases\n");
    print_separator();
    
    strum_init();
    strum_set_enabled(0, 1);
    strum_set_time(0, 40);
    
    printf("Single note (should pass through unchanged):\n");
    uint8_t single_note[] = {60};
    uint8_t delay, velocity;
    strum_process_note(0, 60, 100, single_note, 1, &delay, &velocity);
    printf("  Note 60: delay=%dms, velocity=%d\n", delay, velocity);
    
    printf("\nZero strum time (all notes simultaneous):\n");
    strum_set_time(0, 0);
    uint8_t chord[] = {60, 64, 67};
    for (uint8_t i = 0; i < 3; i++) {
        strum_process_note(0, chord[i], 100, chord, 3, &delay, &velocity);
        printf("  Note %d: delay=%dms\n", chord[i], delay);
    }
    
    printf("\nDisabled strum (should pass through):\n");
    strum_set_enabled(0, 0);
    strum_set_time(0, 40);
    strum_process_note(0, 60, 100, chord, 3, &delay, &velocity);
    printf("  Note 60: delay=%dms, velocity=%d\n", delay, velocity);
    printf("\n");
}

int main(void) {
    printf("========================================\n");
    printf("  Strum Module Example Demonstrations\n");
    printf("========================================\n\n");
    
    example_basic_usage();
    example_velocity_ramping();
    example_alternating_direction();
    example_multi_track();
    example_configuration_queries();
    example_edge_cases();
    
    printf("========================================\n");
    printf("  All examples completed successfully\n");
    printf("========================================\n");
    
    return 0;
}
