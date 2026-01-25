/**
 * @file musette_detune.h
 * @brief Musette Detune - Classic accordion musette/chorus effect
 * 
 * Creates the characteristic accordion "wet" sound by detuning voices.
 * Supports multiple musette styles (French, Italian, American) with
 * configurable detune amounts and voice combinations.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MUSETTE_MAX_TRACKS 4
#define MUSETTE_MAX_VOICES 4  // Original + 3 detuned voices

/**
 * @brief Musette styles (traditional accordion tuning)
 */
typedef enum {
    MUSETTE_STYLE_DRY = 0,      // No detune (single voice)
    MUSETTE_STYLE_LIGHT,        // Subtle detune (±2-5 cents)
    MUSETTE_STYLE_FRENCH,       // Classic French musette (±10-15 cents)
    MUSETTE_STYLE_ITALIAN,      // Italian style (±8-12 cents)
    MUSETTE_STYLE_AMERICAN,     // American swing (±5-8 cents)
    MUSETTE_STYLE_EXTREME,      // Heavy musette (±20+ cents)
    MUSETTE_STYLE_CUSTOM,       // User-defined detune
    MUSETTE_STYLE_COUNT
} musette_style_t;

/**
 * @brief Voice configuration (L-M-M-H pattern common in accordions)
 */
typedef enum {
    MUSETTE_VOICES_1 = 0,       // M only (dry)
    MUSETTE_VOICES_2_LM,        // L-M (bassoon)
    MUSETTE_VOICES_2_MH,        // M-H (violin)
    MUSETTE_VOICES_3_LMH,       // L-M-H (full musette)
    MUSETTE_VOICES_4_LLMH,      // L-L-M-H (super musette)
    MUSETTE_VOICES_COUNT
} musette_voices_t;

/**
 * @brief Initialize musette detune module
 */
void musette_init(void);

/**
 * @brief Set musette style
 * @param track Track index (0-3)
 * @param style Musette style
 */
void musette_set_style(uint8_t track, musette_style_t style);

/**
 * @brief Get musette style
 * @param track Track index (0-3)
 * @return Current style
 */
musette_style_t musette_get_style(uint8_t track);

/**
 * @brief Set voice configuration
 * @param track Track index (0-3)
 * @param voices Voice configuration
 */
void musette_set_voices(uint8_t track, musette_voices_t voices);

/**
 * @brief Get voice configuration
 * @param track Track index (0-3)
 * @return Current voice configuration
 */
musette_voices_t musette_get_voices(uint8_t track);

/**
 * @brief Set custom detune amount (for CUSTOM style)
 * @param track Track index (0-3)
 * @param cents_x10 Detune in 1/10 cents (e.g., 100 = ±10 cents)
 */
void musette_set_custom_detune(uint8_t track, uint16_t cents_x10);

/**
 * @brief Get custom detune amount
 * @param track Track index (0-3)
 * @return Detune in 1/10 cents
 */
uint16_t musette_get_custom_detune(uint8_t track);

/**
 * @brief Set voice balance (volume mix)
 * @param track Track index (0-3)
 * @param voice Voice index (0-3)
 * @param level Volume level 0-100%
 */
void musette_set_voice_level(uint8_t track, uint8_t voice, uint8_t level);

/**
 * @brief Get voice balance
 * @param track Track index (0-3)
 * @param voice Voice index (0-3)
 * @return Volume level 0-100%
 */
uint8_t musette_get_voice_level(uint8_t track, uint8_t voice);

/**
 * @brief Set stereo spread (pan distribution)
 * @param track Track index (0-3)
 * @param spread Spread amount 0-100% (0=mono, 100=wide stereo)
 */
void musette_set_stereo_spread(uint8_t track, uint8_t spread);

/**
 * @brief Get stereo spread
 * @param track Track index (0-3)
 * @return Spread amount
 */
uint8_t musette_get_stereo_spread(uint8_t track);

/**
 * @brief Process incoming MIDI note
 * @param track Track index (0-3)
 * @param note MIDI note number
 * @param velocity Note velocity (0 = note off)
 * @param channel MIDI channel
 */
void musette_process_note(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel);

/**
 * @brief Get style name
 * @param style Musette style
 * @return Style name string
 */
const char* musette_get_style_name(musette_style_t style);

/**
 * @brief Callback for outputting detuned notes
 * @param track Track index
 * @param note MIDI note (may be pitch-bent via CC)
 * @param velocity Velocity (0 = note off)
 * @param channel MIDI channel
 * @param pitchbend Pitchbend value (-8192 to +8191)
 */
typedef void (*musette_output_cb_t)(uint8_t track, uint8_t note, uint8_t velocity, 
                                   uint8_t channel, int16_t pitchbend);

/**
 * @brief Set output callback
 * @param callback Callback function
 */
void musette_set_output_callback(musette_output_cb_t callback);

#ifdef __cplusplus
}
#endif
