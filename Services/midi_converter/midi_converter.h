/**
 * @file midi_converter.h
 * @brief MIDI Message Converters - convert between different MIDI message types
 * 
 * Converts MIDI messages between different types (CC, Aftertouch, Pitchbend, Velocity)
 * with configurable scaling, offset, and inversion transformations.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MIDI_CONVERTER_MAX_TRACKS 4

/**
 * @brief Conversion mode
 */
typedef enum {
    MIDI_CONVERTER_AFTERTOUCH_TO_CC = 0,  // Channel Aftertouch → CC
    MIDI_CONVERTER_CC_TO_AFTERTOUCH,      // CC → Channel Aftertouch
    MIDI_CONVERTER_PITCHBEND_TO_CC,       // Pitchbend → CC
    MIDI_CONVERTER_CC_TO_PITCHBEND,       // CC → Pitchbend
    MIDI_CONVERTER_VELOCITY_TO_CC,        // Note Velocity → CC
    MIDI_CONVERTER_MODWHEEL_TO_CC,        // Modwheel (CC 1) → other CC
    MIDI_CONVERTER_CC_TO_CC,              // CC → different CC
    MIDI_CONVERTER_DISABLED,              // Disabled
    MIDI_CONVERTER_MODE_COUNT
} midi_converter_mode_t;

/**
 * @brief CC output callback function type
 * @param track Track index
 * @param cc_number CC number
 * @param cc_value CC value (0-127)
 * @param channel MIDI channel
 */
typedef void (*midi_converter_cc_callback_t)(uint8_t track, uint8_t cc_number, uint8_t cc_value, uint8_t channel);

/**
 * @brief Aftertouch output callback function type
 * @param track Track index
 * @param pressure Pressure value (0-127)
 * @param channel MIDI channel
 */
typedef void (*midi_converter_aftertouch_callback_t)(uint8_t track, uint8_t pressure, uint8_t channel);

/**
 * @brief Pitchbend output callback function type
 * @param track Track index
 * @param value Pitchbend value (0-16383, 8192=center)
 * @param channel MIDI channel
 */
typedef void (*midi_converter_pitchbend_callback_t)(uint8_t track, uint16_t value, uint8_t channel);

/**
 * @brief Initialize MIDI converter module
 */
void midi_converter_init(void);

/**
 * @brief Set CC output callback
 * @param callback Function to call for CC output
 */
void midi_converter_set_cc_callback(midi_converter_cc_callback_t callback);

/**
 * @brief Set aftertouch output callback
 * @param callback Function to call for aftertouch output
 */
void midi_converter_set_aftertouch_callback(midi_converter_aftertouch_callback_t callback);

/**
 * @brief Set pitchbend output callback
 * @param callback Function to call for pitchbend output
 */
void midi_converter_set_pitchbend_callback(midi_converter_pitchbend_callback_t callback);

/**
 * @brief Enable/disable converter for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void midi_converter_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if converter is enabled for a track
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t midi_converter_is_enabled(uint8_t track);

/**
 * @brief Set conversion mode
 * @param track Track index (0-3)
 * @param mode Conversion mode
 */
void midi_converter_set_mode(uint8_t track, midi_converter_mode_t mode);

/**
 * @brief Get conversion mode
 * @param track Track index (0-3)
 * @return Current conversion mode
 */
midi_converter_mode_t midi_converter_get_mode(uint8_t track);

/**
 * @brief Set source CC number (for CC-based conversions)
 * @param track Track index (0-3)
 * @param cc_number Source CC number (0-127)
 */
void midi_converter_set_source_cc(uint8_t track, uint8_t cc_number);

/**
 * @brief Get source CC number
 * @param track Track index (0-3)
 * @return Source CC number
 */
uint8_t midi_converter_get_source_cc(uint8_t track);

/**
 * @brief Set destination CC number (for CC output conversions)
 * @param track Track index (0-3)
 * @param cc_number Destination CC number (0-127)
 */
void midi_converter_set_dest_cc(uint8_t track, uint8_t cc_number);

/**
 * @brief Get destination CC number
 * @param track Track index (0-3)
 * @return Destination CC number
 */
uint8_t midi_converter_get_dest_cc(uint8_t track);

/**
 * @brief Set scale factor (0-200, 100=no scaling)
 * @param track Track index (0-3)
 * @param scale Scale factor percentage
 */
void midi_converter_set_scale(uint8_t track, uint8_t scale);

/**
 * @brief Get scale factor
 * @param track Track index (0-3)
 * @return Scale factor percentage
 */
uint8_t midi_converter_get_scale(uint8_t track);

/**
 * @brief Set offset value (-64 to +63)
 * @param track Track index (0-3)
 * @param offset Offset value
 */
void midi_converter_set_offset(uint8_t track, int8_t offset);

/**
 * @brief Get offset value
 * @param track Track index (0-3)
 * @return Offset value
 */
int8_t midi_converter_get_offset(uint8_t track);

/**
 * @brief Set invert flag (inverts output value)
 * @param track Track index (0-3)
 * @param invert 1 to invert, 0 for normal
 */
void midi_converter_set_invert(uint8_t track, uint8_t invert);

/**
 * @brief Get invert flag
 * @param track Track index (0-3)
 * @return 1 if inverted, 0 if normal
 */
uint8_t midi_converter_get_invert(uint8_t track);

/**
 * @brief Process CC message
 * @param track Track index (0-3)
 * @param cc_number CC number
 * @param cc_value CC value (0-127)
 * @param channel MIDI channel
 */
void midi_converter_process_cc(uint8_t track, uint8_t cc_number, uint8_t cc_value, uint8_t channel);

/**
 * @brief Process aftertouch message
 * @param track Track index (0-3)
 * @param pressure Pressure value (0-127)
 * @param channel MIDI channel
 */
void midi_converter_process_aftertouch(uint8_t track, uint8_t pressure, uint8_t channel);

/**
 * @brief Process pitchbend message
 * @param track Track index (0-3)
 * @param value Pitchbend value (0-16383, 8192=center)
 * @param channel MIDI channel
 */
void midi_converter_process_pitchbend(uint8_t track, uint16_t value, uint8_t channel);

/**
 * @brief Process note velocity (for velocity→CC conversion)
 * @param track Track index (0-3)
 * @param velocity Note velocity (0-127)
 * @param channel MIDI channel
 */
void midi_converter_process_velocity(uint8_t track, uint8_t velocity, uint8_t channel);

/**
 * @brief Reset converter state for a track
 * @param track Track index (0-3)
 */
void midi_converter_reset(uint8_t track);

/**
 * @brief Reset converter state for all tracks
 */
void midi_converter_reset_all(void);

/**
 * @brief Get mode name string
 * @param mode Conversion mode
 * @return Mode name string
 */
const char* midi_converter_get_mode_name(midi_converter_mode_t mode);

#ifdef __cplusplus
}
#endif
