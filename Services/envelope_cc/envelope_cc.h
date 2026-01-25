/**
 * @file envelope_cc.h
 * @brief ADSR Envelope Generator to CC output
 * 
 * Generates ADSR (Attack/Decay/Sustain/Release) envelopes and outputs
 * them as MIDI CC messages. Useful for modulating synth parameters
 * over time with envelope control.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ENVELOPE_CC_MAX_TRACKS 4
#define ENVELOPE_CC_MAX_TIME_MS 5000

/**
 * @brief Envelope stage
 */
typedef enum {
    ENVELOPE_STAGE_IDLE = 0,
    ENVELOPE_STAGE_ATTACK,
    ENVELOPE_STAGE_DECAY,
    ENVELOPE_STAGE_SUSTAIN,
    ENVELOPE_STAGE_RELEASE,
    ENVELOPE_STAGE_COUNT
} envelope_stage_t;

/**
 * @brief CC output callback function type
 * @param track Track index
 * @param cc_number CC number to send
 * @param cc_value CC value (0-127)
 * @param channel MIDI channel
 */
typedef void (*envelope_cc_callback_t)(uint8_t track, uint8_t cc_number, uint8_t cc_value, uint8_t channel);

/**
 * @brief Initialize envelope CC module
 */
void envelope_cc_init(void);

/**
 * @brief Set CC output callback
 * @param callback Function to call for CC output
 */
void envelope_cc_set_callback(envelope_cc_callback_t callback);

/**
 * @brief Enable/disable envelope for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void envelope_cc_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if envelope is enabled for a track
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t envelope_cc_is_enabled(uint8_t track);

/**
 * @brief Set MIDI channel for envelope output
 * @param track Track index (0-3)
 * @param channel MIDI channel (0-15)
 */
void envelope_cc_set_channel(uint8_t track, uint8_t channel);

/**
 * @brief Get MIDI channel
 * @param track Track index (0-3)
 * @return MIDI channel
 */
uint8_t envelope_cc_get_channel(uint8_t track);

/**
 * @brief Set CC number to modulate
 * @param track Track index (0-3)
 * @param cc_number CC number (0-127)
 */
void envelope_cc_set_cc_number(uint8_t track, uint8_t cc_number);

/**
 * @brief Get CC number
 * @param track Track index (0-3)
 * @return CC number
 */
uint8_t envelope_cc_get_cc_number(uint8_t track);

/**
 * @brief Set attack time
 * @param track Track index (0-3)
 * @param time_ms Attack time in milliseconds (0-5000)
 */
void envelope_cc_set_attack(uint8_t track, uint16_t time_ms);

/**
 * @brief Get attack time
 * @param track Track index (0-3)
 * @return Attack time in milliseconds
 */
uint16_t envelope_cc_get_attack(uint8_t track);

/**
 * @brief Set decay time
 * @param track Track index (0-3)
 * @param time_ms Decay time in milliseconds (0-5000)
 */
void envelope_cc_set_decay(uint8_t track, uint16_t time_ms);

/**
 * @brief Get decay time
 * @param track Track index (0-3)
 * @return Decay time in milliseconds
 */
uint16_t envelope_cc_get_decay(uint8_t track);

/**
 * @brief Set sustain level
 * @param track Track index (0-3)
 * @param level Sustain level (0-127)
 */
void envelope_cc_set_sustain(uint8_t track, uint8_t level);

/**
 * @brief Get sustain level
 * @param track Track index (0-3)
 * @return Sustain level (0-127)
 */
uint8_t envelope_cc_get_sustain(uint8_t track);

/**
 * @brief Set release time
 * @param track Track index (0-3)
 * @param time_ms Release time in milliseconds (0-5000)
 */
void envelope_cc_set_release(uint8_t track, uint16_t time_ms);

/**
 * @brief Get release time
 * @param track Track index (0-3)
 * @return Release time in milliseconds
 */
uint16_t envelope_cc_get_release(uint8_t track);

/**
 * @brief Set minimum output value
 * @param track Track index (0-3)
 * @param min_value Minimum CC value (0-127)
 */
void envelope_cc_set_min_value(uint8_t track, uint8_t min_value);

/**
 * @brief Get minimum output value
 * @param track Track index (0-3)
 * @return Minimum CC value
 */
uint8_t envelope_cc_get_min_value(uint8_t track);

/**
 * @brief Set maximum output value
 * @param track Track index (0-3)
 * @param max_value Maximum CC value (0-127)
 */
void envelope_cc_set_max_value(uint8_t track, uint8_t max_value);

/**
 * @brief Get maximum output value
 * @param track Track index (0-3)
 * @return Maximum CC value
 */
uint8_t envelope_cc_get_max_value(uint8_t track);

/**
 * @brief Trigger envelope (start attack phase)
 * @param track Track index (0-3)
 */
void envelope_cc_trigger(uint8_t track);

/**
 * @brief Release envelope (start release phase)
 * @param track Track index (0-3)
 */
void envelope_cc_release(uint8_t track);

/**
 * @brief Tick function - call every 1ms to update envelope
 * @param time_ms Current time in milliseconds
 */
void envelope_cc_tick(uint32_t time_ms);

/**
 * @brief Get current envelope stage
 * @param track Track index (0-3)
 * @return Current envelope stage
 */
envelope_stage_t envelope_cc_get_stage(uint8_t track);

/**
 * @brief Get current envelope value
 * @param track Track index (0-3)
 * @return Current envelope value (0-127)
 */
uint8_t envelope_cc_get_value(uint8_t track);

/**
 * @brief Reset envelope state for a track
 * @param track Track index (0-3)
 */
void envelope_cc_reset(uint8_t track);

/**
 * @brief Reset envelope state for all tracks
 */
void envelope_cc_reset_all(void);

/**
 * @brief Get stage name string
 * @param stage Envelope stage
 * @return Stage name string
 */
const char* envelope_cc_get_stage_name(envelope_stage_t stage);

#ifdef __cplusplus
}
#endif
