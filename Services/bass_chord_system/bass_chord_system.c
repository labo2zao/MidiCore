/**
 * @file bass_chord_system.c
 * @brief Bass Chord System implementation
 */

#include "Services/bass_chord_system/bass_chord_system.h"
#include <string.h>

static const char* layout_names[] = {
    "120-bass", "96-bass", "72-bass", "48-bass", "Free bass"
};

// Stradella bass layout: Circle of fifths
// C, G, D, A, E, B, F#, C#, G#, D#, A#, F
static const uint8_t circle_of_fifths[] = {0, 7, 2, 9, 4, 11, 6, 1, 8, 3, 10, 5};

// Chord intervals for each Stradella type
static const uint8_t chord_intervals[][6] = {
    {0, 7, 0, 0, 0, 0},          // Counter bass: root + 5th
    {0, 12, 0, 0, 0, 0},         // Bass: root + octave
    {0, 4, 7, 12, 0, 0},         // Major: root, 3rd, 5th, octave
    {0, 3, 7, 12, 0, 0},         // Minor: root, m3rd, 5th, octave
    {0, 4, 7, 10, 12, 0},        // Dom7: root, 3rd, 5th, b7th, octave
    {0, 3, 6, 9, 12, 0},         // Dim7: root, m3rd, d5th, d7th, octave
};

static const uint8_t chord_note_counts[] = {2, 2, 4, 4, 5, 5};

typedef struct {
    bass_layout_t layout;
    uint8_t base_note;
    uint8_t octave_doubling;
    uint8_t voicing_density;
    uint8_t bass_velocity_percent;
    uint8_t chord_velocity_percent;
} bass_chord_config_t;

static bass_chord_config_t g_bass[BASS_CHORD_MAX_TRACKS];
static bass_chord_output_cb_t g_output_callback = NULL;

/**
 * @brief Initialize bass chord system
 */
void bass_chord_init(void) {
    memset(g_bass, 0, sizeof(g_bass));
    
    for (uint8_t t = 0; t < BASS_CHORD_MAX_TRACKS; t++) {
        g_bass[t].layout = BASS_LAYOUT_120;
        g_bass[t].base_note = 36;  // C2
        g_bass[t].octave_doubling = 1;
        g_bass[t].voicing_density = 1;  // Normal
        g_bass[t].bass_velocity_percent = 110;  // Bass slightly louder
        g_bass[t].chord_velocity_percent = 90;  // Chords slightly softer
    }
}

/**
 * @brief Set bass layout
 */
void bass_chord_set_layout(uint8_t track, bass_layout_t layout) {
    if (track >= BASS_CHORD_MAX_TRACKS) return;
    if (layout >= BASS_LAYOUT_COUNT) return;
    g_bass[track].layout = layout;
}

/**
 * @brief Get bass layout
 */
bass_layout_t bass_chord_get_layout(uint8_t track) {
    if (track >= BASS_CHORD_MAX_TRACKS) return BASS_LAYOUT_120;
    return g_bass[track].layout;
}

/**
 * @brief Set bass note range
 */
void bass_chord_set_base_note(uint8_t track, uint8_t start_note) {
    if (track >= BASS_CHORD_MAX_TRACKS) return;
    if (start_note > 127) start_note = 127;
    g_bass[track].base_note = start_note;
}

/**
 * @brief Get bass note range
 */
uint8_t bass_chord_get_base_note(uint8_t track) {
    if (track >= BASS_CHORD_MAX_TRACKS) return 36;
    return g_bass[track].base_note;
}

/**
 * @brief Enable/disable octave doubling
 */
void bass_chord_set_octave_doubling(uint8_t track, uint8_t enabled) {
    if (track >= BASS_CHORD_MAX_TRACKS) return;
    g_bass[track].octave_doubling = enabled ? 1 : 0;
}

/**
 * @brief Check if octave doubling is enabled
 */
uint8_t bass_chord_is_octave_doubling(uint8_t track) {
    if (track >= BASS_CHORD_MAX_TRACKS) return 0;
    return g_bass[track].octave_doubling;
}

/**
 * @brief Set chord voicing density
 */
void bass_chord_set_voicing_density(uint8_t track, uint8_t density) {
    if (track >= BASS_CHORD_MAX_TRACKS) return;
    if (density > 2) density = 2;
    g_bass[track].voicing_density = density;
}

/**
 * @brief Get chord voicing density
 */
uint8_t bass_chord_get_voicing_density(uint8_t track) {
    if (track >= BASS_CHORD_MAX_TRACKS) return 1;
    return g_bass[track].voicing_density;
}

/**
 * @brief Set bass velocity
 */
void bass_chord_set_bass_velocity(uint8_t track, uint8_t percent) {
    if (track >= BASS_CHORD_MAX_TRACKS) return;
    if (percent > 150) percent = 150;
    g_bass[track].bass_velocity_percent = percent;
}

/**
 * @brief Get bass velocity percentage
 */
uint8_t bass_chord_get_bass_velocity(uint8_t track) {
    if (track >= BASS_CHORD_MAX_TRACKS) return 110;
    return g_bass[track].bass_velocity_percent;
}

/**
 * @brief Set chord velocity
 */
void bass_chord_set_chord_velocity(uint8_t track, uint8_t percent) {
    if (track >= BASS_CHORD_MAX_TRACKS) return;
    if (percent > 150) percent = 150;
    g_bass[track].chord_velocity_percent = percent;
}

/**
 * @brief Get chord velocity percentage
 */
uint8_t bass_chord_get_chord_velocity(uint8_t track) {
    if (track >= BASS_CHORD_MAX_TRACKS) return 90;
    return g_bass[track].chord_velocity_percent;
}

/**
 * @brief Map button to Stradella type and root note
 */
void bass_chord_button_to_stradella(uint8_t track, uint8_t button,
                                   stradella_type_t* type, uint8_t* root) {
    if (track >= BASS_CHORD_MAX_TRACKS) return;
    
    // Standard 120-bass layout: 6 rows × 20 buttons
    // Rows: Counter-bass, Bass, Major, Minor, 7th, Dim
    uint8_t row = button / 20;
    uint8_t col = button % 20;
    
    // Map row to Stradella type
    if (row >= STRADELLA_COUNT) row = STRADELLA_BASS;
    if (type) *type = (stradella_type_t)row;
    
    // Map column to root note (circle of fifths)
    if (root) *root = circle_of_fifths[col % 12];
}

/**
 * @brief Process incoming bass button press
 */
void bass_chord_process_button(uint8_t track, uint8_t button, uint8_t velocity, uint8_t channel) {
    if (track >= BASS_CHORD_MAX_TRACKS) return;
    if (!g_output_callback) return;
    
    bass_chord_config_t* cfg = &g_bass[track];
    
    // Decode button to type and root
    stradella_type_t type;
    uint8_t root;
    bass_chord_button_to_stradella(track, button, &type, &root);
    
    // Calculate base note
    uint8_t base_note = cfg->base_note + root;
    
    // Determine velocity multiplier
    uint8_t vel_percent = (type <= STRADELLA_BASS) ? 
                          cfg->bass_velocity_percent : 
                          cfg->chord_velocity_percent;
    
    uint16_t adj_velocity = (velocity * vel_percent) / 100;
    if (adj_velocity > 127) adj_velocity = 127;
    
    // Generate notes based on type
    const uint8_t* intervals = chord_intervals[type];
    uint8_t note_count = chord_note_counts[type];
    
    // Adjust note count based on voicing density
    if (cfg->voicing_density == 0 && note_count > 3) {
        note_count = 3;  // Sparse
    } else if (cfg->voicing_density == 2 && note_count < 5) {
        note_count = 5;  // Dense
    }
    
    for (uint8_t i = 0; i < note_count; i++) {
        if (intervals[i] == 0 && i > 0) continue;
        
        uint8_t note = base_note + intervals[i];
        if (note <= 127) {
            g_output_callback(track, note, (uint8_t)adj_velocity, channel);
            
            // Octave doubling for bass notes
            if (cfg->octave_doubling && type <= STRADELLA_BASS && note >= 12) {
                g_output_callback(track, note - 12, (uint8_t)adj_velocity, channel);
            }
        }
    }
}

/**
 * @brief Get layout name
 */
const char* bass_chord_get_layout_name(bass_layout_t layout) {
    if (layout >= BASS_LAYOUT_COUNT) return "Unknown";
    return layout_names[layout];
}

/**
 * @brief Set output callback
 */
void bass_chord_set_output_callback(bass_chord_output_cb_t callback) {
    g_output_callback = callback;
}
