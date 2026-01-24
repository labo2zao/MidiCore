/**
 * @file gate_time.c
 * @brief Note Length/Gate Time Control implementation
 */

#include "Services/gate_time/gate_time.h"
#include <string.h>

#define DEFAULT_PERCENT 100
#define MIN_PERCENT 10
#define MAX_PERCENT 200
#define DEFAULT_FIXED_MS 500
#define DEFAULT_FIXED_TICKS 96

// Mode name strings
static const char* mode_names[] = {
    "Percent",
    "Fixed ms",
    "Fixed ticks"
};

// Per-track gate time configuration
typedef struct {
    uint8_t enabled;
    gate_time_mode_t mode;
    uint16_t value;                              // Mode-dependent value
    uint16_t min_length_ms;                      // Minimum gate time (0=no limit)
    uint16_t max_length_ms;                      // Maximum gate time (0=no limit)
    gate_time_note_t notes[GATE_TIME_MAX_NOTES_PER_TRACK];
    uint8_t note_count;
    uint32_t total_notes_processed;
} gate_time_config_t;

static gate_time_config_t g_gate_time_config[GATE_TIME_MAX_TRACKS];
static gate_time_note_callback_t g_note_callback = NULL;

/**
 * @brief Find note slot in buffer
 */
static int16_t find_note_slot(uint8_t track, uint8_t note, uint8_t channel) {
    gate_time_config_t* cfg = &g_gate_time_config[track];
    
    for (uint8_t i = 0; i < GATE_TIME_MAX_NOTES_PER_TRACK; i++) {
        if (cfg->notes[i].active && 
            cfg->notes[i].note == note && 
            cfg->notes[i].channel == channel) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Find free note slot
 */
static int16_t find_free_slot(uint8_t track) {
    gate_time_config_t* cfg = &g_gate_time_config[track];
    
    for (uint8_t i = 0; i < GATE_TIME_MAX_NOTES_PER_TRACK; i++) {
        if (!cfg->notes[i].active) {
            return i;
        }
    }
    return -1;
}

/**
 * @brief Apply min/max limits to gate time
 */
static uint32_t apply_limits(uint8_t track, uint32_t length_ms) {
    gate_time_config_t* cfg = &g_gate_time_config[track];
    
    if (cfg->min_length_ms > 0 && length_ms < cfg->min_length_ms) {
        length_ms = cfg->min_length_ms;
    }
    
    if (cfg->max_length_ms > 0 && length_ms > cfg->max_length_ms) {
        length_ms = cfg->max_length_ms;
    }
    
    return length_ms;
}

/**
 * @brief Initialize gate time module
 */
void gate_time_init(void) {
    memset(g_gate_time_config, 0, sizeof(g_gate_time_config));
    
    for (uint8_t i = 0; i < GATE_TIME_MAX_TRACKS; i++) {
        g_gate_time_config[i].enabled = 0;
        g_gate_time_config[i].mode = GATE_TIME_MODE_PERCENT;
        g_gate_time_config[i].value = DEFAULT_PERCENT;
        g_gate_time_config[i].min_length_ms = 0;
        g_gate_time_config[i].max_length_ms = 0;
        g_gate_time_config[i].note_count = 0;
        g_gate_time_config[i].total_notes_processed = 0;
    }
    
    g_note_callback = NULL;
}

/**
 * @brief Set note output callback
 */
void gate_time_set_callback(gate_time_note_callback_t callback) {
    g_note_callback = callback;
}

/**
 * @brief Enable/disable gate time for a track
 */
void gate_time_set_enabled(uint8_t track, uint8_t enabled) {
    if (track >= GATE_TIME_MAX_TRACKS) return;
    g_gate_time_config[track].enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if gate time is enabled for a track
 */
uint8_t gate_time_is_enabled(uint8_t track) {
    if (track >= GATE_TIME_MAX_TRACKS) return 0;
    return g_gate_time_config[track].enabled;
}

/**
 * @brief Set gate time mode
 */
void gate_time_set_mode(uint8_t track, gate_time_mode_t mode) {
    if (track >= GATE_TIME_MAX_TRACKS || mode >= GATE_TIME_MODE_COUNT) return;
    
    g_gate_time_config[track].mode = mode;
    
    // Set appropriate default value for mode
    switch (mode) {
        case GATE_TIME_MODE_PERCENT:
            g_gate_time_config[track].value = DEFAULT_PERCENT;
            break;
        case GATE_TIME_MODE_FIXED_MS:
            g_gate_time_config[track].value = DEFAULT_FIXED_MS;
            break;
        case GATE_TIME_MODE_FIXED_TICKS:
            g_gate_time_config[track].value = DEFAULT_FIXED_TICKS;
            break;
        default:
            break;
    }
}

/**
 * @brief Get gate time mode
 */
gate_time_mode_t gate_time_get_mode(uint8_t track) {
    if (track >= GATE_TIME_MAX_TRACKS) return GATE_TIME_MODE_PERCENT;
    return g_gate_time_config[track].mode;
}

/**
 * @brief Set gate time value
 */
void gate_time_set_value(uint8_t track, uint16_t value) {
    if (track >= GATE_TIME_MAX_TRACKS) return;
    
    gate_time_config_t* cfg = &g_gate_time_config[track];
    
    // Apply mode-specific limits
    if (cfg->mode == GATE_TIME_MODE_PERCENT) {
        if (value < MIN_PERCENT) value = MIN_PERCENT;
        if (value > MAX_PERCENT) value = MAX_PERCENT;
    }
    
    cfg->value = value;
}

/**
 * @brief Get gate time value
 */
uint16_t gate_time_get_value(uint8_t track) {
    if (track >= GATE_TIME_MAX_TRACKS) return DEFAULT_PERCENT;
    return g_gate_time_config[track].value;
}

/**
 * @brief Set minimum gate time
 */
void gate_time_set_min_length(uint8_t track, uint16_t min_ms) {
    if (track >= GATE_TIME_MAX_TRACKS) return;
    g_gate_time_config[track].min_length_ms = min_ms;
}

/**
 * @brief Get minimum gate time
 */
uint16_t gate_time_get_min_length(uint8_t track) {
    if (track >= GATE_TIME_MAX_TRACKS) return 0;
    return g_gate_time_config[track].min_length_ms;
}

/**
 * @brief Set maximum gate time
 */
void gate_time_set_max_length(uint8_t track, uint16_t max_ms) {
    if (track >= GATE_TIME_MAX_TRACKS) return;
    g_gate_time_config[track].max_length_ms = max_ms;
}

/**
 * @brief Get maximum gate time
 */
uint16_t gate_time_get_max_length(uint8_t track) {
    if (track >= GATE_TIME_MAX_TRACKS) return 0;
    return g_gate_time_config[track].max_length_ms;
}

/**
 * @brief Calculate gate time for a note
 */
uint32_t gate_time_calculate_length(uint8_t track, uint32_t original_length_ms) {
    if (track >= GATE_TIME_MAX_TRACKS) return original_length_ms;
    
    gate_time_config_t* cfg = &g_gate_time_config[track];
    uint32_t new_length_ms = original_length_ms;
    
    switch (cfg->mode) {
        case GATE_TIME_MODE_PERCENT:
            new_length_ms = (original_length_ms * cfg->value) / 100;
            break;
            
        case GATE_TIME_MODE_FIXED_MS:
            new_length_ms = cfg->value;
            break;
            
        case GATE_TIME_MODE_FIXED_TICKS:
            // Assume 120 BPM, 96 PPQN for tick conversion
            // 1 quarter = 500ms @ 120 BPM
            // 1 tick = 500ms / 96 = ~5.2ms
            new_length_ms = (cfg->value * 500) / 96;
            break;
            
        default:
            break;
    }
    
    // Apply min/max limits
    new_length_ms = apply_limits(track, new_length_ms);
    
    // Ensure minimum of 1ms
    if (new_length_ms < 1) new_length_ms = 1;
    
    return new_length_ms;
}

/**
 * @brief Process note on event
 */
uint8_t gate_time_process_note_on(uint8_t track, uint8_t note, uint8_t velocity,
                                   uint8_t channel, uint32_t time_ms) {
    if (track >= GATE_TIME_MAX_TRACKS) return 0;
    if (!g_gate_time_config[track].enabled) return 0;
    
    gate_time_config_t* cfg = &g_gate_time_config[track];
    
    // Find free slot
    int16_t slot = find_free_slot(track);
    if (slot < 0) return 0;  // Buffer full
    
    // Calculate gate time (use default 500ms for now, can be updated via note off)
    uint32_t gate_length = gate_time_calculate_length(track, 500);
    
    // Store note
    cfg->notes[slot].note = note;
    cfg->notes[slot].channel = channel;
    cfg->notes[slot].note_on_time_ms = time_ms;
    cfg->notes[slot].note_off_time_ms = time_ms + gate_length;
    cfg->notes[slot].active = 1;
    cfg->note_count++;
    cfg->total_notes_processed++;
    
    // Send note on via callback
    if (g_note_callback) {
        g_note_callback(track, note, velocity, channel);
    }
    
    return 1;
}

/**
 * @brief Process note off event
 */
void gate_time_process_note_off(uint8_t track, uint8_t note, uint8_t channel) {
    if (track >= GATE_TIME_MAX_TRACKS) return;
    
    // Find note
    int16_t slot = find_note_slot(track, note, channel);
    if (slot < 0) return;
    
    // For now, we rely on the tick function to handle note offs
    // In a more advanced implementation, you could recalculate the gate time
    // based on the actual note duration here
    (void)slot;  // Unused for now
}

/**
 * @brief Tick function - call every 1ms
 */
void gate_time_tick(uint32_t time_ms) {
    for (uint8_t track = 0; track < GATE_TIME_MAX_TRACKS; track++) {
        if (!g_gate_time_config[track].enabled) continue;
        
        gate_time_config_t* cfg = &g_gate_time_config[track];
        
        // Check all active notes
        for (uint8_t i = 0; i < GATE_TIME_MAX_NOTES_PER_TRACK; i++) {
            if (!cfg->notes[i].active) continue;
            
            // Check if note should be turned off
            if (time_ms >= cfg->notes[i].note_off_time_ms) {
                // Send note off
                if (g_note_callback) {
                    g_note_callback(track, cfg->notes[i].note, 0, cfg->notes[i].channel);
                }
                
                // Clear slot
                cfg->notes[i].active = 0;
                cfg->note_count--;
            }
        }
    }
}

/**
 * @brief Reset gate time state for a track
 */
void gate_time_reset(uint8_t track) {
    if (track >= GATE_TIME_MAX_TRACKS) return;
    
    gate_time_config_t* cfg = &g_gate_time_config[track];
    
    // Send note offs for all active notes
    for (uint8_t i = 0; i < GATE_TIME_MAX_NOTES_PER_TRACK; i++) {
        if (cfg->notes[i].active) {
            if (g_note_callback) {
                g_note_callback(track, cfg->notes[i].note, 0, cfg->notes[i].channel);
            }
            cfg->notes[i].active = 0;
        }
    }
    
    cfg->note_count = 0;
}

/**
 * @brief Reset gate time state for all tracks
 */
void gate_time_reset_all(void) {
    for (uint8_t i = 0; i < GATE_TIME_MAX_TRACKS; i++) {
        gate_time_reset(i);
    }
}

/**
 * @brief Get mode name string
 */
const char* gate_time_get_mode_name(gate_time_mode_t mode) {
    if (mode >= GATE_TIME_MODE_COUNT) return "Unknown";
    return mode_names[mode];
}

/**
 * @brief Get statistics for a track
 */
void gate_time_get_stats(uint8_t track, uint8_t* active_notes, uint32_t* total_notes) {
    if (track >= GATE_TIME_MAX_TRACKS) {
        if (active_notes) *active_notes = 0;
        if (total_notes) *total_notes = 0;
        return;
    }
    
    gate_time_config_t* cfg = &g_gate_time_config[track];
    
    if (active_notes) *active_notes = cfg->note_count;
    if (total_notes) *total_notes = cfg->total_notes_processed;
}
