# MIDI Channelizer Module

## Overview

The Channelizer module provides intelligent MIDI channel mapping and routing capabilities for the MidiCore system. It enables sophisticated channel management including remapping, voice allocation, zone-based splitting, and polyphonic voice management with configurable voice stealing algorithms.

## Features

### Operating Modes

1. **Bypass Mode** - Pass messages through without modification
2. **Force Mode** - Force all messages to a specific output channel
3. **Remap Mode** - Map input channels to different output channels
4. **Rotate Mode** - Round-robin voice allocation across multiple channels
5. **Zone Mode** - Split keyboard into zones with independent channel routing

### Capabilities

- **Input Channel Filtering** - Select which input channels to process (16-bit mask)
- **Multi-track Support** - Independent configuration for 4 tracks
- **Voice Management** - Track up to 16 simultaneous voices per track
- **Voice Stealing** - Configurable algorithms (oldest, lowest, highest, quietest)
- **Zone-Based Splitting** - Up to 4 zones per track with transpose
- **Channel Rotation** - Distribute notes across multiple channels for polyphony

## Architecture

### Data Structures

#### `channelizer_config_t`
Per-track configuration containing:
- Operating mode
- Input channel filter mask
- Channel mapping tables
- Zone configurations
- Voice allocation table
- Voice stealing parameters

#### `channelizer_zone_t`
Zone definition for split keyboard:
- Note range (min/max)
- Output channel
- Transpose amount

#### `channelizer_voice_t`
Voice state for polyphonic tracking:
- Active flag
- Note number
- Velocity
- Input channel
- Allocation timestamp

## Usage Examples

### Example 1: Force All Input to Channel 1

```c
#include "Services/channelizer/channelizer.h"

void setup_force_mode(void) {
    channelizer_init();
    
    // Configure track 0
    channelizer_set_mode(0, CHANNELIZER_MODE_FORCE);
    channelizer_set_force_channel(0, 0); // Channel 1 (0-indexed)
    channelizer_set_enabled(0, 1);
}
```

### Example 2: Remap Channels

```c
void setup_channel_remap(void) {
    channelizer_init();
    
    // Map input channel 0 -> output channel 5
    // Map input channel 1 -> output channel 6
    channelizer_set_mode(0, CHANNELIZER_MODE_REMAP);
    channelizer_set_channel_remap(0, 0, 5);
    channelizer_set_channel_remap(0, 1, 6);
    channelizer_set_enabled(0, 1);
}
```

### Example 3: Keyboard Split with Zones

```c
void setup_keyboard_split(void) {
    channelizer_init();
    
    channelizer_zone_t zone;
    
    // Lower zone: C0-B3 -> Channel 1
    zone.enabled = 1;
    zone.note_min = 0;
    zone.note_max = 59;
    zone.output_channel = 0;
    zone.transpose = 0;
    channelizer_set_zone(0, 0, &zone);
    
    // Upper zone: C4-G9 -> Channel 2, transpose up 12 semitones
    zone.enabled = 1;
    zone.note_min = 60;
    zone.note_max = 127;
    zone.output_channel = 1;
    zone.transpose = 12;
    channelizer_set_zone(0, 1, &zone);
    
    channelizer_set_mode(0, CHANNELIZER_MODE_ZONE);
    channelizer_set_enabled(0, 1);
}
```

### Example 4: Round-Robin Voice Allocation

```c
void setup_voice_rotation(void) {
    channelizer_init();
    
    // Rotate through channels 0-3 for polyphonic voice allocation
    uint8_t channels[] = {0, 1, 2, 3};
    channelizer_set_rotate_channels(0, channels, 4);
    
    // Use oldest voice stealing when all voices allocated
    channelizer_set_voice_steal_mode(0, CHANNELIZER_VOICE_STEAL_OLDEST);
    channelizer_set_voice_limit(0, 4);
    
    channelizer_set_mode(0, CHANNELIZER_MODE_ROTATE);
    channelizer_set_enabled(0, 1);
}
```

### Example 5: Processing MIDI Messages

```c
void process_midi_message(uint8_t status, uint8_t data1, uint8_t data2) {
    channelizer_output_t outputs[4];
    uint8_t count;
    
    // Process message through track 0
    count = channelizer_process(0, status, data1, data2, outputs, 4);
    
    // Send all output messages
    for (uint8_t i = 0; i < count; i++) {
        midi_send(outputs[i].status, outputs[i].data1, outputs[i].data2);
    }
}
```

### Example 6: Input Channel Filtering

```c
void setup_channel_filter(void) {
    channelizer_init();
    
    // Only process channels 0, 1, 2, 3 (mask: 0x000F)
    channelizer_set_input_channel_mask(0, 0x000F);
    
    // Or enable channels individually
    channelizer_set_input_channel_enabled(0, 0, 1); // Enable channel 1
    channelizer_set_input_channel_enabled(0, 1, 1); // Enable channel 2
    channelizer_set_input_channel_enabled(0, 5, 0); // Disable channel 6
    
    channelizer_set_mode(0, CHANNELIZER_MODE_FORCE);
    channelizer_set_force_channel(0, 0);
    channelizer_set_enabled(0, 1);
}
```

## API Reference

### Initialization

```c
void channelizer_init(void);
```
Initialize the channelizer module. Must be called before any other functions.

### Enable/Disable

```c
void channelizer_set_enabled(uint8_t track, uint8_t enabled);
uint8_t channelizer_is_enabled(uint8_t track);
```

### Mode Configuration

```c
void channelizer_set_mode(uint8_t track, channelizer_mode_t mode);
channelizer_mode_t channelizer_get_mode(uint8_t track);
```

### Input Channel Filtering

```c
void channelizer_set_input_channel_mask(uint8_t track, uint16_t mask);
uint16_t channelizer_get_input_channel_mask(uint8_t track);
void channelizer_set_input_channel_enabled(uint8_t track, uint8_t channel, uint8_t enabled);
uint8_t channelizer_is_input_channel_enabled(uint8_t track, uint8_t channel);
```

### Force Mode

```c
void channelizer_set_force_channel(uint8_t track, uint8_t channel);
uint8_t channelizer_get_force_channel(uint8_t track);
```

### Remap Mode

```c
void channelizer_set_channel_remap(uint8_t track, uint8_t input_channel, uint8_t output_channel);
uint8_t channelizer_get_channel_remap(uint8_t track, uint8_t input_channel);
void channelizer_set_channel_map(uint8_t track, const uint8_t* map);
void channelizer_get_channel_map(uint8_t track, uint8_t* map);
```

### Rotate Mode

```c
void channelizer_set_rotate_channels(uint8_t track, const uint8_t* channels, uint8_t count);
uint8_t channelizer_get_rotate_channels(uint8_t track, uint8_t* channels);
void channelizer_reset_rotation(uint8_t track);
```

### Zone Mode

```c
void channelizer_set_zone(uint8_t track, uint8_t zone_index, const channelizer_zone_t* zone);
void channelizer_get_zone(uint8_t track, uint8_t zone_index, channelizer_zone_t* zone);
void channelizer_set_zone_enabled(uint8_t track, uint8_t zone_index, uint8_t enabled);
uint8_t channelizer_is_zone_enabled(uint8_t track, uint8_t zone_index);
void channelizer_set_zone_range(uint8_t track, uint8_t zone_index, uint8_t note_min, uint8_t note_max);
void channelizer_set_zone_channel(uint8_t track, uint8_t zone_index, uint8_t channel);
void channelizer_set_zone_transpose(uint8_t track, uint8_t zone_index, int8_t transpose);
```

### Voice Management

```c
void channelizer_set_voice_steal_mode(uint8_t track, channelizer_voice_steal_t mode);
channelizer_voice_steal_t channelizer_get_voice_steal_mode(uint8_t track);
void channelizer_set_voice_limit(uint8_t track, uint8_t limit);
uint8_t channelizer_get_voice_limit(uint8_t track);
uint8_t channelizer_get_active_voice_count(uint8_t track);
uint8_t channelizer_release_all_voices(uint8_t track, channelizer_output_t* outputs, uint8_t max_outputs);
```

### Message Processing

```c
uint8_t channelizer_process(uint8_t track, uint8_t status, uint8_t data1, uint8_t data2,
                            channelizer_output_t* outputs, uint8_t max_outputs);
                            
uint8_t channelizer_process_note_on(uint8_t track, uint8_t channel, uint8_t note, uint8_t velocity,
                                     channelizer_output_t* outputs, uint8_t max_outputs);
                                     
uint8_t channelizer_process_note_off(uint8_t track, uint8_t channel, uint8_t note, uint8_t velocity,
                                      channelizer_output_t* outputs, uint8_t max_outputs);
```

### Configuration Management

```c
void channelizer_reset(uint8_t track);
void channelizer_reset_all(void);
const char* channelizer_get_mode_name(channelizer_mode_t mode);
const char* channelizer_get_voice_steal_name(channelizer_voice_steal_t mode);
```

## Implementation Details

### Voice Stealing Algorithms

1. **CHANNELIZER_VOICE_STEAL_OLDEST** - Steals the voice that was allocated first
2. **CHANNELIZER_VOICE_STEAL_LOWEST** - Steals the voice with the lowest note number
3. **CHANNELIZER_VOICE_STEAL_HIGHEST** - Steals the voice with the highest note number
4. **CHANNELIZER_VOICE_STEAL_QUIETEST** - Steals the voice with the lowest velocity

### Voice Tracking

- Each track maintains a voice allocation table
- Voices track note number, velocity, channel, and timestamp
- Voice stealing generates note-off for stolen voice
- Automatic note-off matching for proper voice release

### Zone Processing

- Zones are checked in order (0-3)
- First matching zone processes the note
- Transpose is applied after zone match
- Non-note messages use first enabled zone's channel

### Memory Usage

Per track:
- Configuration: ~150 bytes
- Voice table: 16 voices × 8 bytes = 128 bytes
- Total per track: ~278 bytes
- Total for 4 tracks: ~1112 bytes

## Integration with MidiCore

The Channelizer integrates seamlessly with MidiCore's service architecture:

1. **Initialization** - Call `channelizer_init()` during system startup
2. **Configuration** - Configure modes and parameters as needed
3. **Processing** - Call `channelizer_process()` for each incoming MIDI message
4. **Output** - Handle output messages from the processing function

## Thread Safety

The Channelizer is not inherently thread-safe. If used in a multi-threaded environment, appropriate locking mechanisms should be implemented by the caller.

## Performance Considerations

- Channel filtering is O(1) using bitmask
- Voice lookup is O(n) where n = voice_limit
- Zone matching is O(z) where z = number of zones
- Voice stealing is O(n) where n = voice_limit

## Future Enhancements

Potential future additions:
- Dynamic voice limit adjustment
- Priority-based voice stealing
- Per-zone velocity curves
- Channel pressure remapping
- Pitch bend range configuration
- Multi-zone note output (layering)

## License

Part of the MidiCore project. See main project license.

## Author

Created for MidiCore MIDI processing system.
