# MIDI Channelizer - Quick Reference

## Module Overview
Location: `/home/runner/work/MidiCore/MidiCore/Services/channelizer/`

## Operating Modes

| Mode | Description | Use Case |
|------|-------------|----------|
| `CHANNELIZER_MODE_BYPASS` | Pass through unchanged | Testing, debugging |
| `CHANNELIZER_MODE_FORCE` | Force all to one channel | Merge multiple controllers |
| `CHANNELIZER_MODE_REMAP` | Map input → output channels | Fixed channel routing |
| `CHANNELIZER_MODE_ROTATE` | Round-robin allocation | Polyphonic voice spreading |
| `CHANNELIZER_MODE_ZONE` | Note range → channels | Keyboard splits |

## Voice Stealing Algorithms

| Algorithm | Description |
|-----------|-------------|
| `CHANNELIZER_VOICE_STEAL_OLDEST` | Steal first allocated voice |
| `CHANNELIZER_VOICE_STEAL_LOWEST` | Steal lowest note |
| `CHANNELIZER_VOICE_STEAL_HIGHEST` | Steal highest note |
| `CHANNELIZER_VOICE_STEAL_QUIETEST` | Steal quietest (lowest velocity) |

## Quick Setup Examples

### Force Channel
```c
channelizer_init();
channelizer_set_mode(0, CHANNELIZER_MODE_FORCE);
channelizer_set_force_channel(0, 0);  // Force to channel 1
channelizer_set_enabled(0, 1);
```

### Channel Remap
```c
channelizer_set_mode(0, CHANNELIZER_MODE_REMAP);
channelizer_set_channel_remap(0, 0, 5);  // Ch1 → Ch6
channelizer_set_channel_remap(0, 1, 6);  // Ch2 → Ch7
channelizer_set_enabled(0, 1);
```

### Keyboard Split
```c
channelizer_zone_t zone = {
    .enabled = 1,
    .note_min = 0,
    .note_max = 59,
    .output_channel = 0,
    .transpose = 0
};
channelizer_set_zone(0, 0, &zone);
channelizer_set_mode(0, CHANNELIZER_MODE_ZONE);
channelizer_set_enabled(0, 1);
```

### Voice Rotation
```c
uint8_t channels[] = {0, 1, 2, 3};
channelizer_set_rotate_channels(0, channels, 4);
channelizer_set_voice_limit(0, 4);
channelizer_set_mode(0, CHANNELIZER_MODE_ROTATE);
channelizer_set_enabled(0, 1);
```

## Processing Messages

```c
channelizer_output_t outputs[4];
uint8_t count = channelizer_process(track, status, data1, data2, outputs, 4);

for (uint8_t i = 0; i < count; i++) {
    // Send outputs[i].status, outputs[i].data1, outputs[i].data2
}
```

## Key Constants

- `CHANNELIZER_MAX_TRACKS` = 4
- `CHANNELIZER_MAX_CHANNELS` = 16
- `CHANNELIZER_MAX_ZONES` = 4
- `CHANNELIZER_MAX_VOICES` = 16

## API Categories

1. **Initialization**: `channelizer_init()`
2. **Enable/Disable**: `set_enabled()`, `is_enabled()`
3. **Mode Control**: `set_mode()`, `get_mode()`
4. **Input Filtering**: `set_input_channel_mask()`, `set_input_channel_enabled()`
5. **Force Mode**: `set_force_channel()`, `get_force_channel()`
6. **Remap Mode**: `set_channel_remap()`, `set_channel_map()`
7. **Rotate Mode**: `set_rotate_channels()`, `reset_rotation()`
8. **Zone Mode**: `set_zone()`, `set_zone_range()`, `set_zone_channel()`
9. **Voice Management**: `set_voice_limit()`, `set_voice_steal_mode()`
10. **Processing**: `process()`, `process_note_on()`, `process_note_off()`

## Memory Usage

- Per track: ~278 bytes
- 4 tracks total: ~1112 bytes
- Static allocation (no dynamic memory)

## Files

- `channelizer.h` - API definitions (439 lines)
- `channelizer.c` - Implementation (687 lines)
- `README.md` - Full documentation (338 lines)
- `channelizer_example.c` - Usage examples (323 lines)

## Compatibility

- Uses `stdint.h` types (uint8_t, int8_t, etc.)
- No external dependencies
- Compatible with STM32 and embedded systems
- Follows MidiCore architecture patterns
