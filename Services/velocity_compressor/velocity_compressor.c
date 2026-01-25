/**
 * @file velocity_compressor.c
 * @brief MIDI Velocity Compressor/Limiter implementation
 */

#ifdef STANDALONE_TEST
#include "velocity_compressor.h"
#else
#include "Services/velocity_compressor/velocity_compressor.h"
#endif
#include <string.h>
#include <math.h>

// Ratio names for UI display
static const char* ratio_names[] = {
    "1:1", "2:1", "3:1", "4:1", "6:1", "8:1", "10:1", "∞:1"
};

// Knee type names
static const char* knee_names[] = {
    "Hard", "Soft"
};

// Actual ratio values (slope = 1/ratio)
static const float ratio_values[] = {
    1.0f,      // 1:1 (no compression)
    2.0f,      // 2:1
    3.0f,      // 3:1
    4.0f,      // 4:1
    6.0f,      // 6:1
    8.0f,      // 8:1
    10.0f,     // 10:1
    1000.0f    // ∞:1 (limiter - very high ratio)
};

// Soft knee width in velocity units (transition zone around threshold)
#define SOFT_KNEE_WIDTH 12.0f

// Velocity normalization constants
#define VELOCITY_NORM_FACTOR (1.0f / 127.0f)
#define VELOCITY_DENORM_FACTOR 127.0f

// Per-track compression configuration
typedef struct {
    uint8_t enabled;                    // Compression enabled flag
    uint8_t threshold;                  // Compression threshold (1-127)
    velocity_comp_ratio_t ratio;        // Compression ratio
    int8_t makeup_gain;                 // Makeup gain (-20 to +40)
    velocity_comp_knee_t knee;          // Knee type (hard/soft)
    uint8_t min_velocity;               // Minimum output velocity cap
    uint8_t max_velocity;               // Maximum output velocity cap
} track_config_t;

static track_config_t g_tracks[VELOCITY_COMP_MAX_TRACKS];

// Forward declarations
static float normalize_velocity(uint8_t vel);
static uint8_t denormalize_velocity(float vel);
static float apply_compression(float input, float threshold, float ratio, 
                               velocity_comp_knee_t knee);

/**
 * @brief Normalize velocity to 0.0-1.0 range
 */
static inline float normalize_velocity(uint8_t vel) {
    return (float)vel * VELOCITY_NORM_FACTOR;
}

/**
 * @brief Denormalize velocity from 0.0-1.0 to 1-127 range
 */
static inline uint8_t denormalize_velocity(float vel) {
    if (vel < 0.0f) vel = 0.0f;
    if (vel > 1.0f) vel = 1.0f;
    
    int iv = (int)(vel * VELOCITY_DENORM_FACTOR + 0.5f);
    if (iv < 1) iv = 1;
    if (iv > 127) iv = 127;
    return (uint8_t)iv;
}

/**
 * @brief Apply compression curve
 * @param input Input level (normalized 0.0-1.0)
 * @param threshold Threshold level (normalized 0.0-1.0)
 * @param ratio Compression ratio
 * @param knee Knee type
 * @return Compressed level (normalized 0.0-1.0)
 */
static float apply_compression(float input, float threshold, float ratio, 
                               velocity_comp_knee_t knee) {
    if (input <= threshold) {
        // Below threshold: no compression
        return input;
    }
    
    // Calculate overshoot (how far above threshold)
    float overshoot = input - threshold;
    
    if (knee == COMP_KNEE_HARD) {
        // Hard knee: immediate compression
        // output = threshold + (overshoot / ratio)
        float compressed_overshoot = overshoot / ratio;
        return threshold + compressed_overshoot;
        
    } else {
        // Soft knee: gradual compression
        // Use a smooth transition zone around threshold
        float knee_width = SOFT_KNEE_WIDTH * VELOCITY_NORM_FACTOR;
        float knee_start = threshold - (knee_width / 2.0f);
        float knee_end = threshold + (knee_width / 2.0f);
        
        if (input < knee_start) {
            // Below knee: no compression
            return input;
            
        } else if (input < knee_end) {
            // Within knee: interpolate between no compression and full compression
            // Use a smooth quadratic curve
            float knee_position = (input - knee_start) / knee_width;
            
            // Calculate what output would be with no compression
            float no_comp_output = input;
            
            // Calculate what output would be with full compression
            float overshoot_full = input - threshold;
            float full_comp_output = threshold + (overshoot_full / ratio);
            
            // Blend using smooth curve
            float blend = knee_position * knee_position;
            return no_comp_output + blend * (full_comp_output - no_comp_output);
            
        } else {
            // Above knee: full compression
            float compressed_overshoot = overshoot / ratio;
            return threshold + compressed_overshoot;
        }
    }
}

/**
 * @brief Initialize velocity compressor module
 */
void velocity_compressor_init(void) {
    memset(g_tracks, 0, sizeof(g_tracks));
    
    // Initialize defaults for each track
    for (uint8_t t = 0; t < VELOCITY_COMP_MAX_TRACKS; t++) {
        track_config_t* track = &g_tracks[t];
        track->enabled = 0;
        track->threshold = 80;
        track->ratio = COMP_RATIO_4_1;
        track->makeup_gain = 0;
        track->knee = COMP_KNEE_HARD;
        track->min_velocity = 1;
        track->max_velocity = 127;
    }
}

/**
 * @brief Enable/disable velocity compression for a track
 */
void velocity_compressor_set_enabled(uint8_t track, uint8_t enabled) {
    if (track >= VELOCITY_COMP_MAX_TRACKS) return;
    g_tracks[track].enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if velocity compression is enabled for a track
 */
uint8_t velocity_compressor_is_enabled(uint8_t track) {
    if (track >= VELOCITY_COMP_MAX_TRACKS) return 0;
    return g_tracks[track].enabled;
}

/**
 * @brief Set compression threshold
 */
void velocity_compressor_set_threshold(uint8_t track, uint8_t threshold) {
    if (track >= VELOCITY_COMP_MAX_TRACKS) return;
    if (threshold < 1) threshold = 1;
    if (threshold > 127) threshold = 127;
    g_tracks[track].threshold = threshold;
}

/**
 * @brief Get compression threshold
 */
uint8_t velocity_compressor_get_threshold(uint8_t track) {
    if (track >= VELOCITY_COMP_MAX_TRACKS) return 80;
    return g_tracks[track].threshold;
}

/**
 * @brief Set compression ratio
 */
void velocity_compressor_set_ratio(uint8_t track, velocity_comp_ratio_t ratio) {
    if (track >= VELOCITY_COMP_MAX_TRACKS) return;
    if (ratio >= COMP_RATIO_COUNT) ratio = COMP_RATIO_4_1;
    g_tracks[track].ratio = ratio;
}

/**
 * @brief Get compression ratio
 */
velocity_comp_ratio_t velocity_compressor_get_ratio(uint8_t track) {
    if (track >= VELOCITY_COMP_MAX_TRACKS) return COMP_RATIO_4_1;
    return g_tracks[track].ratio;
}

/**
 * @brief Set makeup gain
 */
void velocity_compressor_set_makeup_gain(uint8_t track, int8_t gain) {
    if (track >= VELOCITY_COMP_MAX_TRACKS) return;
    if (gain < -20) gain = -20;
    if (gain > 40) gain = 40;
    g_tracks[track].makeup_gain = gain;
}

/**
 * @brief Get makeup gain
 */
int8_t velocity_compressor_get_makeup_gain(uint8_t track) {
    if (track >= VELOCITY_COMP_MAX_TRACKS) return 0;
    return g_tracks[track].makeup_gain;
}

/**
 * @brief Set compression knee type
 */
void velocity_compressor_set_knee(uint8_t track, velocity_comp_knee_t knee) {
    if (track >= VELOCITY_COMP_MAX_TRACKS) return;
    if (knee >= COMP_KNEE_COUNT) knee = COMP_KNEE_HARD;
    g_tracks[track].knee = knee;
}

/**
 * @brief Get compression knee type
 */
velocity_comp_knee_t velocity_compressor_get_knee(uint8_t track) {
    if (track >= VELOCITY_COMP_MAX_TRACKS) return COMP_KNEE_HARD;
    return g_tracks[track].knee;
}

/**
 * @brief Set minimum velocity cap
 */
void velocity_compressor_set_min_velocity(uint8_t track, uint8_t min_vel) {
    if (track >= VELOCITY_COMP_MAX_TRACKS) return;
    if (min_vel < 1) min_vel = 1;
    if (min_vel > 127) min_vel = 127;
    g_tracks[track].min_velocity = min_vel;
}

/**
 * @brief Get minimum velocity cap
 */
uint8_t velocity_compressor_get_min_velocity(uint8_t track) {
    if (track >= VELOCITY_COMP_MAX_TRACKS) return 1;
    return g_tracks[track].min_velocity;
}

/**
 * @brief Set maximum velocity cap
 */
void velocity_compressor_set_max_velocity(uint8_t track, uint8_t max_vel) {
    if (track >= VELOCITY_COMP_MAX_TRACKS) return;
    if (max_vel < 1) max_vel = 1;
    if (max_vel > 127) max_vel = 127;
    g_tracks[track].max_velocity = max_vel;
}

/**
 * @brief Get maximum velocity cap
 */
uint8_t velocity_compressor_get_max_velocity(uint8_t track) {
    if (track >= VELOCITY_COMP_MAX_TRACKS) return 127;
    return g_tracks[track].max_velocity;
}

/**
 * @brief Process a velocity value through the compressor
 */
uint8_t velocity_compressor_process(uint8_t track, uint8_t velocity) {
    if (track >= VELOCITY_COMP_MAX_TRACKS) return velocity;
    
    track_config_t* cfg = &g_tracks[track];
    
    // Bypass if disabled
    if (!cfg->enabled) return velocity;
    
    // Clamp input velocity
    if (velocity < 1) velocity = 1;
    if (velocity > 127) velocity = 127;
    
    // Normalize to 0.0-1.0
    float input = normalize_velocity(velocity);
    float threshold = normalize_velocity(cfg->threshold);
    float ratio = ratio_values[cfg->ratio];
    
    // Apply compression
    float compressed = apply_compression(input, threshold, ratio, cfg->knee);
    
    // Apply makeup gain (in normalized space)
    float makeup_norm = (float)cfg->makeup_gain * VELOCITY_NORM_FACTOR;
    compressed += makeup_norm;
    
    // Denormalize back to 1-127
    uint8_t output = denormalize_velocity(compressed);
    
    // Apply min/max caps
    if (output < cfg->min_velocity) output = cfg->min_velocity;
    if (output > cfg->max_velocity) output = cfg->max_velocity;
    
    return output;
}

/**
 * @brief Reset compression settings to defaults for a track
 */
void velocity_compressor_reset_track(uint8_t track) {
    if (track >= VELOCITY_COMP_MAX_TRACKS) return;
    
    track_config_t* cfg = &g_tracks[track];
    cfg->enabled = 0;
    cfg->threshold = 80;
    cfg->ratio = COMP_RATIO_4_1;
    cfg->makeup_gain = 0;
    cfg->knee = COMP_KNEE_HARD;
    cfg->min_velocity = 1;
    cfg->max_velocity = 127;
}

/**
 * @brief Reset all tracks to default settings
 */
void velocity_compressor_reset_all(void) {
    for (uint8_t t = 0; t < VELOCITY_COMP_MAX_TRACKS; t++) {
        velocity_compressor_reset_track(t);
    }
}

/**
 * @brief Get compression ratio name
 */
const char* velocity_compressor_get_ratio_name(velocity_comp_ratio_t ratio) {
    if (ratio >= COMP_RATIO_COUNT) ratio = COMP_RATIO_4_1;
    return ratio_names[ratio];
}

/**
 * @brief Get knee type name
 */
const char* velocity_compressor_get_knee_name(velocity_comp_knee_t knee) {
    if (knee >= COMP_KNEE_COUNT) knee = COMP_KNEE_HARD;
    return knee_names[knee];
}

/**
 * @brief Calculate gain reduction for a given input velocity
 */
uint8_t velocity_compressor_get_gain_reduction(uint8_t track, uint8_t velocity) {
    if (track >= VELOCITY_COMP_MAX_TRACKS) return 0;
    
    track_config_t* cfg = &g_tracks[track];
    
    // No gain reduction if disabled or below threshold
    if (!cfg->enabled || velocity <= cfg->threshold) return 0;
    
    // Calculate what output would be without compression
    float input = normalize_velocity(velocity);
    float threshold = normalize_velocity(cfg->threshold);
    float ratio = ratio_values[cfg->ratio];
    
    // Apply compression
    float compressed = apply_compression(input, threshold, ratio, cfg->knee);
    
    // Gain reduction is the difference (before makeup gain)
    float reduction = input - compressed;
    
    // Convert to velocity units
    uint8_t reduction_vel = (uint8_t)(reduction * VELOCITY_DENORM_FACTOR + 0.5f);
    
    return reduction_vel;
}
