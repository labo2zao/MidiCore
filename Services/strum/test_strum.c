/**
 * @file test_strum.c
 * @brief Unit tests for strum module
 */

#include "strum.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define TEST_PASS() printf("  ✓ PASS\n")
#define TEST_FAIL(msg) printf("  ✗ FAIL: %s\n", msg)

void test_init(void) {
    printf("Test: Initialization\n");
    strum_init();
    
    for (uint8_t t = 0; t < 4; t++) {
        assert(strum_is_enabled(t) == 0);
        assert(strum_get_time(t) == 30);
        assert(strum_get_direction(t) == STRUM_DIR_DOWN);
        assert(strum_get_velocity_ramp(t) == STRUM_RAMP_NONE);
        assert(strum_get_ramp_amount(t) == 20);
    }
    TEST_PASS();
}

void test_enable_disable(void) {
    printf("Test: Enable/Disable\n");
    strum_init();
    
    strum_set_enabled(0, 1);
    assert(strum_is_enabled(0) == 1);
    
    strum_set_enabled(0, 0);
    assert(strum_is_enabled(0) == 0);
    
    TEST_PASS();
}

void test_time_setting(void) {
    printf("Test: Time Setting\n");
    strum_init();
    
    strum_set_time(0, 50);
    assert(strum_get_time(0) == 50);
    
    strum_set_time(0, 250);
    assert(strum_get_time(0) == 200);
    
    TEST_PASS();
}

void test_direction_up(void) {
    printf("Test: Direction Up\n");
    strum_init();
    strum_set_enabled(0, 1);
    strum_set_time(0, 60);
    strum_set_direction(0, STRUM_DIR_UP);
    
    uint8_t chord[] = {60, 64, 67};
    uint8_t delay, velocity;
    
    strum_process_note(0, 60, 100, chord, 3, &delay, &velocity);
    assert(delay == 0);
    
    strum_process_note(0, 64, 100, chord, 3, &delay, &velocity);
    assert(delay == 30);
    
    strum_process_note(0, 67, 100, chord, 3, &delay, &velocity);
    assert(delay == 60);
    
    TEST_PASS();
}

void test_direction_down(void) {
    printf("Test: Direction Down\n");
    strum_init();
    strum_set_enabled(0, 1);
    strum_set_time(0, 60);
    strum_set_direction(0, STRUM_DIR_DOWN);
    
    uint8_t chord[] = {60, 64, 67};
    uint8_t delay, velocity;
    
    strum_process_note(0, 60, 100, chord, 3, &delay, &velocity);
    assert(delay == 60);
    
    strum_process_note(0, 64, 100, chord, 3, &delay, &velocity);
    assert(delay == 30);
    
    strum_process_note(0, 67, 100, chord, 3, &delay, &velocity);
    assert(delay == 0);
    
    TEST_PASS();
}

void test_velocity_ramp_increase(void) {
    printf("Test: Velocity Ramp Increase\n");
    strum_init();
    strum_set_enabled(0, 1);
    strum_set_time(0, 60);
    strum_set_direction(0, STRUM_DIR_UP);
    strum_set_velocity_ramp(0, STRUM_RAMP_INCREASE);
    strum_set_ramp_amount(0, 20);
    
    uint8_t chord[] = {60, 64, 67};
    uint8_t delay, velocity;
    
    strum_process_note(0, 60, 100, chord, 3, &delay, &velocity);
    assert(velocity == 80);
    
    strum_process_note(0, 64, 100, chord, 3, &delay, &velocity);
    assert(velocity == 100);
    
    strum_process_note(0, 67, 100, chord, 3, &delay, &velocity);
    assert(velocity == 120);
    
    TEST_PASS();
}

void test_velocity_ramp_decrease(void) {
    printf("Test: Velocity Ramp Decrease\n");
    strum_init();
    strum_set_enabled(0, 1);
    strum_set_time(0, 60);
    strum_set_direction(0, STRUM_DIR_UP);
    strum_set_velocity_ramp(0, STRUM_RAMP_DECREASE);
    strum_set_ramp_amount(0, 20);
    
    uint8_t chord[] = {60, 64, 67};
    uint8_t delay, velocity;
    
    strum_process_note(0, 60, 100, chord, 3, &delay, &velocity);
    assert(velocity == 120);
    
    strum_process_note(0, 67, 100, chord, 3, &delay, &velocity);
    assert(velocity == 80);
    
    TEST_PASS();
}

void test_single_note_passthrough(void) {
    printf("Test: Single Note Passthrough\n");
    strum_init();
    strum_set_enabled(0, 1);
    strum_set_time(0, 60);
    
    uint8_t chord[] = {60};
    uint8_t delay, velocity;
    
    strum_process_note(0, 60, 100, chord, 1, &delay, &velocity);
    assert(delay == 0);
    assert(velocity == 100);
    
    TEST_PASS();
}

void test_disabled_passthrough(void) {
    printf("Test: Disabled Passthrough\n");
    strum_init();
    strum_set_enabled(0, 0);
    strum_set_time(0, 60);
    
    uint8_t chord[] = {60, 64, 67};
    uint8_t delay, velocity;
    
    strum_process_note(0, 60, 100, chord, 3, &delay, &velocity);
    assert(delay == 0);
    assert(velocity == 100);
    
    TEST_PASS();
}

void test_multi_track(void) {
    printf("Test: Multi-Track Independence\n");
    strum_init();
    
    strum_set_enabled(0, 1);
    strum_set_time(0, 40);
    strum_set_direction(0, STRUM_DIR_UP);
    
    strum_set_enabled(1, 1);
    strum_set_time(1, 80);
    strum_set_direction(1, STRUM_DIR_DOWN);
    
    uint8_t chord[] = {60, 64, 67};
    uint8_t delay0, delay1, vel;
    
    strum_process_note(0, 67, 100, chord, 3, &delay0, &vel);
    strum_process_note(1, 67, 100, chord, 3, &delay1, &vel);
    
    assert(delay0 == 40);
    assert(delay1 == 0);
    
    TEST_PASS();
}

void test_boundary_conditions(void) {
    printf("Test: Boundary Conditions\n");
    strum_init();
    
    strum_set_enabled(5, 1);
    assert(strum_is_enabled(5) == 0);
    
    strum_set_time(0, 250);
    assert(strum_get_time(0) == 200);
    
    strum_set_ramp_amount(0, 150);
    assert(strum_get_ramp_amount(0) == 100);
    
    TEST_PASS();
}

void test_string_functions(void) {
    printf("Test: String Functions\n");
    
    const char* name = strum_get_direction_name(STRUM_DIR_UP);
    assert(strcmp(name, "Up") == 0);
    
    name = strum_get_ramp_name(STRUM_RAMP_INCREASE);
    assert(strcmp(name, "Increase") == 0);
    
    TEST_PASS();
}

int main(void) {
    printf("========================================\n");
    printf("  Strum Module Unit Tests\n");
    printf("========================================\n\n");
    
    test_init();
    test_enable_disable();
    test_time_setting();
    test_direction_up();
    test_direction_down();
    test_velocity_ramp_increase();
    test_velocity_ramp_decrease();
    test_single_note_passthrough();
    test_disabled_passthrough();
    test_multi_track();
    test_boundary_conditions();
    test_string_functions();
    
    printf("\n========================================\n");
    printf("  All tests passed! ✓\n");
    printf("========================================\n");
    
    return 0;
}
