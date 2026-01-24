/**
 * @file channelizer.c
 * @brief MIDI Channelizer implementation
 */

#include "Services/channelizer/channelizer.h"
#include <string.h>

// MIDI status byte masks and constants
#define MIDI_STATUS_MASK           0xF0
#define MIDI_CHANNEL_MASK          0x0F
#define MIDI_STATUS_NOTE_OFF       0x80
#define MIDI_STATUS_NOTE_ON        0x90
#define MIDI_STATUS_POLY_AT        0xA0
#define MIDI_STATUS_CC             0xB0
#define MIDI_STATUS_PROGRAM        0xC0
#define MIDI_STATUS_CHAN_AT        0xD0
#define MIDI_STATUS_PITCH_BEND     0xE0

// Mode names
static const char* mode_names[] = {
    "Bypass",
    "Force",
    "Remap",
    "Rotate",
    "Zone"
};

// Voice stealing algorithm names
static const char* voice_steal_names[] = {
    "Oldest",
    "Lowest",
    "Highest",
    "Quietest"
};

// Global configuration for all tracks
static channelizer_config_t g_channelizer_config[CHANNELIZER_MAX_TRACKS];

/**
 * @brief Helper to check if a message is a channel message
 */
static inline uint8_t is_channel_message(uint8_t status) {
    uint8_t status_type = status & MIDI_STATUS_MASK;
    return (status_type >= MIDI_STATUS_NOTE_OFF && status_type <= MIDI_STATUS_PITCH_BEND);
}

/**
 * @brief Helper to get channel from status byte
 */
static inline uint8_t get_channel(uint8_t status) {
    return status & MIDI_CHANNEL_MASK;
}

/**
 * @brief Helper to set channel in status byte
 */
static inline uint8_t set_channel(uint8_t status, uint8_t channel) {
    return (status & MIDI_STATUS_MASK) | (channel & MIDI_CHANNEL_MASK);
}

/**
 * @brief Clamp note value to valid MIDI range
 */
static inline uint8_t clamp_note(int16_t note) {
    if (note < 0) return 0;
    if (note > 127) return 127;
    return (uint8_t)note;
}

/**
 * @brief Find a voice by note and channel
 */
static int8_t find_voice(channelizer_config_t* cfg, uint8_t note, uint8_t channel) {
    for (uint8_t i = 0; i < cfg->voice_limit && i < CHANNELIZER_MAX_VOICES; i++) {
        if (cfg->voices[i].active && 
            cfg->voices[i].note == note && 
            cfg->voices[i].channel == channel) {
            return (int8_t)i;
        }
    }
    return -1;
}

/**
 * @brief Find a free voice slot
 */
static int8_t find_free_voice(channelizer_config_t* cfg) {
    for (uint8_t i = 0; i < cfg->voice_limit && i < CHANNELIZER_MAX_VOICES; i++) {
        if (!cfg->voices[i].active) {
            return (int8_t)i;
        }
    }
    return -1;
}

/**
 * @brief Steal a voice based on configured algorithm
 */
static uint8_t steal_voice(channelizer_config_t* cfg) {
    uint8_t victim = 0;
    uint32_t oldest_time = cfg->voices[0].timestamp;
    uint8_t lowest_note = cfg->voices[0].note;
    uint8_t highest_note = cfg->voices[0].note;
    uint8_t quietest_vel = cfg->voices[0].velocity;
    
    for (uint8_t i = 1; i < cfg->voice_limit && i < CHANNELIZER_MAX_VOICES; i++) {
        if (!cfg->voices[i].active) continue;
        
        switch (cfg->voice_steal_mode) {
            case CHANNELIZER_VOICE_STEAL_OLDEST:
                if (cfg->voices[i].timestamp < oldest_time) {
                    oldest_time = cfg->voices[i].timestamp;
                    victim = i;
                }
                break;
                
            case CHANNELIZER_VOICE_STEAL_LOWEST:
                if (cfg->voices[i].note < lowest_note) {
                    lowest_note = cfg->voices[i].note;
                    victim = i;
                }
                break;
                
            case CHANNELIZER_VOICE_STEAL_HIGHEST:
                if (cfg->voices[i].note > highest_note) {
                    highest_note = cfg->voices[i].note;
                    victim = i;
                }
                break;
                
            case CHANNELIZER_VOICE_STEAL_QUIETEST:
                if (cfg->voices[i].velocity < quietest_vel) {
                    quietest_vel = cfg->voices[i].velocity;
                    victim = i;
                }
                break;
        }
    }
    
    return victim;
}

/**
 * @brief Find matching zone for a note
 */
static int8_t find_zone_for_note(channelizer_config_t* cfg, uint8_t note) {
    for (uint8_t i = 0; i < CHANNELIZER_MAX_ZONES; i++) {
        if (cfg->zones[i].enabled && 
            note >= cfg->zones[i].note_min && 
            note <= cfg->zones[i].note_max) {
            return (int8_t)i;
        }
    }
    return -1;
}

// ==================== Initialization ====================

void channelizer_init(void) {
    memset(g_channelizer_config, 0, sizeof(g_channelizer_config));
    
    for (uint8_t i = 0; i < CHANNELIZER_MAX_TRACKS; i++) {
        channelizer_reset(i);
    }
}

// ==================== Enable/Disable ====================

void channelizer_set_enabled(uint8_t track, uint8_t enabled) {
    if (track >= CHANNELIZER_MAX_TRACKS) return;
    g_channelizer_config[track].enabled = enabled ? 1 : 0;
}

uint8_t channelizer_is_enabled(uint8_t track) {
    if (track >= CHANNELIZER_MAX_TRACKS) return 0;
    return g_channelizer_config[track].enabled;
}

// ==================== Mode Configuration ====================

void channelizer_set_mode(uint8_t track, channelizer_mode_t mode) {
    if (track >= CHANNELIZER_MAX_TRACKS) return;
    g_channelizer_config[track].mode = mode;
}

channelizer_mode_t channelizer_get_mode(uint8_t track) {
    if (track >= CHANNELIZER_MAX_TRACKS) return CHANNELIZER_MODE_BYPASS;
    return g_channelizer_config[track].mode;
}

// ==================== Input Channel Filtering ====================

void channelizer_set_input_channel_mask(uint8_t track, uint16_t mask) {
    if (track >= CHANNELIZER_MAX_TRACKS) return;
    g_channelizer_config[track].input_channel_mask = mask;
}

uint16_t channelizer_get_input_channel_mask(uint8_t track) {
    if (track >= CHANNELIZER_MAX_TRACKS) return 0xFFFF;
    return g_channelizer_config[track].input_channel_mask;
}

void channelizer_set_input_channel_enabled(uint8_t track, uint8_t channel, uint8_t enabled) {
    if (track >= CHANNELIZER_MAX_TRACKS || channel >= CHANNELIZER_MAX_CHANNELS) return;
    
    if (enabled) {
        g_channelizer_config[track].input_channel_mask |= (1 << channel);
    } else {
        g_channelizer_config[track].input_channel_mask &= ~(1 << channel);
    }
}

uint8_t channelizer_is_input_channel_enabled(uint8_t track, uint8_t channel) {
    if (track >= CHANNELIZER_MAX_TRACKS || channel >= CHANNELIZER_MAX_CHANNELS) return 0;
    return (g_channelizer_config[track].input_channel_mask & (1 << channel)) ? 1 : 0;
}

// ==================== Force Mode ====================

void channelizer_set_force_channel(uint8_t track, uint8_t channel) {
    if (track >= CHANNELIZER_MAX_TRACKS || channel >= CHANNELIZER_MAX_CHANNELS) return;
    g_channelizer_config[track].force_channel = channel;
}

uint8_t channelizer_get_force_channel(uint8_t track) {
    if (track >= CHANNELIZER_MAX_TRACKS) return 0;
    return g_channelizer_config[track].force_channel;
}

// ==================== Remap Mode ====================

void channelizer_set_channel_remap(uint8_t track, uint8_t input_channel, uint8_t output_channel) {
    if (track >= CHANNELIZER_MAX_TRACKS || 
        input_channel >= CHANNELIZER_MAX_CHANNELS || 
        output_channel >= CHANNELIZER_MAX_CHANNELS) return;
    
    g_channelizer_config[track].channel_map[input_channel] = output_channel;
}

uint8_t channelizer_get_channel_remap(uint8_t track, uint8_t input_channel) {
    if (track >= CHANNELIZER_MAX_TRACKS || input_channel >= CHANNELIZER_MAX_CHANNELS) return 0;
    return g_channelizer_config[track].channel_map[input_channel];
}

void channelizer_set_channel_map(uint8_t track, const uint8_t* map) {
    if (track >= CHANNELIZER_MAX_TRACKS || !map) return;
    memcpy(g_channelizer_config[track].channel_map, map, CHANNELIZER_MAX_CHANNELS);
}

void channelizer_get_channel_map(uint8_t track, uint8_t* map) {
    if (track >= CHANNELIZER_MAX_TRACKS || !map) return;
    memcpy(map, g_channelizer_config[track].channel_map, CHANNELIZER_MAX_CHANNELS);
}

// ==================== Rotate Mode ====================

void channelizer_set_rotate_channels(uint8_t track, const uint8_t* channels, uint8_t count) {
    if (track >= CHANNELIZER_MAX_TRACKS || !channels || count == 0 || count > CHANNELIZER_MAX_CHANNELS) return;
    
    memcpy(g_channelizer_config[track].rotate_channels, channels, count);
    g_channelizer_config[track].rotate_count = count;
    g_channelizer_config[track].rotate_index = 0;
}

uint8_t channelizer_get_rotate_channels(uint8_t track, uint8_t* channels) {
    if (track >= CHANNELIZER_MAX_TRACKS || !channels) return 0;
    
    memcpy(channels, g_channelizer_config[track].rotate_channels, 
           g_channelizer_config[track].rotate_count);
    return g_channelizer_config[track].rotate_count;
}

void channelizer_reset_rotation(uint8_t track) {
    if (track >= CHANNELIZER_MAX_TRACKS) return;
    g_channelizer_config[track].rotate_index = 0;
}

// ==================== Zone Mode ====================

void channelizer_set_zone(uint8_t track, uint8_t zone_index, const channelizer_zone_t* zone) {
    if (track >= CHANNELIZER_MAX_TRACKS || zone_index >= CHANNELIZER_MAX_ZONES || !zone) return;
    memcpy(&g_channelizer_config[track].zones[zone_index], zone, sizeof(channelizer_zone_t));
}

void channelizer_get_zone(uint8_t track, uint8_t zone_index, channelizer_zone_t* zone) {
    if (track >= CHANNELIZER_MAX_TRACKS || zone_index >= CHANNELIZER_MAX_ZONES || !zone) return;
    memcpy(zone, &g_channelizer_config[track].zones[zone_index], sizeof(channelizer_zone_t));
}

void channelizer_set_zone_enabled(uint8_t track, uint8_t zone_index, uint8_t enabled) {
    if (track >= CHANNELIZER_MAX_TRACKS || zone_index >= CHANNELIZER_MAX_ZONES) return;
    g_channelizer_config[track].zones[zone_index].enabled = enabled ? 1 : 0;
}

uint8_t channelizer_is_zone_enabled(uint8_t track, uint8_t zone_index) {
    if (track >= CHANNELIZER_MAX_TRACKS || zone_index >= CHANNELIZER_MAX_ZONES) return 0;
    return g_channelizer_config[track].zones[zone_index].enabled;
}

void channelizer_set_zone_range(uint8_t track, uint8_t zone_index, uint8_t note_min, uint8_t note_max) {
    if (track >= CHANNELIZER_MAX_TRACKS || zone_index >= CHANNELIZER_MAX_ZONES) return;
    g_channelizer_config[track].zones[zone_index].note_min = note_min;
    g_channelizer_config[track].zones[zone_index].note_max = note_max;
}

void channelizer_set_zone_channel(uint8_t track, uint8_t zone_index, uint8_t channel) {
    if (track >= CHANNELIZER_MAX_TRACKS || zone_index >= CHANNELIZER_MAX_ZONES || 
        channel >= CHANNELIZER_MAX_CHANNELS) return;
    g_channelizer_config[track].zones[zone_index].output_channel = channel;
}

void channelizer_set_zone_transpose(uint8_t track, uint8_t zone_index, int8_t transpose) {
    if (track >= CHANNELIZER_MAX_TRACKS || zone_index >= CHANNELIZER_MAX_ZONES) return;
    g_channelizer_config[track].zones[zone_index].transpose = transpose;
}

// ==================== Voice Management ====================

void channelizer_set_voice_steal_mode(uint8_t track, channelizer_voice_steal_t mode) {
    if (track >= CHANNELIZER_MAX_TRACKS) return;
    g_channelizer_config[track].voice_steal_mode = mode;
}

channelizer_voice_steal_t channelizer_get_voice_steal_mode(uint8_t track) {
    if (track >= CHANNELIZER_MAX_TRACKS) return CHANNELIZER_VOICE_STEAL_OLDEST;
    return g_channelizer_config[track].voice_steal_mode;
}

void channelizer_set_voice_limit(uint8_t track, uint8_t limit) {
    if (track >= CHANNELIZER_MAX_TRACKS) return;
    if (limit < 1) limit = 1;
    if (limit > CHANNELIZER_MAX_VOICES) limit = CHANNELIZER_MAX_VOICES;
    g_channelizer_config[track].voice_limit = limit;
}

uint8_t channelizer_get_voice_limit(uint8_t track) {
    if (track >= CHANNELIZER_MAX_TRACKS) return CHANNELIZER_MAX_VOICES;
    return g_channelizer_config[track].voice_limit;
}

uint8_t channelizer_get_active_voice_count(uint8_t track) {
    if (track >= CHANNELIZER_MAX_TRACKS) return 0;
    
    uint8_t count = 0;
    channelizer_config_t* cfg = &g_channelizer_config[track];
    
    for (uint8_t i = 0; i < cfg->voice_limit && i < CHANNELIZER_MAX_VOICES; i++) {
        if (cfg->voices[i].active) {
            count++;
        }
    }
    
    return count;
}

uint8_t channelizer_release_all_voices(uint8_t track, channelizer_output_t* outputs, uint8_t max_outputs) {
    if (track >= CHANNELIZER_MAX_TRACKS || !outputs || max_outputs == 0) return 0;
    
    channelizer_config_t* cfg = &g_channelizer_config[track];
    uint8_t output_count = 0;
    
    for (uint8_t i = 0; i < cfg->voice_limit && i < CHANNELIZER_MAX_VOICES && output_count < max_outputs; i++) {
        if (cfg->voices[i].active) {
            outputs[output_count].status = MIDI_STATUS_NOTE_OFF | cfg->voices[i].channel;
            outputs[output_count].data1 = cfg->voices[i].note;
            outputs[output_count].data2 = 0;
            output_count++;
            
            cfg->voices[i].active = 0;
        }
    }
    
    return output_count;
}

// ==================== Message Processing ====================

uint8_t channelizer_process_note_on(uint8_t track, uint8_t channel, uint8_t note, uint8_t velocity,
                                     channelizer_output_t* outputs, uint8_t max_outputs) {
    if (track >= CHANNELIZER_MAX_TRACKS || !outputs || max_outputs == 0) return 0;
    
    channelizer_config_t* cfg = &g_channelizer_config[track];
    uint8_t output_count = 0;
    
    // Velocity 0 is note off
    if (velocity == 0) {
        return channelizer_process_note_off(track, channel, note, 0, outputs, max_outputs);
    }
    
    // Process based on mode
    switch (cfg->mode) {
        case CHANNELIZER_MODE_BYPASS:
            outputs[0].status = MIDI_STATUS_NOTE_ON | channel;
            outputs[0].data1 = note;
            outputs[0].data2 = velocity;
            return 1;
            
        case CHANNELIZER_MODE_FORCE:
            outputs[0].status = MIDI_STATUS_NOTE_ON | cfg->force_channel;
            outputs[0].data1 = note;
            outputs[0].data2 = velocity;
            return 1;
            
        case CHANNELIZER_MODE_REMAP: {
            uint8_t out_ch = cfg->channel_map[channel];
            outputs[0].status = MIDI_STATUS_NOTE_ON | out_ch;
            outputs[0].data1 = note;
            outputs[0].data2 = velocity;
            return 1;
        }
        
        case CHANNELIZER_MODE_ROTATE: {
            if (cfg->rotate_count == 0) return 0;
            
            uint8_t out_ch = cfg->rotate_channels[cfg->rotate_index];
            cfg->rotate_index = (cfg->rotate_index + 1) % cfg->rotate_count;
            
            // Allocate voice
            int8_t voice_idx = find_free_voice(cfg);
            if (voice_idx < 0) {
                voice_idx = steal_voice(cfg);
                // Generate note off for stolen voice first
                if (output_count < max_outputs) {
                    outputs[output_count].status = MIDI_STATUS_NOTE_OFF | cfg->voices[voice_idx].channel;
                    outputs[output_count].data1 = cfg->voices[voice_idx].note;
                    outputs[output_count].data2 = 0;
                    output_count++;
                }
            }
            
            // Generate note on for new note
            if (output_count < max_outputs) {
                outputs[output_count].status = MIDI_STATUS_NOTE_ON | out_ch;
                outputs[output_count].data1 = note;
                outputs[output_count].data2 = velocity;
                output_count++;
            }
            
            cfg->voices[voice_idx].active = 1;
            cfg->voices[voice_idx].note = note;
            cfg->voices[voice_idx].velocity = velocity;
            cfg->voices[voice_idx].channel = out_ch;
            cfg->voices[voice_idx].timestamp = cfg->voice_timestamp++;
            
            return output_count;
        }
        
        case CHANNELIZER_MODE_ZONE: {
            int8_t zone_idx = find_zone_for_note(cfg, note);
            if (zone_idx < 0) return 0;
            
            channelizer_zone_t* zone = &cfg->zones[zone_idx];
            int16_t transposed = (int16_t)note + zone->transpose;
            uint8_t out_note = clamp_note(transposed);
            
            // Allocate voice
            int8_t voice_idx = find_free_voice(cfg);
            if (voice_idx < 0) {
                voice_idx = steal_voice(cfg);
                // Generate note off for stolen voice first
                if (output_count < max_outputs) {
                    outputs[output_count].status = MIDI_STATUS_NOTE_OFF | cfg->voices[voice_idx].channel;
                    outputs[output_count].data1 = cfg->voices[voice_idx].note;
                    outputs[output_count].data2 = 0;
                    output_count++;
                }
            }
            
            // Generate note on for new note
            if (output_count < max_outputs) {
                outputs[output_count].status = MIDI_STATUS_NOTE_ON | zone->output_channel;
                outputs[output_count].data1 = out_note;
                outputs[output_count].data2 = velocity;
                output_count++;
            }
            
            cfg->voices[voice_idx].active = 1;
            cfg->voices[voice_idx].note = out_note;
            cfg->voices[voice_idx].velocity = velocity;
            cfg->voices[voice_idx].channel = zone->output_channel;
            cfg->voices[voice_idx].timestamp = cfg->voice_timestamp++;
            
            return output_count;
        }
    }
    
    return 0;
}

uint8_t channelizer_process_note_off(uint8_t track, uint8_t channel, uint8_t note, uint8_t velocity,
                                      channelizer_output_t* outputs, uint8_t max_outputs) {
    if (track >= CHANNELIZER_MAX_TRACKS || !outputs || max_outputs == 0) return 0;
    
    channelizer_config_t* cfg = &g_channelizer_config[track];
    
    // Process based on mode
    switch (cfg->mode) {
        case CHANNELIZER_MODE_BYPASS:
            outputs[0].status = MIDI_STATUS_NOTE_OFF | channel;
            outputs[0].data1 = note;
            outputs[0].data2 = velocity;
            return 1;
            
        case CHANNELIZER_MODE_FORCE:
            outputs[0].status = MIDI_STATUS_NOTE_OFF | cfg->force_channel;
            outputs[0].data1 = note;
            outputs[0].data2 = velocity;
            return 1;
            
        case CHANNELIZER_MODE_REMAP: {
            uint8_t out_ch = cfg->channel_map[channel];
            outputs[0].status = MIDI_STATUS_NOTE_OFF | out_ch;
            outputs[0].data1 = note;
            outputs[0].data2 = velocity;
            return 1;
        }
        
        case CHANNELIZER_MODE_ROTATE:
        case CHANNELIZER_MODE_ZONE: {
            // Find and release voice
            int8_t voice_idx = find_voice(cfg, note, channel);
            if (voice_idx < 0) {
                // Voice not found in table, try to find by note in any zone's transposed range
                if (cfg->mode == CHANNELIZER_MODE_ZONE) {
                    int8_t zone_idx = find_zone_for_note(cfg, note);
                    if (zone_idx < 0) return 0;
                    
                    channelizer_zone_t* zone = &cfg->zones[zone_idx];
                    int16_t transposed = (int16_t)note + zone->transpose;
                    uint8_t out_note = clamp_note(transposed);
                    
                    outputs[0].status = MIDI_STATUS_NOTE_OFF | zone->output_channel;
                    outputs[0].data1 = out_note;
                    outputs[0].data2 = velocity;
                    return 1;
                }
                return 0;
            }
            
            outputs[0].status = MIDI_STATUS_NOTE_OFF | cfg->voices[voice_idx].channel;
            outputs[0].data1 = cfg->voices[voice_idx].note;
            outputs[0].data2 = velocity;
            
            cfg->voices[voice_idx].active = 0;
            return 1;
        }
    }
    
    return 0;
}

uint8_t channelizer_process(uint8_t track, uint8_t status, uint8_t data1, uint8_t data2,
                            channelizer_output_t* outputs, uint8_t max_outputs) {
    if (track >= CHANNELIZER_MAX_TRACKS || !outputs || max_outputs == 0) return 0;
    
    channelizer_config_t* cfg = &g_channelizer_config[track];
    
    // Check if enabled
    if (!cfg->enabled) {
        outputs[0].status = status;
        outputs[0].data1 = data1;
        outputs[0].data2 = data2;
        return 1;
    }
    
    // Check if channel message
    if (!is_channel_message(status)) {
        outputs[0].status = status;
        outputs[0].data1 = data1;
        outputs[0].data2 = data2;
        return 1;
    }
    
    uint8_t channel = get_channel(status);
    uint8_t status_type = status & MIDI_STATUS_MASK;
    
    // Check input channel filter
    if (!(cfg->input_channel_mask & (1 << channel))) {
        return 0; // Filtered out
    }
    
    // Handle note messages with voice management
    if (status_type == MIDI_STATUS_NOTE_ON) {
        return channelizer_process_note_on(track, channel, data1, data2, outputs, max_outputs);
    } else if (status_type == MIDI_STATUS_NOTE_OFF) {
        return channelizer_process_note_off(track, channel, data1, data2, outputs, max_outputs);
    }
    
    // For other channel messages, apply channel transformation without voice management
    uint8_t out_channel = channel;
    
    switch (cfg->mode) {
        case CHANNELIZER_MODE_BYPASS:
            out_channel = channel;
            break;
            
        case CHANNELIZER_MODE_FORCE:
            out_channel = cfg->force_channel;
            break;
            
        case CHANNELIZER_MODE_REMAP:
            out_channel = cfg->channel_map[channel];
            break;
            
        case CHANNELIZER_MODE_ROTATE:
            if (cfg->rotate_count > 0) {
                out_channel = cfg->rotate_channels[0];
            }
            break;
            
        case CHANNELIZER_MODE_ZONE:
            // Non-note messages in zone mode use first enabled zone's channel
            for (uint8_t i = 0; i < CHANNELIZER_MAX_ZONES; i++) {
                if (cfg->zones[i].enabled) {
                    out_channel = cfg->zones[i].output_channel;
                    break;
                }
            }
            break;
    }
    
    outputs[0].status = set_channel(status, out_channel);
    outputs[0].data1 = data1;
    outputs[0].data2 = data2;
    return 1;
}

// ==================== Configuration Management ====================

void channelizer_reset(uint8_t track) {
    if (track >= CHANNELIZER_MAX_TRACKS) return;
    
    channelizer_config_t* cfg = &g_channelizer_config[track];
    
    cfg->enabled = 0;
    cfg->mode = CHANNELIZER_MODE_BYPASS;
    cfg->input_channel_mask = 0xFFFF; // All channels enabled by default
    cfg->force_channel = 0;
    
    // Initialize identity channel map
    for (uint8_t i = 0; i < CHANNELIZER_MAX_CHANNELS; i++) {
        cfg->channel_map[i] = i;
    }
    
    // Initialize rotate channels to sequential 1-16
    for (uint8_t i = 0; i < CHANNELIZER_MAX_CHANNELS; i++) {
        cfg->rotate_channels[i] = i;
    }
    cfg->rotate_count = CHANNELIZER_MAX_CHANNELS;
    cfg->rotate_index = 0;
    
    // Clear zones
    memset(cfg->zones, 0, sizeof(cfg->zones));
    cfg->zone_count = 0;
    
    // Initialize default zones (keyboard split)
    cfg->zones[0].enabled = 0;
    cfg->zones[0].note_min = 0;
    cfg->zones[0].note_max = 59; // C0-B3
    cfg->zones[0].output_channel = 0;
    cfg->zones[0].transpose = 0;
    
    cfg->zones[1].enabled = 0;
    cfg->zones[1].note_min = 60;
    cfg->zones[1].note_max = 127; // C4-G9
    cfg->zones[1].output_channel = 1;
    cfg->zones[1].transpose = 0;
    
    // Voice management defaults
    memset(cfg->voices, 0, sizeof(cfg->voices));
    cfg->voice_steal_mode = CHANNELIZER_VOICE_STEAL_OLDEST;
    cfg->voice_limit = CHANNELIZER_MAX_VOICES;
    cfg->voice_timestamp = 0;
}

void channelizer_reset_all(void) {
    for (uint8_t i = 0; i < CHANNELIZER_MAX_TRACKS; i++) {
        channelizer_reset(i);
    }
}

const char* channelizer_get_mode_name(channelizer_mode_t mode) {
    if (mode >= sizeof(mode_names) / sizeof(mode_names[0])) {
        return "Unknown";
    }
    return mode_names[mode];
}

const char* channelizer_get_voice_steal_name(channelizer_voice_steal_t mode) {
    if (mode >= sizeof(voice_steal_names) / sizeof(voice_steal_names[0])) {
        return "Unknown";
    }
    return voice_steal_names[mode];
}
