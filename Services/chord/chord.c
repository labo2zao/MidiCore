/**
 * @file chord.c
 * @brief Chord trigger implementation
 */

#include "Services/chord/chord.h"
#include <string.h>

// Chord interval definitions (semitones from root)
static const uint8_t chord_intervals[][CHORD_MAX_NOTES] = {
    {0, 4, 7, 0, 0, 0},          // CHORD_TYPE_MAJOR
    {0, 3, 7, 0, 0, 0},          // CHORD_TYPE_MINOR
    {0, 3, 6, 0, 0, 0},          // CHORD_TYPE_DIMINISHED
    {0, 4, 8, 0, 0, 0},          // CHORD_TYPE_AUGMENTED
    {0, 2, 7, 0, 0, 0},          // CHORD_TYPE_SUS2
    {0, 5, 7, 0, 0, 0},          // CHORD_TYPE_SUS4
    {0, 4, 7, 11, 0, 0},         // CHORD_TYPE_MAJ7
    {0, 3, 7, 10, 0, 0},         // CHORD_TYPE_MIN7
    {0, 4, 7, 10, 0, 0},         // CHORD_TYPE_DOM7
    {0, 3, 6, 9, 0, 0},          // CHORD_TYPE_DIM7
    {0, 3, 6, 10, 0, 0},         // CHORD_TYPE_HALFIDIM7
    {0, 4, 8, 10, 0, 0},         // CHORD_TYPE_AUG7
    {0, 4, 7, 11, 14, 0},        // CHORD_TYPE_MAJ9
    {0, 3, 7, 10, 14, 0},        // CHORD_TYPE_MIN9
    {0, 4, 7, 10, 14, 0},        // CHORD_TYPE_DOM9
    {0, 7, 12, 0, 0, 0},         // CHORD_TYPE_POWER
    {0, 12, 24, 0, 0, 0},        // CHORD_TYPE_OCTAVE
};

// Number of notes per chord type
static const uint8_t chord_note_counts[] = {
    3,  // MAJOR
    3,  // MINOR
    3,  // DIMINISHED
    3,  // AUGMENTED
    3,  // SUS2
    3,  // SUS4
    4,  // MAJ7
    4,  // MIN7
    4,  // DOM7
    4,  // DIM7
    4,  // HALFIDIM7
    4,  // AUG7
    5,  // MAJ9
    5,  // MIN9
    5,  // DOM9
    3,  // POWER
    3,  // OCTAVE
};

// Chord type names
static const char* chord_type_names[] = {
    "Major", "Minor", "Dim", "Aug", "Sus2", "Sus4",
    "Maj7", "Min7", "Dom7", "Dim7", "m7b5", "Aug7",
    "Maj9", "Min9", "Dom9", "Power", "Octave"
};

// Per-track chord configuration
typedef struct {
    uint8_t enabled;
    chord_type_t type;
    uint8_t inversion;
    chord_voicing_t voicing;
} chord_config_t;

static chord_config_t g_chord_config[CHORD_MAX_TRACKS];

/**
 * @brief Initialize chord module
 */
void chord_init(void) {
    memset(g_chord_config, 0, sizeof(g_chord_config));
    
    // Initialize defaults
    for (uint8_t i = 0; i < CHORD_MAX_TRACKS; i++) {
        g_chord_config[i].enabled = 0;
        g_chord_config[i].type = CHORD_TYPE_MAJOR;
        g_chord_config[i].inversion = 0;
        g_chord_config[i].voicing = CHORD_VOICING_CLOSE;
    }
}

/**
 * @brief Enable/disable chord trigger for a track
 */
void chord_set_enabled(uint8_t track, uint8_t enabled) {
    if (track >= CHORD_MAX_TRACKS) return;
    g_chord_config[track].enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if chord trigger is enabled for a track
 */
uint8_t chord_is_enabled(uint8_t track) {
    if (track >= CHORD_MAX_TRACKS) return 0;
    return g_chord_config[track].enabled;
}

/**
 * @brief Set chord type for a track
 */
void chord_set_type(uint8_t track, chord_type_t type) {
    if (track >= CHORD_MAX_TRACKS) return;
    if (type >= CHORD_TYPE_COUNT) return;
    g_chord_config[track].type = type;
}

/**
 * @brief Get chord type for a track
 */
chord_type_t chord_get_type(uint8_t track) {
    if (track >= CHORD_MAX_TRACKS) return CHORD_TYPE_MAJOR;
    return g_chord_config[track].type;
}

/**
 * @brief Set chord inversion
 */
void chord_set_inversion(uint8_t track, uint8_t inversion) {
    if (track >= CHORD_MAX_TRACKS) return;
    if (inversion > 3) inversion = 3;  // Max 3 inversions
    g_chord_config[track].inversion = inversion;
}

/**
 * @brief Get chord inversion
 */
uint8_t chord_get_inversion(uint8_t track) {
    if (track >= CHORD_MAX_TRACKS) return 0;
    return g_chord_config[track].inversion;
}

/**
 * @brief Set chord voicing
 */
void chord_set_voicing(uint8_t track, chord_voicing_t voicing) {
    if (track >= CHORD_MAX_TRACKS) return;
    if (voicing >= CHORD_VOICING_COUNT) return;
    g_chord_config[track].voicing = voicing;
}

/**
 * @brief Get chord voicing
 */
chord_voicing_t chord_get_voicing(uint8_t track) {
    if (track >= CHORD_MAX_TRACKS) return CHORD_VOICING_CLOSE;
    return g_chord_config[track].voicing;
}

/**
 * @brief Apply inversion to chord notes
 */
static void apply_inversion(uint8_t* notes, uint8_t count, uint8_t inversion) {
    if (inversion == 0 || count == 0) return;
    
    // Limit inversions to number of notes - 1
    if (inversion >= count) inversion = count - 1;
    
    // Move bottom notes up by octave for each inversion
    for (uint8_t inv = 0; inv < inversion; inv++) {
        uint8_t bottom = notes[0];
        
        // Shift all notes down
        for (uint8_t i = 0; i < count - 1; i++) {
            notes[i] = notes[i + 1];
        }
        
        // Move bottom note to top + octave
        notes[count - 1] = bottom + 12;
        
        // Sort to maintain ascending order
        for (uint8_t i = 0; i < count - 1; i++) {
            for (uint8_t j = i + 1; j < count; j++) {
                if (notes[i] > notes[j]) {
                    uint8_t temp = notes[i];
                    notes[i] = notes[j];
                    notes[j] = temp;
                }
            }
        }
    }
}

/**
 * @brief Apply voicing to chord notes
 */
static void apply_voicing(uint8_t* notes, uint8_t count, chord_voicing_t voicing) {
    if (voicing == CHORD_VOICING_CLOSE || count < 3) return;
    
    switch (voicing) {
        case CHORD_VOICING_DROP2:
            // Move 2nd highest note down an octave
            if (count >= 3) {
                notes[count - 2] -= 12;
            }
            break;
            
        case CHORD_VOICING_DROP3:
            // Move 3rd highest note down an octave
            if (count >= 4) {
                notes[count - 3] -= 12;
            }
            break;
            
        case CHORD_VOICING_SPREAD:
            // Spread notes over wider range
            for (uint8_t i = 1; i < count; i++) {
                if (i % 2 == 0) {
                    notes[i] += 12;  // Move alternate notes up
                }
            }
            break;
            
        default:
            break;
    }
    
    // Sort to maintain ascending order
    for (uint8_t i = 0; i < count - 1; i++) {
        for (uint8_t j = i + 1; j < count; j++) {
            if (notes[i] > notes[j]) {
                uint8_t temp = notes[i];
                notes[i] = notes[j];
                notes[j] = temp;
            }
        }
    }
}

/**
 * @brief Generate chord notes from a root note
 */
uint8_t chord_generate(uint8_t track, uint8_t root_note, uint8_t* notes) {
    if (track >= CHORD_MAX_TRACKS || !notes) return 0;
    
    chord_config_t* cfg = &g_chord_config[track];
    if (!cfg->enabled) {
        notes[0] = root_note;
        return 1;
    }
    
    if (cfg->type >= CHORD_TYPE_COUNT) {
        notes[0] = root_note;
        return 1;
    }
    
    // Get chord intervals and count
    const uint8_t* intervals = chord_intervals[cfg->type];
    uint8_t count = chord_note_counts[cfg->type];
    
    // Generate chord notes from intervals
    uint8_t valid_count = 0;
    for (uint8_t i = 0; i < count; i++) {
        int16_t note = root_note + intervals[i];
        if (note >= 0 && note <= 127) {
            notes[valid_count++] = (uint8_t)note;
        }
    }
    
    if (valid_count == 0) {
        notes[0] = root_note;
        return 1;
    }
    
    // Apply inversion
    apply_inversion(notes, valid_count, cfg->inversion);
    
    // Apply voicing
    apply_voicing(notes, valid_count, cfg->voicing);
    
    // Ensure all notes are within MIDI range
    for (uint8_t i = 0; i < valid_count; i++) {
        if (notes[i] > 127) notes[i] = 127;
        if (notes[i] < 0) notes[i] = 0;
    }
    
    return valid_count;
}

/**
 * @brief Get chord type name
 */
const char* chord_get_type_name(chord_type_t type) {
    if (type >= CHORD_TYPE_COUNT) return "Unknown";
    return chord_type_names[type];
}
