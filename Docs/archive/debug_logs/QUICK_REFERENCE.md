# MIDI Filter Quick Reference

## Initialization
```c
midi_filter_init();
```

## Enable/Disable
```c
midi_filter_set_enabled(track, 1);  // Enable
midi_filter_set_enabled(track, 0);  // Disable
```

## Message Type Filtering
```c
// Allow only notes
midi_filter_set_allowed_messages(track, 
    MIDI_FILTER_MSG_NOTE_ON | MIDI_FILTER_MSG_NOTE_OFF);

// Block MIDI clock
midi_filter_set_message_enabled(track, MIDI_FILTER_MSG_CLOCK, 0);
```

## Channel Filtering
```c
// Allow only channels 1-4
midi_filter_set_channel_mode(track, MIDI_FILTER_CHANNEL_MODE_ALLOW);
midi_filter_set_channel_mask(track, 0x000F);

// Block channel 10 (drums)
midi_filter_set_channel_mode(track, MIDI_FILTER_CHANNEL_MODE_BLOCK);
midi_filter_set_channel_mask(track, 0x0200);
```

## Note Range
```c
midi_filter_set_note_range_enabled(track, 1);
midi_filter_set_note_range(track, 60, 84);  // C4 to C6
```

## Velocity Range
```c
midi_filter_set_velocity_range_enabled(track, 1);
midi_filter_set_velocity_range(track, 40, 120);
```

## CC Filtering
```c
midi_filter_set_cc_filter_enabled(track, 1);
midi_filter_set_cc_enabled(track, 7, 0);   // Block CC#7 (volume)
midi_filter_set_cc_enabled(track, 64, 0);  // Block CC#64 (sustain)
```

## Test Message
```c
if (midi_filter_test_message(track, status, data1, data2) 
    == MIDI_FILTER_RESULT_PASS) {
    // Pass through
    midi_router_send3(src, status, data1, data2);
}
```

## Reset
```c
midi_filter_reset(track);      // Reset one track
midi_filter_reset_all();       // Reset all tracks
```

## Message Type Flags
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

## Channel Modes
- `MIDI_FILTER_CHANNEL_MODE_ALL` - Pass all channels
- `MIDI_FILTER_CHANNEL_MODE_ALLOW` - Whitelist mode
- `MIDI_FILTER_CHANNEL_MODE_BLOCK` - Blacklist mode

## Filter Results
- `MIDI_FILTER_RESULT_PASS` - Message passes
- `MIDI_FILTER_RESULT_BLOCK` - Message blocked

## Important Notes
- 4 tracks (0-3)
- 16 channels (0-15, corresponds to MIDI channels 1-16)
- Notes: 0-127
- Velocity: 0-127
- CCs: 0-127
- When using ALLOW/BLOCK modes, clear channel mask first:
  `midi_filter_set_channel_mask(track, 0x0000)`
