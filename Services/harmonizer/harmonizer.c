/**
 * @file harmonizer.c
 * @brief MIDI harmonizer implementation
 */

#include "Services/harmonizer/harmonizer.h"
#include "Services/scale/scale.h"
#include <string.h>

// Interval names
static const char* interval_names[] = {
    "Unison", "3rd Up", "3rd Down", "5th Up", "5th Down",
    "Octave Up", "Octave Down", "4th Up", "4th Down", "6th Up", "6th Down"
};

// Diatonic interval steps in scale degrees
static const int8_t interval_steps[] = {
    0,   // UNISON
    2,   // THIRD_UP (2 scale steps)
    -2,  // THIRD_DOWN
    4,   // FIFTH_UP (4 scale steps)
    -4,  // FIFTH_DOWN
    7,   // OCTAVE_UP (7 scale steps = octave)
    -7,  // OCTAVE_DOWN
    3,   // FOURTH_UP (3 scale steps)
    -3,  // FOURTH_DOWN
    5,   // SIXTH_UP (5 scale steps)
    -5,  // SIXTH_DOWN
};

// Voice configuration
typedef struct {
    uint8_t enabled;
    harmonizer_interval_t interval;
    int8_t velocity_offset;
} voice_config_t;

// Per-track harmonizer configuration
typedef struct {
    uint8_t enabled;
    uint8_t scale_type;
    uint8_t scale_root;
    voice_config_t voices[HARMONIZER_MAX_VOICES];
} harmonizer_config_t;

static harmonizer_config_t g_harmonizer[HARMONIZER_MAX_TRACKS];

/**
 * @brief Initialize harmonizer module
 */
void harmonizer_init(void) {
    memset(g_harmonizer, 0, sizeof(g_harmonizer));
    
    // Initialize defaults
    for (uint8_t t = 0; t < HARMONIZER_MAX_TRACKS; t++) {
        g_harmonizer[t].enabled = 0;
        g_harmonizer[t].scale_type = 0;  // Chromatic
        g_harmonizer[t].scale_root = 0;  // C
        
        // Voice 0 is original note (always enabled)
        g_harmonizer[t].voices[0].enabled = 1;
        g_harmonizer[t].voices[0].interval = HARM_INTERVAL_UNISON;
        g_harmonizer[t].voices[0].velocity_offset = 0;
        
        // Other voices disabled by default
        for (uint8_t v = 1; v < HARMONIZER_MAX_VOICES; v++) {
            g_harmonizer[t].voices[v].enabled = 0;
            g_harmonizer[t].voices[v].interval = HARM_INTERVAL_THIRD_UP;
            g_harmonizer[t].voices[v].velocity_offset = -10;  // Slightly quieter
        }
    }
}

/**
 * @brief Enable/disable harmonizer for a track
 */
void harmonizer_set_enabled(uint8_t track, uint8_t enabled) {
    if (track >= HARMONIZER_MAX_TRACKS) return;
    g_harmonizer[track].enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if harmonizer is enabled for a track
 */
uint8_t harmonizer_is_enabled(uint8_t track) {
    if (track >= HARMONIZER_MAX_TRACKS) return 0;
    return g_harmonizer[track].enabled;
}

/**
 * @brief Set harmony voice interval
 */
void harmonizer_set_voice_interval(uint8_t track, uint8_t voice, harmonizer_interval_t interval) {
    if (track >= HARMONIZER_MAX_TRACKS) return;
    if (voice >= HARMONIZER_MAX_VOICES) return;
    if (interval >= HARM_INTERVAL_COUNT) return;
    
    g_harmonizer[track].voices[voice].interval = interval;
}

/**
 * @brief Get harmony voice interval
 */
harmonizer_interval_t harmonizer_get_voice_interval(uint8_t track, uint8_t voice) {
    if (track >= HARMONIZER_MAX_TRACKS) return HARM_INTERVAL_UNISON;
    if (voice >= HARMONIZER_MAX_VOICES) return HARM_INTERVAL_UNISON;
    
    return g_harmonizer[track].voices[voice].interval;
}

/**
 * @brief Enable/disable a harmony voice
 */
void harmonizer_set_voice_enabled(uint8_t track, uint8_t voice, uint8_t enabled) {
    if (track >= HARMONIZER_MAX_TRACKS) return;
    if (voice >= HARMONIZER_MAX_VOICES) return;
    
    g_harmonizer[track].voices[voice].enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if a harmony voice is enabled
 */
uint8_t harmonizer_is_voice_enabled(uint8_t track, uint8_t voice) {
    if (track >= HARMONIZER_MAX_TRACKS) return 0;
    if (voice >= HARMONIZER_MAX_VOICES) return 0;
    
    return g_harmonizer[track].voices[voice].enabled;
}

/**
 * @brief Set voice velocity offset
 */
void harmonizer_set_voice_velocity(uint8_t track, uint8_t voice, int8_t offset) {
    if (track >= HARMONIZER_MAX_TRACKS) return;
    if (voice >= HARMONIZER_MAX_VOICES) return;
    
    // Clamp offset to reasonable range
    if (offset < -64) offset = -64;
    if (offset > 63) offset = 63;
    
    g_harmonizer[track].voices[voice].velocity_offset = offset;
}

/**
 * @brief Get voice velocity offset
 */
int8_t harmonizer_get_voice_velocity(uint8_t track, uint8_t voice) {
    if (track >= HARMONIZER_MAX_TRACKS) return 0;
    if (voice >= HARMONIZER_MAX_VOICES) return 0;
    
    return g_harmonizer[track].voices[voice].velocity_offset;
}

/**
 * @brief Set scale for harmonization
 */
void harmonizer_set_scale(uint8_t track, uint8_t scale_type, uint8_t root) {
    if (track >= HARMONIZER_MAX_TRACKS) return;
    
    g_harmonizer[track].scale_type = scale_type;
    g_harmonizer[track].scale_root = root % 12;
}

/**
 * @brief Get scale for harmonization
 */
void harmonizer_get_scale(uint8_t track, uint8_t* scale_type, uint8_t* root) {
    if (track >= HARMONIZER_MAX_TRACKS) return;
    
    if (scale_type) *scale_type = g_harmonizer[track].scale_type;
    if (root) *root = g_harmonizer[track].scale_root;
}

/**
 * @brief Find note at scale degree offset from input note
 */
static uint8_t find_harmony_note(uint8_t input_note, int8_t degree_offset, 
                                 uint8_t scale_type, uint8_t scale_root) {
    if (degree_offset == 0) return input_note;
    
    // For chromatic scale or octaves, use simple semitone math
    if (scale_type == 0 || degree_offset == 7 || degree_offset == -7) {
        int16_t semitone_offset = (degree_offset == 7) ? 12 : (degree_offset == -7) ? -12 : 0;
        int16_t result = input_note + semitone_offset;
        if (result < 0) result = 0;
        if (result > 127) result = 127;
        return (uint8_t)result;
    }
    
    // Get scale intervals (array of semitones from root)
    uint8_t scale_intervals[12];
    uint8_t scale_length = 0;
    
    // Build scale pattern (simplified - using common patterns)
    // Major scale pattern: 0, 2, 4, 5, 7, 9, 11
    static const uint8_t major_pattern[] = {0, 2, 4, 5, 7, 9, 11};
    static const uint8_t minor_pattern[] = {0, 2, 3, 5, 7, 8, 10};
    
    const uint8_t* pattern = (scale_type == 1) ? major_pattern : 
                             (scale_type == 2 || scale_type == 3 || scale_type == 4) ? minor_pattern :
                             major_pattern;  // Default to major
    scale_length = 7;
    
    for (uint8_t i = 0; i < scale_length; i++) {
        scale_intervals[i] = pattern[i];
    }
    
    // Find input note's position in scale
    uint8_t note_in_octave = input_note % 12;
    uint8_t octave = input_note / 12;
    uint8_t relative_note = (note_in_octave + 12 - scale_root) % 12;
    
    // Find closest scale degree to input note
    int8_t current_degree = 0;
    uint8_t closest_interval = 0;
    
    for (uint8_t i = 0; i < scale_length; i++) {
        if (scale_intervals[i] == relative_note) {
            current_degree = i;
            closest_interval = scale_intervals[i];
            break;
        }
        if (i > 0 && scale_intervals[i] > relative_note) {
            // Between two scale notes - pick closer one
            if ((relative_note - scale_intervals[i-1]) <= (scale_intervals[i] - relative_note)) {
                current_degree = i - 1;
                closest_interval = scale_intervals[i-1];
            } else {
                current_degree = i;
                closest_interval = scale_intervals[i];
            }
            break;
        }
    }
    
    // Calculate target degree
    int16_t target_degree = current_degree + degree_offset;
    int16_t octave_offset = 0;
    
    // Handle wrapping around octaves
    while (target_degree < 0) {
        target_degree += scale_length;
        octave_offset--;
    }
    while (target_degree >= scale_length) {
        target_degree -= scale_length;
        octave_offset++;
    }
    
    // Get target interval
    uint8_t target_interval = scale_intervals[target_degree];
    
    // Calculate final note
    int16_t result = (octave + octave_offset) * 12 + scale_root + target_interval;
    
    // Clamp to MIDI range
    if (result < 0) result = 0;
    if (result > 127) result = 127;
    
    return (uint8_t)result;
}

/**
 * @brief Generate harmony notes from an input note
 */
uint8_t harmonizer_generate(uint8_t track, uint8_t input_note, uint8_t input_velocity,
                            uint8_t* output_notes, uint8_t* output_velocities) {
    if (track >= HARMONIZER_MAX_TRACKS || !output_notes || !output_velocities) {
        if (output_notes) output_notes[0] = input_note;
        if (output_velocities) output_velocities[0] = input_velocity;
        return 1;
    }
    
    harmonizer_config_t* cfg = &g_harmonizer[track];
    
    // If disabled, just pass through
    if (!cfg->enabled) {
        output_notes[0] = input_note;
        output_velocities[0] = input_velocity;
        return 1;
    }
    
    uint8_t count = 0;
    
    // Generate each voice
    for (uint8_t v = 0; v < HARMONIZER_MAX_VOICES; v++) {
        if (!cfg->voices[v].enabled) continue;
        
        harmonizer_interval_t interval = cfg->voices[v].interval;
        if (interval >= HARM_INTERVAL_COUNT) continue;
        
        int8_t degree_offset = interval_steps[interval];
        
        // Generate harmony note
        uint8_t harmony_note = find_harmony_note(input_note, degree_offset, 
                                                 cfg->scale_type, cfg->scale_root);
        
        // Calculate velocity with offset
        int16_t velocity = input_velocity + cfg->voices[v].velocity_offset;
        if (velocity < 1) velocity = 1;
        if (velocity > 127) velocity = 127;
        
        output_notes[count] = harmony_note;
        output_velocities[count] = (uint8_t)velocity;
        count++;
    }
    
    // If no voices produced output, pass through original
    if (count == 0) {
        output_notes[0] = input_note;
        output_velocities[0] = input_velocity;
        return 1;
    }
    
    return count;
}

/**
 * @brief Get interval name
 */
const char* harmonizer_get_interval_name(harmonizer_interval_t interval) {
    if (interval >= HARM_INTERVAL_COUNT) return "Unknown";
    return interval_names[interval];
}
