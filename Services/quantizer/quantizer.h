/**
 * @file quantizer.h
 * @brief Timing Quantizer - snaps MIDI note timing to rhythmic grid
 * 
 * Quantizes MIDI note timing to musical grid positions with configurable
 * strength, resolution, and handling of early/late notes. Provides
 * tempo-synced timing correction for tighter rhythmic performances.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define QUANTIZER_MAX_TRACKS 4
#define QUANTIZER_MAX_NOTES_PER_TRACK 16  // Maximum notes buffered per track

/**
 * @brief Quantize grid resolution
 */
typedef enum {
    QUANTIZER_RES_QUARTER = 0,     // 1/4 note
    QUANTIZER_RES_8TH,             // 1/8 note
    QUANTIZER_RES_8TH_TRIPLET,     // 1/8 triplet
    QUANTIZER_RES_16TH,            // 1/16 note
    QUANTIZER_RES_16TH_TRIPLET,    // 1/16 triplet
    QUANTIZER_RES_32ND,            // 1/32 note
    QUANTIZER_RES_32ND_TRIPLET,    // 1/32 triplet
    QUANTIZER_RES_64TH,            // 1/64 note
    QUANTIZER_RES_COUNT
} quantizer_resolution_t;

/**
 * @brief Late note handling mode
 */
typedef enum {
    QUANTIZER_LATE_SNAP_NEAREST = 0,  // Snap to nearest grid point
    QUANTIZER_LATE_SNAP_FORWARD,      // Always snap forward to next grid
    QUANTIZER_LATE_SNAP_BACKWARD,     // Always snap backward to previous grid
    QUANTIZER_LATE_QUANTIZE_OFF,      // Don't quantize late notes
    QUANTIZER_LATE_COUNT
} quantizer_late_mode_t;

/**
 * @brief Quantized note event structure
 */
typedef struct {
    uint8_t note;                   // MIDI note number
    uint8_t velocity;               // Note velocity
    uint8_t channel;                // MIDI channel
    uint32_t original_time_ms;      // Original timing in milliseconds
    uint32_t quantized_time_ms;     // Quantized timing in milliseconds
    uint8_t active;                 // 1 if note slot is active
} quantizer_note_t;

/**
 * @brief Initialize quantizer module
 * @param tempo Initial tempo in BPM (for grid calculations)
 * @param ppqn Pulses per quarter note (typically 96 or 480)
 */
void quantizer_init(uint16_t tempo, uint16_t ppqn);

/**
 * @brief Update tempo (recalculates grid timing)
 * @param tempo Tempo in BPM (20-300)
 */
void quantizer_set_tempo(uint16_t tempo);

/**
 * @brief Get current tempo
 * @return Tempo in BPM
 */
uint16_t quantizer_get_tempo(void);

/**
 * @brief Update PPQN (recalculates grid timing)
 * @param ppqn Pulses per quarter note
 */
void quantizer_set_ppqn(uint16_t ppqn);

/**
 * @brief Get current PPQN
 * @return Pulses per quarter note
 */
uint16_t quantizer_get_ppqn(void);

/**
 * @brief Enable/disable quantizer for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void quantizer_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if quantizer is enabled for a track
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t quantizer_is_enabled(uint8_t track);

/**
 * @brief Set quantize grid resolution
 * @param track Track index (0-3)
 * @param resolution Grid resolution
 */
void quantizer_set_resolution(uint8_t track, quantizer_resolution_t resolution);

/**
 * @brief Get quantize grid resolution
 * @param track Track index (0-3)
 * @return Current grid resolution
 */
quantizer_resolution_t quantizer_get_resolution(uint8_t track);

/**
 * @brief Set quantize strength
 * @param track Track index (0-3)
 * @param strength Strength percentage (0-100, where 100=hard snap, 0=no quantize)
 */
void quantizer_set_strength(uint8_t track, uint8_t strength);

/**
 * @brief Get quantize strength
 * @param track Track index (0-3)
 * @return Strength percentage (0-100)
 */
uint8_t quantizer_get_strength(uint8_t track);

/**
 * @brief Set look-ahead window for early notes
 * @param track Track index (0-3)
 * @param window_ms Look-ahead window in milliseconds (0-500)
 */
void quantizer_set_lookahead(uint8_t track, uint16_t window_ms);

/**
 * @brief Get look-ahead window
 * @param track Track index (0-3)
 * @return Look-ahead window in milliseconds
 */
uint16_t quantizer_get_lookahead(uint8_t track);

/**
 * @brief Set late note handling mode
 * @param track Track index (0-3)
 * @param mode Late note handling mode
 */
void quantizer_set_late_mode(uint8_t track, quantizer_late_mode_t mode);

/**
 * @brief Get late note handling mode
 * @param track Track index (0-3)
 * @return Current late note handling mode
 */
quantizer_late_mode_t quantizer_get_late_mode(uint8_t track);

/**
 * @brief Set swing amount (applies to quantized timing)
 * @param track Track index (0-3)
 * @param swing Swing amount (0-100, where 50=straight, >50=swing)
 */
void quantizer_set_swing(uint8_t track, uint8_t swing);

/**
 * @brief Get swing amount
 * @param track Track index (0-3)
 * @return Swing amount (0-100)
 */
uint8_t quantizer_get_swing(uint8_t track);

/**
 * @brief Process a note-on event (adds to quantize buffer)
 * @param track Track index (0-3)
 * @param note MIDI note number
 * @param velocity Note velocity
 * @param channel MIDI channel
 * @param time_ms Current time in milliseconds
 * @return 1 if note was buffered, 0 if buffer full
 */
uint8_t quantizer_process_note_on(uint8_t track, uint8_t note, uint8_t velocity, 
                                   uint8_t channel, uint32_t time_ms);

/**
 * @brief Process a note-on event using tick position
 * @param track Track index (0-3)
 * @param note MIDI note number
 * @param velocity Note velocity
 * @param channel MIDI channel
 * @param tick_position Current position in ticks
 * @return 1 if note was buffered, 0 if buffer full
 */
uint8_t quantizer_process_note_on_ticks(uint8_t track, uint8_t note, uint8_t velocity,
                                         uint8_t channel, uint32_t tick_position);

/**
 * @brief Calculate quantized timing for a note
 * @param track Track index (0-3)
 * @param time_ms Original note time in milliseconds
 * @return Quantized time in milliseconds
 */
uint32_t quantizer_calculate_time(uint8_t track, uint32_t time_ms);

/**
 * @brief Calculate quantized timing using tick position
 * @param track Track index (0-3)
 * @param tick_position Original note position in ticks
 * @return Quantized position in ticks
 */
uint32_t quantizer_calculate_ticks(uint8_t track, uint32_t tick_position);

/**
 * @brief Get timing offset applied by quantizer
 * @param track Track index (0-3)
 * @param time_ms Original note time in milliseconds
 * @return Timing offset in milliseconds (positive = delayed, negative = advanced)
 */
int32_t quantizer_get_offset(uint8_t track, uint32_t time_ms);

/**
 * @brief Get next grid point in milliseconds
 * @param track Track index (0-3)
 * @param time_ms Current time in milliseconds
 * @return Next grid point time in milliseconds
 */
uint32_t quantizer_get_next_grid(uint8_t track, uint32_t time_ms);

/**
 * @brief Get previous grid point in milliseconds
 * @param track Track index (0-3)
 * @param time_ms Current time in milliseconds
 * @return Previous grid point time in milliseconds
 */
uint32_t quantizer_get_prev_grid(uint8_t track, uint32_t time_ms);

/**
 * @brief Get grid interval in milliseconds
 * @param track Track index (0-3)
 * @return Grid interval in milliseconds
 */
uint32_t quantizer_get_grid_interval_ms(uint8_t track);

/**
 * @brief Get grid interval in ticks
 * @param track Track index (0-3)
 * @return Grid interval in ticks
 */
uint32_t quantizer_get_grid_interval_ticks(uint8_t track);

/**
 * @brief Check if a time position is on the grid
 * @param track Track index (0-3)
 * @param time_ms Time in milliseconds
 * @param tolerance_ms Tolerance in milliseconds
 * @return 1 if on grid, 0 if off grid
 */
uint8_t quantizer_is_on_grid(uint8_t track, uint32_t time_ms, uint16_t tolerance_ms);

/**
 * @brief Get buffered notes ready to be played
 * @param track Track index (0-3)
 * @param current_time_ms Current time in milliseconds
 * @param notes Output array for notes (must have space for QUANTIZER_MAX_NOTES_PER_TRACK)
 * @return Number of notes ready to play
 */
uint8_t quantizer_get_ready_notes(uint8_t track, uint32_t current_time_ms, 
                                   quantizer_note_t* notes);

/**
 * @brief Reset quantizer state for a track (clears buffer)
 * @param track Track index (0-3)
 */
void quantizer_reset(uint8_t track);

/**
 * @brief Reset quantizer state for all tracks
 */
void quantizer_reset_all(void);

/**
 * @brief Get resolution name string
 * @param resolution Resolution type
 * @return Resolution name string
 */
const char* quantizer_get_resolution_name(quantizer_resolution_t resolution);

/**
 * @brief Get late mode name string
 * @param mode Late mode type
 * @return Late mode name string
 */
const char* quantizer_get_late_mode_name(quantizer_late_mode_t mode);

/**
 * @brief Get quantizer statistics for a track
 * @param track Track index (0-3)
 * @param notes_buffered Output: number of notes currently buffered
 * @param notes_quantized Output: total notes quantized since init/reset
 * @param avg_offset_ms Output: average timing offset applied (ms)
 */
void quantizer_get_stats(uint8_t track, uint8_t* notes_buffered, 
                        uint32_t* notes_quantized, int32_t* avg_offset_ms);

#ifdef __cplusplus
}
#endif
