/**
 * @file midi_filter.c
 * @brief MIDI Filter implementation
 */

#include "Services/midi_filter/midi_filter.h"
#include <string.h>

// MIDI status byte masks
#define MIDI_STATUS_MASK           0xF0
#define MIDI_CHANNEL_MASK          0x0F
#define MIDI_STATUS_NOTE_OFF       0x80
#define MIDI_STATUS_NOTE_ON        0x90
#define MIDI_STATUS_POLY_AT        0xA0
#define MIDI_STATUS_CC             0xB0
#define MIDI_STATUS_PROGRAM        0xC0
#define MIDI_STATUS_CHAN_AT        0xD0
#define MIDI_STATUS_PITCH_BEND     0xE0
#define MIDI_STATUS_SYSEX_START    0xF0
#define MIDI_STATUS_CLOCK          0xF8
#define MIDI_STATUS_START          0xFA
#define MIDI_STATUS_CONTINUE       0xFB
#define MIDI_STATUS_STOP           0xFC
#define MIDI_STATUS_ACTIVE_SENSING 0xFE
#define MIDI_STATUS_SYSTEM_RESET   0xFF

// Message type names
static const char* message_type_names[] = {
    "Note On",
    "Note Off",
    "Poly AT",
    "CC",
    "Program",
    "Chan AT",
    "Pitch Bend",
    "SysEx",
    "Clock",
    "Start",
    "Continue",
    "Stop",
    "Active Sensing",
    "System Reset",
    "All Messages"
};

// Channel mode names
static const char* channel_mode_names[] = {
    "All Channels",
    "Allow List",
    "Block List"
};

// Per-track filter configuration
typedef struct {
    uint8_t enabled;
    uint16_t allowed_msg_types;                  // Bitwise OR of MIDI_FILTER_MSG_* flags
    
    // Channel filtering
    midi_filter_channel_mode_t channel_mode;
    uint16_t channel_mask;                       // 16-bit mask (1 bit per channel)
    
    // Note filtering
    uint8_t note_range_enabled;
    uint8_t min_note;
    uint8_t max_note;
    
    // Velocity filtering
    uint8_t velocity_range_enabled;
    uint8_t min_velocity;
    uint8_t max_velocity;
    
    // CC filtering
    uint8_t cc_filter_enabled;
    uint8_t cc_mask[MIDI_FILTER_MAX_CC / 8];    // Bit array for 128 CC numbers
} midi_filter_config_t;

static midi_filter_config_t g_filter_config[MIDI_FILTER_MAX_TRACKS];

/**
 * @brief Set a bit in the CC mask
 */
static inline void set_cc_bit(uint8_t* mask, uint8_t cc, uint8_t value) {
    if (cc >= MIDI_FILTER_MAX_CC) return;
    uint8_t byte_idx = cc / 8;
    uint8_t bit_idx = cc % 8;
    if (value) {
        mask[byte_idx] |= (1 << bit_idx);
    } else {
        mask[byte_idx] &= ~(1 << bit_idx);
    }
}

/**
 * @brief Get a bit from the CC mask
 */
static inline uint8_t get_cc_bit(const uint8_t* mask, uint8_t cc) {
    if (cc >= MIDI_FILTER_MAX_CC) return 0;
    uint8_t byte_idx = cc / 8;
    uint8_t bit_idx = cc % 8;
    return (mask[byte_idx] & (1 << bit_idx)) ? 1 : 0;
}

/**
 * @brief Initialize MIDI filter module
 */
void midi_filter_init(void) {
    memset(g_filter_config, 0, sizeof(g_filter_config));
    
    // Initialize defaults for all tracks
    for (uint8_t i = 0; i < MIDI_FILTER_MAX_TRACKS; i++) {
        midi_filter_reset(i);
    }
}

/**
 * @brief Enable/disable filter for a track
 */
void midi_filter_set_enabled(uint8_t track, uint8_t enabled) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return;
    g_filter_config[track].enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if filter is enabled for a track
 */
uint8_t midi_filter_is_enabled(uint8_t track) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return 0;
    return g_filter_config[track].enabled;
}

/**
 * @brief Set which message types to allow
 */
void midi_filter_set_allowed_messages(uint8_t track, uint16_t msg_types) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return;
    g_filter_config[track].allowed_msg_types = msg_types;
}

/**
 * @brief Get allowed message types mask
 */
uint16_t midi_filter_get_allowed_messages(uint8_t track) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return MIDI_FILTER_MSG_ALL;
    return g_filter_config[track].allowed_msg_types;
}

/**
 * @brief Enable/disable specific message type
 */
void midi_filter_set_message_enabled(uint8_t track, midi_filter_msg_type_t msg_type, uint8_t enabled) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return;
    
    if (enabled) {
        g_filter_config[track].allowed_msg_types |= msg_type;
    } else {
        g_filter_config[track].allowed_msg_types &= ~msg_type;
    }
}

/**
 * @brief Check if specific message type is enabled
 */
uint8_t midi_filter_is_message_enabled(uint8_t track, midi_filter_msg_type_t msg_type) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return 1;
    return (g_filter_config[track].allowed_msg_types & msg_type) ? 1 : 0;
}

/**
 * @brief Set channel filter mode
 */
void midi_filter_set_channel_mode(uint8_t track, midi_filter_channel_mode_t mode) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return;
    g_filter_config[track].channel_mode = mode;
}

/**
 * @brief Get channel filter mode
 */
midi_filter_channel_mode_t midi_filter_get_channel_mode(uint8_t track) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return MIDI_FILTER_CHANNEL_MODE_ALL;
    return g_filter_config[track].channel_mode;
}

/**
 * @brief Enable/disable specific MIDI channel
 */
void midi_filter_set_channel_enabled(uint8_t track, uint8_t channel, uint8_t enabled) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return;
    if (channel >= MIDI_FILTER_MAX_CHANNELS) return;
    
    if (enabled) {
        g_filter_config[track].channel_mask |= (1 << channel);
    } else {
        g_filter_config[track].channel_mask &= ~(1 << channel);
    }
}

/**
 * @brief Check if specific MIDI channel is enabled
 */
uint8_t midi_filter_is_channel_enabled(uint8_t track, uint8_t channel) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return 1;
    if (channel >= MIDI_FILTER_MAX_CHANNELS) return 0;
    return (g_filter_config[track].channel_mask & (1 << channel)) ? 1 : 0;
}

/**
 * @brief Set all channels enabled/disabled at once
 */
void midi_filter_set_channel_mask(uint8_t track, uint16_t channel_mask) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return;
    g_filter_config[track].channel_mask = channel_mask;
}

/**
 * @brief Get channel enable mask
 */
uint16_t midi_filter_get_channel_mask(uint8_t track) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return 0xFFFF;
    return g_filter_config[track].channel_mask;
}

/**
 * @brief Set note range filter
 */
void midi_filter_set_note_range(uint8_t track, uint8_t min_note, uint8_t max_note) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return;
    if (min_note >= MIDI_FILTER_MAX_NOTE) min_note = MIDI_FILTER_MAX_NOTE - 1;
    if (max_note >= MIDI_FILTER_MAX_NOTE) max_note = MIDI_FILTER_MAX_NOTE - 1;
    
    // Ensure min <= max
    if (min_note > max_note) {
        uint8_t temp = min_note;
        min_note = max_note;
        max_note = temp;
    }
    
    g_filter_config[track].min_note = min_note;
    g_filter_config[track].max_note = max_note;
}

/**
 * @brief Get note range filter
 */
void midi_filter_get_note_range(uint8_t track, uint8_t* min_note, uint8_t* max_note) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return;
    if (!min_note || !max_note) return;
    
    *min_note = g_filter_config[track].min_note;
    *max_note = g_filter_config[track].max_note;
}

/**
 * @brief Enable/disable note range filter
 */
void midi_filter_set_note_range_enabled(uint8_t track, uint8_t enabled) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return;
    g_filter_config[track].note_range_enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if note range filter is enabled
 */
uint8_t midi_filter_is_note_range_enabled(uint8_t track) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return 0;
    return g_filter_config[track].note_range_enabled;
}

/**
 * @brief Set velocity range filter
 */
void midi_filter_set_velocity_range(uint8_t track, uint8_t min_velocity, uint8_t max_velocity) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return;
    if (min_velocity > 127) min_velocity = 127;
    if (max_velocity > 127) max_velocity = 127;
    
    // Ensure min <= max
    if (min_velocity > max_velocity) {
        uint8_t temp = min_velocity;
        min_velocity = max_velocity;
        max_velocity = temp;
    }
    
    g_filter_config[track].min_velocity = min_velocity;
    g_filter_config[track].max_velocity = max_velocity;
}

/**
 * @brief Get velocity range filter
 */
void midi_filter_get_velocity_range(uint8_t track, uint8_t* min_velocity, uint8_t* max_velocity) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return;
    if (!min_velocity || !max_velocity) return;
    
    *min_velocity = g_filter_config[track].min_velocity;
    *max_velocity = g_filter_config[track].max_velocity;
}

/**
 * @brief Enable/disable velocity range filter
 */
void midi_filter_set_velocity_range_enabled(uint8_t track, uint8_t enabled) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return;
    g_filter_config[track].velocity_range_enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if velocity range filter is enabled
 */
uint8_t midi_filter_is_velocity_range_enabled(uint8_t track) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return 0;
    return g_filter_config[track].velocity_range_enabled;
}

/**
 * @brief Enable/disable specific CC number
 */
void midi_filter_set_cc_enabled(uint8_t track, uint8_t cc_number, uint8_t enabled) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return;
    set_cc_bit(g_filter_config[track].cc_mask, cc_number, enabled);
}

/**
 * @brief Check if specific CC number is enabled
 */
uint8_t midi_filter_is_cc_enabled(uint8_t track, uint8_t cc_number) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return 1;
    return get_cc_bit(g_filter_config[track].cc_mask, cc_number);
}

/**
 * @brief Enable/disable CC filtering
 */
void midi_filter_set_cc_filter_enabled(uint8_t track, uint8_t enabled) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return;
    g_filter_config[track].cc_filter_enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if CC filtering is enabled
 */
uint8_t midi_filter_is_cc_filter_enabled(uint8_t track) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return 0;
    return g_filter_config[track].cc_filter_enabled;
}

/**
 * @brief Test if a MIDI message passes the filter
 */
midi_filter_result_t midi_filter_test_message(uint8_t track, uint8_t status, uint8_t data1, uint8_t data2) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return MIDI_FILTER_RESULT_PASS;
    
    midi_filter_config_t* cfg = &g_filter_config[track];
    
    // If filter is disabled, pass everything
    if (!cfg->enabled) return MIDI_FILTER_RESULT_PASS;
    
    uint8_t status_type = status & MIDI_STATUS_MASK;
    uint8_t channel = status & MIDI_CHANNEL_MASK;
    
    // Check message type filter
    uint16_t msg_flag = 0;
    
    switch (status_type) {
        case MIDI_STATUS_NOTE_OFF:
            msg_flag = MIDI_FILTER_MSG_NOTE_OFF;
            break;
        case MIDI_STATUS_NOTE_ON:
            msg_flag = MIDI_FILTER_MSG_NOTE_ON;
            break;
        case MIDI_STATUS_POLY_AT:
            msg_flag = MIDI_FILTER_MSG_POLY_AFTERTOUCH;
            break;
        case MIDI_STATUS_CC:
            msg_flag = MIDI_FILTER_MSG_CONTROL_CHANGE;
            break;
        case MIDI_STATUS_PROGRAM:
            msg_flag = MIDI_FILTER_MSG_PROGRAM_CHANGE;
            break;
        case MIDI_STATUS_CHAN_AT:
            msg_flag = MIDI_FILTER_MSG_CHAN_AFTERTOUCH;
            break;
        case MIDI_STATUS_PITCH_BEND:
            msg_flag = MIDI_FILTER_MSG_PITCH_BEND;
            break;
        case MIDI_STATUS_SYSEX_START:
            msg_flag = MIDI_FILTER_MSG_SYSEX;
            break;
    }
    
    // Handle realtime messages (0xF8-0xFF)
    if (status >= MIDI_STATUS_CLOCK) {
        switch (status) {
            case MIDI_STATUS_CLOCK:
                msg_flag = MIDI_FILTER_MSG_CLOCK;
                break;
            case MIDI_STATUS_START:
                msg_flag = MIDI_FILTER_MSG_START;
                break;
            case MIDI_STATUS_CONTINUE:
                msg_flag = MIDI_FILTER_MSG_CONTINUE;
                break;
            case MIDI_STATUS_STOP:
                msg_flag = MIDI_FILTER_MSG_STOP;
                break;
            case MIDI_STATUS_ACTIVE_SENSING:
                msg_flag = MIDI_FILTER_MSG_ACTIVE_SENSING;
                break;
            case MIDI_STATUS_SYSTEM_RESET:
                msg_flag = MIDI_FILTER_MSG_SYSTEM_RESET;
                break;
        }
    }
    
    // Check if this message type is allowed
    if (msg_flag != 0 && !(cfg->allowed_msg_types & msg_flag)) {
        return MIDI_FILTER_RESULT_BLOCK;
    }
    
    // Channel filtering (only for channel messages 0x80-0xEF)
    if (status_type < 0xF0) {
        switch (cfg->channel_mode) {
            case MIDI_FILTER_CHANNEL_MODE_ALL:
                // All channels pass
                break;
            
            case MIDI_FILTER_CHANNEL_MODE_ALLOW:
                // Only channels in mask are allowed
                if (!(cfg->channel_mask & (1 << channel))) {
                    return MIDI_FILTER_RESULT_BLOCK;
                }
                break;
            
            case MIDI_FILTER_CHANNEL_MODE_BLOCK:
                // Channels in mask are blocked
                if (cfg->channel_mask & (1 << channel)) {
                    return MIDI_FILTER_RESULT_BLOCK;
                }
                break;
        }
    }
    
    // Note-specific filters
    if (status_type == MIDI_STATUS_NOTE_ON || status_type == MIDI_STATUS_NOTE_OFF) {
        uint8_t note = data1;
        uint8_t velocity = data2;
        
        // Note range filter
        if (cfg->note_range_enabled) {
            if (note < cfg->min_note || note > cfg->max_note) {
                return MIDI_FILTER_RESULT_BLOCK;
            }
        }
        
        // Velocity range filter (only for Note On)
        if (status_type == MIDI_STATUS_NOTE_ON && cfg->velocity_range_enabled) {
            if (velocity < cfg->min_velocity || velocity > cfg->max_velocity) {
                return MIDI_FILTER_RESULT_BLOCK;
            }
        }
    }
    
    // CC-specific filters
    if (status_type == MIDI_STATUS_CC) {
        if (cfg->cc_filter_enabled) {
            uint8_t cc_number = data1;
            if (!get_cc_bit(cfg->cc_mask, cc_number)) {
                return MIDI_FILTER_RESULT_BLOCK;
            }
        }
    }
    
    return MIDI_FILTER_RESULT_PASS;
}

/**
 * @brief Reset filter configuration for a track to defaults
 */
void midi_filter_reset(uint8_t track) {
    if (track >= MIDI_FILTER_MAX_TRACKS) return;
    
    midi_filter_config_t* cfg = &g_filter_config[track];
    
    cfg->enabled = 0;
    cfg->allowed_msg_types = MIDI_FILTER_MSG_ALL;
    cfg->channel_mode = MIDI_FILTER_CHANNEL_MODE_ALL;
    cfg->channel_mask = 0xFFFF;  // All channels enabled
    
    cfg->note_range_enabled = 0;
    cfg->min_note = 0;
    cfg->max_note = 127;
    
    cfg->velocity_range_enabled = 0;
    cfg->min_velocity = 0;
    cfg->max_velocity = 127;
    
    cfg->cc_filter_enabled = 0;
    // Enable all CCs by default
    memset(cfg->cc_mask, 0xFF, sizeof(cfg->cc_mask));
}

/**
 * @brief Reset all tracks to default filter configuration
 */
void midi_filter_reset_all(void) {
    for (uint8_t i = 0; i < MIDI_FILTER_MAX_TRACKS; i++) {
        midi_filter_reset(i);
    }
}

/**
 * @brief Get message type name
 */
const char* midi_filter_get_message_type_name(midi_filter_msg_type_t msg_type) {
    // Find which bit is set and return corresponding name
    for (uint8_t i = 0; i < 15; i++) {
        if (msg_type == (uint16_t)(1 << i)) {
            return message_type_names[i];
        }
    }
    if (msg_type == MIDI_FILTER_MSG_ALL) {
        return message_type_names[14];
    }
    return "Unknown";
}

/**
 * @brief Get channel mode name
 */
const char* midi_filter_get_channel_mode_name(midi_filter_channel_mode_t mode) {
    if (mode < 3) {
        return channel_mode_names[mode];
    }
    return "Unknown";
}
