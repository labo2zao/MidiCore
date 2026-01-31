#pragma once

/**
 * @brief Start the AIN MIDI task (legacy task-based architecture)
 * @deprecated Use ain_midi_process_events() with MIOS32-like main task instead
 */
void app_start_ain_midi_task(void);

/**
 * @brief Process AIN events and convert to MIDI (MIOS32-like service)
 * 
 * This function processes all pending AIN events and converts them to MIDI
 * messages. It should be called from the main task tick loop.
 * 
 * Non-blocking, bounded execution time.
 */
void ain_midi_process_events(void);
