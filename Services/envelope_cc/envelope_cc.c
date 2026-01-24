/**
 * @file envelope_cc.c
 * @brief ADSR Envelope Generator to CC implementation
 */

#include "Services/envelope_cc/envelope_cc.h"
#include <string.h>

#define DEFAULT_ATTACK_MS 100
#define DEFAULT_DECAY_MS 200
#define DEFAULT_SUSTAIN_LEVEL 100
#define DEFAULT_RELEASE_MS 300
#define DEFAULT_CC_NUMBER 74  // Filter cutoff (typical modulation target)

// Stage name strings
static const char* stage_names[] = {
    "Idle",
    "Attack",
    "Decay",
    "Sustain",
    "Release"
};

// Per-track envelope configuration
typedef struct {
    uint8_t enabled;
    uint8_t channel;                    // MIDI channel
    uint8_t cc_number;                  // CC number to modulate
    
    // ADSR parameters
    uint16_t attack_ms;
    uint16_t decay_ms;
    uint8_t sustain_level;              // 0-127
    uint16_t release_ms;
    
    // Value range
    uint8_t min_value;                  // Minimum CC output
    uint8_t max_value;                  // Maximum CC output
    
    // Runtime state
    envelope_stage_t stage;
    uint32_t stage_start_time;          // When current stage started
    uint8_t current_value;              // Current envelope value (0-127)
    uint8_t last_sent_value;            // Last CC value sent (for change detection)
} envelope_cc_config_t;

static envelope_cc_config_t g_envelope_config[ENVELOPE_CC_MAX_TRACKS];
static envelope_cc_callback_t g_cc_callback = NULL;

/**
 * @brief Calculate envelope value for current stage
 */
static uint8_t calculate_envelope_value(uint8_t track, uint32_t time_ms) {
    envelope_cc_config_t* cfg = &g_envelope_config[track];
    
    // Check for time wraparound or uninitialized stage_start_time
    if (time_ms < cfg->stage_start_time) {
        return cfg->min_value;
    }
    
    uint32_t elapsed = time_ms - cfg->stage_start_time;
    uint32_t value = 0;
    
    switch (cfg->stage) {
        case ENVELOPE_STAGE_IDLE:
            value = cfg->min_value;
            break;
            
        case ENVELOPE_STAGE_ATTACK:
            if (cfg->attack_ms == 0) {
                value = cfg->max_value;
            } else if (elapsed >= cfg->attack_ms) {
                value = cfg->max_value;
                // Move to decay stage
                cfg->stage = ENVELOPE_STAGE_DECAY;
                cfg->stage_start_time = time_ms;
            } else {
                // Linear attack from min to max (use 32-bit to prevent overflow)
                uint32_t range = cfg->max_value - cfg->min_value;
                value = cfg->min_value + (range * elapsed) / cfg->attack_ms;
            }
            break;
            
        case ENVELOPE_STAGE_DECAY:
            if (cfg->decay_ms == 0) {
                value = cfg->sustain_level;
            } else if (elapsed >= cfg->decay_ms) {
                value = cfg->sustain_level;
                // Move to sustain stage
                cfg->stage = ENVELOPE_STAGE_SUSTAIN;
                cfg->stage_start_time = time_ms;
            } else {
                // Linear decay from max to sustain (use 32-bit to prevent overflow)
                // Handle case where sustain > max
                if (cfg->sustain_level > cfg->max_value) {
                    value = cfg->sustain_level;
                } else {
                    uint32_t range = cfg->max_value - cfg->sustain_level;
                    value = cfg->max_value - (range * elapsed) / cfg->decay_ms;
                }
            }
            break;
            
        case ENVELOPE_STAGE_SUSTAIN:
            value = cfg->sustain_level;
            break;
            
        case ENVELOPE_STAGE_RELEASE:
            if (cfg->release_ms == 0) {
                value = cfg->min_value;
                cfg->stage = ENVELOPE_STAGE_IDLE;
            } else if (elapsed >= cfg->release_ms) {
                value = cfg->min_value;
                // Move to idle
                cfg->stage = ENVELOPE_STAGE_IDLE;
                cfg->stage_start_time = time_ms;
            } else {
                // Linear release from sustain to min (use 32-bit to prevent overflow)
                uint32_t start_value = cfg->sustain_level;
                // Handle case where min > sustain
                if (cfg->min_value > start_value) {
                    value = cfg->min_value;
                } else {
                    uint32_t range = start_value - cfg->min_value;
                    value = start_value - (range * elapsed) / cfg->release_ms;
                }
            }
            break;
            
        default:
            value = cfg->min_value;
            break;
    }
    
    // Clamp to valid CC range
    if (value > 127) value = 127;
    
    return (uint8_t)value;
}

/**
 * @brief Initialize envelope CC module
 */
void envelope_cc_init(void) {
    memset(g_envelope_config, 0, sizeof(g_envelope_config));
    
    for (uint8_t i = 0; i < ENVELOPE_CC_MAX_TRACKS; i++) {
        g_envelope_config[i].enabled = 0;
        g_envelope_config[i].channel = 0;
        g_envelope_config[i].cc_number = DEFAULT_CC_NUMBER;
        g_envelope_config[i].attack_ms = DEFAULT_ATTACK_MS;
        g_envelope_config[i].decay_ms = DEFAULT_DECAY_MS;
        g_envelope_config[i].sustain_level = DEFAULT_SUSTAIN_LEVEL;
        g_envelope_config[i].release_ms = DEFAULT_RELEASE_MS;
        g_envelope_config[i].min_value = 0;
        g_envelope_config[i].max_value = 127;
        g_envelope_config[i].stage = ENVELOPE_STAGE_IDLE;
        g_envelope_config[i].current_value = 0;
        g_envelope_config[i].last_sent_value = 0;
    }
    
    g_cc_callback = NULL;
}

/**
 * @brief Set CC output callback
 */
void envelope_cc_set_callback(envelope_cc_callback_t callback) {
    g_cc_callback = callback;
}

/**
 * @brief Enable/disable envelope for a track
 */
void envelope_cc_set_enabled(uint8_t track, uint8_t enabled) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return;
    g_envelope_config[track].enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if envelope is enabled for a track
 */
uint8_t envelope_cc_is_enabled(uint8_t track) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return 0;
    return g_envelope_config[track].enabled;
}

/**
 * @brief Set MIDI channel
 */
void envelope_cc_set_channel(uint8_t track, uint8_t channel) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return;
    if (channel > 15) channel = 15;
    g_envelope_config[track].channel = channel;
}

/**
 * @brief Get MIDI channel
 */
uint8_t envelope_cc_get_channel(uint8_t track) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return 0;
    return g_envelope_config[track].channel;
}

/**
 * @brief Set CC number
 */
void envelope_cc_set_cc_number(uint8_t track, uint8_t cc_number) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return;
    if (cc_number > 127) cc_number = 127;
    g_envelope_config[track].cc_number = cc_number;
}

/**
 * @brief Get CC number
 */
uint8_t envelope_cc_get_cc_number(uint8_t track) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return 0;
    return g_envelope_config[track].cc_number;
}

/**
 * @brief Set attack time
 */
void envelope_cc_set_attack(uint8_t track, uint16_t time_ms) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return;
    if (time_ms > ENVELOPE_CC_MAX_TIME_MS) time_ms = ENVELOPE_CC_MAX_TIME_MS;
    g_envelope_config[track].attack_ms = time_ms;
}

/**
 * @brief Get attack time
 */
uint16_t envelope_cc_get_attack(uint8_t track) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return 0;
    return g_envelope_config[track].attack_ms;
}

/**
 * @brief Set decay time
 */
void envelope_cc_set_decay(uint8_t track, uint16_t time_ms) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return;
    if (time_ms > ENVELOPE_CC_MAX_TIME_MS) time_ms = ENVELOPE_CC_MAX_TIME_MS;
    g_envelope_config[track].decay_ms = time_ms;
}

/**
 * @brief Get decay time
 */
uint16_t envelope_cc_get_decay(uint8_t track) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return 0;
    return g_envelope_config[track].decay_ms;
}

/**
 * @brief Set sustain level
 */
void envelope_cc_set_sustain(uint8_t track, uint8_t level) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return;
    if (level > 127) level = 127;
    g_envelope_config[track].sustain_level = level;
}

/**
 * @brief Get sustain level
 */
uint8_t envelope_cc_get_sustain(uint8_t track) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return 0;
    return g_envelope_config[track].sustain_level;
}

/**
 * @brief Set release time
 */
void envelope_cc_set_release(uint8_t track, uint16_t time_ms) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return;
    if (time_ms > ENVELOPE_CC_MAX_TIME_MS) time_ms = ENVELOPE_CC_MAX_TIME_MS;
    g_envelope_config[track].release_ms = time_ms;
}

/**
 * @brief Get release time
 */
uint16_t envelope_cc_get_release(uint8_t track) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return 0;
    return g_envelope_config[track].release_ms;
}

/**
 * @brief Set minimum output value
 */
void envelope_cc_set_min_value(uint8_t track, uint8_t min_value) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return;
    if (min_value > 127) min_value = 127;
    g_envelope_config[track].min_value = min_value;
}

/**
 * @brief Get minimum output value
 */
uint8_t envelope_cc_get_min_value(uint8_t track) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return 0;
    return g_envelope_config[track].min_value;
}

/**
 * @brief Set maximum output value
 */
void envelope_cc_set_max_value(uint8_t track, uint8_t max_value) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return;
    if (max_value > 127) max_value = 127;
    g_envelope_config[track].max_value = max_value;
}

/**
 * @brief Get maximum output value
 */
uint8_t envelope_cc_get_max_value(uint8_t track) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return 127;
    return g_envelope_config[track].max_value;
}

/**
 * @brief Trigger envelope
 */
void envelope_cc_trigger(uint8_t track) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return;
    if (!g_envelope_config[track].enabled) return;
    
    envelope_cc_config_t* cfg = &g_envelope_config[track];
    cfg->stage = ENVELOPE_STAGE_ATTACK;
    cfg->stage_start_time = 0;  // Will be set by next tick
}

/**
 * @brief Release envelope
 */
void envelope_cc_release(uint8_t track) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return;
    if (!g_envelope_config[track].enabled) return;
    
    envelope_cc_config_t* cfg = &g_envelope_config[track];
    
    // Only transition to release if not already idle or releasing
    if (cfg->stage != ENVELOPE_STAGE_IDLE && cfg->stage != ENVELOPE_STAGE_RELEASE) {
        cfg->stage = ENVELOPE_STAGE_RELEASE;
        cfg->stage_start_time = 0;  // Will be set by next tick
    }
}

/**
 * @brief Tick function - call every 1ms
 */
void envelope_cc_tick(uint32_t time_ms) {
    for (uint8_t track = 0; track < ENVELOPE_CC_MAX_TRACKS; track++) {
        if (!g_envelope_config[track].enabled) continue;
        
        envelope_cc_config_t* cfg = &g_envelope_config[track];
        
        // Initialize stage start time if needed
        if (cfg->stage_start_time == 0 && cfg->stage != ENVELOPE_STAGE_IDLE) {
            cfg->stage_start_time = time_ms;
        }
        
        // Skip idle envelopes
        if (cfg->stage == ENVELOPE_STAGE_IDLE) continue;
        
        // Calculate current value
        uint8_t new_value = calculate_envelope_value(track, time_ms);
        cfg->current_value = new_value;
        
        // Send CC if value changed
        if (new_value != cfg->last_sent_value) {
            if (g_cc_callback) {
                g_cc_callback(track, cfg->cc_number, new_value, cfg->channel);
            }
            cfg->last_sent_value = new_value;
        }
    }
}

/**
 * @brief Get current envelope stage
 */
envelope_stage_t envelope_cc_get_stage(uint8_t track) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return ENVELOPE_STAGE_IDLE;
    return g_envelope_config[track].stage;
}

/**
 * @brief Get current envelope value
 */
uint8_t envelope_cc_get_value(uint8_t track) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return 0;
    return g_envelope_config[track].current_value;
}

/**
 * @brief Reset envelope state for a track
 */
void envelope_cc_reset(uint8_t track) {
    if (track >= ENVELOPE_CC_MAX_TRACKS) return;
    
    envelope_cc_config_t* cfg = &g_envelope_config[track];
    cfg->stage = ENVELOPE_STAGE_IDLE;
    cfg->current_value = cfg->min_value;
    cfg->last_sent_value = cfg->min_value;
    cfg->stage_start_time = 0;
    
    // Send min value
    if (g_cc_callback) {
        g_cc_callback(track, cfg->cc_number, cfg->min_value, cfg->channel);
    }
}

/**
 * @brief Reset envelope state for all tracks
 */
void envelope_cc_reset_all(void) {
    for (uint8_t i = 0; i < ENVELOPE_CC_MAX_TRACKS; i++) {
        envelope_cc_reset(i);
    }
}

/**
 * @brief Get stage name string
 */
const char* envelope_cc_get_stage_name(envelope_stage_t stage) {
    if (stage >= ENVELOPE_STAGE_COUNT) return "Unknown";
    return stage_names[stage];
}
