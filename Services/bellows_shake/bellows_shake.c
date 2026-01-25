/**
 * @file bellows_shake.c
 * @brief Bellows Shake implementation
 */

#include "Services/bellows_shake/bellows_shake.h"
#include <string.h>

#define HISTORY_SIZE 32  // Pressure history for shake detection

typedef struct {
    int32_t pressure;
    uint32_t timestamp_ms;
} pressure_sample_t;

typedef struct {
    uint8_t enabled;
    uint8_t sensitivity;
    uint8_t depth;
    shake_target_t target;
    uint8_t min_freq_hz;
    uint8_t max_freq_hz;
    pressure_sample_t history[HISTORY_SIZE];
    uint8_t history_head;
    uint8_t shake_detected;
    uint8_t detected_freq_hz;
    uint8_t current_modulation;
    uint32_t last_zero_crossing;
    uint8_t oscillation_count;
} bellows_shake_config_t;

static bellows_shake_config_t g_shake[BELLOWS_SHAKE_MAX_TRACKS];
static uint32_t g_tick_counter = 0;
static bellows_shake_cc_output_cb_t g_cc_callback = NULL;
static bellows_shake_pb_output_cb_t g_pb_callback = NULL;

/**
 * @brief Initialize bellows shake module
 */
void bellows_shake_init(void) {
    memset(g_shake, 0, sizeof(g_shake));
    
    for (uint8_t t = 0; t < BELLOWS_SHAKE_MAX_TRACKS; t++) {
        g_shake[t].enabled = 0;
        g_shake[t].sensitivity = 50;
        g_shake[t].depth = 50;
        g_shake[t].target = SHAKE_TARGET_VOLUME;
        g_shake[t].min_freq_hz = 4;   // 4 Hz minimum
        g_shake[t].max_freq_hz = 12;  // 12 Hz maximum
    }
}

/**
 * @brief Enable/disable shake detection
 */
void bellows_shake_set_enabled(uint8_t track, uint8_t enabled) {
    if (track >= BELLOWS_SHAKE_MAX_TRACKS) return;
    g_shake[track].enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if shake detection is enabled
 */
uint8_t bellows_shake_is_enabled(uint8_t track) {
    if (track >= BELLOWS_SHAKE_MAX_TRACKS) return 0;
    return g_shake[track].enabled;
}

/**
 * @brief Set shake sensitivity
 */
void bellows_shake_set_sensitivity(uint8_t track, uint8_t sensitivity) {
    if (track >= BELLOWS_SHAKE_MAX_TRACKS) return;
    if (sensitivity > 100) sensitivity = 100;
    g_shake[track].sensitivity = sensitivity;
}

/**
 * @brief Get shake sensitivity
 */
uint8_t bellows_shake_get_sensitivity(uint8_t track) {
    if (track >= BELLOWS_SHAKE_MAX_TRACKS) return 50;
    return g_shake[track].sensitivity;
}

/**
 * @brief Set tremolo depth
 */
void bellows_shake_set_depth(uint8_t track, uint8_t depth) {
    if (track >= BELLOWS_SHAKE_MAX_TRACKS) return;
    if (depth > 100) depth = 100;
    g_shake[track].depth = depth;
}

/**
 * @brief Get tremolo depth
 */
uint8_t bellows_shake_get_depth(uint8_t track) {
    if (track >= BELLOWS_SHAKE_MAX_TRACKS) return 50;
    return g_shake[track].depth;
}

/**
 * @brief Set tremolo target
 */
void bellows_shake_set_target(uint8_t track, shake_target_t target) {
    if (track >= BELLOWS_SHAKE_MAX_TRACKS) return;
    if (target >= SHAKE_TARGET_COUNT) return;
    g_shake[track].target = target;
}

/**
 * @brief Get tremolo target
 */
shake_target_t bellows_shake_get_target(uint8_t track) {
    if (track >= BELLOWS_SHAKE_MAX_TRACKS) return SHAKE_TARGET_VOLUME;
    return g_shake[track].target;
}

/**
 * @brief Set frequency range
 */
void bellows_shake_set_freq_range(uint8_t track, uint8_t min_hz, uint8_t max_hz) {
    if (track >= BELLOWS_SHAKE_MAX_TRACKS) return;
    if (min_hz < 2) min_hz = 2;
    if (max_hz > 20) max_hz = 20;
    if (min_hz > max_hz) min_hz = max_hz;
    
    g_shake[track].min_freq_hz = min_hz;
    g_shake[track].max_freq_hz = max_hz;
}

/**
 * @brief Get frequency range
 */
void bellows_shake_get_freq_range(uint8_t track, uint8_t* min_hz, uint8_t* max_hz) {
    if (track >= BELLOWS_SHAKE_MAX_TRACKS) return;
    if (min_hz) *min_hz = g_shake[track].min_freq_hz;
    if (max_hz) *max_hz = g_shake[track].max_freq_hz;
}

/**
 * @brief Detect zero crossings and calculate frequency
 */
static void detect_shake(uint8_t track) {
    bellows_shake_config_t* cfg = &g_shake[track];
    
    // Simple zero-crossing detection
    // Look at recent pressure samples
    if (cfg->history_head < 2) return;
    
    uint8_t prev_idx = (cfg->history_head - 1) % HISTORY_SIZE;
    uint8_t curr_idx = cfg->history_head % HISTORY_SIZE;
    
    int32_t prev_pressure = cfg->history[prev_idx].pressure;
    int32_t curr_pressure = cfg->history[curr_idx].pressure;
    
    // Check for zero crossing
    if ((prev_pressure < 0 && curr_pressure >= 0) || 
        (prev_pressure >= 0 && curr_pressure < 0)) {
        
        uint32_t current_time = cfg->history[curr_idx].timestamp_ms;
        
        if (cfg->last_zero_crossing > 0) {
            uint32_t period_ms = current_time - cfg->last_zero_crossing;
            
            if (period_ms > 0) {
                // Calculate frequency
                uint16_t freq_hz = (1000 / period_ms) / 2;  // Divide by 2 for full period
                
                // Check if within valid range
                if (freq_hz >= cfg->min_freq_hz && freq_hz <= cfg->max_freq_hz) {
                    cfg->oscillation_count++;
                    
                    // Need at least 2 cycles to confirm
                    if (cfg->oscillation_count >= 2) {
                        cfg->shake_detected = 1;
                        cfg->detected_freq_hz = (uint8_t)freq_hz;
                    }
                } else {
                    cfg->oscillation_count = 0;
                    cfg->shake_detected = 0;
                }
            }
        }
        
        cfg->last_zero_crossing = current_time;
    }
    
    // Timeout shake detection if no oscillations for 500ms
    if (cfg->shake_detected) {
        uint32_t elapsed = g_tick_counter - cfg->last_zero_crossing;
        if (elapsed > 500) {
            cfg->shake_detected = 0;
            cfg->oscillation_count = 0;
        }
    }
}

/**
 * @brief Process bellows pressure reading
 */
void bellows_shake_process_pressure(uint8_t track, int32_t pressure_pa,
                                   uint32_t timestamp_ms, uint8_t channel) {
    if (track >= BELLOWS_SHAKE_MAX_TRACKS) return;
    
    bellows_shake_config_t* cfg = &g_shake[track];
    if (!cfg->enabled) return;
    
    // Add to history
    uint8_t idx = cfg->history_head % HISTORY_SIZE;
    cfg->history[idx].pressure = pressure_pa;
    cfg->history[idx].timestamp_ms = timestamp_ms;
    cfg->history_head++;
    
    // Detect shake
    detect_shake(track);
    
    // Generate tremolo if shake detected
    if (cfg->shake_detected) {
        // Simple sine-wave approximation based on detected frequency
        // Phase = (timestamp * freq * 2π) / 1000
        uint32_t phase = (timestamp_ms * cfg->detected_freq_hz) % 1000;
        
        // Convert to modulation value (0-127)
        // Simple triangle wave for now
        uint8_t mod_value;
        if (phase < 500) {
            mod_value = (uint8_t)((phase * 127) / 500);
        } else {
            mod_value = (uint8_t)(127 - ((phase - 500) * 127) / 500);
        }
        
        // Apply depth
        mod_value = 64 + (((int16_t)mod_value - 64) * cfg->depth) / 100;
        cfg->current_modulation = mod_value;
        
        // Output based on target
        if (cfg->target == SHAKE_TARGET_VOLUME && g_cc_callback) {
            g_cc_callback(track, 11, mod_value, channel);  // Expression CC
        } else if (cfg->target == SHAKE_TARGET_PITCH && g_pb_callback) {
            int16_t pb = ((int16_t)mod_value - 64) * 64;  // ±4096 range
            g_pb_callback(track, pb, channel);
        } else if (cfg->target == SHAKE_TARGET_FILTER && g_cc_callback) {
            g_cc_callback(track, 74, mod_value, channel);  // Filter CC
        } else if (cfg->target == SHAKE_TARGET_BOTH) {
            if (g_cc_callback) g_cc_callback(track, 11, mod_value, channel);
            if (g_pb_callback) {
                int16_t pb = ((int16_t)mod_value - 64) * 32;  // Smaller pitch mod
                g_pb_callback(track, pb, channel);
            }
        }
    }
}

/**
 * @brief Get current shake detection state
 */
uint8_t bellows_shake_is_detected(uint8_t track) {
    if (track >= BELLOWS_SHAKE_MAX_TRACKS) return 0;
    return g_shake[track].shake_detected;
}

/**
 * @brief Get detected shake frequency
 */
uint8_t bellows_shake_get_frequency(uint8_t track) {
    if (track >= BELLOWS_SHAKE_MAX_TRACKS) return 0;
    return g_shake[track].detected_freq_hz;
}

/**
 * @brief Get current tremolo modulation value
 */
uint8_t bellows_shake_get_modulation(uint8_t track) {
    if (track >= BELLOWS_SHAKE_MAX_TRACKS) return 64;
    return g_shake[track].current_modulation;
}

/**
 * @brief Called every 1ms for processing
 */
void bellows_shake_tick_1ms(void) {
    g_tick_counter++;
}

/**
 * @brief Set CC output callback
 */
void bellows_shake_set_cc_callback(bellows_shake_cc_output_cb_t callback) {
    g_cc_callback = callback;
}

/**
 * @brief Set pitchbend output callback
 */
void bellows_shake_set_pb_callback(bellows_shake_pb_output_cb_t callback) {
    g_pb_callback = callback;
}
