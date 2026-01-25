/**
 * @file bellows_expression.c
 * @brief Bellows Expression implementation
 */

#include "Services/bellows_expression/bellows_expression.h"
#include <string.h>

typedef struct {
    bellows_curve_t curve;
    int32_t min_pa;
    int32_t max_pa;
    uint8_t bidirectional;
    uint8_t expression_cc;
    uint8_t breath_cc;
    uint8_t smoothing;
    uint16_t attack_ms;
    uint16_t release_ms;
    bellows_direction_t direction;
    uint8_t current_expression;
    uint8_t target_expression;
    uint32_t last_update_ms;
} bellows_config_t;

static bellows_config_t g_bellows[BELLOWS_MAX_TRACKS];
static uint32_t g_tick_counter = 0;
static bellows_cc_output_cb_t g_output_callback = NULL;

/**
 * @brief Apply expression curve
 */
static uint8_t apply_curve(bellows_curve_t curve, uint8_t linear_value) {
    uint16_t value = linear_value;
    
    switch (curve) {
        case BELLOWS_CURVE_LINEAR:
            return linear_value;
            
        case BELLOWS_CURVE_EXPONENTIAL:
            // y = x^2 / 127
            value = (linear_value * linear_value) / 127;
            break;
            
        case BELLOWS_CURVE_LOGARITHMIC:
            // y = sqrt(x * 127)
            value = 127;
            for (uint8_t i = 0; i < linear_value; i++) {
                if ((i * i) / 127 > linear_value) {
                    value = i;
                    break;
                }
            }
            break;
            
        case BELLOWS_CURVE_S_CURVE:
            // Simple S-curve approximation
            if (linear_value < 32) {
                value = linear_value / 2;
            } else if (linear_value < 96) {
                value = 16 + ((linear_value - 32) * 3) / 2;
            } else {
                value = 112 + (linear_value - 96) / 2;
            }
            break;
            
        default:
            return linear_value;
    }
    
    if (value > 127) value = 127;
    return (uint8_t)value;
}

/**
 * @brief Initialize bellows expression module
 */
void bellows_init(void) {
    memset(g_bellows, 0, sizeof(g_bellows));
    
    for (uint8_t t = 0; t < BELLOWS_MAX_TRACKS; t++) {
        g_bellows[t].curve = BELLOWS_CURVE_LINEAR;
        g_bellows[t].min_pa = -500;   // -500 Pa
        g_bellows[t].max_pa = 500;    // +500 Pa
        g_bellows[t].bidirectional = 1;
        g_bellows[t].expression_cc = 11;  // Expression
        g_bellows[t].breath_cc = 2;       // Breath
        g_bellows[t].smoothing = 30;      // 30% smoothing
        g_bellows[t].attack_ms = 10;
        g_bellows[t].release_ms = 50;
        g_bellows[t].direction = BELLOWS_DIR_NEUTRAL;
    }
}

/**
 * @brief Set expression curve
 */
void bellows_set_curve(uint8_t track, bellows_curve_t curve) {
    if (track >= BELLOWS_MAX_TRACKS) return;
    if (curve >= BELLOWS_CURVE_COUNT) return;
    g_bellows[track].curve = curve;
}

/**
 * @brief Get expression curve
 */
bellows_curve_t bellows_get_curve(uint8_t track) {
    if (track >= BELLOWS_MAX_TRACKS) return BELLOWS_CURVE_LINEAR;
    return g_bellows[track].curve;
}

/**
 * @brief Set pressure range
 */
void bellows_set_pressure_range(uint8_t track, int32_t min_pa, int32_t max_pa) {
    if (track >= BELLOWS_MAX_TRACKS) return;
    g_bellows[track].min_pa = min_pa;
    g_bellows[track].max_pa = max_pa;
}

/**
 * @brief Get pressure range
 */
void bellows_get_pressure_range(uint8_t track, int32_t* min_pa, int32_t* max_pa) {
    if (track >= BELLOWS_MAX_TRACKS) return;
    if (min_pa) *min_pa = g_bellows[track].min_pa;
    if (max_pa) *max_pa = g_bellows[track].max_pa;
}

/**
 * @brief Enable/disable bidirectional mode
 */
void bellows_set_bidirectional(uint8_t track, uint8_t enabled) {
    if (track >= BELLOWS_MAX_TRACKS) return;
    g_bellows[track].bidirectional = enabled ? 1 : 0;
}

/**
 * @brief Check if bidirectional mode is enabled
 */
uint8_t bellows_is_bidirectional(uint8_t track) {
    if (track >= BELLOWS_MAX_TRACKS) return 0;
    return g_bellows[track].bidirectional;
}

/**
 * @brief Set expression CC number
 */
void bellows_set_expression_cc(uint8_t track, uint8_t cc_num) {
    if (track >= BELLOWS_MAX_TRACKS) return;
    if (cc_num > 127) cc_num = 127;
    g_bellows[track].expression_cc = cc_num;
}

/**
 * @brief Get expression CC number
 */
uint8_t bellows_get_expression_cc(uint8_t track) {
    if (track >= BELLOWS_MAX_TRACKS) return 11;
    return g_bellows[track].expression_cc;
}

/**
 * @brief Set breath CC number
 */
void bellows_set_breath_cc(uint8_t track, uint8_t cc_num) {
    if (track >= BELLOWS_MAX_TRACKS) return;
    if (cc_num > 127) cc_num = 127;
    g_bellows[track].breath_cc = cc_num;
}

/**
 * @brief Get breath CC number
 */
uint8_t bellows_get_breath_cc(uint8_t track) {
    if (track >= BELLOWS_MAX_TRACKS) return 2;
    return g_bellows[track].breath_cc;
}

/**
 * @brief Set smoothing amount
 */
void bellows_set_smoothing(uint8_t track, uint8_t amount) {
    if (track >= BELLOWS_MAX_TRACKS) return;
    if (amount > 100) amount = 100;
    g_bellows[track].smoothing = amount;
}

/**
 * @brief Get smoothing amount
 */
uint8_t bellows_get_smoothing(uint8_t track) {
    if (track >= BELLOWS_MAX_TRACKS) return 30;
    return g_bellows[track].smoothing;
}

/**
 * @brief Set attack/release time
 */
void bellows_set_attack_release(uint8_t track, uint16_t attack_ms, uint16_t release_ms) {
    if (track >= BELLOWS_MAX_TRACKS) return;
    g_bellows[track].attack_ms = attack_ms;
    g_bellows[track].release_ms = release_ms;
}

/**
 * @brief Process bellows pressure reading
 */
void bellows_process_pressure(uint8_t track, int32_t pressure_pa, uint8_t channel) {
    if (track >= BELLOWS_MAX_TRACKS) return;
    
    bellows_config_t* cfg = &g_bellows[track];
    
    // Determine direction
    if (pressure_pa > 10) {
        cfg->direction = BELLOWS_DIR_PUSH;
    } else if (pressure_pa < -10) {
        cfg->direction = BELLOWS_DIR_PULL;
    } else {
        cfg->direction = BELLOWS_DIR_NEUTRAL;
    }
    
    // Normalize pressure to 0-127
    int32_t range = cfg->max_pa - cfg->min_pa;
    if (range == 0) range = 1;
    
    int32_t normalized = ((pressure_pa - cfg->min_pa) * 127) / range;
    if (normalized < 0) normalized = 0;
    if (normalized > 127) normalized = 127;
    
    // Apply curve
    uint8_t curved_value = apply_curve(cfg->curve, (uint8_t)normalized);
    
    // Apply smoothing (exponential moving average)
    if (cfg->smoothing > 0) {
        uint16_t alpha = 100 - cfg->smoothing;  // Inverted for EMA
        cfg->target_expression = curved_value;
        curved_value = (uint8_t)(((uint16_t)cfg->current_expression * cfg->smoothing + 
                                   (uint16_t)curved_value * alpha) / 100);
    }
    
    // Update current value
    if (curved_value != cfg->current_expression) {
        cfg->current_expression = curved_value;
        
        // Send CC messages
        if (g_output_callback) {
            g_output_callback(track, cfg->expression_cc, curved_value, channel);
            g_output_callback(track, cfg->breath_cc, curved_value, channel);
        }
    }
    
    cfg->last_update_ms = g_tick_counter;
}

/**
 * @brief Get current bellows direction
 */
bellows_direction_t bellows_get_direction(uint8_t track) {
    if (track >= BELLOWS_MAX_TRACKS) return BELLOWS_DIR_NEUTRAL;
    return g_bellows[track].direction;
}

/**
 * @brief Get current expression value
 */
uint8_t bellows_get_expression_value(uint8_t track) {
    if (track >= BELLOWS_MAX_TRACKS) return 0;
    return g_bellows[track].current_expression;
}

/**
 * @brief Called every 1ms for smoothing
 */
void bellows_tick_1ms(void) {
    g_tick_counter++;
    // Smoothing is handled in process_pressure
}

/**
 * @brief Set output callback
 */
void bellows_set_output_callback(bellows_cc_output_cb_t callback) {
    g_output_callback = callback;
}
