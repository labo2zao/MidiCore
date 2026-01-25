/**
 * @file register_coupling.c
 * @brief Register Coupling implementation
 */

#include "Services/register_coupling/register_coupling.h"
#include <string.h>

static const char* register_names[] = {
    "Master", "Musette", "Bandoneon", "Accordion", "Violin",
    "Clarinet", "Bassoon", "Piccolo", "Organ", "Harmonium",
    "Oboe", "Flute", "Custom 1", "Custom 2", "Custom 3", "Custom 4"
};

// Standard reed configurations for each register
// L1, L2, M1, M2, H
static const reed_set_config_t preset_configs[] = {
    {1, 1, 1, 1, 1},  // MASTER - all reeds
    {1, 0, 1, 0, 1},  // MUSETTE - L-M-H
    {1, 0, 1, 0, 0},  // BANDONEON - L-M
    {0, 0, 1, 1, 0},  // ACCORDION - M-M
    {0, 0, 1, 0, 1},  // VIOLIN - M-H
    {0, 0, 1, 0, 0},  // CLARINET - M
    {1, 0, 0, 0, 0},  // BASSOON - L
    {0, 0, 0, 0, 1},  // PICCOLO - H
    {0, 0, 1, 1, 1},  // ORGAN - M-M-H
    {1, 0, 1, 1, 1},  // HARMONIUM - L-M-M-H
    {1, 0, 0, 0, 1},  // OBOE - L-H
    {0, 0, 1, 0, 1},  // FLUTE - M-H (same as violin but different voicing)
    {0, 0, 0, 0, 0},  // CUSTOM_1
    {0, 0, 0, 0, 0},  // CUSTOM_2
    {0, 0, 0, 0, 0},  // CUSTOM_3
    {0, 0, 0, 0, 0},  // CUSTOM_4
};

typedef struct {
    accordion_register_t current_register;
    uint8_t smooth_transition;
    uint16_t transition_time_ms;
    uint8_t memory_enabled;
    reed_set_config_t custom_configs[4];  // For CUSTOM_1-4
    uint8_t transitioning;
    uint32_t transition_start_ms;
    accordion_register_t previous_register;
} reg_coupling_config_t;

static reg_coupling_config_t g_coupling[REG_COUPLING_MAX_TRACKS];
static uint32_t g_tick_counter = 0;
static reg_coupling_output_cb_t g_output_callback = NULL;

/**
 * @brief Initialize register coupling module
 */
void reg_coupling_init(void) {
    memset(g_coupling, 0, sizeof(g_coupling));
    
    for (uint8_t t = 0; t < REG_COUPLING_MAX_TRACKS; t++) {
        g_coupling[t].current_register = REG_CLARINET;  // Default single reed
        g_coupling[t].smooth_transition = 1;
        g_coupling[t].transition_time_ms = 50;
        g_coupling[t].memory_enabled = 0;
    }
}

/**
 * @brief Set current register
 */
void reg_coupling_set_register(uint8_t track, accordion_register_t reg) {
    if (track >= REG_COUPLING_MAX_TRACKS) return;
    if (reg >= REG_COUNT) return;
    
    reg_coupling_config_t* cfg = &g_coupling[track];
    
    if (cfg->smooth_transition && reg != cfg->current_register) {
        cfg->previous_register = cfg->current_register;
        cfg->transitioning = 1;
        cfg->transition_start_ms = g_tick_counter;
    }
    
    cfg->current_register = reg;
}

/**
 * @brief Get current register
 */
accordion_register_t reg_coupling_get_register(uint8_t track) {
    if (track >= REG_COUPLING_MAX_TRACKS) return REG_CLARINET;
    return g_coupling[track].current_register;
}

/**
 * @brief Get reed configuration for a register
 */
void reg_coupling_get_reed_config(uint8_t track, accordion_register_t reg, reed_set_config_t* config) {
    if (track >= REG_COUPLING_MAX_TRACKS || !config) return;
    if (reg >= REG_COUNT) return;
    
    // Check if it's a custom register
    if (reg >= REG_CUSTOM_1 && reg <= REG_CUSTOM_4) {
        uint8_t custom_idx = reg - REG_CUSTOM_1;
        *config = g_coupling[track].custom_configs[custom_idx];
    } else {
        *config = preset_configs[reg];
    }
}

/**
 * @brief Set custom reed configuration
 */
void reg_coupling_set_custom_config(uint8_t track, accordion_register_t reg, const reed_set_config_t* config) {
    if (track >= REG_COUPLING_MAX_TRACKS || !config) return;
    if (reg < REG_CUSTOM_1 || reg > REG_CUSTOM_4) return;
    
    uint8_t custom_idx = reg - REG_CUSTOM_1;
    g_coupling[track].custom_configs[custom_idx] = *config;
}

/**
 * @brief Enable/disable smooth register transitions
 */
void reg_coupling_set_smooth_transition(uint8_t track, uint8_t enabled) {
    if (track >= REG_COUPLING_MAX_TRACKS) return;
    g_coupling[track].smooth_transition = enabled ? 1 : 0;
}

/**
 * @brief Check if smooth transitions are enabled
 */
uint8_t reg_coupling_is_smooth_transition(uint8_t track) {
    if (track >= REG_COUPLING_MAX_TRACKS) return 0;
    return g_coupling[track].smooth_transition;
}

/**
 * @brief Set transition time
 */
void reg_coupling_set_transition_time(uint8_t track, uint16_t ms) {
    if (track >= REG_COUPLING_MAX_TRACKS) return;
    if (ms < 10) ms = 10;
    if (ms > 500) ms = 500;
    g_coupling[track].transition_time_ms = ms;
}

/**
 * @brief Get transition time
 */
uint16_t reg_coupling_get_transition_time(uint8_t track) {
    if (track >= REG_COUPLING_MAX_TRACKS) return 50;
    return g_coupling[track].transition_time_ms;
}

/**
 * @brief Enable/disable register memory
 */
void reg_coupling_set_memory_enabled(uint8_t track, uint8_t enabled) {
    if (track >= REG_COUPLING_MAX_TRACKS) return;
    g_coupling[track].memory_enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if register memory is enabled
 */
uint8_t reg_coupling_is_memory_enabled(uint8_t track) {
    if (track >= REG_COUPLING_MAX_TRACKS) return 0;
    return g_coupling[track].memory_enabled;
}

/**
 * @brief Process note with current register configuration
 */
void reg_coupling_process_note(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel) {
    if (track >= REG_COUPLING_MAX_TRACKS) return;
    if (!g_output_callback) return;
    
    reg_coupling_config_t* cfg = &g_coupling[track];
    reed_set_config_t config;
    reg_coupling_get_reed_config(track, cfg->current_register, &config);
    
    // Send note for each active reed
    const uint8_t* reeds = (const uint8_t*)&config;
    for (uint8_t i = 0; i < 5; i++) {
        if (reeds[i]) {
            g_output_callback(track, note, velocity, channel, i);
        }
    }
}

/**
 * @brief Cycle to next register
 */
void reg_coupling_next_register(uint8_t track) {
    if (track >= REG_COUPLING_MAX_TRACKS) return;
    
    accordion_register_t current = g_coupling[track].current_register;
    accordion_register_t next = (accordion_register_t)((current + 1) % REG_COUNT);
    reg_coupling_set_register(track, next);
}

/**
 * @brief Cycle to previous register
 */
void reg_coupling_prev_register(uint8_t track) {
    if (track >= REG_COUPLING_MAX_TRACKS) return;
    
    accordion_register_t current = g_coupling[track].current_register;
    accordion_register_t prev = (accordion_register_t)((current + REG_COUNT - 1) % REG_COUNT);
    reg_coupling_set_register(track, prev);
}

/**
 * @brief Get register name
 */
const char* reg_coupling_get_register_name(accordion_register_t reg) {
    if (reg >= REG_COUNT) return "Unknown";
    return register_names[reg];
}

/**
 * @brief Called every 1ms for smooth transitions
 */
void reg_coupling_tick_1ms(void) {
    g_tick_counter++;
    
    // Handle smooth transitions
    for (uint8_t t = 0; t < REG_COUPLING_MAX_TRACKS; t++) {
        reg_coupling_config_t* cfg = &g_coupling[t];
        
        if (cfg->transitioning) {
            uint32_t elapsed = g_tick_counter - cfg->transition_start_ms;
            if (elapsed >= cfg->transition_time_ms) {
                cfg->transitioning = 0;
            }
        }
    }
}

/**
 * @brief Set output callback
 */
void reg_coupling_set_output_callback(reg_coupling_output_cb_t callback) {
    g_output_callback = callback;
}
