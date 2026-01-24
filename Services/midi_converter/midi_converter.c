/**
 * @file midi_converter.c
 * @brief MIDI Message Converters implementation
 */

#include "Services/midi_converter/midi_converter.h"
#include <string.h>

#define DEFAULT_SOURCE_CC 1    // Modwheel
#define DEFAULT_DEST_CC 74     // Filter cutoff
#define DEFAULT_SCALE 100
#define DEFAULT_OFFSET 0

// Mode name strings
static const char* mode_names[] = {
    "AT→CC",
    "CC→AT",
    "PB→CC",
    "CC→PB",
    "Vel→CC",
    "MW→CC",
    "CC→CC",
    "Disabled"
};

// Per-track converter configuration
typedef struct {
    uint8_t enabled;
    midi_converter_mode_t mode;
    uint8_t source_cc;              // Source CC number (for CC-based modes)
    uint8_t dest_cc;                // Destination CC number (for CC output modes)
    uint8_t scale;                  // Scale factor (0-200, 100=no scaling)
    int8_t offset;                  // Offset value (-64 to +63)
    uint8_t invert;                 // Invert flag
    uint8_t last_output_value;      // Last output value (for change detection)
} midi_converter_config_t;

static midi_converter_config_t g_converter_config[MIDI_CONVERTER_MAX_TRACKS];
static midi_converter_cc_callback_t g_cc_callback = NULL;
static midi_converter_aftertouch_callback_t g_aftertouch_callback = NULL;
static midi_converter_pitchbend_callback_t g_pitchbend_callback = NULL;

/**
 * @brief Apply transformations to value (scale, offset, invert)
 */
static uint8_t transform_value(uint8_t track, uint8_t input_value) {
    midi_converter_config_t* cfg = &g_converter_config[track];
    int16_t value = input_value;
    
    // Apply inversion first
    if (cfg->invert) {
        value = 127 - value;
    }
    
    // Apply scaling
    value = (value * cfg->scale) / 100;
    
    // Apply offset
    value += cfg->offset;
    
    // Clamp to valid MIDI range
    if (value < 0) value = 0;
    if (value > 127) value = 127;
    
    return (uint8_t)value;
}

/**
 * @brief Convert 14-bit pitchbend to 7-bit CC value
 */
static uint8_t pitchbend_to_cc(uint16_t pitchbend) {
    // Pitchbend: 0-16383, center=8192
    // CC: 0-127, center=64
    return (uint8_t)((pitchbend >> 7) & 0x7F);
}

/**
 * @brief Convert 7-bit CC value to 14-bit pitchbend
 */
static uint16_t cc_to_pitchbend(uint8_t cc_value) {
    // CC: 0-127 → Pitchbend: 0-16383
    return (uint16_t)(cc_value << 7);
}

/**
 * @brief Initialize MIDI converter module
 */
void midi_converter_init(void) {
    memset(g_converter_config, 0, sizeof(g_converter_config));
    
    for (uint8_t i = 0; i < MIDI_CONVERTER_MAX_TRACKS; i++) {
        g_converter_config[i].enabled = 0;
        g_converter_config[i].mode = MIDI_CONVERTER_DISABLED;
        g_converter_config[i].source_cc = DEFAULT_SOURCE_CC;
        g_converter_config[i].dest_cc = DEFAULT_DEST_CC;
        g_converter_config[i].scale = DEFAULT_SCALE;
        g_converter_config[i].offset = DEFAULT_OFFSET;
        g_converter_config[i].invert = 0;
        g_converter_config[i].last_output_value = 0;
    }
    
    g_cc_callback = NULL;
    g_aftertouch_callback = NULL;
    g_pitchbend_callback = NULL;
}

/**
 * @brief Set CC output callback
 */
void midi_converter_set_cc_callback(midi_converter_cc_callback_t callback) {
    g_cc_callback = callback;
}

/**
 * @brief Set aftertouch output callback
 */
void midi_converter_set_aftertouch_callback(midi_converter_aftertouch_callback_t callback) {
    g_aftertouch_callback = callback;
}

/**
 * @brief Set pitchbend output callback
 */
void midi_converter_set_pitchbend_callback(midi_converter_pitchbend_callback_t callback) {
    g_pitchbend_callback = callback;
}

/**
 * @brief Enable/disable converter
 */
void midi_converter_set_enabled(uint8_t track, uint8_t enabled) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return;
    g_converter_config[track].enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if converter is enabled
 */
uint8_t midi_converter_is_enabled(uint8_t track) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return 0;
    return g_converter_config[track].enabled;
}

/**
 * @brief Set conversion mode
 */
void midi_converter_set_mode(uint8_t track, midi_converter_mode_t mode) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS || mode >= MIDI_CONVERTER_MODE_COUNT) return;
    g_converter_config[track].mode = mode;
}

/**
 * @brief Get conversion mode
 */
midi_converter_mode_t midi_converter_get_mode(uint8_t track) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return MIDI_CONVERTER_DISABLED;
    return g_converter_config[track].mode;
}

/**
 * @brief Set source CC number
 */
void midi_converter_set_source_cc(uint8_t track, uint8_t cc_number) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return;
    if (cc_number > 127) cc_number = 127;
    g_converter_config[track].source_cc = cc_number;
}

/**
 * @brief Get source CC number
 */
uint8_t midi_converter_get_source_cc(uint8_t track) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return 0;
    return g_converter_config[track].source_cc;
}

/**
 * @brief Set destination CC number
 */
void midi_converter_set_dest_cc(uint8_t track, uint8_t cc_number) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return;
    if (cc_number > 127) cc_number = 127;
    g_converter_config[track].dest_cc = cc_number;
}

/**
 * @brief Get destination CC number
 */
uint8_t midi_converter_get_dest_cc(uint8_t track) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return 0;
    return g_converter_config[track].dest_cc;
}

/**
 * @brief Set scale factor
 */
void midi_converter_set_scale(uint8_t track, uint8_t scale) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return;
    if (scale > 200) scale = 200;
    g_converter_config[track].scale = scale;
}

/**
 * @brief Get scale factor
 */
uint8_t midi_converter_get_scale(uint8_t track) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return DEFAULT_SCALE;
    return g_converter_config[track].scale;
}

/**
 * @brief Set offset value
 */
void midi_converter_set_offset(uint8_t track, int8_t offset) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return;
    if (offset < -64) offset = -64;
    if (offset > 63) offset = 63;
    g_converter_config[track].offset = offset;
}

/**
 * @brief Get offset value
 */
int8_t midi_converter_get_offset(uint8_t track) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return DEFAULT_OFFSET;
    return g_converter_config[track].offset;
}

/**
 * @brief Set invert flag
 */
void midi_converter_set_invert(uint8_t track, uint8_t invert) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return;
    g_converter_config[track].invert = invert ? 1 : 0;
}

/**
 * @brief Get invert flag
 */
uint8_t midi_converter_get_invert(uint8_t track) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return 0;
    return g_converter_config[track].invert;
}

/**
 * @brief Process CC message
 */
void midi_converter_process_cc(uint8_t track, uint8_t cc_number, uint8_t cc_value, uint8_t channel) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return;
    if (!g_converter_config[track].enabled) return;
    
    midi_converter_config_t* cfg = &g_converter_config[track];
    
    // Check if this CC matches our source
    if (cfg->mode == MIDI_CONVERTER_CC_TO_AFTERTOUCH ||
        cfg->mode == MIDI_CONVERTER_CC_TO_PITCHBEND ||
        cfg->mode == MIDI_CONVERTER_CC_TO_CC ||
        cfg->mode == MIDI_CONVERTER_MODWHEEL_TO_CC) {
        
        if (cc_number != cfg->source_cc) return;
    }
    
    uint8_t output_value = transform_value(track, cc_value);
    
    switch (cfg->mode) {
        case MIDI_CONVERTER_CC_TO_AFTERTOUCH:
            if (g_aftertouch_callback) {
                g_aftertouch_callback(track, output_value, channel);
            }
            break;
            
        case MIDI_CONVERTER_CC_TO_PITCHBEND:
            if (g_pitchbend_callback) {
                uint16_t pb_value = cc_to_pitchbend(output_value);
                g_pitchbend_callback(track, pb_value, channel);
            }
            break;
            
        case MIDI_CONVERTER_MODWHEEL_TO_CC:
        case MIDI_CONVERTER_CC_TO_CC:
            if (g_cc_callback) {
                g_cc_callback(track, cfg->dest_cc, output_value, channel);
            }
            break;
            
        default:
            break;
    }
}

/**
 * @brief Process aftertouch message
 */
void midi_converter_process_aftertouch(uint8_t track, uint8_t pressure, uint8_t channel) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return;
    if (!g_converter_config[track].enabled) return;
    
    midi_converter_config_t* cfg = &g_converter_config[track];
    
    if (cfg->mode == MIDI_CONVERTER_AFTERTOUCH_TO_CC) {
        uint8_t output_value = transform_value(track, pressure);
        
        if (g_cc_callback) {
            g_cc_callback(track, cfg->dest_cc, output_value, channel);
        }
    }
}

/**
 * @brief Process pitchbend message
 */
void midi_converter_process_pitchbend(uint8_t track, uint16_t value, uint8_t channel) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return;
    if (!g_converter_config[track].enabled) return;
    
    midi_converter_config_t* cfg = &g_converter_config[track];
    
    if (cfg->mode == MIDI_CONVERTER_PITCHBEND_TO_CC) {
        // Convert 14-bit pitchbend to 7-bit CC
        uint8_t cc_value = pitchbend_to_cc(value);
        uint8_t output_value = transform_value(track, cc_value);
        
        if (g_cc_callback) {
            g_cc_callback(track, cfg->dest_cc, output_value, channel);
        }
    }
}

/**
 * @brief Process note velocity
 */
void midi_converter_process_velocity(uint8_t track, uint8_t velocity, uint8_t channel) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return;
    if (!g_converter_config[track].enabled) return;
    
    midi_converter_config_t* cfg = &g_converter_config[track];
    
    if (cfg->mode == MIDI_CONVERTER_VELOCITY_TO_CC) {
        uint8_t output_value = transform_value(track, velocity);
        
        if (g_cc_callback) {
            g_cc_callback(track, cfg->dest_cc, output_value, channel);
        }
    }
}

/**
 * @brief Reset converter state for a track
 */
void midi_converter_reset(uint8_t track) {
    if (track >= MIDI_CONVERTER_MAX_TRACKS) return;
    g_converter_config[track].last_output_value = 0;
}

/**
 * @brief Reset converter state for all tracks
 */
void midi_converter_reset_all(void) {
    for (uint8_t i = 0; i < MIDI_CONVERTER_MAX_TRACKS; i++) {
        midi_converter_reset(i);
    }
}

/**
 * @brief Get mode name string
 */
const char* midi_converter_get_mode_name(midi_converter_mode_t mode) {
    if (mode >= MIDI_CONVERTER_MODE_COUNT) return "Unknown";
    return mode_names[mode];
}
