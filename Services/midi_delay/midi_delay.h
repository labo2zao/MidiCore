/**
 * @file midi_delay.h
 * @brief MIDI delay/echo effect with tempo sync and feedback
 * 
 * Repeats MIDI notes with tempo-synced delay time and adjustable feedback.
 * Can be disabled with MODULE_ENABLE_MIDI_DELAY_FX=0 to save 3KB RAM.
 * Most modern synths have built-in delay effects, so this may not be needed.
 */

#pragma once
#include <stdint.h>
#include "Config/module_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MIDI_DELAY_MAX_TRACKS 4

// Maximum delayed events per track (configurable for memory optimization)
// Each event: ~12 bytes
// Default: 128 events = 6KB total (128 * 12 * 4 tracks ≈ 6KB)
// Reduced: 64 events = 3KB total (saves 3KB RAM)
#ifndef MIDI_DELAY_MAX_EVENTS
  #define MIDI_DELAY_MAX_EVENTS 64  // Reduced from 128 to save 3KB RAM
#endif

/**
 * @brief Delay time divisions
 */
typedef enum {
    DELAY_DIV_1_64 = 0,   // 1/64 note
    DELAY_DIV_1_32,       // 1/32 note
    DELAY_DIV_1_16,       // 1/16 note
    DELAY_DIV_1_8,        // 1/8 note
    DELAY_DIV_1_4,        // 1/4 note
    DELAY_DIV_1_2,        // 1/2 note
    DELAY_DIV_1_1,        // Whole note
    DELAY_DIV_1_16T,      // 1/16 triplet
    DELAY_DIV_1_8T,       // 1/8 triplet
    DELAY_DIV_1_4T,       // 1/4 triplet
    DELAY_DIV_1_16D,      // 1/16 dotted
    DELAY_DIV_1_8D,       // 1/8 dotted
    DELAY_DIV_1_4D,       // 1/4 dotted
    DELAY_DIV_COUNT
} midi_delay_division_t;

/**
 * @brief Callback for outputting delayed notes (set by user)
 * @param track Track index
 * @param note MIDI note
 * @param velocity Velocity
 * @param channel MIDI channel
 * @param is_note_on 1 for note on, 0 for note off
 */
typedef void (*midi_delay_output_cb_t)(uint8_t track, uint8_t note, uint8_t velocity, 
                                       uint8_t channel, uint8_t is_note_on);

#if MODULE_ENABLE_MIDI_DELAY_FX

// Full implementation available
void midi_delay_init(uint16_t tempo);
void midi_delay_set_tempo(uint16_t tempo);
void midi_delay_tick_1ms(void);
void midi_delay_set_enabled(uint8_t track, uint8_t enabled);
uint8_t midi_delay_is_enabled(uint8_t track);
void midi_delay_set_division(uint8_t track, midi_delay_division_t division);
midi_delay_division_t midi_delay_get_division(uint8_t track);
void midi_delay_set_feedback(uint8_t track, uint8_t feedback);
uint8_t midi_delay_get_feedback(uint8_t track);
void midi_delay_set_mix(uint8_t track, uint8_t mix);
uint8_t midi_delay_get_mix(uint8_t track);
void midi_delay_set_velocity_decay(uint8_t track, uint8_t decay);
uint8_t midi_delay_get_velocity_decay(uint8_t track);
void midi_delay_process_note(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel);
void midi_delay_clear(uint8_t track);
void midi_delay_clear_all(void);
const char* midi_delay_get_division_name(midi_delay_division_t division);
void midi_delay_set_output_callback(midi_delay_output_cb_t callback);

#else

// Stub implementations (no-op) when module is disabled
static inline void midi_delay_init(uint16_t tempo) { (void)tempo; }
static inline void midi_delay_set_tempo(uint16_t tempo) { (void)tempo; }
static inline void midi_delay_tick_1ms(void) {}
static inline void midi_delay_set_enabled(uint8_t track, uint8_t enabled) { (void)track; (void)enabled; }
static inline uint8_t midi_delay_is_enabled(uint8_t track) { (void)track; return 0; }
static inline void midi_delay_set_division(uint8_t track, midi_delay_division_t division) { (void)track; (void)division; }
static inline midi_delay_division_t midi_delay_get_division(uint8_t track) { (void)track; return DELAY_DIV_1_8; }
static inline void midi_delay_set_feedback(uint8_t track, uint8_t feedback) { (void)track; (void)feedback; }
static inline uint8_t midi_delay_get_feedback(uint8_t track) { (void)track; return 0; }
static inline void midi_delay_set_mix(uint8_t track, uint8_t mix) { (void)track; (void)mix; }
static inline uint8_t midi_delay_get_mix(uint8_t track) { (void)track; return 0; }
static inline void midi_delay_set_velocity_decay(uint8_t track, uint8_t decay) { (void)track; (void)decay; }
static inline uint8_t midi_delay_get_velocity_decay(uint8_t track) { (void)track; return 0; }
static inline void midi_delay_process_note(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel) { 
    (void)track; (void)note; (void)velocity; (void)channel; 
}
static inline void midi_delay_clear(uint8_t track) { (void)track; }
static inline void midi_delay_clear_all(void) {}
static inline const char* midi_delay_get_division_name(midi_delay_division_t division) { (void)division; return "Disabled"; }
static inline void midi_delay_set_output_callback(midi_delay_output_cb_t callback) { (void)callback; }

#endif // MODULE_ENABLE_MIDI_DELAY_FX

#ifdef __cplusplus
}
#endif
