/**
 * @file swing.c
 * @brief Swing/Groove MIDI FX implementation
 */

#include "Services/swing/swing.h"
#include <string.h>

#define MAX_CUSTOM_PATTERN_LENGTH 16
#define DEFAULT_TEMPO 120

// Groove template names
static const char* groove_names[] = {
    "Straight",
    "Swing",
    "Shuffle",
    "Triplet",
    "Dotted",
    "Half-Time",
    "Custom"
};

// Resolution names
static const char* resolution_names[] = {
    "8th",
    "16th",
    "32nd"
};

// Predefined groove patterns (0-100, 50=no offset)
// Each pattern defines timing for 16 subdivisions
static const uint8_t groove_patterns[][16] = {
    // SWING_GROOVE_STRAIGHT - No swing
    {50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50},
    
    // SWING_GROOVE_SWING - Classic swing (66% on offbeats)
    {50, 66, 50, 66, 50, 66, 50, 66, 50, 66, 50, 66, 50, 66, 50, 66},
    
    // SWING_GROOVE_SHUFFLE - Heavy shuffle (75% on offbeats)
    {50, 75, 50, 75, 50, 75, 50, 75, 50, 75, 50, 75, 50, 75, 50, 75},
    
    // SWING_GROOVE_TRIPLET - Triplet feel (67% spacing)
    {50, 67, 50, 67, 50, 67, 50, 67, 50, 67, 50, 67, 50, 67, 50, 67},
    
    // SWING_GROOVE_DOTTED - Dotted 8th feel
    {50, 62, 50, 62, 50, 62, 50, 62, 50, 62, 50, 62, 50, 62, 50, 62},
    
    // SWING_GROOVE_HALF_TIME - Half-time shuffle
    {50, 50, 50, 75, 50, 50, 50, 50, 50, 50, 50, 75, 50, 50, 50, 50},
    
    // SWING_GROOVE_CUSTOM - Will be overridden by user
    {50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50}
};

// Per-track swing configuration
typedef struct {
    uint8_t enabled;
    uint8_t amount;                              // 0-100 (50=no swing)
    swing_groove_t groove;
    swing_resolution_t resolution;
    uint8_t depth;                               // 0-100 (percentage of beats affected)
    uint8_t custom_pattern[MAX_CUSTOM_PATTERN_LENGTH];
    uint8_t custom_pattern_length;
    uint32_t beat_counter;                       // For tracking position
} swing_config_t;

static swing_config_t g_swing_config[SWING_MAX_TRACKS];
static uint16_t g_tempo = DEFAULT_TEMPO;

/**
 * @brief Calculate ticks per subdivision based on resolution
 */
static uint16_t get_ticks_per_subdivision(swing_resolution_t resolution, uint16_t ppqn) {
    switch (resolution) {
        case SWING_RESOLUTION_8TH:
            return ppqn / 2;  // 8th note = quarter note / 2
        case SWING_RESOLUTION_16TH:
            return ppqn / 4;  // 16th note = quarter note / 4
        case SWING_RESOLUTION_32ND:
            return ppqn / 8;  // 32nd note = quarter note / 8
        default:
            return ppqn / 2;
    }
}

/**
 * @brief Get pattern value for current position
 */
static uint8_t get_pattern_value(uint8_t track, uint8_t position) {
    swing_config_t* cfg = &g_swing_config[track];
    
    if (cfg->groove == SWING_GROOVE_CUSTOM) {
        if (cfg->custom_pattern_length == 0) return 50;
        return cfg->custom_pattern[position % cfg->custom_pattern_length];
    }
    
    return groove_patterns[cfg->groove][position % 16];
}

/**
 * @brief Calculate milliseconds per subdivision at current tempo
 */
static uint32_t get_ms_per_subdivision(swing_resolution_t resolution) {
    // Calculate ms per quarter note at current tempo
    uint32_t ms_per_quarter = (60000 / g_tempo);
    
    switch (resolution) {
        case SWING_RESOLUTION_8TH:
            return ms_per_quarter / 2;
        case SWING_RESOLUTION_16TH:
            return ms_per_quarter / 4;
        case SWING_RESOLUTION_32ND:
            return ms_per_quarter / 8;
        default:
            return ms_per_quarter / 2;
    }
}

/**
 * @brief Initialize swing module
 */
void swing_init(uint16_t tempo) {
    memset(g_swing_config, 0, sizeof(g_swing_config));
    
    g_tempo = tempo;
    if (g_tempo < 20) g_tempo = 20;
    if (g_tempo > 300) g_tempo = 300;
    
    // Initialize defaults
    for (uint8_t i = 0; i < SWING_MAX_TRACKS; i++) {
        g_swing_config[i].enabled = 0;
        g_swing_config[i].amount = 50;  // No swing
        g_swing_config[i].groove = SWING_GROOVE_STRAIGHT;
        g_swing_config[i].resolution = SWING_RESOLUTION_16TH;
        g_swing_config[i].depth = 100;
        g_swing_config[i].custom_pattern_length = 0;
        g_swing_config[i].beat_counter = 0;
        
        // Initialize custom pattern to straight
        for (uint8_t j = 0; j < MAX_CUSTOM_PATTERN_LENGTH; j++) {
            g_swing_config[i].custom_pattern[j] = 50;
        }
    }
}

/**
 * @brief Update tempo
 */
void swing_set_tempo(uint16_t tempo) {
    if (tempo < 20) tempo = 20;
    if (tempo > 300) tempo = 300;
    g_tempo = tempo;
}

/**
 * @brief Get current tempo
 */
uint16_t swing_get_tempo(void) {
    return g_tempo;
}

/**
 * @brief Enable/disable swing for a track
 */
void swing_set_enabled(uint8_t track, uint8_t enabled) {
    if (track >= SWING_MAX_TRACKS) return;
    g_swing_config[track].enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if swing is enabled for a track
 */
uint8_t swing_is_enabled(uint8_t track) {
    if (track >= SWING_MAX_TRACKS) return 0;
    return g_swing_config[track].enabled;
}

/**
 * @brief Set swing amount
 */
void swing_set_amount(uint8_t track, uint8_t amount) {
    if (track >= SWING_MAX_TRACKS) return;
    if (amount > 100) amount = 100;
    g_swing_config[track].amount = amount;
}

/**
 * @brief Get swing amount
 */
uint8_t swing_get_amount(uint8_t track) {
    if (track >= SWING_MAX_TRACKS) return 50;
    return g_swing_config[track].amount;
}

/**
 * @brief Set groove template
 */
void swing_set_groove(uint8_t track, swing_groove_t groove) {
    if (track >= SWING_MAX_TRACKS) return;
    if (groove >= SWING_GROOVE_COUNT) return;
    g_swing_config[track].groove = groove;
}

/**
 * @brief Get groove template
 */
swing_groove_t swing_get_groove(uint8_t track) {
    if (track >= SWING_MAX_TRACKS) return SWING_GROOVE_STRAIGHT;
    return g_swing_config[track].groove;
}

/**
 * @brief Set swing resolution
 */
void swing_set_resolution(uint8_t track, swing_resolution_t resolution) {
    if (track >= SWING_MAX_TRACKS) return;
    if (resolution >= SWING_RESOLUTION_COUNT) return;
    g_swing_config[track].resolution = resolution;
}

/**
 * @brief Get swing resolution
 */
swing_resolution_t swing_get_resolution(uint8_t track) {
    if (track >= SWING_MAX_TRACKS) return SWING_RESOLUTION_8TH;
    return g_swing_config[track].resolution;
}

/**
 * @brief Set swing depth
 */
void swing_set_depth(uint8_t track, uint8_t depth) {
    if (track >= SWING_MAX_TRACKS) return;
    if (depth > 100) depth = 100;
    g_swing_config[track].depth = depth;
}

/**
 * @brief Get swing depth
 */
uint8_t swing_get_depth(uint8_t track) {
    if (track >= SWING_MAX_TRACKS) return 100;
    return g_swing_config[track].depth;
}

/**
 * @brief Calculate timing offset based on tick position
 */
int16_t swing_calculate_offset(uint8_t track, uint32_t tick_position, uint16_t ppqn) {
    if (track >= SWING_MAX_TRACKS) return 0;
    
    swing_config_t* cfg = &g_swing_config[track];
    if (!cfg->enabled) return 0;
    if (cfg->amount == 50 && cfg->groove == SWING_GROOVE_STRAIGHT) return 0;
    
    // Calculate position within subdivision grid
    uint16_t ticks_per_sub = get_ticks_per_subdivision(cfg->resolution, ppqn);
    if (ticks_per_sub == 0) return 0;
    
    uint8_t subdivision_index = (tick_position / ticks_per_sub) % 16;
    
    // Get pattern value for this position
    uint8_t pattern_value = get_pattern_value(track, subdivision_index);
    
    // Calculate base offset from pattern
    // Pattern value: 50 = no offset, >50 = delay, <50 = advance
    int16_t base_offset = (int16_t)pattern_value - 50;
    
    // Scale by swing amount (amount acts as intensity multiplier)
    // amount=50 means use pattern as-is, amount>50 increases effect, amount<50 decreases effect
    int32_t scaled_offset = (base_offset * (int32_t)cfg->amount) / 50;
    
    // Apply depth (percentage of beats affected)
    scaled_offset = (scaled_offset * cfg->depth) / 100;
    
    // Convert to milliseconds
    // Maximum offset is ±25% of subdivision length
    uint32_t ms_per_sub = (60000 / g_tempo);
    switch (cfg->resolution) {
        case SWING_RESOLUTION_8TH:
            ms_per_sub = ms_per_sub / 2;
            break;
        case SWING_RESOLUTION_16TH:
            ms_per_sub = ms_per_sub / 4;
            break;
        case SWING_RESOLUTION_32ND:
            ms_per_sub = ms_per_sub / 8;
            break;
        case SWING_RESOLUTION_COUNT:
        default:
            ms_per_sub = ms_per_sub / 4;
            break;
    }
    
    // Map offset value (-50 to +50) to ±25% of subdivision time
    int16_t max_offset_ms = ms_per_sub / 4;
    int16_t offset_ms = (scaled_offset * max_offset_ms) / 50;
    
    // Clamp to reasonable values
    if (offset_ms > max_offset_ms) offset_ms = max_offset_ms;
    if (offset_ms < -max_offset_ms) offset_ms = -max_offset_ms;
    
    return offset_ms;
}

/**
 * @brief Calculate timing offset based on time in milliseconds
 */
int16_t swing_calculate_offset_ms(uint8_t track, uint32_t time_ms) {
    if (track >= SWING_MAX_TRACKS) return 0;
    
    swing_config_t* cfg = &g_swing_config[track];
    if (!cfg->enabled) return 0;
    if (cfg->amount == 50 && cfg->groove == SWING_GROOVE_STRAIGHT) return 0;
    
    // Calculate which subdivision we're in based on time
    uint32_t ms_per_sub = get_ms_per_subdivision(cfg->resolution);
    if (ms_per_sub == 0) return 0;
    
    uint8_t subdivision_index = (time_ms / ms_per_sub) % 16;
    
    // Get pattern value for this position
    uint8_t pattern_value = get_pattern_value(track, subdivision_index);
    
    // Calculate base offset from pattern
    int16_t base_offset = (int16_t)pattern_value - 50;
    
    // Scale by swing amount
    int32_t scaled_offset = (base_offset * (int32_t)cfg->amount) / 50;
    
    // Apply depth
    scaled_offset = (scaled_offset * cfg->depth) / 100;
    
    // Convert to milliseconds (±25% of subdivision time)
    int16_t max_offset_ms = ms_per_sub / 4;
    int16_t offset_ms = (scaled_offset * max_offset_ms) / 50;
    
    // Clamp
    if (offset_ms > max_offset_ms) offset_ms = max_offset_ms;
    if (offset_ms < -max_offset_ms) offset_ms = -max_offset_ms;
    
    return offset_ms;
}

/**
 * @brief Set custom groove pattern
 */
void swing_set_custom_pattern(uint8_t track, const uint8_t* pattern, uint8_t length) {
    if (track >= SWING_MAX_TRACKS) return;
    if (!pattern || length == 0 || length > MAX_CUSTOM_PATTERN_LENGTH) return;
    
    swing_config_t* cfg = &g_swing_config[track];
    cfg->custom_pattern_length = length;
    
    for (uint8_t i = 0; i < length; i++) {
        uint8_t value = pattern[i];
        if (value > 100) value = 100;
        cfg->custom_pattern[i] = value;
    }
    
    // Fill remaining slots with neutral value
    for (uint8_t i = length; i < MAX_CUSTOM_PATTERN_LENGTH; i++) {
        cfg->custom_pattern[i] = 50;
    }
}

/**
 * @brief Get custom groove pattern
 */
void swing_get_custom_pattern(uint8_t track, uint8_t* pattern, uint8_t* length) {
    if (track >= SWING_MAX_TRACKS) return;
    if (!pattern || !length) return;
    
    swing_config_t* cfg = &g_swing_config[track];
    *length = cfg->custom_pattern_length;
    
    for (uint8_t i = 0; i < MAX_CUSTOM_PATTERN_LENGTH; i++) {
        pattern[i] = cfg->custom_pattern[i];
    }
}

/**
 * @brief Reset swing state for a track
 */
void swing_reset(uint8_t track) {
    if (track >= SWING_MAX_TRACKS) return;
    g_swing_config[track].beat_counter = 0;
}

/**
 * @brief Reset swing state for all tracks
 */
void swing_reset_all(void) {
    for (uint8_t i = 0; i < SWING_MAX_TRACKS; i++) {
        swing_reset(i);
    }
}

/**
 * @brief Get groove template name
 */
const char* swing_get_groove_name(swing_groove_t groove) {
    if (groove >= SWING_GROOVE_COUNT) return "Unknown";
    return groove_names[groove];
}

/**
 * @brief Get resolution name
 */
const char* swing_get_resolution_name(swing_resolution_t resolution) {
    if (resolution >= SWING_RESOLUTION_COUNT) return "Unknown";
    return resolution_names[resolution];
}
