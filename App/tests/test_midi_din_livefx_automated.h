/**
 * @file test_midi_din_livefx_automated.h
 * @brief Automated test suite for MODULE_TEST_MIDI_DIN with LiveFX
 * 
 * This test suite validates all features of the MIDI DIN LiveFX module:
 * - MIDI I/O
 * - LiveFX transformations
 * - MIDI learn CC commands
 * - Channel filtering
 * - Preset save/load
 * - Velocity curves
 * - Note range limiting
 * - Statistics tracking
 * - Looper integration
 * - UI sync
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Test result structure
 */
typedef struct {
  uint32_t tests_run;
  uint32_t tests_passed;
  uint32_t tests_failed;
  uint32_t tests_skipped;
} test_result_t;

/**
 * @brief Run all automated tests for MIDI DIN LiveFX
 * @return Test results
 */
test_result_t test_midi_din_livefx_run_all(void);

/**
 * @brief Test basic MIDI I/O functionality
 * @return 0 on success, -1 on failure
 */
int test_midi_io_basic(void);

/**
 * @brief Test LiveFX transpose feature
 * @return 0 on success, -1 on failure
 */
int test_livefx_transpose(void);

/**
 * @brief Test LiveFX velocity scaling
 * @return 0 on success, -1 on failure
 */
int test_livefx_velocity_scale(void);

/**
 * @brief Test LiveFX force-to-scale
 * @return 0 on success, -1 on failure
 */
int test_livefx_force_to_scale(void);

/**
 * @brief Test MIDI channel filtering
 * @return 0 on success, -1 on failure
 */
int test_channel_filter(void);

/**
 * @brief Test preset save/load
 * @return 0 on success, -1 on failure
 */
int test_preset_save_load(void);

/**
 * @brief Test velocity curves
 * @return 0 on success, -1 on failure
 */
int test_velocity_curves(void);

/**
 * @brief Test note range limiting
 * @return 0 on success, -1 on failure
 */
int test_note_range_limiting(void);

/**
 * @brief Test looper integration
 * @return 0 on success, -1 on failure
 */
int test_looper_integration(void);

/**
 * @brief Test UI sync integration
 * @return 0 on success, -1 on failure
 */
int test_ui_sync(void);

/**
 * @brief Test statistics tracking
 * @return 0 on success, -1 on failure
 */
int test_statistics_tracking(void);

/**
 * @brief Test all MIDI learn CC commands
 * @return 0 on success, -1 on failure
 */
int test_midi_learn_commands(void);

/**
 * @brief Performance test - measure latency
 * @return Average latency in microseconds
 */
uint32_t test_performance_latency(void);

/**
 * @brief Stress test - process 1000 notes
 * @return 0 on success, -1 on failure
 */
int test_stress_processing(void);

#ifdef __cplusplus
}
#endif
