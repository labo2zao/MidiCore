/**
 * @file swing.h
 * @brief Swing/Groove MIDI FX - applies timing adjustments for musical feel
 * 
 * Adds swing, shuffle, and groove timing adjustments to MIDI notes.
 * Applies subtle timing shifts based on note position within the beat
 * to create various rhythmic feels.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SWING_MAX_TRACKS 4

/**
 * @brief Groove template types
 */
typedef enum {
    SWING_GROOVE_STRAIGHT = 0,   // No swing (50% straight timing)
    SWING_GROOVE_SWING,          // Classic swing feel (8th notes)
    SWING_GROOVE_SHUFFLE,        // Shuffle feel (heavy swing)
    SWING_GROOVE_TRIPLET,        // Triplet feel (3/3 timing)
    SWING_GROOVE_DOTTED,         // Dotted 8th feel
    SWING_GROOVE_HALF_TIME,      // Half-time shuffle
    SWING_GROOVE_CUSTOM,         // Custom groove pattern
    SWING_GROOVE_COUNT
} swing_groove_t;

/**
 * @brief Swing timing resolution (what subdivision to swing)
 */
typedef enum {
    SWING_RESOLUTION_8TH = 0,    // Swing 8th notes
    SWING_RESOLUTION_16TH,       // Swing 16th notes
    SWING_RESOLUTION_32ND,       // Swing 32nd notes
    SWING_RESOLUTION_COUNT
} swing_resolution_t;

/**
 * @brief Initialize swing module
 * @param tempo Initial tempo in BPM (for timing calculations)
 */
void swing_init(uint16_t tempo);

/**
 * @brief Update tempo (affects timing calculations)
 * @param tempo Tempo in BPM (20-300)
 */
void swing_set_tempo(uint16_t tempo);

/**
 * @brief Get current tempo
 * @return Tempo in BPM
 */
uint16_t swing_get_tempo(void);

/**
 * @brief Enable/disable swing for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void swing_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if swing is enabled for a track
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t swing_is_enabled(uint8_t track);

/**
 * @brief Set swing amount
 * @param track Track index (0-3)
 * @param amount Swing amount (0-100, where 50=no swing, >50=swing late, <50=swing early)
 */
void swing_set_amount(uint8_t track, uint8_t amount);

/**
 * @brief Get swing amount
 * @param track Track index (0-3)
 * @return Swing amount (0-100)
 */
uint8_t swing_get_amount(uint8_t track);

/**
 * @brief Set groove template
 * @param track Track index (0-3)
 * @param groove Groove template type
 */
void swing_set_groove(uint8_t track, swing_groove_t groove);

/**
 * @brief Get groove template
 * @param track Track index (0-3)
 * @return Current groove template
 */
swing_groove_t swing_get_groove(uint8_t track);

/**
 * @brief Set swing resolution (what note division to swing)
 * @param track Track index (0-3)
 * @param resolution Swing resolution
 */
void swing_set_resolution(uint8_t track, swing_resolution_t resolution);

/**
 * @brief Get swing resolution
 * @param track Track index (0-3)
 * @return Current swing resolution
 */
swing_resolution_t swing_get_resolution(uint8_t track);

/**
 * @brief Set swing depth (percentage of beats affected)
 * @param track Track index (0-3)
 * @param depth Depth percentage (0-100, 100=all beats affected)
 */
void swing_set_depth(uint8_t track, uint8_t depth);

/**
 * @brief Get swing depth
 * @param track Track index (0-3)
 * @return Swing depth percentage
 */
uint8_t swing_get_depth(uint8_t track);

/**
 * @brief Calculate timing offset for a note based on its position in the beat
 * @param track Track index (0-3)
 * @param tick_position Position in ticks (PPQN-based)
 * @param ppqn Pulses per quarter note (typically 96 or 480)
 * @return Timing offset in milliseconds (positive = delay, negative = advance)
 */
int16_t swing_calculate_offset(uint8_t track, uint32_t tick_position, uint16_t ppqn);

/**
 * @brief Calculate timing offset for a note at a specific time
 * @param track Track index (0-3)
 * @param time_ms Current time in milliseconds
 * @return Timing offset in milliseconds (positive = delay, negative = advance)
 */
int16_t swing_calculate_offset_ms(uint8_t track, uint32_t time_ms);

/**
 * @brief Set custom groove pattern (up to 16 steps)
 * @param track Track index (0-3)
 * @param pattern Array of 16 timing offsets (0-100, 50=no offset)
 * @param length Pattern length in steps (1-16)
 */
void swing_set_custom_pattern(uint8_t track, const uint8_t* pattern, uint8_t length);

/**
 * @brief Get custom groove pattern
 * @param track Track index (0-3)
 * @param pattern Output array for pattern (must have space for 16 values)
 * @param length Output: pattern length in steps
 */
void swing_get_custom_pattern(uint8_t track, uint8_t* pattern, uint8_t* length);

/**
 * @brief Reset swing state for a track
 * @param track Track index (0-3)
 */
void swing_reset(uint8_t track);

/**
 * @brief Reset swing state for all tracks
 */
void swing_reset_all(void);

/**
 * @brief Get groove template name
 * @param groove Groove type
 * @return Groove name string
 */
const char* swing_get_groove_name(swing_groove_t groove);

/**
 * @brief Get resolution name
 * @param resolution Resolution type
 * @return Resolution name string
 */
const char* swing_get_resolution_name(swing_resolution_t resolution);

#ifdef __cplusplus
}
#endif
