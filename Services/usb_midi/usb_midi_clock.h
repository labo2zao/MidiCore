#pragma once
#include <stdint.h>

/**
 * @file usb_midi_clock.h
 * @brief USB MIDI Clock synchronization support
 * 
 * Provides MIDI Clock (0xF8) generation and reception with BPM tracking.
 * Supports Start (0xFA), Stop (0xFC), and Continue (0xFB) messages.
 * 
 * MIDI Clock timing:
 * - 24 clock pulses per quarter note (PPQN = 24)
 * - At 120 BPM: 48 clocks/second = 20.833ms per clock
 * - At 60 BPM: 24 clocks/second = 41.667ms per clock
 */

#ifdef __cplusplus
extern "C" {
#endif

// Clock source configuration
typedef enum {
  MIDI_CLOCK_SOURCE_INTERNAL = 0,  // Generate clock internally
  MIDI_CLOCK_SOURCE_EXTERNAL = 1,  // Sync to external clock (USB, DIN)
  MIDI_CLOCK_SOURCE_OFF = 2        // Clock disabled
} midi_clock_source_t;

// Clock state
typedef enum {
  MIDI_CLOCK_STOPPED = 0,
  MIDI_CLOCK_PLAYING = 1,
  MIDI_CLOCK_PAUSED = 2
} midi_clock_state_t;

// MIDI Clock configuration
typedef struct {
  uint8_t source;                  // midi_clock_source_t
  uint8_t send_clock;              // 1 = transmit clock, 0 = don't transmit
  uint8_t send_transport;          // 1 = transmit Start/Stop/Continue, 0 = don't
  uint8_t cable;                   // USB MIDI cable (0-3) for output
  uint16_t internal_bpm;           // BPM for internal clock (20-300)
  uint8_t reserved[2];
} midi_clock_config_t;

// Clock statistics
typedef struct {
  uint32_t clock_count;            // Total clock messages received/sent
  uint16_t detected_bpm;           // BPM detected from external clock (0 = no sync)
  uint16_t jitter_us;              // Timing jitter in microseconds
  uint32_t last_clock_time_us;    // Timestamp of last clock (microseconds)
  uint8_t state;                   // midi_clock_state_t
  uint8_t is_synced;               // 1 = locked to external clock, 0 = not synced
  uint8_t reserved[2];
} midi_clock_stats_t;

/**
 * @brief Initialize MIDI Clock module
 */
void midi_clock_init(void);

/**
 * @brief Set clock configuration
 * @param config Pointer to configuration structure
 */
void midi_clock_set_config(const midi_clock_config_t* config);

/**
 * @brief Get current clock configuration
 * @param out Pointer to output configuration structure
 */
void midi_clock_get_config(midi_clock_config_t* out);

/**
 * @brief Get clock statistics
 * @param out Pointer to output statistics structure
 */
void midi_clock_get_stats(midi_clock_stats_t* out);

/**
 * @brief Send MIDI Start message (0xFA)
 */
void midi_clock_send_start(void);

/**
 * @brief Send MIDI Stop message (0xFC)
 */
void midi_clock_send_stop(void);

/**
 * @brief Send MIDI Continue message (0xFB)
 */
void midi_clock_send_continue(void);

/**
 * @brief Send MIDI Clock message (0xF8)
 * 
 * Only sends if configured to transmit clock.
 * Called automatically by internal clock generator.
 */
void midi_clock_send_tick(void);

/**
 * @brief Process received MIDI Clock message (0xF8)
 * 
 * Updates BPM detection and synchronization.
 * Called from USB MIDI RX path.
 */
void midi_clock_on_rx_clock(void);

/**
 * @brief Process received MIDI Start message (0xFA)
 */
void midi_clock_on_rx_start(void);

/**
 * @brief Process received MIDI Stop message (0xFC)
 */
void midi_clock_on_rx_stop(void);

/**
 * @brief Process received MIDI Continue message (0xFB)
 */
void midi_clock_on_rx_continue(void);

/**
 * @brief Update clock (call from 1ms timer)
 * 
 * Generates internal clock if configured.
 * Updates BPM tracking and jitter measurements.
 */
void midi_clock_tick_1ms(void);

/**
 * @brief Reset clock position to zero
 * 
 * Resets internal clock counter without sending Stop.
 * Used for re-synchronization.
 */
void midi_clock_reset_position(void);

/**
 * @brief Get current clock position in ticks (24 PPQN)
 * @return Clock position (0-based, wraps at max uint32)
 */
uint32_t midi_clock_get_position(void);

#ifdef __cplusplus
}
#endif
