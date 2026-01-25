/**
 * @file cc_smoother.c
 * @brief MIDI CC smoother implementation
 */

#include "Services/cc_smoother/cc_smoother.h"
#include <string.h>
#include <math.h>

// Mode names
static const char* mode_names[] = {
    "Off", "Light", "Medium", "Heavy", "Custom"
};

// Preset parameters for each mode
// Attack/release times in milliseconds, smoothing factor (0.0-1.0)
typedef struct {
    uint16_t attack_ms;
    uint16_t release_ms;
    float smoothing_factor;  // EMA alpha coefficient (0.0 = slow, 1.0 = fast)
} mode_preset_t;

static const mode_preset_t mode_presets[] = {
    {1,    1,    1.0f},    // OFF: No smoothing
    {20,   30,   0.7f},    // LIGHT: Fast response
    {50,   100,  0.4f},    // MEDIUM: Balanced
    {100,  200,  0.2f},    // HEAVY: Slow, smooth
    {50,   100,  0.5f},    // CUSTOM: Default values (user can override)
};

// CC state for each CC number
typedef struct {
    uint8_t enabled;           // Is smoothing enabled for this CC?
    float current_value;       // Current smoothed value (float for precision)
    float target_value;        // Target value from input
    uint8_t last_output;       // Last integer value sent (for change detection)
    uint8_t channel;           // MIDI channel for this CC
    uint32_t last_update_ms;   // Last update timestamp
} cc_state_t;

// Per-track configuration
typedef struct {
    uint8_t enabled;                              // Is smoothing enabled for this track?
    cc_smoother_mode_t mode;                      // Smoothing mode
    uint8_t custom_amount;                        // Custom smoothing amount (0-100)
    uint16_t attack_ms;                           // Attack time in milliseconds
    uint16_t release_ms;                          // Release time in milliseconds
    uint8_t slew_limit;                           // Slew rate limit (max change per ms)
    cc_state_t cc_states[CC_SMOOTHER_MAX_CC_NUMBERS];  // State for each CC number
} track_config_t;

static track_config_t g_tracks[CC_SMOOTHER_MAX_TRACKS];
static uint32_t g_tick_counter = 0;
static cc_smoother_output_cb_t g_output_callback = NULL;

// Forward declarations
static float calculate_smoothing_coefficient(uint16_t time_ms);
static void apply_smoothing(track_config_t* track, cc_state_t* cc, float dt_ms);

/**
 * @brief Initialize CC smoother module
 */
void cc_smoother_init(void) {
    memset(g_tracks, 0, sizeof(g_tracks));
    g_tick_counter = 0;
    g_output_callback = NULL;
    
    // Initialize defaults for each track
    for (uint8_t t = 0; t < CC_SMOOTHER_MAX_TRACKS; t++) {
        track_config_t* track = &g_tracks[t];
        track->enabled = 0;
        track->mode = CC_SMOOTH_MODE_MEDIUM;
        track->custom_amount = 50;  // 50% smoothing
        track->attack_ms = 50;
        track->release_ms = 100;
        track->slew_limit = 127;  // No slew limiting by default
        
        // Enable all CC numbers by default
        for (uint16_t cc = 0; cc < CC_SMOOTHER_MAX_CC_NUMBERS; cc++) {
            track->cc_states[cc].enabled = 1;
            track->cc_states[cc].current_value = 0.0f;
            track->cc_states[cc].target_value = 0.0f;
            track->cc_states[cc].last_output = 0;
            track->cc_states[cc].channel = 0;
            track->cc_states[cc].last_update_ms = 0;
        }
    }
}

/**
 * @brief Enable/disable CC smoothing for a track
 */
void cc_smoother_set_enabled(uint8_t track, uint8_t enabled) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return;
    g_tracks[track].enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if CC smoothing is enabled for a track
 */
uint8_t cc_smoother_is_enabled(uint8_t track) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return 0;
    return g_tracks[track].enabled;
}

/**
 * @brief Set smoothing mode for a track
 */
void cc_smoother_set_mode(uint8_t track, cc_smoother_mode_t mode) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return;
    if (mode >= CC_SMOOTH_MODE_COUNT) return;
    
    track_config_t* t = &g_tracks[track];
    t->mode = mode;
    
    // Apply preset parameters (except for custom mode)
    if (mode != CC_SMOOTH_MODE_CUSTOM) {
        t->attack_ms = mode_presets[mode].attack_ms;
        t->release_ms = mode_presets[mode].release_ms;
    }
}

/**
 * @brief Get smoothing mode for a track
 */
cc_smoother_mode_t cc_smoother_get_mode(uint8_t track) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return CC_SMOOTH_MODE_OFF;
    return g_tracks[track].mode;
}

/**
 * @brief Set smoothing amount (for custom mode)
 */
void cc_smoother_set_amount(uint8_t track, uint8_t amount) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return;
    if (amount > 100) amount = 100;
    g_tracks[track].custom_amount = amount;
}

/**
 * @brief Get smoothing amount
 */
uint8_t cc_smoother_get_amount(uint8_t track) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return 0;
    return g_tracks[track].custom_amount;
}

/**
 * @brief Set attack time
 */
void cc_smoother_set_attack(uint8_t track, uint16_t attack_ms) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return;
    if (attack_ms < 1) attack_ms = 1;
    if (attack_ms > 1000) attack_ms = 1000;
    g_tracks[track].attack_ms = attack_ms;
}

/**
 * @brief Get attack time
 */
uint16_t cc_smoother_get_attack(uint8_t track) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return 0;
    return g_tracks[track].attack_ms;
}

/**
 * @brief Set release time
 */
void cc_smoother_set_release(uint8_t track, uint16_t release_ms) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return;
    if (release_ms < 1) release_ms = 1;
    if (release_ms > 1000) release_ms = 1000;
    g_tracks[track].release_ms = release_ms;
}

/**
 * @brief Get release time
 */
uint16_t cc_smoother_get_release(uint8_t track) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return 0;
    return g_tracks[track].release_ms;
}

/**
 * @brief Set slew rate limit
 */
void cc_smoother_set_slew_limit(uint8_t track, uint8_t slew_limit) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return;
    if (slew_limit < 1) slew_limit = 1;
    if (slew_limit > 127) slew_limit = 127;
    g_tracks[track].slew_limit = slew_limit;
}

/**
 * @brief Get slew rate limit
 */
uint8_t cc_smoother_get_slew_limit(uint8_t track) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return 127;
    return g_tracks[track].slew_limit;
}

/**
 * @brief Enable/disable smoothing for a specific CC number
 */
void cc_smoother_set_cc_enabled(uint8_t track, uint8_t cc_number, uint8_t enabled) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return;
    if (cc_number >= CC_SMOOTHER_MAX_CC_NUMBERS) return;
    g_tracks[track].cc_states[cc_number].enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if smoothing is enabled for a specific CC number
 */
uint8_t cc_smoother_is_cc_enabled(uint8_t track, uint8_t cc_number) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return 0;
    if (cc_number >= CC_SMOOTHER_MAX_CC_NUMBERS) return 0;
    return g_tracks[track].cc_states[cc_number].enabled;
}

/**
 * @brief Calculate smoothing coefficient from time constant
 * 
 * Uses exponential moving average formula:
 * alpha = 1 - exp(-dt / tau)
 * where tau is the time constant in milliseconds
 */
static float calculate_smoothing_coefficient(uint16_t time_ms) {
    if (time_ms < 1) time_ms = 1;
    
    // For 1ms update rate: alpha = 1 - exp(-1 / time_ms)
    // Simplified approximation for embedded systems: alpha ≈ 1 / time_ms
    // More accurate: use expf(-1.0f / time_ms)
    float tau = (float)time_ms;
    float alpha = 1.0f - expf(-1.0f / tau);
    
    // Clamp to valid range
    if (alpha < 0.001f) alpha = 0.001f;
    if (alpha > 1.0f) alpha = 1.0f;
    
    return alpha;
}

/**
 * @brief Apply smoothing to a CC value
 */
static void apply_smoothing(track_config_t* track, cc_state_t* cc, float dt_ms) {
    if (dt_ms < 0.1f) return;  // Skip if time delta too small
    
    // Determine if we're attacking (increasing) or releasing (decreasing)
    float diff = cc->target_value - cc->current_value;
    uint16_t time_constant;
    
    if (diff > 0.0f) {
        time_constant = track->attack_ms;
    } else if (diff < 0.0f) {
        time_constant = track->release_ms;
    } else {
        return;  // Already at target
    }
    
    // Calculate smoothing coefficient based on mode
    float alpha;
    if (track->mode == CC_SMOOTH_MODE_OFF) {
        alpha = 1.0f;  // No smoothing
    } else if (track->mode == CC_SMOOTH_MODE_CUSTOM) {
        // Custom mode: use amount to scale time constant
        // Amount 0-100 maps to faster/slower response
        float scale = 1.0f + ((100 - track->custom_amount) / 100.0f) * 4.0f;  // 1.0 to 5.0x
        alpha = calculate_smoothing_coefficient((uint16_t)(time_constant / scale));
    } else {
        // Use preset smoothing factor
        alpha = mode_presets[track->mode].smoothing_factor;
    }
    
    // Apply exponential moving average
    // y[n] = alpha * target + (1 - alpha) * y[n-1]
    float new_value = alpha * cc->target_value + (1.0f - alpha) * cc->current_value;
    
    // Apply slew rate limiting
    if (track->slew_limit < 127) {
        float max_change = track->slew_limit * dt_ms;
        float change = new_value - cc->current_value;
        
        if (change > max_change) {
            new_value = cc->current_value + max_change;
        } else if (change < -max_change) {
            new_value = cc->current_value - max_change;
        }
    }
    
    cc->current_value = new_value;
}

/**
 * @brief Process a CC message
 */
uint8_t cc_smoother_process(uint8_t track, uint8_t cc_number, uint8_t value) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return value;
    if (cc_number >= CC_SMOOTHER_MAX_CC_NUMBERS) return value;
    
    track_config_t* t = &g_tracks[track];
    
    // If smoothing disabled or CC not enabled, pass through
    if (!t->enabled || !t->cc_states[cc_number].enabled || t->mode == CC_SMOOTH_MODE_OFF) {
        return value;
    }
    
    cc_state_t* cc = &t->cc_states[cc_number];
    
    // Update target value
    cc->target_value = (float)value;
    cc->last_update_ms = g_tick_counter;
    
    // If this is the first value or mode is OFF, snap to target
    if (cc->current_value == 0.0f && cc->last_output == 0 && value > 0) {
        cc->current_value = (float)value;
    }
    
    // Apply smoothing
    apply_smoothing(t, cc, 1.0f);  // 1ms time step
    
    // Convert to integer
    uint8_t output = (uint8_t)(cc->current_value + 0.5f);  // Round to nearest
    if (output > 127) output = 127;
    
    cc->last_output = output;
    return output;
}

/**
 * @brief Update smoothing (call every 1ms)
 */
void cc_smoother_tick_1ms(void) {
    g_tick_counter++;
    
    for (uint8_t t = 0; t < CC_SMOOTHER_MAX_TRACKS; t++) {
        track_config_t* track = &g_tracks[t];
        if (!track->enabled) continue;
        
        for (uint16_t cc_num = 0; cc_num < CC_SMOOTHER_MAX_CC_NUMBERS; cc_num++) {
            cc_state_t* cc = &track->cc_states[cc_num];
            if (!cc->enabled) continue;
            
            // Skip if no updates recently (idle CC)
            uint32_t idle_time = g_tick_counter - cc->last_update_ms;
            if (idle_time > 1000) continue;  // Idle for more than 1 second
            
            // Check if we're close enough to target
            float diff = fabsf(cc->target_value - cc->current_value);
            if (diff < 0.1f) continue;  // Close enough
            
            // Apply smoothing
            apply_smoothing(track, cc, 1.0f);
            
            // Convert to integer and check if changed
            uint8_t output = (uint8_t)(cc->current_value + 0.5f);
            if (output > 127) output = 127;
            
            // If value changed and we have a callback, send it
            if (output != cc->last_output && g_output_callback) {
                g_output_callback(t, cc_num, output, cc->channel);
                cc->last_output = output;
            }
        }
    }
}

/**
 * @brief Reset all smoothing state for a track
 */
void cc_smoother_reset_track(uint8_t track) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return;
    
    track_config_t* t = &g_tracks[track];
    for (uint16_t cc = 0; cc < CC_SMOOTHER_MAX_CC_NUMBERS; cc++) {
        t->cc_states[cc].current_value = t->cc_states[cc].target_value;
        t->cc_states[cc].last_output = (uint8_t)(t->cc_states[cc].target_value + 0.5f);
    }
}

/**
 * @brief Reset smoothing state for a specific CC number
 */
void cc_smoother_reset_cc(uint8_t track, uint8_t cc_number) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return;
    if (cc_number >= CC_SMOOTHER_MAX_CC_NUMBERS) return;
    
    cc_state_t* cc = &g_tracks[track].cc_states[cc_number];
    cc->current_value = cc->target_value;
    cc->last_output = (uint8_t)(cc->target_value + 0.5f);
}

/**
 * @brief Reset all smoothing state for all tracks
 */
void cc_smoother_reset_all(void) {
    for (uint8_t t = 0; t < CC_SMOOTHER_MAX_TRACKS; t++) {
        cc_smoother_reset_track(t);
    }
}

/**
 * @brief Get current smoothed value for a CC
 */
uint8_t cc_smoother_get_current_value(uint8_t track, uint8_t cc_number) {
    if (track >= CC_SMOOTHER_MAX_TRACKS) return 0;
    if (cc_number >= CC_SMOOTHER_MAX_CC_NUMBERS) return 0;
    
    return g_tracks[track].cc_states[cc_number].last_output;
}

/**
 * @brief Get smoothing mode name
 */
const char* cc_smoother_get_mode_name(cc_smoother_mode_t mode) {
    if (mode >= CC_SMOOTH_MODE_COUNT) return "Unknown";
    return mode_names[mode];
}

/**
 * @brief Set output callback for smoothed CC messages
 */
void cc_smoother_set_output_callback(cc_smoother_output_cb_t callback) {
    g_output_callback = callback;
}
