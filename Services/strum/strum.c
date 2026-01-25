/**
 * @file strum.c
 * @brief MIDI Strum Effect Implementation
 */

#include "strum.h"
#include <string.h>
#include <stdlib.h>

typedef struct {
    uint8_t enabled;
    uint8_t time_ms;
    strum_direction_t direction;
    strum_ramp_t velocity_ramp;
    uint8_t ramp_amount;
    uint8_t last_direction_was_up;
} strum_config_t;

static strum_config_t g_strum[STRUM_MAX_TRACKS];

static const char* direction_names[] = {
    "Up",
    "Down",
    "Up-Down",
    "Random"
};

static const char* ramp_names[] = {
    "None",
    "Increase",
    "Decrease"
};

void strum_init(void) {
    memset(g_strum, 0, sizeof(g_strum));
    
    for (uint8_t i = 0; i < STRUM_MAX_TRACKS; i++) {
        g_strum[i].enabled = 0;
        g_strum[i].time_ms = 30;
        g_strum[i].direction = STRUM_DIR_DOWN;
        g_strum[i].velocity_ramp = STRUM_RAMP_NONE;
        g_strum[i].ramp_amount = 20;
        g_strum[i].last_direction_was_up = 0;
    }
}

void strum_set_enabled(uint8_t track, uint8_t enabled) {
    if (track >= STRUM_MAX_TRACKS) return;
    g_strum[track].enabled = enabled ? 1 : 0;
}

uint8_t strum_is_enabled(uint8_t track) {
    if (track >= STRUM_MAX_TRACKS) return 0;
    return g_strum[track].enabled;
}

void strum_set_time(uint8_t track, uint8_t time_ms) {
    if (track >= STRUM_MAX_TRACKS) return;
    if (time_ms > STRUM_MAX_TIME_MS) {
        time_ms = STRUM_MAX_TIME_MS;
    }
    g_strum[track].time_ms = time_ms;
}

uint8_t strum_get_time(uint8_t track) {
    if (track >= STRUM_MAX_TRACKS) return 30;
    return g_strum[track].time_ms;
}

void strum_set_direction(uint8_t track, strum_direction_t direction) {
    if (track >= STRUM_MAX_TRACKS) return;
    if (direction >= STRUM_DIR_COUNT) return;
    g_strum[track].direction = direction;
}

strum_direction_t strum_get_direction(uint8_t track) {
    if (track >= STRUM_MAX_TRACKS) return STRUM_DIR_UP;
    return g_strum[track].direction;
}

void strum_set_velocity_ramp(uint8_t track, strum_ramp_t ramp) {
    if (track >= STRUM_MAX_TRACKS) return;
    if (ramp >= STRUM_RAMP_COUNT) return;
    g_strum[track].velocity_ramp = ramp;
}

strum_ramp_t strum_get_velocity_ramp(uint8_t track) {
    if (track >= STRUM_MAX_TRACKS) return STRUM_RAMP_NONE;
    return g_strum[track].velocity_ramp;
}

void strum_set_ramp_amount(uint8_t track, uint8_t amount) {
    if (track >= STRUM_MAX_TRACKS) return;
    if (amount > 100) {
        amount = 100;
    }
    g_strum[track].ramp_amount = amount;
}

uint8_t strum_get_ramp_amount(uint8_t track) {
    if (track >= STRUM_MAX_TRACKS) return 20;
    return g_strum[track].ramp_amount;
}

static uint8_t find_note_index(uint8_t note, const uint8_t* chord_notes, uint8_t chord_size) {
    for (uint8_t i = 0; i < chord_size; i++) {
        if (chord_notes[i] == note) {
            return i;
        }
    }
    return 0;
}

static uint8_t calculate_velocity(uint8_t original_velocity, uint8_t note_index, 
                                  uint8_t chord_size, strum_ramp_t ramp, uint8_t ramp_amount) {
    if (ramp == STRUM_RAMP_NONE || chord_size <= 1) {
        return original_velocity;
    }
    
    int16_t velocity = original_velocity;
    int16_t max_change = (original_velocity * ramp_amount) / 100;
    int16_t velocity_step = (max_change * 2) / (chord_size - 1);
    
    if (ramp == STRUM_RAMP_INCREASE) {
        velocity = original_velocity - max_change + (velocity_step * note_index);
    } else if (ramp == STRUM_RAMP_DECREASE) {
        velocity = original_velocity + max_change - (velocity_step * note_index);
    }
    
    if (velocity < 1) velocity = 1;
    if (velocity > 127) velocity = 127;
    
    return (uint8_t)velocity;
}

void strum_process_note(uint8_t track, uint8_t note, uint8_t velocity,
                        const uint8_t* chord_notes, uint8_t chord_size,
                        uint8_t* delay_ms, uint8_t* new_velocity) {
    *delay_ms = 0;
    *new_velocity = velocity;
    
    if (track >= STRUM_MAX_TRACKS) return;
    if (!g_strum[track].enabled) return;
    if (chord_size == 0 || chord_size > STRUM_MAX_CHORD_NOTES) return;
    if (chord_notes == NULL) return;
    
    if (chord_size == 1) {
        return;
    }
    
    strum_config_t* cfg = &g_strum[track];
    uint8_t note_index = find_note_index(note, chord_notes, chord_size);
    uint8_t effective_index = note_index;
    
    switch (cfg->direction) {
        case STRUM_DIR_UP:
            effective_index = note_index;
            break;
            
        case STRUM_DIR_DOWN:
            effective_index = (chord_size - 1) - note_index;
            break;
            
        case STRUM_DIR_UP_DOWN:
            if (cfg->last_direction_was_up) {
                effective_index = (chord_size - 1) - note_index;
            } else {
                effective_index = note_index;
            }
            if (note_index == chord_size - 1) {
                cfg->last_direction_was_up = !cfg->last_direction_was_up;
            }
            break;
            
        case STRUM_DIR_RANDOM:
            effective_index = rand() % chord_size;
            break;
            
        default:
            effective_index = note_index;
            break;
    }
    
    if (cfg->time_ms > 0 && chord_size > 1) {
        *delay_ms = (cfg->time_ms * effective_index) / (chord_size - 1);
    }
    
    *new_velocity = calculate_velocity(velocity, effective_index, chord_size, 
                                       cfg->velocity_ramp, cfg->ramp_amount);
}

void strum_reset(uint8_t track) {
    if (track >= STRUM_MAX_TRACKS) return;
    g_strum[track].last_direction_was_up = 0;
}

const char* strum_get_direction_name(strum_direction_t direction) {
    if (direction >= STRUM_DIR_COUNT) {
        return "Unknown";
    }
    return direction_names[direction];
}

const char* strum_get_ramp_name(strum_ramp_t ramp) {
    if (ramp >= STRUM_RAMP_COUNT) {
        return "Unknown";
    }
    return ramp_names[ramp];
}
