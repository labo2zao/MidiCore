/**
 * @file one_finger_chord.c
 * @brief One-Finger Chord implementation
 */

#include "Services/one_finger_chord/one_finger_chord.h"
#include <string.h>

static const char* mode_names[] = {
    "Disabled", "Auto", "Split", "Single Note"
};

// Major scale intervals (semitones from root)
static const uint8_t major_intervals[] = {0, 4, 7};  // Root, 3rd, 5th
static const uint8_t minor_intervals[] = {0, 3, 7};  // Root, minor 3rd, 5th

typedef struct {
    uint8_t active;
    uint8_t note;
    uint8_t velocity;
    uint8_t is_chord_note;  // 1 if generated chord note, 0 if melody
} active_note_t;

typedef struct {
    ofc_mode_t mode;
    ofc_voicing_t voicing;
    uint8_t split_point;
    uint8_t chord_velocity_percent;
    uint8_t bass_enabled;
    uint8_t current_root;
    uint8_t is_minor;
    active_note_t notes[16];  // Track active notes
} ofc_config_t;

static ofc_config_t g_ofc[ONE_FINGER_MAX_TRACKS];
static ofc_output_cb_t g_output_callback = NULL;

/**
 * @brief Initialize one-finger chord module
 */
void ofc_init(void) {
    memset(g_ofc, 0, sizeof(g_ofc));
    
    for (uint8_t t = 0; t < ONE_FINGER_MAX_TRACKS; t++) {
        g_ofc[t].mode = OFC_MODE_DISABLED;
        g_ofc[t].voicing = OFC_VOICING_TRIAD;
        g_ofc[t].split_point = 60;  // C4
        g_ofc[t].chord_velocity_percent = 70;
        g_ofc[t].bass_enabled = 1;
        g_ofc[t].current_root = 0;  // C
        g_ofc[t].is_minor = 0;
    }
}

/**
 * @brief Set mode for a track
 */
void ofc_set_mode(uint8_t track, ofc_mode_t mode) {
    if (track >= ONE_FINGER_MAX_TRACKS) return;
    if (mode >= OFC_MODE_COUNT) return;
    g_ofc[track].mode = mode;
}

/**
 * @brief Get current mode
 */
ofc_mode_t ofc_get_mode(uint8_t track) {
    if (track >= ONE_FINGER_MAX_TRACKS) return OFC_MODE_DISABLED;
    return g_ofc[track].mode;
}

/**
 * @brief Set chord voicing style
 */
void ofc_set_voicing(uint8_t track, ofc_voicing_t voicing) {
    if (track >= ONE_FINGER_MAX_TRACKS) return;
    if (voicing >= OFC_VOICING_COUNT) return;
    g_ofc[track].voicing = voicing;
}

/**
 * @brief Get chord voicing style
 */
ofc_voicing_t ofc_get_voicing(uint8_t track) {
    if (track >= ONE_FINGER_MAX_TRACKS) return OFC_VOICING_TRIAD;
    return g_ofc[track].voicing;
}

/**
 * @brief Set keyboard split point
 */
void ofc_set_split_point(uint8_t track, uint8_t split_note) {
    if (track >= ONE_FINGER_MAX_TRACKS) return;
    if (split_note > 127) split_note = 127;
    g_ofc[track].split_point = split_note;
}

/**
 * @brief Get split point
 */
uint8_t ofc_get_split_point(uint8_t track) {
    if (track >= ONE_FINGER_MAX_TRACKS) return 60;
    return g_ofc[track].split_point;
}

/**
 * @brief Set chord velocity relative to melody
 */
void ofc_set_chord_velocity(uint8_t track, uint8_t percent) {
    if (track >= ONE_FINGER_MAX_TRACKS) return;
    if (percent > 100) percent = 100;
    g_ofc[track].chord_velocity_percent = percent;
}

/**
 * @brief Get chord velocity percentage
 */
uint8_t ofc_get_chord_velocity(uint8_t track) {
    if (track >= ONE_FINGER_MAX_TRACKS) return 70;
    return g_ofc[track].chord_velocity_percent;
}

/**
 * @brief Enable/disable bass note generation
 */
void ofc_set_bass_enabled(uint8_t track, uint8_t enabled) {
    if (track >= ONE_FINGER_MAX_TRACKS) return;
    g_ofc[track].bass_enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if bass is enabled
 */
uint8_t ofc_is_bass_enabled(uint8_t track) {
    if (track >= ONE_FINGER_MAX_TRACKS) return 0;
    return g_ofc[track].bass_enabled;
}

/**
 * @brief Generate chord notes from root
 */
static void generate_chord_notes(uint8_t track, uint8_t root_note, uint8_t velocity, 
                                 uint8_t channel, uint8_t is_note_on) {
    if (!g_output_callback) return;
    
    ofc_config_t* cfg = &g_ofc[track];
    const uint8_t* intervals = cfg->is_minor ? minor_intervals : major_intervals;
    
    // Calculate chord velocity
    uint8_t chord_vel = (velocity * cfg->chord_velocity_percent) / 100;
    if (chord_vel < 1 && is_note_on) chord_vel = 1;
    
    // Generate bass note (one octave down)
    if (cfg->bass_enabled && root_note >= 12) {
        g_output_callback(track, root_note - 12, is_note_on ? chord_vel : 0, channel);
    }
    
    // Generate chord tones based on voicing
    uint8_t num_notes = 0;
    switch (cfg->voicing) {
        case OFC_VOICING_SIMPLE:
            num_notes = 2;  // Root + 5th
            g_output_callback(track, root_note, is_note_on ? chord_vel : 0, channel);
            if (root_note + 7 <= 127) {
                g_output_callback(track, root_note + 7, is_note_on ? chord_vel : 0, channel);
            }
            break;
            
        case OFC_VOICING_TRIAD:
        case OFC_VOICING_SEVENTH:
        case OFC_VOICING_FULL:
            // Generate triad
            for (uint8_t i = 0; i < 3; i++) {
                uint8_t note = root_note + intervals[i];
                if (note <= 127) {
                    g_output_callback(track, note, is_note_on ? chord_vel : 0, channel);
                }
            }
            break;
            
        default:
            break;
    }
}

/**
 * @brief Process incoming MIDI note
 */
void ofc_process_note(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel) {
    if (track >= ONE_FINGER_MAX_TRACKS) return;
    
    ofc_config_t* cfg = &g_ofc[track];
    uint8_t is_note_on = (velocity > 0);
    
    // Pass through original note
    if (g_output_callback) {
        g_output_callback(track, note, velocity, channel);
    }
    
    if (cfg->mode == OFC_MODE_DISABLED) {
        return;
    }
    
    // Determine if this is a chord zone note or melody zone note
    uint8_t is_chord_zone = (note < cfg->split_point);
    
    if (cfg->mode == OFC_MODE_SPLIT_KEYBOARD) {
        if (is_chord_zone && is_note_on) {
            // Update chord from note in chord zone
            cfg->current_root = note % 12;
            // Simple heuristic: lower notes more likely minor
            cfg->is_minor = (note < 48) ? 1 : 0;
        } else if (!is_chord_zone && is_note_on) {
            // Melody note - generate chord accompaniment
            generate_chord_notes(track, note - 12, velocity, channel, 1);
        }
    } else if (cfg->mode == OFC_MODE_SINGLE_NOTE_CHORD) {
        if (is_note_on) {
            // Every note triggers a chord
            cfg->current_root = note % 12;
            generate_chord_notes(track, note, velocity, channel, 1);
        } else {
            // Note off - stop chord
            generate_chord_notes(track, note, 0, channel, 0);
        }
    } else if (cfg->mode == OFC_MODE_AUTO) {
        // Auto-detect chord from melody
        if (is_note_on) {
            cfg->current_root = note % 12;
            generate_chord_notes(track, note, velocity, channel, 1);
        } else {
            generate_chord_notes(track, note, 0, channel, 0);
        }
    }
}

/**
 * @brief Manually set the current chord
 */
void ofc_set_chord(uint8_t track, uint8_t root_note, uint8_t is_minor) {
    if (track >= ONE_FINGER_MAX_TRACKS) return;
    if (root_note > 11) root_note = 11;
    
    g_ofc[track].current_root = root_note;
    g_ofc[track].is_minor = is_minor ? 1 : 0;
}

/**
 * @brief Get mode name
 */
const char* ofc_get_mode_name(ofc_mode_t mode) {
    if (mode >= OFC_MODE_COUNT) return "Unknown";
    return mode_names[mode];
}

/**
 * @brief Set output callback
 */
void ofc_set_output_callback(ofc_output_cb_t callback) {
    g_output_callback = callback;
}
