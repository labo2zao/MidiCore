#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Metronome module - Synchronized to looper BPM
 * 
 * Provides a click track output synchronized to the looper transport.
 * Can output to MIDI or audio (if supported by hardware).
 */

// Metronome output mode
typedef enum {
  METRONOME_MODE_OFF = 0,
  METRONOME_MODE_MIDI,      // MIDI note output
  METRONOME_MODE_AUDIO      // Audio click (if supported)
} metronome_mode_t;

// Metronome configuration
typedef struct {
  uint8_t enabled;            // 1 = enabled, 0 = disabled
  uint8_t mode;               // metronome_mode_t
  uint8_t midi_channel;       // 0-15 (MIDI channel for note output)
  uint8_t accent_note;        // MIDI note for accented beat (downbeat)
  uint8_t regular_note;       // MIDI note for regular beat
  uint8_t accent_velocity;    // Velocity for accented beat (1-127)
  uint8_t regular_velocity;   // Velocity for regular beat (1-127)
  uint8_t output_port;        // MIDI output port/node
  uint8_t count_in_bars;      // Number of bars for count-in (0 = no count-in)
} metronome_config_t;

/**
 * @brief Initialize metronome module
 */
void metronome_init(void);

/**
 * @brief Set metronome configuration
 * @param config Pointer to configuration structure
 */
void metronome_set_config(const metronome_config_t* config);

/**
 * @brief Get current metronome configuration
 * @param out Pointer to output configuration structure
 */
void metronome_get_config(metronome_config_t* out);

/**
 * @brief Enable/disable metronome
 * @param enable 1 to enable, 0 to disable
 */
void metronome_set_enabled(uint8_t enable);

/**
 * @brief Get metronome enabled status
 * @return 1 if enabled, 0 if disabled
 */
uint8_t metronome_get_enabled(void);

/**
 * @brief Sync metronome to looper tempo and time signature
 * @param bpm Beats per minute (20-300)
 * @param ts_num Time signature numerator (e.g., 4 for 4/4)
 * @param ts_den Time signature denominator (e.g., 4 for 4/4)
 * 
 * Called automatically by looper when tempo changes.
 */
void metronome_sync_tempo(uint16_t bpm, uint8_t ts_num, uint8_t ts_den);

/**
 * @brief Update metronome (call from 1ms timer)
 * @param current_tick Current playback position in ticks (from looper)
 * @param is_playing 1 if looper is playing, 0 if stopped
 * 
 * Generates metronome clicks based on current position.
 */
void metronome_tick_1ms(uint32_t current_tick, uint8_t is_playing);

/**
 * @brief Start count-in (if configured)
 * 
 * Starts a count-in period before recording/playback.
 * Metronome will click for count_in_bars before signaling ready.
 */
void metronome_start_count_in(void);

/**
 * @brief Check if count-in is active
 * @return 1 if count-in active, 0 otherwise
 */
uint8_t metronome_is_count_in_active(void);

/**
 * @brief Cancel count-in
 */
void metronome_cancel_count_in(void);

#ifdef __cplusplus
}
#endif
