/**
 * @file musette_detune.c
 * @brief Musette Detune implementation
 */

#include "Services/musette_detune/musette_detune.h"
#include <string.h>

static const char* style_names[] = {
    "Dry", "Light", "French", "Italian", "American", "Extreme", "Custom"
};

// Detune amounts in 1/10 cents for each style
static const uint16_t style_detune_x10[] = {
    0,    // Dry
    35,   // Light (±3.5 cents)
    125,  // French (±12.5 cents)
    100,  // Italian (±10 cents)
    65,   // American (±6.5 cents)
    220,  // Extreme (±22 cents)
    100   // Custom (default)
};

// Voice detune patterns (Low, Mid, Mid, High)
// Values are in 1/100 of the base detune
static const int16_t voice_detune_patterns[MUSETTE_VOICES_COUNT][MUSETTE_MAX_VOICES] = {
    {0, 0, 0, 0},           // 1 voice (M only)
    {-100, 0, 0, 0},        // 2 voices L-M
    {0, 0, 100, 0},         // 2 voices M-H
    {-100, 0, 100, 0},      // 3 voices L-M-H
    {-150, -50, 0, 100},    // 4 voices L-L-M-H
};

typedef struct {
    musette_style_t style;
    musette_voices_t voices;
    uint16_t custom_detune_x10;
    uint8_t voice_levels[MUSETTE_MAX_VOICES];
    uint8_t stereo_spread;
} musette_config_t;

static musette_config_t g_musette[MUSETTE_MAX_TRACKS];
static musette_output_cb_t g_output_callback = NULL;

/**
 * @brief Initialize musette detune module
 */
void musette_init(void) {
    memset(g_musette, 0, sizeof(g_musette));
    
    for (uint8_t t = 0; t < MUSETTE_MAX_TRACKS; t++) {
        g_musette[t].style = MUSETTE_STYLE_DRY;
        g_musette[t].voices = MUSETTE_VOICES_3_LMH;
        g_musette[t].custom_detune_x10 = 100;
        g_musette[t].stereo_spread = 50;
        
        // Equal levels for all voices
        for (uint8_t v = 0; v < MUSETTE_MAX_VOICES; v++) {
            g_musette[t].voice_levels[v] = 100;
        }
    }
}

/**
 * @brief Set musette style
 */
void musette_set_style(uint8_t track, musette_style_t style) {
    if (track >= MUSETTE_MAX_TRACKS) return;
    if (style >= MUSETTE_STYLE_COUNT) return;
    g_musette[track].style = style;
}

/**
 * @brief Get musette style
 */
musette_style_t musette_get_style(uint8_t track) {
    if (track >= MUSETTE_MAX_TRACKS) return MUSETTE_STYLE_DRY;
    return g_musette[track].style;
}

/**
 * @brief Set voice configuration
 */
void musette_set_voices(uint8_t track, musette_voices_t voices) {
    if (track >= MUSETTE_MAX_TRACKS) return;
    if (voices >= MUSETTE_VOICES_COUNT) return;
    g_musette[track].voices = voices;
}

/**
 * @brief Get voice configuration
 */
musette_voices_t musette_get_voices(uint8_t track) {
    if (track >= MUSETTE_MAX_TRACKS) return MUSETTE_VOICES_1;
    return g_musette[track].voices;
}

/**
 * @brief Set custom detune amount
 */
void musette_set_custom_detune(uint8_t track, uint16_t cents_x10) {
    if (track >= MUSETTE_MAX_TRACKS) return;
    g_musette[track].custom_detune_x10 = cents_x10;
}

/**
 * @brief Get custom detune amount
 */
uint16_t musette_get_custom_detune(uint8_t track) {
    if (track >= MUSETTE_MAX_TRACKS) return 100;
    return g_musette[track].custom_detune_x10;
}

/**
 * @brief Set voice balance
 */
void musette_set_voice_level(uint8_t track, uint8_t voice, uint8_t level) {
    if (track >= MUSETTE_MAX_TRACKS) return;
    if (voice >= MUSETTE_MAX_VOICES) return;
    if (level > 100) level = 100;
    g_musette[track].voice_levels[voice] = level;
}

/**
 * @brief Get voice balance
 */
uint8_t musette_get_voice_level(uint8_t track, uint8_t voice) {
    if (track >= MUSETTE_MAX_TRACKS) return 100;
    if (voice >= MUSETTE_MAX_VOICES) return 100;
    return g_musette[track].voice_levels[voice];
}

/**
 * @brief Set stereo spread
 */
void musette_set_stereo_spread(uint8_t track, uint8_t spread) {
    if (track >= MUSETTE_MAX_TRACKS) return;
    if (spread > 100) spread = 100;
    g_musette[track].stereo_spread = spread;
}

/**
 * @brief Get stereo spread
 */
uint8_t musette_get_stereo_spread(uint8_t track) {
    if (track >= MUSETTE_MAX_TRACKS) return 50;
    return g_musette[track].stereo_spread;
}

/**
 * @brief Convert cents to pitchbend value
 * Pitchbend range is ±2 semitones (±200 cents) = ±8192
 * So 1 cent = 8192/200 = 40.96 ≈ 41
 */
static int16_t cents_to_pitchbend(int16_t cents_x10) {
    // cents_x10 is in 1/10 cents, convert to pitchbend
    // 1 cent = 41 pitchbend units
    // 1/10 cent = 4.1 pitchbend units
    int32_t pb = ((int32_t)cents_x10 * 41) / 10;
    if (pb < -8192) pb = -8192;
    if (pb > 8191) pb = 8191;
    return (int16_t)pb;
}

/**
 * @brief Process incoming MIDI note
 */
void musette_process_note(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel) {
    if (track >= MUSETTE_MAX_TRACKS) return;
    if (!g_output_callback) return;
    
    musette_config_t* cfg = &g_musette[track];
    
    // Get base detune for style
    uint16_t base_detune_x10 = (cfg->style == MUSETTE_STYLE_CUSTOM) ? 
                                cfg->custom_detune_x10 : 
                                style_detune_x10[cfg->style];
    
    // Get voice pattern
    const int16_t* pattern = voice_detune_patterns[cfg->voices];
    
    // Generate each voice
    for (uint8_t v = 0; v < MUSETTE_MAX_VOICES; v++) {
        if (pattern[v] == 0 && v > 0) continue;  // Skip unused voices
        
        // Calculate detune for this voice
        int32_t voice_detune_x10 = ((int32_t)base_detune_x10 * pattern[v]) / 100;
        int16_t pitchbend = cents_to_pitchbend((int16_t)voice_detune_x10);
        
        // Calculate velocity with level adjustment
        uint16_t adj_velocity = (velocity * cfg->voice_levels[v]) / 100;
        if (adj_velocity > 127) adj_velocity = 127;
        
        // Send note with pitchbend
        g_output_callback(track, note, (uint8_t)adj_velocity, channel, pitchbend);
    }
}

/**
 * @brief Get style name
 */
const char* musette_get_style_name(musette_style_t style) {
    if (style >= MUSETTE_STYLE_COUNT) return "Unknown";
    return style_names[style];
}

/**
 * @brief Set output callback
 */
void musette_set_output_callback(musette_output_cb_t callback) {
    g_output_callback = callback;
}
