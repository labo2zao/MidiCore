/**
 * @file legato.c
 * @brief Legato/Mono/Priority module implementation
 */

#include "Services/legato/legato.h"
#include <string.h>

// Priority mode names
static const char* priority_names[] = {
    "Last",
    "Highest",
    "Lowest",
    "First"
};

// Retrigger mode names
static const char* retrigger_names[] = {
    "Off",
    "On"
};

// Note entry structure for tracking held notes
typedef struct {
    uint8_t note;           // MIDI note number
    uint8_t velocity;       // Note velocity
    uint8_t channel;        // MIDI channel
    uint8_t active;         // 1 if note is held, 0 if released
    uint32_t timestamp;     // Order of note press (for priority tracking)
} note_entry_t;

// Per-track legato configuration and state
typedef struct {
    // Configuration
    uint8_t enabled;                    // Legato mode enabled
    uint8_t mono_mode;                  // Mono mode enabled
    legato_priority_t priority;         // Note priority mode
    legato_retrigger_t retrigger;       // Retrigger mode
    uint16_t glide_time_ms;            // Portamento/glide time
    
    // State
    note_entry_t notes[LEGATO_MAX_NOTES];  // Held notes buffer
    uint8_t note_count;                 // Number of currently held notes
    uint8_t active_note;                // Currently playing note (0xFF = none)
    uint8_t active_velocity;            // Currently playing velocity
    uint8_t active_channel;             // Currently playing channel
    uint32_t note_counter;              // Incremental counter for note order
} legato_track_t;

static legato_track_t g_tracks[LEGATO_MAX_TRACKS];
static legato_event_cb_t g_event_callback = NULL;

// Forward declarations
static void update_active_note(uint8_t track);
static uint8_t find_priority_note(legato_track_t* t);
static void send_event(uint8_t track, const legato_event_t* event);
static int8_t find_note_index(legato_track_t* t, uint8_t note, uint8_t channel);

/**
 * @brief Initialize legato module
 */
void legato_init(void) {
    memset(g_tracks, 0, sizeof(g_tracks));
    
    // Initialize defaults
    for (uint8_t i = 0; i < LEGATO_MAX_TRACKS; i++) {
        g_tracks[i].enabled = 0;
        g_tracks[i].mono_mode = 1;
        g_tracks[i].priority = LEGATO_PRIORITY_LAST;
        g_tracks[i].retrigger = LEGATO_RETRIGGER_OFF;
        g_tracks[i].glide_time_ms = 0;
        g_tracks[i].active_note = 0xFF;
        g_tracks[i].note_count = 0;
        g_tracks[i].note_counter = 0;
    }
    
    g_event_callback = NULL;
}

/**
 * @brief Enable/disable legato mode for a track
 */
void legato_set_enabled(uint8_t track, uint8_t enabled) {
    if (track >= LEGATO_MAX_TRACKS) return;
    g_tracks[track].enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if legato mode is enabled for a track
 */
uint8_t legato_is_enabled(uint8_t track) {
    if (track >= LEGATO_MAX_TRACKS) return 0;
    return g_tracks[track].enabled;
}

/**
 * @brief Set note priority mode
 */
void legato_set_priority(uint8_t track, legato_priority_t priority) {
    if (track >= LEGATO_MAX_TRACKS) return;
    if (priority >= LEGATO_PRIORITY_COUNT) return;
    g_tracks[track].priority = priority;
    
    // Recalculate active note with new priority
    update_active_note(track);
}

/**
 * @brief Get note priority mode
 */
legato_priority_t legato_get_priority(uint8_t track) {
    if (track >= LEGATO_MAX_TRACKS) return LEGATO_PRIORITY_LAST;
    return g_tracks[track].priority;
}

/**
 * @brief Set retrigger mode
 */
void legato_set_retrigger(uint8_t track, legato_retrigger_t retrigger) {
    if (track >= LEGATO_MAX_TRACKS) return;
    if (retrigger >= LEGATO_RETRIGGER_COUNT) return;
    g_tracks[track].retrigger = retrigger;
}

/**
 * @brief Get retrigger mode
 */
legato_retrigger_t legato_get_retrigger(uint8_t track) {
    if (track >= LEGATO_MAX_TRACKS) return LEGATO_RETRIGGER_OFF;
    return g_tracks[track].retrigger;
}

/**
 * @brief Set portamento/glide time in milliseconds
 */
void legato_set_glide_time(uint8_t track, uint16_t time_ms) {
    if (track >= LEGATO_MAX_TRACKS) return;
    if (time_ms > 2000) time_ms = 2000;  // Clamp to max
    g_tracks[track].glide_time_ms = time_ms;
}

/**
 * @brief Get portamento/glide time
 */
uint16_t legato_get_glide_time(uint8_t track) {
    if (track >= LEGATO_MAX_TRACKS) return 0;
    return g_tracks[track].glide_time_ms;
}

/**
 * @brief Process note on event
 */
uint8_t legato_process_note_on(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel) {
    if (track >= LEGATO_MAX_TRACKS) return 1;
    
    legato_track_t* t = &g_tracks[track];
    
    // If legato not enabled, pass through
    if (!t->enabled) return 1;
    
    // Check if note already exists (retrigger case)
    int8_t existing_idx = find_note_index(t, note, channel);
    if (existing_idx >= 0) {
        // Update velocity and reactivate
        t->notes[existing_idx].velocity = velocity;
        t->notes[existing_idx].active = 1;
        
        // If this note is currently active and retrigger is on, send retrigger event
        if (t->active_note == note && t->retrigger == LEGATO_RETRIGGER_ON) {
            legato_event_t event = {
                .type = LEGATO_EVENT_RETRIGGER,
                .note = note,
                .velocity = velocity,
                .channel = channel,
                .prev_note = note,
                .is_legato = 0
            };
            send_event(track, &event);
            return 1;  // Allow retrigger through
        }
        
        // Update active note selection
        update_active_note(track);
        return 0;  // Suppress duplicate note on
    }
    
    // Find empty slot for new note
    uint8_t slot = 0xFF;
    for (uint8_t i = 0; i < LEGATO_MAX_NOTES; i++) {
        if (!t->notes[i].active) {
            slot = i;
            break;
        }
    }
    
    // If buffer full, steal oldest note
    if (slot == 0xFF) {
        uint32_t oldest_time = 0xFFFFFFFF;
        for (uint8_t i = 0; i < LEGATO_MAX_NOTES; i++) {
            if (t->notes[i].timestamp < oldest_time) {
                oldest_time = t->notes[i].timestamp;
                slot = i;
            }
        }
    }
    
    // Add note to buffer
    t->notes[slot].note = note;
    t->notes[slot].velocity = velocity;
    t->notes[slot].channel = channel;
    t->notes[slot].active = 1;
    t->notes[slot].timestamp = t->note_counter++;
    t->note_count++;
    
    // Determine if this should become the active note
    uint8_t prev_note = t->active_note;
    update_active_note(track);
    
    // Check if active note changed
    if (t->active_note != prev_note) {
        // Determine if this is a legato transition
        uint8_t is_legato = (prev_note != 0xFF) && (t->retrigger == LEGATO_RETRIGGER_OFF);
        
        // Send appropriate event
        legato_event_t event;
        if (prev_note == 0xFF) {
            // First note - send note on
            event.type = LEGATO_EVENT_NOTE_ON;
            event.note = t->active_note;
            event.velocity = t->active_velocity;
            event.channel = t->active_channel;
            event.prev_note = 0xFF;
            event.is_legato = 0;
            send_event(track, &event);
            return 1;  // Allow first note through
        } else {
            // Note change - send note change event
            event.type = LEGATO_EVENT_NOTE_CHANGE;
            event.note = t->active_note;
            event.velocity = t->active_velocity;
            event.channel = t->active_channel;
            event.prev_note = prev_note;
            event.is_legato = is_legato;
            send_event(track, &event);
            
            // If retrigger mode, allow note through; otherwise suppress
            return (t->retrigger == LEGATO_RETRIGGER_ON) ? 1 : 0;
        }
    }
    
    // Note added but didn't become active (lower priority)
    return 0;
}

/**
 * @brief Process note off event
 */
uint8_t legato_process_note_off(uint8_t track, uint8_t note, uint8_t channel) {
    if (track >= LEGATO_MAX_TRACKS) return 1;
    
    legato_track_t* t = &g_tracks[track];
    
    // If legato not enabled, pass through
    if (!t->enabled) return 1;
    
    // Find and deactivate the note
    int8_t idx = find_note_index(t, note, channel);
    if (idx < 0) {
        return 0;  // Note not found, suppress
    }
    
    t->notes[idx].active = 0;
    t->note_count--;
    
    // If this was the active note, select new active note
    if (t->active_note == note) {
        uint8_t prev_note = t->active_note;
        update_active_note(track);
        
        if (t->active_note == 0xFF) {
            // No more notes - send note off
            legato_event_t event = {
                .type = LEGATO_EVENT_NOTE_OFF,
                .note = prev_note,
                .velocity = 0,
                .channel = t->active_channel,
                .prev_note = prev_note,
                .is_legato = 0
            };
            send_event(track, &event);
            return 1;  // Allow note off through
        } else {
            // Switched to another held note - send note change
            uint8_t is_legato = (t->retrigger == LEGATO_RETRIGGER_OFF);
            legato_event_t event = {
                .type = LEGATO_EVENT_NOTE_CHANGE,
                .note = t->active_note,
                .velocity = t->active_velocity,
                .channel = t->active_channel,
                .prev_note = prev_note,
                .is_legato = is_legato
            };
            send_event(track, &event);
            
            // In legato mode, suppress the note off
            return 0;
        }
    }
    
    // Released note wasn't active, suppress note off
    return 0;
}

/**
 * @brief Get currently active note for a track
 */
uint8_t legato_get_active_note(uint8_t track) {
    if (track >= LEGATO_MAX_TRACKS) return 0xFF;
    return g_tracks[track].active_note;
}

/**
 * @brief Get currently active velocity for a track
 */
uint8_t legato_get_active_velocity(uint8_t track) {
    if (track >= LEGATO_MAX_TRACKS) return 0;
    return g_tracks[track].active_velocity;
}

/**
 * @brief Get number of notes currently held on a track
 */
uint8_t legato_get_held_note_count(uint8_t track) {
    if (track >= LEGATO_MAX_TRACKS) return 0;
    return g_tracks[track].note_count;
}

/**
 * @brief Clear all held notes on a track (panic/reset)
 */
void legato_clear_all_notes(uint8_t track) {
    if (track >= LEGATO_MAX_TRACKS) return;
    
    legato_track_t* t = &g_tracks[track];
    
    // Send note off if there was an active note
    if (t->active_note != 0xFF) {
        legato_event_t event = {
            .type = LEGATO_EVENT_NOTE_OFF,
            .note = t->active_note,
            .velocity = 0,
            .channel = t->active_channel,
            .prev_note = t->active_note,
            .is_legato = 0
        };
        send_event(track, &event);
    }
    
    // Clear all notes
    for (uint8_t i = 0; i < LEGATO_MAX_NOTES; i++) {
        t->notes[i].active = 0;
    }
    t->note_count = 0;
    t->active_note = 0xFF;
    t->active_velocity = 0;
}

/**
 * @brief Clear all held notes on all tracks (global panic)
 */
void legato_clear_all_tracks(void) {
    for (uint8_t i = 0; i < LEGATO_MAX_TRACKS; i++) {
        legato_clear_all_notes(i);
    }
}

/**
 * @brief Get priority mode name
 */
const char* legato_get_priority_name(legato_priority_t priority) {
    if (priority >= LEGATO_PRIORITY_COUNT) return "Unknown";
    return priority_names[priority];
}

/**
 * @brief Get retrigger mode name
 */
const char* legato_get_retrigger_name(legato_retrigger_t retrigger) {
    if (retrigger >= LEGATO_RETRIGGER_COUNT) return "Unknown";
    return retrigger_names[retrigger];
}

/**
 * @brief Set event callback
 */
void legato_set_event_callback(legato_event_cb_t callback) {
    g_event_callback = callback;
}

/**
 * @brief Enable/disable mono mode
 */
void legato_set_mono_mode(uint8_t track, uint8_t enabled) {
    if (track >= LEGATO_MAX_TRACKS) return;
    g_tracks[track].mono_mode = enabled ? 1 : 0;
}

/**
 * @brief Check if mono mode is enabled
 */
uint8_t legato_is_mono_mode(uint8_t track) {
    if (track >= LEGATO_MAX_TRACKS) return 0;
    return g_tracks[track].mono_mode;
}

// ============================================================================
// Internal helper functions
// ============================================================================

/**
 * @brief Update the active note based on priority mode
 */
static void update_active_note(uint8_t track) {
    if (track >= LEGATO_MAX_TRACKS) return;
    
    legato_track_t* t = &g_tracks[track];
    
    // If no notes held, clear active note
    if (t->note_count == 0) {
        t->active_note = 0xFF;
        t->active_velocity = 0;
        return;
    }
    
    // Find note with priority
    uint8_t priority_idx = find_priority_note(t);
    if (priority_idx < LEGATO_MAX_NOTES) {
        t->active_note = t->notes[priority_idx].note;
        t->active_velocity = t->notes[priority_idx].velocity;
        t->active_channel = t->notes[priority_idx].channel;
    }
}

/**
 * @brief Find note with highest priority based on priority mode
 */
static uint8_t find_priority_note(legato_track_t* t) {
    uint8_t best_idx = 0xFF;
    
    switch (t->priority) {
        case LEGATO_PRIORITY_LAST: {
            // Find most recently pressed note
            uint32_t newest_time = 0;
            for (uint8_t i = 0; i < LEGATO_MAX_NOTES; i++) {
                if (t->notes[i].active && t->notes[i].timestamp >= newest_time) {
                    newest_time = t->notes[i].timestamp;
                    best_idx = i;
                }
            }
            break;
        }
        
        case LEGATO_PRIORITY_FIRST: {
            // Find first pressed note (oldest timestamp)
            uint32_t oldest_time = 0xFFFFFFFF;
            for (uint8_t i = 0; i < LEGATO_MAX_NOTES; i++) {
                if (t->notes[i].active && t->notes[i].timestamp < oldest_time) {
                    oldest_time = t->notes[i].timestamp;
                    best_idx = i;
                }
            }
            break;
        }
        
        case LEGATO_PRIORITY_HIGHEST: {
            // Find highest note number
            uint8_t highest_note = 0;
            for (uint8_t i = 0; i < LEGATO_MAX_NOTES; i++) {
                if (t->notes[i].active && t->notes[i].note >= highest_note) {
                    highest_note = t->notes[i].note;
                    best_idx = i;
                }
            }
            break;
        }
        
        case LEGATO_PRIORITY_LOWEST: {
            // Find lowest note number
            uint8_t lowest_note = 0xFF;
            for (uint8_t i = 0; i < LEGATO_MAX_NOTES; i++) {
                if (t->notes[i].active && t->notes[i].note < lowest_note) {
                    lowest_note = t->notes[i].note;
                    best_idx = i;
                }
            }
            break;
        }
        
        default:
            break;
    }
    
    return best_idx;
}

/**
 * @brief Send event to callback if registered
 */
static void send_event(uint8_t track, const legato_event_t* event) {
    if (g_event_callback != NULL) {
        g_event_callback(track, event);
    }
}

/**
 * @brief Find note index in buffer
 * @return Index if found, -1 if not found
 */
static int8_t find_note_index(legato_track_t* t, uint8_t note, uint8_t channel) {
    for (uint8_t i = 0; i < LEGATO_MAX_NOTES; i++) {
        if (t->notes[i].active && 
            t->notes[i].note == note && 
            t->notes[i].channel == channel) {
            return i;
        }
    }
    return -1;
}
