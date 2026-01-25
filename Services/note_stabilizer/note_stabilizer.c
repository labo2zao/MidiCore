/**
 * @file note_stabilizer.c
 * @brief Note Stabilizer implementation
 */

#include "Services/note_stabilizer/note_stabilizer.h"
#include <string.h>

#define MAX_PENDING_NOTES 16

typedef struct {
    uint8_t active;
    uint8_t note;
    uint8_t velocity;
    uint8_t channel;
    uint32_t note_on_time;
} pending_note_t;

typedef struct {
    uint8_t enabled;
    uint16_t min_duration_ms;
    uint16_t retrigger_delay_ms;
    uint8_t neighbor_range;
    uint8_t velocity_threshold;
    uint8_t averaging_enabled;
    uint32_t last_note_time[128];  // Last time each note was played
    uint8_t last_note_active;
    uint8_t last_note_number;
    pending_note_t pending[MAX_PENDING_NOTES];
    uint32_t filtered_count;
    uint32_t passed_count;
} note_stab_config_t;

static note_stab_config_t g_stab[NOTE_STAB_MAX_TRACKS];
static uint32_t g_tick_counter = 0;
static note_stab_output_cb_t g_output_callback = NULL;

/**
 * @brief Initialize note stabilizer module
 */
void note_stab_init(void) {
    memset(g_stab, 0, sizeof(g_stab));
    
    for (uint8_t t = 0; t < NOTE_STAB_MAX_TRACKS; t++) {
        g_stab[t].enabled = 0;
        g_stab[t].min_duration_ms = 50;      // 50ms minimum
        g_stab[t].retrigger_delay_ms = 100;  // 100ms between retrigggers
        g_stab[t].neighbor_range = 1;        // ±1 semitone
        g_stab[t].velocity_threshold = 10;   // Ignore small velocity changes
        g_stab[t].averaging_enabled = 1;
    }
}

/**
 * @brief Enable/disable stabilizer
 */
void note_stab_set_enabled(uint8_t track, uint8_t enabled) {
    if (track >= NOTE_STAB_MAX_TRACKS) return;
    g_stab[track].enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if stabilizer is enabled
 */
uint8_t note_stab_is_enabled(uint8_t track) {
    if (track >= NOTE_STAB_MAX_TRACKS) return 0;
    return g_stab[track].enabled;
}

/**
 * @brief Set minimum note duration
 */
void note_stab_set_min_duration_ms(uint8_t track, uint16_t ms) {
    if (track >= NOTE_STAB_MAX_TRACKS) return;
    if (ms < 10) ms = 10;
    if (ms > 500) ms = 500;
    g_stab[track].min_duration_ms = ms;
}

/**
 * @brief Get minimum note duration
 */
uint16_t note_stab_get_min_duration_ms(uint8_t track) {
    if (track >= NOTE_STAB_MAX_TRACKS) return 50;
    return g_stab[track].min_duration_ms;
}

/**
 * @brief Set retrigger delay
 */
void note_stab_set_retrigger_delay_ms(uint8_t track, uint16_t ms) {
    if (track >= NOTE_STAB_MAX_TRACKS) return;
    if (ms < 10) ms = 10;
    if (ms > 1000) ms = 1000;
    g_stab[track].retrigger_delay_ms = ms;
}

/**
 * @brief Get retrigger delay
 */
uint16_t note_stab_get_retrigger_delay_ms(uint8_t track) {
    if (track >= NOTE_STAB_MAX_TRACKS) return 100;
    return g_stab[track].retrigger_delay_ms;
}

/**
 * @brief Set neighboring key filter range
 */
void note_stab_set_neighbor_range(uint8_t track, uint8_t semitones) {
    if (track >= NOTE_STAB_MAX_TRACKS) return;
    if (semitones > 12) semitones = 12;
    g_stab[track].neighbor_range = semitones;
}

/**
 * @brief Get neighboring key filter range
 */
uint8_t note_stab_get_neighbor_range(uint8_t track) {
    if (track >= NOTE_STAB_MAX_TRACKS) return 1;
    return g_stab[track].neighbor_range;
}

/**
 * @brief Set velocity threshold
 */
void note_stab_set_velocity_threshold(uint8_t track, uint8_t threshold) {
    if (track >= NOTE_STAB_MAX_TRACKS) return;
    if (threshold > 127) threshold = 127;
    g_stab[track].velocity_threshold = threshold;
}

/**
 * @brief Get velocity threshold
 */
uint8_t note_stab_get_velocity_threshold(uint8_t track) {
    if (track >= NOTE_STAB_MAX_TRACKS) return 10;
    return g_stab[track].velocity_threshold;
}

/**
 * @brief Enable/disable averaging
 */
void note_stab_set_averaging_enabled(uint8_t track, uint8_t enabled) {
    if (track >= NOTE_STAB_MAX_TRACKS) return;
    g_stab[track].averaging_enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if averaging is enabled
 */
uint8_t note_stab_is_averaging_enabled(uint8_t track) {
    if (track >= NOTE_STAB_MAX_TRACKS) return 0;
    return g_stab[track].averaging_enabled;
}

/**
 * @brief Check if note is too close to last note (neighbor filter)
 */
static uint8_t is_neighbor_note(note_stab_config_t* cfg, uint8_t note) {
    if (cfg->neighbor_range == 0 || !cfg->last_note_active) return 0;
    
    int16_t diff = (int16_t)note - (int16_t)cfg->last_note_number;
    if (diff < 0) diff = -diff;
    
    return (diff <= cfg->neighbor_range);
}

/**
 * @brief Process incoming MIDI note
 */
void note_stab_process_note(uint8_t track, uint8_t note, uint8_t velocity,
                           uint8_t channel, uint32_t timestamp_ms) {
    if (track >= NOTE_STAB_MAX_TRACKS) return;
    
    note_stab_config_t* cfg = &g_stab[track];
    
    if (!cfg->enabled) {
        // Pass through
        if (g_output_callback) {
            g_output_callback(track, note, velocity, channel);
        }
        cfg->passed_count++;
        return;
    }
    
    uint8_t is_note_on = (velocity > 0);
    
    if (is_note_on) {
        // Check retrigger delay
        uint32_t last_time = cfg->last_note_time[note];
        if (last_time > 0) {
            uint32_t elapsed = timestamp_ms - last_time;
            if (elapsed < cfg->retrigger_delay_ms) {
                // Too soon - filter out
                cfg->filtered_count++;
                return;
            }
        }
        
        // Check neighbor filter
        if (is_neighbor_note(cfg, note)) {
            cfg->filtered_count++;
            return;
        }
        
        // Send note on
        if (g_output_callback) {
            g_output_callback(track, note, velocity, channel);
        }
        
        // Add to pending for duration check
        for (uint8_t i = 0; i < MAX_PENDING_NOTES; i++) {
            if (!cfg->pending[i].active) {
                cfg->pending[i].active = 1;
                cfg->pending[i].note = note;
                cfg->pending[i].velocity = velocity;
                cfg->pending[i].channel = channel;
                cfg->pending[i].note_on_time = timestamp_ms;
                break;
            }
        }
        
        cfg->last_note_time[note] = timestamp_ms;
        cfg->last_note_active = 1;
        cfg->last_note_number = note;
        cfg->passed_count++;
        
    } else {
        // Note off - check if it meets minimum duration
        for (uint8_t i = 0; i < MAX_PENDING_NOTES; i++) {
            if (cfg->pending[i].active && 
                cfg->pending[i].note == note && 
                cfg->pending[i].channel == channel) {
                
                uint32_t duration = timestamp_ms - cfg->pending[i].note_on_time;
                
                if (duration >= cfg->min_duration_ms) {
                    // Duration OK - send note off
                    if (g_output_callback) {
                        g_output_callback(track, note, 0, channel);
                    }
                } else {
                    // Too short - already sent note on, need to send quick note off
                    if (g_output_callback) {
                        g_output_callback(track, note, 0, channel);
                    }
                }
                
                cfg->pending[i].active = 0;
                break;
            }
        }
    }
}

/**
 * @brief Called every 1ms to process delayed notes
 */
void note_stab_tick_1ms(void) {
    g_tick_counter++;
    // Currently no time-based processing needed
}

/**
 * @brief Get statistics
 */
void note_stab_get_stats(uint8_t track, uint32_t* filtered_count, uint32_t* passed_count) {
    if (track >= NOTE_STAB_MAX_TRACKS) return;
    
    if (filtered_count) *filtered_count = g_stab[track].filtered_count;
    if (passed_count) *passed_count = g_stab[track].passed_count;
}

/**
 * @brief Reset statistics
 */
void note_stab_reset_stats(uint8_t track) {
    if (track >= NOTE_STAB_MAX_TRACKS) return;
    
    g_stab[track].filtered_count = 0;
    g_stab[track].passed_count = 0;
}

/**
 * @brief Set output callback
 */
void note_stab_set_output_callback(note_stab_output_cb_t callback) {
    g_output_callback = callback;
}
