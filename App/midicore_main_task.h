/**
 * @file midicore_main_task.h
 * @brief MidiCore Main Task - Cooperative service-based architecture
 * 
 * This implements a single main task that calls service tick functions
 * cooperatively, following embedded best practices:
 * - ONE main task with deterministic periodic tick
 * - Logic lives in services, not tasks
 * - Services are non-blocking with bounded execution time
 * - Minimal FreeRTOS task overhead
 * 
 * @note This replaces multiple feature-specific tasks with a single
 *       cooperative execution model for better determinism and stack safety.
 */

#ifndef MIDICORE_MAIN_TASK_H
#define MIDICORE_MAIN_TASK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * ARCHITECTURE OVERVIEW
 * ============================================================================
 * 
 * MidiCore cooperative design with minimal task count:
 * 
 * REQUIRED TASKS:
 * - MidiCore_MainTask: Single main task, 1-2ms tick period
 *   - Calls all service tick functions cooperatively
 *   - Stack: 4096-6144 bytes
 *   - Priority: Normal
 * 
 * OPTIONAL TASKS (only if strictly justified):
 * - IO_Task: USB/MIDI buffering only (if needed for high-bandwidth I/O)
 * - InitTask: One-time initialization, deletes itself after
 * 
 * REMOVED TASKS (logic moved to services):
 * - AinTask → ain_service_tick()
 * - AinMidiTask → ain_midi_service_tick()
 * - OledDemoTask → ui_service_tick()
 * - CliTask → cli_service_tick() (processed every N ticks)
 * - CalibrationTask → calibration_service_tick() (state machine)
 * - PressureTask → pressure_service_tick()
 * - StackMonitorTask → stack_monitor_service_tick() (periodic only)
 * ============================================================================
 */

/* ============================================================================
 * CONFIGURATION
 * ============================================================================
 */

/** Main task tick period in milliseconds (1ms recommended for responsive MIDI) */
#define MIDICORE_MAIN_TICK_MS       1

/** Main task stack size in bytes */
#define MIDICORE_MAIN_STACK_SIZE    5120

/** Main task priority */
#define MIDICORE_MAIN_PRIORITY      osPriorityNormal

/* ============================================================================
 * SERVICE TICK INTERVALS (in main tick counts)
 * ============================================================================
 * 
 * Services are called at different rates based on their needs.
 * A tick count of N means the service is called every N milliseconds.
 */

/** AIN scanning interval (fast for responsive analog input) */
#define MIDICORE_TICK_AIN           5     /* Every 5ms */

/** Pressure sensor reading interval */
#define MIDICORE_TICK_PRESSURE      5     /* Every 5ms */

/** MIDI processing interval (matches USB MIDI frame rate) */
#define MIDICORE_TICK_MIDI          1     /* Every 1ms */

/** Expression/CC processing interval */
#define MIDICORE_TICK_EXPRESSION    1     /* Every 1ms */

/** UI/OLED update interval */
#define MIDICORE_TICK_UI            20    /* Every 20ms (50 Hz) */

/** CLI processing interval (lower priority) */
#define MIDICORE_TICK_CLI           5     /* Every 5ms */

/** Stack monitor interval (periodic diagnostics) */
#define MIDICORE_TICK_STACK_MON     5000  /* Every 5 seconds */

/** Watchdog kick interval */
#define MIDICORE_TICK_WATCHDOG      100   /* Every 100ms */

/* ============================================================================
 * PUBLIC API
 * ============================================================================
 */

/**
 * @brief Initialize and start the MidiCore main task
 * 
 * This creates the single main task that runs all services cooperatively.
 * Should be called from app_init_and_start() after all services are initialized.
 * 
 * @return 0 on success, -1 on failure
 */
int midicore_main_task_start(void);

/**
 * @brief Get main task tick counter
 * 
 * Returns the number of main loop iterations since startup.
 * Useful for timing and diagnostics.
 * 
 * @return Tick count (wraps at UINT32_MAX)
 */
uint32_t midicore_main_get_tick_count(void);

/**
 * @brief Check if main task is running
 * 
 * @return 1 if running, 0 if not started or stopped
 */
uint8_t midicore_main_is_running(void);

#ifdef __cplusplus
}
#endif

#endif /* MIDICORE_MAIN_TASK_H */
