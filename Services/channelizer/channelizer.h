/**
 * @file channelizer.h
 * @brief MIDI Channelizer - intelligent channel mapping and voice management
 * 
 * Provides comprehensive MIDI channel mapping and routing features including:
 * - Input channel filtering (which channels to process)
 * - Output channel remapping (map input channels to different output channels)
 * - Per-track configuration (4 tracks)
 * - Voice stealing for polyphonic channel management
 * - Zone-based channel splitting (route note ranges to different channels)
 * - Force channel mode (override all input channels to specific channel)
 * - Multi-channel to single channel merging
 * - Channel rotation for round-robin voice allocation
 * 
 * Use cases:
 * - Map keyboard zones to different synthesizer channels
 * - Merge multiple MIDI controllers to single channel
 * - Create layered sounds by duplicating to multiple channels
 * - Implement polyphonic voice allocation with voice stealing
 * - Split keyboard into multiple zones with independent channel routing
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CHANNELIZER_MAX_TRACKS 4
#define CHANNELIZER_MAX_CHANNELS 16
#define CHANNELIZER_MAX_ZONES 4
#define CHANNELIZER_MAX_VOICES 16

/**
 * @brief Channelizer mode
 */
typedef enum {
    CHANNELIZER_MODE_BYPASS = 0,    // Pass through without modification
    CHANNELIZER_MODE_FORCE,         // Force all messages to output channel
    CHANNELIZER_MODE_REMAP,         // Remap input channels to output channels
    CHANNELIZER_MODE_ROTATE,        // Rotate through output channels for voice allocation
    CHANNELIZER_MODE_ZONE           // Zone-based channel splitting
} channelizer_mode_t;

/**
 * @brief Voice stealing algorithm
 */
typedef enum {
    CHANNELIZER_VOICE_STEAL_OLDEST = 0,  // Steal oldest note
    CHANNELIZER_VOICE_STEAL_LOWEST,      // Steal lowest note
    CHANNELIZER_VOICE_STEAL_HIGHEST,     // Steal highest note
    CHANNELIZER_VOICE_STEAL_QUIETEST     // Steal quietest note (lowest velocity)
} channelizer_voice_steal_t;

/**
 * @brief Zone configuration for channel splitting
 */
typedef struct {
    uint8_t enabled;             // Zone enabled flag
    uint8_t note_min;            // Minimum note number (0-127)
    uint8_t note_max;            // Maximum note number (0-127)
    uint8_t output_channel;      // Output channel (0-15)
    int8_t transpose;            // Transpose amount in semitones (-127 to +127)
} channelizer_zone_t;

/**
 * @brief Voice state for polyphonic management
 */
typedef struct {
    uint8_t active;              // Voice is active
    uint8_t note;                // Note number
    uint8_t velocity;            // Note velocity
    uint8_t channel;             // Original input channel
    uint32_t timestamp;          // Voice allocation timestamp
} channelizer_voice_t;

/**
 * @brief Per-track channelizer configuration
 */
typedef struct {
    uint8_t enabled;                                      // Track enabled flag
    channelizer_mode_t mode;                              // Operating mode
    
    // Input channel filtering
    uint16_t input_channel_mask;                          // 16-bit mask (1=enabled)
    
    // Force mode configuration
    uint8_t force_channel;                                // Output channel for force mode (0-15)
    
    // Remap mode configuration
    uint8_t channel_map[CHANNELIZER_MAX_CHANNELS];        // Input to output channel mapping
    
    // Rotate mode configuration
    uint8_t rotate_channels[CHANNELIZER_MAX_CHANNELS];    // List of channels to rotate through
    uint8_t rotate_count;                                 // Number of channels in rotation
    uint8_t rotate_index;                                 // Current rotation index
    
    // Zone mode configuration
    channelizer_zone_t zones[CHANNELIZER_MAX_ZONES];      // Zone definitions
    uint8_t zone_count;                                   // Number of active zones
    
    // Voice management
    channelizer_voice_t voices[CHANNELIZER_MAX_VOICES];   // Voice allocation table
    channelizer_voice_steal_t voice_steal_mode;           // Voice stealing algorithm
    uint8_t voice_limit;                                  // Max simultaneous voices (1-16)
    uint32_t voice_timestamp;                             // Monotonic timestamp counter
} channelizer_config_t;

/**
 * @brief Processing result
 */
typedef enum {
    CHANNELIZER_RESULT_PASS = 0,    // Message processed, pass through
    CHANNELIZER_RESULT_MODIFIED,    // Message modified
    CHANNELIZER_RESULT_DROPPED,     // Message dropped/filtered
    CHANNELIZER_RESULT_SPLIT        // Message split into multiple (zones)
} channelizer_result_t;

/**
 * @brief Output message for processing
 */
typedef struct {
    uint8_t status;              // MIDI status byte
    uint8_t data1;               // First data byte
    uint8_t data2;               // Second data byte
} channelizer_output_t;

/**
 * @brief Initialize channelizer module
 */
void channelizer_init(void);

/**
 * @brief Enable/disable channelizer for a track
 * @param track Track index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void channelizer_set_enabled(uint8_t track, uint8_t enabled);

/**
 * @brief Check if channelizer is enabled for a track
 * @param track Track index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t channelizer_is_enabled(uint8_t track);

/**
 * @brief Set channelizer mode
 * @param track Track index (0-3)
 * @param mode Operating mode
 */
void channelizer_set_mode(uint8_t track, channelizer_mode_t mode);

/**
 * @brief Get channelizer mode
 * @param track Track index (0-3)
 * @return Operating mode
 */
channelizer_mode_t channelizer_get_mode(uint8_t track);

/**
 * @brief Set input channel filter mask
 * @param track Track index (0-3)
 * @param mask 16-bit mask where each bit represents a channel (1=enabled)
 */
void channelizer_set_input_channel_mask(uint8_t track, uint16_t mask);

/**
 * @brief Get input channel filter mask
 * @param track Track index (0-3)
 * @return 16-bit mask where each bit represents a channel
 */
uint16_t channelizer_get_input_channel_mask(uint8_t track);

/**
 * @brief Enable/disable specific input channel
 * @param track Track index (0-3)
 * @param channel Input MIDI channel (0-15)
 * @param enabled 1 to enable, 0 to disable
 */
void channelizer_set_input_channel_enabled(uint8_t track, uint8_t channel, uint8_t enabled);

/**
 * @brief Check if specific input channel is enabled
 * @param track Track index (0-3)
 * @param channel Input MIDI channel (0-15)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t channelizer_is_input_channel_enabled(uint8_t track, uint8_t channel);

// ==================== Force Mode ====================

/**
 * @brief Set force channel (force all messages to this channel)
 * @param track Track index (0-3)
 * @param channel Output MIDI channel (0-15)
 */
void channelizer_set_force_channel(uint8_t track, uint8_t channel);

/**
 * @brief Get force channel
 * @param track Track index (0-3)
 * @return Output MIDI channel (0-15)
 */
uint8_t channelizer_get_force_channel(uint8_t track);

// ==================== Remap Mode ====================

/**
 * @brief Set channel remapping for specific input channel
 * @param track Track index (0-3)
 * @param input_channel Input MIDI channel (0-15)
 * @param output_channel Output MIDI channel (0-15)
 */
void channelizer_set_channel_remap(uint8_t track, uint8_t input_channel, uint8_t output_channel);

/**
 * @brief Get output channel for input channel remapping
 * @param track Track index (0-3)
 * @param input_channel Input MIDI channel (0-15)
 * @return Output MIDI channel (0-15)
 */
uint8_t channelizer_get_channel_remap(uint8_t track, uint8_t input_channel);

/**
 * @brief Set all channel remappings at once
 * @param track Track index (0-3)
 * @param map Array of 16 output channels (indexed by input channel)
 */
void channelizer_set_channel_map(uint8_t track, const uint8_t* map);

/**
 * @brief Get all channel remappings
 * @param track Track index (0-3)
 * @param map Output: array of 16 output channels
 */
void channelizer_get_channel_map(uint8_t track, uint8_t* map);

// ==================== Rotate Mode ====================

/**
 * @brief Set channels for rotation mode
 * @param track Track index (0-3)
 * @param channels Array of channels to rotate through
 * @param count Number of channels (1-16)
 */
void channelizer_set_rotate_channels(uint8_t track, const uint8_t* channels, uint8_t count);

/**
 * @brief Get rotate channels configuration
 * @param track Track index (0-3)
 * @param channels Output: array to receive channel list
 * @return Number of channels in rotation
 */
uint8_t channelizer_get_rotate_channels(uint8_t track, uint8_t* channels);

/**
 * @brief Reset rotation index to start
 * @param track Track index (0-3)
 */
void channelizer_reset_rotation(uint8_t track);

// ==================== Zone Mode ====================

/**
 * @brief Configure a zone
 * @param track Track index (0-3)
 * @param zone_index Zone index (0-3)
 * @param zone Zone configuration
 */
void channelizer_set_zone(uint8_t track, uint8_t zone_index, const channelizer_zone_t* zone);

/**
 * @brief Get zone configuration
 * @param track Track index (0-3)
 * @param zone_index Zone index (0-3)
 * @param zone Output: zone configuration
 */
void channelizer_get_zone(uint8_t track, uint8_t zone_index, channelizer_zone_t* zone);

/**
 * @brief Enable/disable a zone
 * @param track Track index (0-3)
 * @param zone_index Zone index (0-3)
 * @param enabled 1 to enable, 0 to disable
 */
void channelizer_set_zone_enabled(uint8_t track, uint8_t zone_index, uint8_t enabled);

/**
 * @brief Check if a zone is enabled
 * @param track Track index (0-3)
 * @param zone_index Zone index (0-3)
 * @return 1 if enabled, 0 if disabled
 */
uint8_t channelizer_is_zone_enabled(uint8_t track, uint8_t zone_index);

/**
 * @brief Set zone note range
 * @param track Track index (0-3)
 * @param zone_index Zone index (0-3)
 * @param note_min Minimum note (0-127)
 * @param note_max Maximum note (0-127)
 */
void channelizer_set_zone_range(uint8_t track, uint8_t zone_index, uint8_t note_min, uint8_t note_max);

/**
 * @brief Set zone output channel
 * @param track Track index (0-3)
 * @param zone_index Zone index (0-3)
 * @param channel Output channel (0-15)
 */
void channelizer_set_zone_channel(uint8_t track, uint8_t zone_index, uint8_t channel);

/**
 * @brief Set zone transpose
 * @param track Track index (0-3)
 * @param zone_index Zone index (0-3)
 * @param transpose Transpose in semitones (-127 to +127)
 */
void channelizer_set_zone_transpose(uint8_t track, uint8_t zone_index, int8_t transpose);

// ==================== Voice Management ====================

/**
 * @brief Set voice stealing algorithm
 * @param track Track index (0-3)
 * @param mode Voice stealing algorithm
 */
void channelizer_set_voice_steal_mode(uint8_t track, channelizer_voice_steal_t mode);

/**
 * @brief Get voice stealing algorithm
 * @param track Track index (0-3)
 * @return Voice stealing algorithm
 */
channelizer_voice_steal_t channelizer_get_voice_steal_mode(uint8_t track);

/**
 * @brief Set maximum simultaneous voices
 * @param track Track index (0-3)
 * @param limit Voice limit (1-16)
 */
void channelizer_set_voice_limit(uint8_t track, uint8_t limit);

/**
 * @brief Get maximum simultaneous voices
 * @param track Track index (0-3)
 * @return Voice limit (1-16)
 */
uint8_t channelizer_get_voice_limit(uint8_t track);

/**
 * @brief Get number of active voices
 * @param track Track index (0-3)
 * @return Number of active voices
 */
uint8_t channelizer_get_active_voice_count(uint8_t track);

/**
 * @brief Release all active voices (send note offs)
 * @param track Track index (0-3)
 * @param outputs Output buffer for note off messages
 * @param max_outputs Maximum outputs buffer can hold
 * @return Number of note off messages generated
 */
uint8_t channelizer_release_all_voices(uint8_t track, channelizer_output_t* outputs, uint8_t max_outputs);

// ==================== Message Processing ====================

/**
 * @brief Process a MIDI message through channelizer
 * @param track Track index (0-3)
 * @param status MIDI status byte
 * @param data1 First data byte
 * @param data2 Second data byte
 * @param outputs Output buffer for processed messages
 * @param max_outputs Maximum outputs buffer can hold
 * @return Number of output messages generated (0 if dropped)
 */
uint8_t channelizer_process(uint8_t track, uint8_t status, uint8_t data1, uint8_t data2,
                            channelizer_output_t* outputs, uint8_t max_outputs);

/**
 * @brief Process note on with voice management
 * @param track Track index (0-3)
 * @param channel Input channel (0-15)
 * @param note Note number (0-127)
 * @param velocity Velocity (0-127)
 * @param outputs Output buffer for processed messages
 * @param max_outputs Maximum outputs buffer can hold
 * @return Number of output messages generated
 */
uint8_t channelizer_process_note_on(uint8_t track, uint8_t channel, uint8_t note, uint8_t velocity,
                                     channelizer_output_t* outputs, uint8_t max_outputs);

/**
 * @brief Process note off with voice management
 * @param track Track index (0-3)
 * @param channel Input channel (0-15)
 * @param note Note number (0-127)
 * @param velocity Release velocity (0-127)
 * @param outputs Output buffer for processed messages
 * @param max_outputs Maximum outputs buffer can hold
 * @return Number of output messages generated
 */
uint8_t channelizer_process_note_off(uint8_t track, uint8_t channel, uint8_t note, uint8_t velocity,
                                      channelizer_output_t* outputs, uint8_t max_outputs);

// ==================== Configuration Management ====================

/**
 * @brief Reset channelizer configuration for a track to defaults
 * @param track Track index (0-3)
 */
void channelizer_reset(uint8_t track);

/**
 * @brief Reset all tracks to default configuration
 */
void channelizer_reset_all(void);

/**
 * @brief Get mode name string
 * @param mode Channelizer mode
 * @return Mode name string
 */
const char* channelizer_get_mode_name(channelizer_mode_t mode);

/**
 * @brief Get voice stealing algorithm name string
 * @param mode Voice stealing mode
 * @return Algorithm name string
 */
const char* channelizer_get_voice_steal_name(channelizer_voice_steal_t mode);

#ifdef __cplusplus
}
#endif
