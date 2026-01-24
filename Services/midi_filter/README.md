# MIDI Filter Module

## Overview

The MIDI Filter module provides comprehensive filtering capabilities for MIDI messages in the MidiCore system. It allows precise control over which MIDI messages are passed through or blocked based on multiple criteria.

## Features

### Message Type Filtering
- **Note Messages**: Filter Note On/Off independently
- **Aftertouch**: Polyphonic and Channel Aftertouch
- **Control Change**: With per-CC number filtering
- **Program Change**: Filter program changes
- **Pitch Bend**: Filter pitch bend messages
- **System Messages**: SysEx filtering
- **Realtime Messages**: Clock, Start, Continue, Stop, Active Sensing, System Reset

### Channel Filtering
Three modes available:
- **ALL**: Pass all channels (no filtering)
- **ALLOW**: Only specified channels pass (whitelist)
- **BLOCK**: Specified channels are blocked (blacklist)

### Note Range Filtering
- Set minimum and maximum note numbers (0-127)
- Only notes within the range pass through
- Applies to Note On/Off messages

### Velocity Filtering
- Set minimum and maximum velocity thresholds
- Only velocities within the range pass through
- Applies to Note On messages

### CC Filtering
- Enable/disable individual CC numbers (0-127)
- Independent control for each CC
- Optional: disable CC filtering to pass all CCs

### Per-Track Configuration
- 4 independent tracks (0-3)
- Each track has its own complete filter configuration
- Enable/disable filtering per track

## API Reference

### Initialization

```c
void midi_filter_init(void);
```

Initialize the MIDI filter module. Call once at startup.

### Enable/Disable Filter

```c
void midi_filter_set_enabled(uint8_t track, uint8_t enabled);
uint8_t midi_filter_is_enabled(uint8_t track);
```

Enable or disable filtering for a specific track.

### Message Type Filtering

```c
// Set all allowed message types at once
void midi_filter_set_allowed_messages(uint8_t track, uint16_t msg_types);

// Enable/disable individual message types
void midi_filter_set_message_enabled(uint8_t track, midi_filter_msg_type_t msg_type, uint8_t enabled);

// Check if message type is enabled
uint8_t midi_filter_is_message_enabled(uint8_t track, midi_filter_msg_type_t msg_type);
```

Message type flags:
- `MIDI_FILTER_MSG_NOTE_ON`
- `MIDI_FILTER_MSG_NOTE_OFF`
- `MIDI_FILTER_MSG_POLY_AFTERTOUCH`
- `MIDI_FILTER_MSG_CONTROL_CHANGE`
- `MIDI_FILTER_MSG_PROGRAM_CHANGE`
- `MIDI_FILTER_MSG_CHAN_AFTERTOUCH`
- `MIDI_FILTER_MSG_PITCH_BEND`
- `MIDI_FILTER_MSG_SYSEX`
- `MIDI_FILTER_MSG_CLOCK`
- `MIDI_FILTER_MSG_START`
- `MIDI_FILTER_MSG_CONTINUE`
- `MIDI_FILTER_MSG_STOP`
- `MIDI_FILTER_MSG_ACTIVE_SENSING`
- `MIDI_FILTER_MSG_SYSTEM_RESET`
- `MIDI_FILTER_MSG_ALL`

### Channel Filtering

```c
// Set channel filter mode
void midi_filter_set_channel_mode(uint8_t track, midi_filter_channel_mode_t mode);

// Enable/disable individual channels
void midi_filter_set_channel_enabled(uint8_t track, uint8_t channel, uint8_t enabled);

// Set all channels at once with bitmask
void midi_filter_set_channel_mask(uint8_t track, uint16_t channel_mask);
```

Channel modes:
- `MIDI_FILTER_CHANNEL_MODE_ALL`: Pass all channels
- `MIDI_FILTER_CHANNEL_MODE_ALLOW`: Only enabled channels pass (whitelist)
- `MIDI_FILTER_CHANNEL_MODE_BLOCK`: Enabled channels are blocked (blacklist)

**Important**: When using ALLOW or BLOCK mode, it's recommended to first clear the channel mask with `midi_filter_set_channel_mask(track, 0x0000)` before enabling specific channels, as the default state has all channels enabled (0xFFFF).

### Note Range Filtering

```c
// Set note range
void midi_filter_set_note_range(uint8_t track, uint8_t min_note, uint8_t max_note);

// Enable/disable note range filter
void midi_filter_set_note_range_enabled(uint8_t track, uint8_t enabled);

// Get current note range
void midi_filter_get_note_range(uint8_t track, uint8_t* min_note, uint8_t* max_note);
```

### Velocity Filtering

```c
// Set velocity range
void midi_filter_set_velocity_range(uint8_t track, uint8_t min_velocity, uint8_t max_velocity);

// Enable/disable velocity filter
void midi_filter_set_velocity_range_enabled(uint8_t track, uint8_t enabled);

// Get current velocity range
void midi_filter_get_velocity_range(uint8_t track, uint8_t* min_velocity, uint8_t* max_velocity);
```

### CC Filtering

```c
// Enable/disable individual CC numbers
void midi_filter_set_cc_enabled(uint8_t track, uint8_t cc_number, uint8_t enabled);

// Enable/disable CC filtering entirely
void midi_filter_set_cc_filter_enabled(uint8_t track, uint8_t enabled);

// Check if CC is enabled
uint8_t midi_filter_is_cc_enabled(uint8_t track, uint8_t cc_number);
```

### Testing Messages

```c
midi_filter_result_t midi_filter_test_message(uint8_t track, uint8_t status, uint8_t data1, uint8_t data2);
```

Test if a MIDI message passes the filter. Returns:
- `MIDI_FILTER_RESULT_PASS`: Message passes all filters
- `MIDI_FILTER_RESULT_BLOCK`: Message is blocked

### Reset

```c
// Reset single track to defaults
void midi_filter_reset(uint8_t track);

// Reset all tracks to defaults
void midi_filter_reset_all(void);
```

## Usage Examples

### Example 1: Filter Only Note Messages on Channel 1

```c
// Initialize
midi_filter_init();

// Enable filter for track 0
midi_filter_set_enabled(0, 1);

// Only allow note messages
midi_filter_set_allowed_messages(0, 
    MIDI_FILTER_MSG_NOTE_ON | MIDI_FILTER_MSG_NOTE_OFF);

// Set channel mode to allow only channel 1 (channel 0 in 0-indexed)
midi_filter_set_channel_mode(0, MIDI_FILTER_CHANNEL_MODE_ALLOW);
midi_filter_set_channel_mask(0, 0x0000);  // Clear all channels first
midi_filter_set_channel_enabled(0, 0, 1);  // Enable channel 0 (channel 1 in MIDI)

// Test a message
uint8_t status = 0x90;  // Note On, channel 1
uint8_t note = 60;      // Middle C
uint8_t velocity = 100;

if (midi_filter_test_message(0, status, note, velocity) == MIDI_FILTER_RESULT_PASS) {
    // Message passed - send it through
}
```

### Example 2: Filter Note Range (Piano Middle Octave)

```c
// Enable filter
midi_filter_set_enabled(0, 1);

// Allow all message types
midi_filter_set_allowed_messages(0, MIDI_FILTER_MSG_ALL);

// Filter notes to middle octave (C4-B4, notes 60-71)
midi_filter_set_note_range_enabled(0, 1);
midi_filter_set_note_range(0, 60, 71);
```

### Example 3: Block MIDI Clock Messages

```c
// Enable filter
midi_filter_set_enabled(0, 1);

// Start with all messages allowed
midi_filter_set_allowed_messages(0, MIDI_FILTER_MSG_ALL);

// Disable clock messages
midi_filter_set_message_enabled(0, MIDI_FILTER_MSG_CLOCK, 0);
```

### Example 4: Filter Velocity (Soft Notes Only)

```c
// Enable filter
midi_filter_set_enabled(0, 1);

// Allow all message types
midi_filter_set_allowed_messages(0, MIDI_FILTER_MSG_ALL);

// Only pass soft notes (velocity 1-50)
midi_filter_set_velocity_range_enabled(0, 1);
midi_filter_set_velocity_range(0, 1, 50);
```

### Example 5: Block Specific CC Numbers

```c
// Enable filter
midi_filter_set_enabled(0, 1);

// Allow all message types
midi_filter_set_allowed_messages(0, MIDI_FILTER_MSG_ALL);

// Enable CC filtering
midi_filter_set_cc_filter_enabled(0, 1);

// By default all CCs are enabled, so disable specific ones
midi_filter_set_cc_enabled(0, 7, 0);   // Block CC#7 (Volume)
midi_filter_set_cc_enabled(0, 11, 0);  // Block CC#11 (Expression)
```

### Example 6: Multi-Track Setup

```c
// Track 0: Only notes on channels 1-4
midi_filter_set_enabled(0, 1);
midi_filter_set_allowed_messages(0, 
    MIDI_FILTER_MSG_NOTE_ON | MIDI_FILTER_MSG_NOTE_OFF);
midi_filter_set_channel_mode(0, MIDI_FILTER_CHANNEL_MODE_ALLOW);
midi_filter_set_channel_mask(0, 0x000F);  // Channels 0-3

// Track 1: Only CCs on channel 10
midi_filter_set_enabled(1, 1);
midi_filter_set_allowed_messages(1, MIDI_FILTER_MSG_CONTROL_CHANGE);
midi_filter_set_channel_mode(1, MIDI_FILTER_CHANNEL_MODE_ALLOW);
midi_filter_set_channel_enabled(1, 9, 1);  // Channel 10 (index 9)

// Track 2: Block realtime messages
midi_filter_set_enabled(2, 1);
midi_filter_set_allowed_messages(2, MIDI_FILTER_MSG_ALL);
midi_filter_set_message_enabled(2, MIDI_FILTER_MSG_CLOCK, 0);
midi_filter_set_message_enabled(2, MIDI_FILTER_MSG_START, 0);
midi_filter_set_message_enabled(2, MIDI_FILTER_MSG_CONTINUE, 0);
midi_filter_set_message_enabled(2, MIDI_FILTER_MSG_STOP, 0);
```

## Integration with MidiCore

The MIDI Filter module is designed to integrate seamlessly with the MidiCore MIDI router:

```c
// In your MIDI processing callback
void process_midi_message(uint8_t status, uint8_t data1, uint8_t data2) {
    uint8_t track = get_current_track();
    
    // Test message against filter
    if (midi_filter_test_message(track, status, data1, data2) == MIDI_FILTER_RESULT_PASS) {
        // Message passed - route it
        midi_router_send3(MIDI_ROUTER_SRC_INTERNAL, status, data1, data2);
    }
    // Otherwise, message is blocked - do nothing
}
```

## Implementation Notes

### Memory Usage
- Per-track configuration: ~40 bytes
- Total memory: ~160 bytes for 4 tracks
- No dynamic memory allocation

### Performance
- Filter testing is very fast (typically < 1μs)
- No floating point operations
- All operations are bitwise or integer comparisons

### Thread Safety
- Module is not thread-safe by default
- If using from multiple contexts, add mutex protection around configuration changes
- Reading filter state during message testing is safe if configuration isn't being modified

## Default Configuration

When initialized or reset, each track has:
- Filter disabled
- All message types allowed
- All channels pass (channel mode = ALL)
- Note range: 0-127 (disabled)
- Velocity range: 0-127 (disabled)
- All CCs enabled (CC filter disabled)

## Compatibility

- Compatible with STM32 HAL
- Uses only standard C99 features
- No external dependencies beyond `stdint.h` and `string.h`
- Follows MidiCore coding conventions

## Version

Version 1.0.0 - Initial release
