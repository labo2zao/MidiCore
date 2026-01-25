/**
 * @file note_repeat.c
 * @brief Note repeat/ratchet/stutter implementation
 */

#include "Services/note_repeat/note_repeat.h"
#include <string.h>

// Rate names
static const char* rate_names[] = {
    "1/4", "1/8", "1/16", "1/32", "1/64",
    "1/8T", "1/16T", "1/32T"
};

// Rate intervals in ms at 120 BPM (60000ms/BPM / 4 = ms per quarter note)
// These are multipliers based on 1/16 note intervals
static const uint8_t rate_divisors[] = {
    4,   // 1/4 = 4 x 1/16
    2,   // 1/8 = 2 x 1/16
    1,   // 1/16
    1,   // 1/32 (handled separately with faster timing)
    1,   // 1/64 (handled separately with faster timing)
    1,   // 1/8T
    1,   // 1/16T
    1,   // 1/32T
};

// Active repeat state
typedef struct {
    uint8_t active;
    uint8_t note;
    uint8_t base_velocity;
    uint8_t channel;
    uint32_t last_trigger_ms;
    uint8_t repeat_count;
    uint8_t note_on;  // Current note on/off state
} repeat_state_t;

// Per-track configuration
typedef struct {
    uint8_t enabled;
    note_repeat_rate_t rate;
    uint8_t gate;            // 10-95%
    uint8_t velocity_decay;  // 0-50%
    uint8_t accent_pattern;  // 8-bit mask
    repeat_state_t state;
} repeat_config_t;

static repeat_config_t g_repeat[NOTE_REPEAT_MAX_TRACKS];
static uint16_t g_tempo = 120;
static uint32_t g_tick_counter = 0;
static note_repeat_output_cb_t g_output_callback = NULL;

/**
 * @brief Calculate repeat interval in milliseconds
 */
static uint32_t calculate_interval_ms(note_repeat_rate_t rate) {
    if (rate >= REPEAT_RATE_COUNT) return 125;
    
    // Base: 1/16 note at current tempo
    // At 120 BPM: quarter note = 500ms, 1/16 = 125ms
    uint32_t quarter_note_ms = (60000 / g_tempo);
    uint32_t sixteenth_ms = quarter_note_ms / 4;
    
    switch (rate) {
        case REPEAT_RATE_1_4:   return quarter_note_ms;
        case REPEAT_RATE_1_8:   return sixteenth_ms * 2;
        case REPEAT_RATE_1_16:  return sixteenth_ms;
        case REPEAT_RATE_1_32:  return sixteenth_ms / 2;
        case REPEAT_RATE_1_64:  return sixteenth_ms / 4;
        case REPEAT_RATE_1_8T:  return (sixteenth_ms * 2 * 2) / 3;  // Triplet
        case REPEAT_RATE_1_16T: return (sixteenth_ms * 2) / 3;
        case REPEAT_RATE_1_32T: return sixteenth_ms / 3;
        default: return sixteenth_ms;
    }
}

/**
 * @brief Initialize note repeat module
 */
void note_repeat_init(uint16_t tempo) {
    memset(g_repeat, 0, sizeof(g_repeat));
    g_tempo = tempo;
    g_tick_counter = 0;
    
    for (uint8_t t = 0; t < NOTE_REPEAT_MAX_TRACKS; t++) {
        g_repeat[t].enabled = 0;
        g_repeat[t].rate = REPEAT_RATE_1_16;
        g_repeat[t].gate = 50;
        g_repeat[t].velocity_decay = 10;
        g_repeat[t].accent_pattern = 0x01;  // Accent first repeat only
    }
}

/**
 * @brief Update tempo
 */
void note_repeat_set_tempo(uint16_t tempo) {
    if (tempo < 20) tempo = 20;
    if (tempo > 300) tempo = 300;
    g_tempo = tempo;
}

/**
 * @brief Called every 1ms to generate repeats
 */
void note_repeat_tick_1ms(void) {
    g_tick_counter++;
    
    if (!g_output_callback) return;
    
    for (uint8_t t = 0; t < NOTE_REPEAT_MAX_TRACKS; t++) {
        repeat_config_t* cfg = &g_repeat[t];
        if (!cfg->enabled || !cfg->state.active) continue;
        
        uint32_t interval = calculate_interval_ms(cfg->rate);
        uint32_t gate_time = (interval * cfg->gate) / 100;
        uint32_t elapsed = g_tick_counter - cfg->state.last_trigger_ms;
        
        // Check if it's time to send note off
        if (cfg->state.note_on && elapsed >= gate_time) {
            g_output_callback(t, cfg->state.note, 0, cfg->state.channel, 0);
            cfg->state.note_on = 0;
        }
        
        // Check if it's time for next repeat
        if (elapsed >= interval) {
            // Calculate velocity with decay
            uint16_t velocity = cfg->state.base_velocity;
            uint16_t decay_amount = (cfg->state.repeat_count * cfg->velocity_decay * velocity) / 100;
            if (velocity > decay_amount) {
                velocity -= decay_amount;
            }
            if (velocity < 1) velocity = 1;
            if (velocity > 127) velocity = 127;
            
            // Apply accent pattern
            uint8_t accent_bit = 1 << (cfg->state.repeat_count % 8);
            if (cfg->accent_pattern & accent_bit) {
                velocity = (velocity * 120) / 100;  // +20% for accent
                if (velocity > 127) velocity = 127;
            }
            
            // Send note on
            g_output_callback(t, cfg->state.note, (uint8_t)velocity, cfg->state.channel, 1);
            cfg->state.note_on = 1;
            cfg->state.last_trigger_ms = g_tick_counter;
            cfg->state.repeat_count++;
        }
    }
}

/**
 * @brief Enable/disable note repeat for a track
 */
void note_repeat_set_enabled(uint8_t track, uint8_t enabled) {
    if (track >= NOTE_REPEAT_MAX_TRACKS) return;
    g_repeat[track].enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if note repeat is enabled for a track
 */
uint8_t note_repeat_is_enabled(uint8_t track) {
    if (track >= NOTE_REPEAT_MAX_TRACKS) return 0;
    return g_repeat[track].enabled;
}

/**
 * @brief Set repeat rate
 */
void note_repeat_set_rate(uint8_t track, note_repeat_rate_t rate) {
    if (track >= NOTE_REPEAT_MAX_TRACKS) return;
    if (rate >= REPEAT_RATE_COUNT) return;
    g_repeat[track].rate = rate;
}

/**
 * @brief Get repeat rate
 */
note_repeat_rate_t note_repeat_get_rate(uint8_t track) {
    if (track >= NOTE_REPEAT_MAX_TRACKS) return REPEAT_RATE_1_16;
    return g_repeat[track].rate;
}

/**
 * @brief Set gate length
 */
void note_repeat_set_gate(uint8_t track, uint8_t gate) {
    if (track >= NOTE_REPEAT_MAX_TRACKS) return;
    if (gate < 10) gate = 10;
    if (gate > 95) gate = 95;
    g_repeat[track].gate = gate;
}

/**
 * @brief Get gate length
 */
uint8_t note_repeat_get_gate(uint8_t track) {
    if (track >= NOTE_REPEAT_MAX_TRACKS) return 50;
    return g_repeat[track].gate;
}

/**
 * @brief Set velocity decay per repeat
 */
void note_repeat_set_velocity_decay(uint8_t track, uint8_t decay) {
    if (track >= NOTE_REPEAT_MAX_TRACKS) return;
    if (decay > 50) decay = 50;
    g_repeat[track].velocity_decay = decay;
}

/**
 * @brief Get velocity decay
 */
uint8_t note_repeat_get_velocity_decay(uint8_t track) {
    if (track >= NOTE_REPEAT_MAX_TRACKS) return 10;
    return g_repeat[track].velocity_decay;
}

/**
 * @brief Set accent pattern
 */
void note_repeat_set_accent_pattern(uint8_t track, uint8_t pattern) {
    if (track >= NOTE_REPEAT_MAX_TRACKS) return;
    g_repeat[track].accent_pattern = pattern;
}

/**
 * @brief Get accent pattern
 */
uint8_t note_repeat_get_accent_pattern(uint8_t track) {
    if (track >= NOTE_REPEAT_MAX_TRACKS) return 0x01;
    return g_repeat[track].accent_pattern;
}

/**
 * @brief Trigger note repeat
 */
void note_repeat_trigger(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel) {
    if (track >= NOTE_REPEAT_MAX_TRACKS) return;
    
    repeat_config_t* cfg = &g_repeat[track];
    if (!cfg->enabled) return;
    
    cfg->state.active = 1;
    cfg->state.note = note;
    cfg->state.base_velocity = velocity;
    cfg->state.channel = channel;
    cfg->state.last_trigger_ms = g_tick_counter;
    cfg->state.repeat_count = 0;
    cfg->state.note_on = 0;
}

/**
 * @brief Stop note repeat
 */
void note_repeat_stop(uint8_t track, uint8_t note, uint8_t channel) {
    if (track >= NOTE_REPEAT_MAX_TRACKS) return;
    
    repeat_config_t* cfg = &g_repeat[track];
    if (cfg->state.active && cfg->state.note == note && cfg->state.channel == channel) {
        // Send final note off if needed
        if (cfg->state.note_on && g_output_callback) {
            g_output_callback(track, note, 0, channel, 0);
        }
        cfg->state.active = 0;
        cfg->state.note_on = 0;
    }
}

/**
 * @brief Stop all repeats on a track
 */
void note_repeat_stop_all(uint8_t track) {
    if (track >= NOTE_REPEAT_MAX_TRACKS) return;
    
    repeat_config_t* cfg = &g_repeat[track];
    if (cfg->state.active && cfg->state.note_on && g_output_callback) {
        g_output_callback(track, cfg->state.note, 0, cfg->state.channel, 0);
    }
    cfg->state.active = 0;
    cfg->state.note_on = 0;
}

/**
 * @brief Get rate name
 */
const char* note_repeat_get_rate_name(note_repeat_rate_t rate) {
    if (rate >= REPEAT_RATE_COUNT) return "Unknown";
    return rate_names[rate];
}

/**
 * @brief Set output callback
 */
void note_repeat_set_output_callback(note_repeat_output_cb_t callback) {
    g_output_callback = callback;
}
