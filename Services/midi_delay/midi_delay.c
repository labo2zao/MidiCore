/**
 * @file midi_delay.c
 * @brief MIDI delay/echo implementation
 */

#include "Config/module_config.h"

#if MODULE_ENABLE_MIDI_DELAY_FX

#include "Services/midi_delay/midi_delay.h"
#include <string.h>

// Division names
static const char* division_names[] = {
    "1/64", "1/32", "1/16", "1/8", "1/4", "1/2", "1/1",
    "1/16T", "1/8T", "1/4T", "1/16.", "1/8.", "1/4."
};

// Division multipliers (in 1/64th notes)
static const uint16_t division_multipliers[] = {
    1,    // 1/64
    2,    // 1/32
    4,    // 1/16
    8,    // 1/8
    16,   // 1/4
    32,   // 1/2
    64,   // 1/1
    3,    // 1/16T (1/16 triplet = 1/24, but relative to 1/64: 64/24 ≈ 2.67, use 3)
    5,    // 1/8T  (1/8 triplet = 1/12, relative: 64/12 ≈ 5.33, use 5)
    11,   // 1/4T  (1/4 triplet = 1/6, relative: 64/6 ≈ 10.67, use 11)
    6,    // 1/16. (1/16 dotted = 3/32, relative: 64*3/32 = 6)
    12,   // 1/8.  (1/8 dotted = 3/16, relative: 64*3/16 = 12)
    24,   // 1/4.  (1/4 dotted = 3/8, relative: 64*3/8 = 24)
};

// Delayed event structure
typedef struct {
    uint8_t active;
    uint8_t note;
    uint8_t velocity;
    uint8_t channel;
    uint32_t trigger_time_ms;
    uint8_t repeat_count;
} delayed_event_t;

// Per-track delay configuration
typedef struct {
    uint8_t enabled;
    midi_delay_division_t division;
    uint8_t feedback;         // 0-100%
    uint8_t mix;              // 0-100%
    uint8_t velocity_decay;   // 0-100%
    delayed_event_t events[MIDI_DELAY_MAX_EVENTS];
} delay_config_t;

static delay_config_t g_delay[MIDI_DELAY_MAX_TRACKS];
static uint16_t g_tempo = 120;  // BPM
static uint32_t g_tick_counter = 0;
static midi_delay_output_cb_t g_output_callback = NULL;

/**
 * @brief Calculate delay time in milliseconds for a division
 */
static uint32_t calculate_delay_ms(midi_delay_division_t division) {
    if (division >= DELAY_DIV_COUNT) return 500;
    
    // Calculate time for one 1/64th note in ms
    // At 120 BPM: 1 beat = 500ms, 1/4 note = 500ms, 1/64 note = 500/16 = 31.25ms
    // Formula: (60000 / BPM) / 16 = ms per 1/64th note
    uint32_t ms_per_64th = (60000 / g_tempo) / 16;
    
    // Multiply by division multiplier
    return ms_per_64th * division_multipliers[division];
}

/**
 * @brief Initialize MIDI delay module
 */
void midi_delay_init(uint16_t tempo) {
    memset(g_delay, 0, sizeof(g_delay));
    g_tempo = tempo;
    g_tick_counter = 0;
    
    // Initialize defaults
    for (uint8_t t = 0; t < MIDI_DELAY_MAX_TRACKS; t++) {
        g_delay[t].enabled = 0;
        g_delay[t].division = DELAY_DIV_1_8;  // 1/8 note default
        g_delay[t].feedback = 50;              // 50% feedback
        g_delay[t].mix = 50;                   // 50/50 mix
        g_delay[t].velocity_decay = 20;        // 20% decay per repeat
    }
}

/**
 * @brief Update tempo
 */
void midi_delay_set_tempo(uint16_t tempo) {
    if (tempo < 20) tempo = 20;
    if (tempo > 300) tempo = 300;
    g_tempo = tempo;
}

/**
 * @brief Called every 1ms to process delayed events
 */
void midi_delay_tick_1ms(void) {
    g_tick_counter++;
    
    if (!g_output_callback) return;
    
    for (uint8_t t = 0; t < MIDI_DELAY_MAX_TRACKS; t++) {
        delay_config_t* cfg = &g_delay[t];
        if (!cfg->enabled) continue;
        
        uint32_t delay_time = calculate_delay_ms(cfg->division);
        
        for (uint8_t i = 0; i < MIDI_DELAY_MAX_EVENTS; i++) {
            delayed_event_t* evt = &cfg->events[i];
            if (!evt->active) continue;
            
            // Check if it's time to trigger this event
            uint32_t elapsed = g_tick_counter - evt->trigger_time_ms;
            if (elapsed >= delay_time) {
                // Send note on
                g_output_callback(t, evt->note, evt->velocity, evt->channel, 1);
                
                // Calculate next repeat with feedback
                uint8_t max_repeats = (cfg->feedback * 10) / 100;  // Feedback % -> repeats
                evt->repeat_count++;
                
                if (evt->repeat_count < max_repeats) {
                    // Update for next repeat
                    evt->trigger_time_ms = g_tick_counter;
                    
                    // Apply velocity decay
                    uint16_t new_velocity = evt->velocity;
                    new_velocity = (new_velocity * (100 - cfg->velocity_decay)) / 100;
                    if (new_velocity < 1) new_velocity = 1;
                    evt->velocity = (uint8_t)new_velocity;
                } else {
                    // No more repeats
                    evt->active = 0;
                }
            }
        }
    }
}

/**
 * @brief Enable/disable delay for a track
 */
void midi_delay_set_enabled(uint8_t track, uint8_t enabled) {
    if (track >= MIDI_DELAY_MAX_TRACKS) return;
    g_delay[track].enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if delay is enabled for a track
 */
uint8_t midi_delay_is_enabled(uint8_t track) {
    if (track >= MIDI_DELAY_MAX_TRACKS) return 0;
    return g_delay[track].enabled;
}

/**
 * @brief Set delay time division
 */
void midi_delay_set_division(uint8_t track, midi_delay_division_t division) {
    if (track >= MIDI_DELAY_MAX_TRACKS) return;
    if (division >= DELAY_DIV_COUNT) return;
    g_delay[track].division = division;
}

/**
 * @brief Get delay time division
 */
midi_delay_division_t midi_delay_get_division(uint8_t track) {
    if (track >= MIDI_DELAY_MAX_TRACKS) return DELAY_DIV_1_8;
    return g_delay[track].division;
}

/**
 * @brief Set feedback amount
 */
void midi_delay_set_feedback(uint8_t track, uint8_t feedback) {
    if (track >= MIDI_DELAY_MAX_TRACKS) return;
    if (feedback > 100) feedback = 100;
    g_delay[track].feedback = feedback;
}

/**
 * @brief Get feedback amount
 */
uint8_t midi_delay_get_feedback(uint8_t track) {
    if (track >= MIDI_DELAY_MAX_TRACKS) return 50;
    return g_delay[track].feedback;
}

/**
 * @brief Set mix
 */
void midi_delay_set_mix(uint8_t track, uint8_t mix) {
    if (track >= MIDI_DELAY_MAX_TRACKS) return;
    if (mix > 100) mix = 100;
    g_delay[track].mix = mix;
}

/**
 * @brief Get mix
 */
uint8_t midi_delay_get_mix(uint8_t track) {
    if (track >= MIDI_DELAY_MAX_TRACKS) return 50;
    return g_delay[track].mix;
}

/**
 * @brief Set velocity decay per repeat
 */
void midi_delay_set_velocity_decay(uint8_t track, uint8_t decay) {
    if (track >= MIDI_DELAY_MAX_TRACKS) return;
    if (decay > 100) decay = 100;
    g_delay[track].velocity_decay = decay;
}

/**
 * @brief Get velocity decay
 */
uint8_t midi_delay_get_velocity_decay(uint8_t track) {
    if (track >= MIDI_DELAY_MAX_TRACKS) return 20;
    return g_delay[track].velocity_decay;
}

/**
 * @brief Process input MIDI note
 */
void midi_delay_process_note(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel) {
    if (track >= MIDI_DELAY_MAX_TRACKS) return;
    
    delay_config_t* cfg = &g_delay[track];
    if (!cfg->enabled) return;
    
    // Find free event slot
    for (uint8_t i = 0; i < MIDI_DELAY_MAX_EVENTS; i++) {
        if (!cfg->events[i].active) {
            cfg->events[i].active = 1;
            cfg->events[i].note = note;
            cfg->events[i].velocity = velocity;
            cfg->events[i].channel = channel;
            cfg->events[i].trigger_time_ms = g_tick_counter;
            cfg->events[i].repeat_count = 0;
            break;
        }
    }
}

/**
 * @brief Clear all delayed events for a track
 */
void midi_delay_clear(uint8_t track) {
    if (track >= MIDI_DELAY_MAX_TRACKS) return;
    memset(g_delay[track].events, 0, sizeof(g_delay[track].events));
}

/**
 * @brief Clear all delayed events for all tracks
 */
void midi_delay_clear_all(void) {
    for (uint8_t t = 0; t < MIDI_DELAY_MAX_TRACKS; t++) {
        midi_delay_clear(t);
    }
}

/**
 * @brief Get division name
 */
const char* midi_delay_get_division_name(midi_delay_division_t division) {
    if (division >= DELAY_DIV_COUNT) return "Unknown";
    return division_names[division];
}

/**
 * @brief Set output callback
 */
void midi_delay_set_output_callback(midi_delay_output_cb_t callback) {
    g_output_callback = callback;
}

#endif // MODULE_ENABLE_MIDI_DELAY_FX
