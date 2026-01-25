/**
 * @file assist_hold.c
 * @brief Assist Hold implementation
 */

#include "Services/assist_hold/assist_hold.h"
#include <string.h>

static const char* mode_names[] = {
    "Disabled", "Latch", "Timed", "Next Note", "Infinite"
};

typedef struct {
    uint8_t active;
    uint8_t note;
    uint8_t velocity;
    uint8_t channel;
    uint32_t start_time_ms;
} held_note_t;

typedef struct {
    hold_mode_t mode;
    uint16_t duration_ms;
    uint8_t velocity_threshold;
    uint8_t mono_mode;
    held_note_t notes[ASSIST_HOLD_MAX_NOTES];
} assist_hold_config_t;

static assist_hold_config_t g_assist[ASSIST_HOLD_MAX_TRACKS];
static uint32_t g_tick_counter = 0;
static assist_hold_output_cb_t g_output_callback = NULL;

/**
 * @brief Initialize assist hold module
 */
void assist_hold_init(void) {
    memset(g_assist, 0, sizeof(g_assist));
    
    for (uint8_t t = 0; t < ASSIST_HOLD_MAX_TRACKS; t++) {
        g_assist[t].mode = HOLD_MODE_DISABLED;
        g_assist[t].duration_ms = 2000;  // 2 seconds default
        g_assist[t].velocity_threshold = 1;
        g_assist[t].mono_mode = 0;
    }
}

/**
 * @brief Set hold mode
 */
void assist_hold_set_mode(uint8_t track, hold_mode_t mode) {
    if (track >= ASSIST_HOLD_MAX_TRACKS) return;
    if (mode >= HOLD_MODE_COUNT) return;
    g_assist[track].mode = mode;
}

/**
 * @brief Get hold mode
 */
hold_mode_t assist_hold_get_mode(uint8_t track) {
    if (track >= ASSIST_HOLD_MAX_TRACKS) return HOLD_MODE_DISABLED;
    return g_assist[track].mode;
}

/**
 * @brief Set hold duration
 */
void assist_hold_set_duration_ms(uint8_t track, uint16_t ms) {
    if (track >= ASSIST_HOLD_MAX_TRACKS) return;
    if (ms < 100) ms = 100;
    if (ms > 10000) ms = 10000;
    g_assist[track].duration_ms = ms;
}

/**
 * @brief Get hold duration
 */
uint16_t assist_hold_get_duration_ms(uint8_t track) {
    if (track >= ASSIST_HOLD_MAX_TRACKS) return 2000;
    return g_assist[track].duration_ms;
}

/**
 * @brief Set velocity threshold
 */
void assist_hold_set_velocity_threshold(uint8_t track, uint8_t threshold) {
    if (track >= ASSIST_HOLD_MAX_TRACKS) return;
    if (threshold < 1) threshold = 1;
    if (threshold > 127) threshold = 127;
    g_assist[track].velocity_threshold = threshold;
}

/**
 * @brief Get velocity threshold
 */
uint8_t assist_hold_get_velocity_threshold(uint8_t track) {
    if (track >= ASSIST_HOLD_MAX_TRACKS) return 1;
    return g_assist[track].velocity_threshold;
}

/**
 * @brief Set mono mode
 */
void assist_hold_set_mono_mode(uint8_t track, uint8_t enabled) {
    if (track >= ASSIST_HOLD_MAX_TRACKS) return;
    g_assist[track].mono_mode = enabled ? 1 : 0;
}

/**
 * @brief Check if mono mode is enabled
 */
uint8_t assist_hold_is_mono_mode(uint8_t track) {
    if (track >= ASSIST_HOLD_MAX_TRACKS) return 0;
    return g_assist[track].mono_mode;
}

/**
 * @brief Release a held note
 */
static void release_note(uint8_t track, held_note_t* held) {
    if (!held->active) return;
    
    if (g_output_callback) {
        g_output_callback(track, held->note, 0, held->channel);
    }
    held->active = 0;
}

/**
 * @brief Find held note by note number
 */
static held_note_t* find_held_note(uint8_t track, uint8_t note, uint8_t channel) {
    assist_hold_config_t* cfg = &g_assist[track];
    
    for (uint8_t i = 0; i < ASSIST_HOLD_MAX_NOTES; i++) {
        if (cfg->notes[i].active && 
            cfg->notes[i].note == note && 
            cfg->notes[i].channel == channel) {
            return &cfg->notes[i];
        }
    }
    return NULL;
}

/**
 * @brief Process incoming MIDI note
 */
void assist_hold_process_note(uint8_t track, uint8_t note, uint8_t velocity, 
                              uint8_t channel, uint32_t timestamp_ms) {
    if (track >= ASSIST_HOLD_MAX_TRACKS) return;
    
    assist_hold_config_t* cfg = &g_assist[track];
    
    if (cfg->mode == HOLD_MODE_DISABLED) {
        // Pass through unchanged
        if (g_output_callback) {
            g_output_callback(track, note, velocity, channel);
        }
        return;
    }
    
    uint8_t is_note_on = (velocity >= cfg->velocity_threshold);
    
    if (is_note_on) {
        // Send note on
        if (g_output_callback) {
            g_output_callback(track, note, velocity, channel);
        }
        
        // Check for latch mode - if note already held, release it
        if (cfg->mode == HOLD_MODE_LATCH) {
            held_note_t* existing = find_held_note(track, note, channel);
            if (existing) {
                release_note(track, existing);
                return;  // Don't add to held notes
            }
        }
        
        // Release previous note in mono mode or next-note mode
        if (cfg->mono_mode || cfg->mode == HOLD_MODE_NEXT_NOTE) {
            for (uint8_t i = 0; i < ASSIST_HOLD_MAX_NOTES; i++) {
                if (cfg->notes[i].active) {
                    release_note(track, &cfg->notes[i]);
                }
            }
        }
        
        // Add to held notes
        for (uint8_t i = 0; i < ASSIST_HOLD_MAX_NOTES; i++) {
            if (!cfg->notes[i].active) {
                cfg->notes[i].active = 1;
                cfg->notes[i].note = note;
                cfg->notes[i].velocity = velocity;
                cfg->notes[i].channel = channel;
                cfg->notes[i].start_time_ms = timestamp_ms;
                break;
            }
        }
    } else {
        // Note off received - handle based on mode
        if (cfg->mode == HOLD_MODE_DISABLED) {
            // Pass through
            if (g_output_callback) {
                g_output_callback(track, note, 0, channel);
            }
        }
        // In other modes, ignore note off (held by system)
    }
}

/**
 * @brief Called every 1ms to process timed releases
 */
void assist_hold_tick_1ms(void) {
    g_tick_counter++;
    
    for (uint8_t t = 0; t < ASSIST_HOLD_MAX_TRACKS; t++) {
        assist_hold_config_t* cfg = &g_assist[t];
        
        if (cfg->mode != HOLD_MODE_TIMED) continue;
        
        for (uint8_t i = 0; i < ASSIST_HOLD_MAX_NOTES; i++) {
            held_note_t* held = &cfg->notes[i];
            if (!held->active) continue;
            
            uint32_t elapsed = g_tick_counter - held->start_time_ms;
            if (elapsed >= cfg->duration_ms) {
                release_note(t, held);
            }
        }
    }
}

/**
 * @brief Release all held notes
 */
void assist_hold_release_all(uint8_t track) {
    if (track >= ASSIST_HOLD_MAX_TRACKS) return;
    
    assist_hold_config_t* cfg = &g_assist[track];
    
    for (uint8_t i = 0; i < ASSIST_HOLD_MAX_NOTES; i++) {
        if (cfg->notes[i].active) {
            release_note(track, &cfg->notes[i]);
        }
    }
}

/**
 * @brief Get number of currently held notes
 */
uint8_t assist_hold_get_held_count(uint8_t track) {
    if (track >= ASSIST_HOLD_MAX_TRACKS) return 0;
    
    assist_hold_config_t* cfg = &g_assist[track];
    uint8_t count = 0;
    
    for (uint8_t i = 0; i < ASSIST_HOLD_MAX_NOTES; i++) {
        if (cfg->notes[i].active) count++;
    }
    
    return count;
}

/**
 * @brief Get mode name
 */
const char* assist_hold_get_mode_name(hold_mode_t mode) {
    if (mode >= HOLD_MODE_COUNT) return "Unknown";
    return mode_names[mode];
}

/**
 * @brief Set output callback
 */
void assist_hold_set_output_callback(assist_hold_output_cb_t callback) {
    g_output_callback = callback;
}
