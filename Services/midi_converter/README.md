# MIDI Converter Module

MIDI Message Converters for MidiCore MIDI FX suite.

## Overview

The midi_converter module converts MIDI messages between different types (CC, Aftertouch, Pitchbend, Velocity) with configurable scaling, offset, and inversion transformations. Perfect for routing MIDI data between incompatible devices or creating creative modulation routings.

## Features

- **8 Conversion Modes:**
  - Aftertouch → CC
  - CC → Aftertouch
  - Pitchbend → CC
  - CC → Pitchbend
  - Velocity → CC
  - ModWheel → CC
  - CC → CC (different CC)
  - Disabled
- **Transformations:**
  - Scale: 0-200% (compress/expand range)
  - Offset: -64 to +63 (shift values)
  - Invert: Flip output (127-x)
- **Per-track configuration** (4 tracks)
- **Multiple output callbacks** for different message types

## Usage Example

```c
#include "Services/midi_converter/midi_converter.h"

// Callbacks for output
void my_cc_callback(uint8_t track, uint8_t cc_number, uint8_t cc_value, uint8_t channel) {
    MIDI_SendCC(channel, cc_number, cc_value);
}

void my_aftertouch_callback(uint8_t track, uint8_t pressure, uint8_t channel) {
    MIDI_SendAftertouch(channel, pressure);
}

void my_pitchbend_callback(uint8_t track, uint16_t value, uint8_t channel) {
    MIDI_SendPitchbend(channel, value);
}

// Initialize
midi_converter_init();
midi_converter_set_cc_callback(my_cc_callback);
midi_converter_set_aftertouch_callback(my_aftertouch_callback);
midi_converter_set_pitchbend_callback(my_pitchbend_callback);

// Configure track 0: Convert aftertouch to CC 74 (filter cutoff)
midi_converter_set_enabled(0, 1);
midi_converter_set_mode(0, MIDI_CONVERTER_AFTERTOUCH_TO_CC);
midi_converter_set_dest_cc(0, 74);
midi_converter_set_scale(0, 150);    // 150% scaling (expand range)
midi_converter_set_offset(0, 10);    // +10 offset
midi_converter_set_invert(0, 0);     // No inversion

// Configure track 1: Convert CC 1 (modwheel) to CC 7 (volume)
midi_converter_set_enabled(1, 1);
midi_converter_set_mode(1, MIDI_CONVERTER_MODWHEEL_TO_CC);
midi_converter_set_source_cc(1, 1);  // Source: Modwheel
midi_converter_set_dest_cc(1, 7);    // Dest: Volume
midi_converter_set_scale(1, 80);     // 80% scaling (compress)
midi_converter_set_invert(1, 1);     // Inverted

// Process incoming messages
midi_converter_process_aftertouch(0, 64, 0);  // Will output CC 74
midi_converter_process_cc(1, 1, 100, 0);      // Will output CC 7
```

## API Reference

### Initialization

- `midi_converter_init()` - Initialize module
- `midi_converter_set_cc_callback()` - Set CC output callback
- `midi_converter_set_aftertouch_callback()` - Set aftertouch callback
- `midi_converter_set_pitchbend_callback()` - Set pitchbend callback

### Configuration

- `midi_converter_set_enabled()` - Enable/disable per track
- `midi_converter_set_mode()` - Set conversion mode
- `midi_converter_set_source_cc()` - Set source CC number
- `midi_converter_set_dest_cc()` - Set destination CC number
- `midi_converter_set_scale()` - Set scale factor (0-200%)
- `midi_converter_set_offset()` - Set offset (-64 to +63)
- `midi_converter_set_invert()` - Set invert flag

### Processing

- `midi_converter_process_cc()` - Process CC message
- `midi_converter_process_aftertouch()` - Process aftertouch
- `midi_converter_process_pitchbend()` - Process pitchbend
- `midi_converter_process_velocity()` - Process note velocity

### Utilities

- `midi_converter_reset()` - Reset track state
- `midi_converter_get_mode_name()` - Get mode name string

## Conversion Modes

### Aftertouch → CC
Converts channel aftertouch pressure to CC messages. Useful for devices without aftertouch support.

### CC → Aftertouch
Converts CC messages to aftertouch. Useful for controlling aftertouch-only parameters.

### Pitchbend → CC
Converts pitchbend (14-bit) to CC (7-bit). Converts to MSB (upper 7 bits).

### CC → Pitchbend
Converts CC to pitchbend. CC value is shifted to create 14-bit pitchbend.

### Velocity → CC
Converts note velocity to CC on each note on. Useful for velocity-to-parameter mapping.

### ModWheel → CC
Convenience mode for converting modwheel (CC 1) to another CC.

### CC → CC
General CC-to-CC converter with transformations. Remap any CC to any other CC.

## Transformations

Transformations are applied in this order:
1. **Invert** - If enabled, value = 127 - value
2. **Scale** - value = value × (scale / 100)
3. **Offset** - value = value + offset
4. **Clamp** - Ensure 0-127 range

## Integration

1. Include header in your project
2. Call `midi_converter_init()` at startup
3. Set callbacks for desired output types
4. Configure conversion modes per track
5. Process incoming MIDI messages through the module

## Common Applications

- **Aftertouch to filter**: AT → CC 74 for synths without AT support
- **Expression to volume**: CC 11 → CC 7 with scaling
- **Velocity dynamics**: Velocity → CC for dynamic parameter control
- **Pitchbend alternative**: PB → CC for CC-only targets
- **Inverted controls**: Any message inverted for reverse control
- **Range compression**: Scale to fit smaller ranges
- **Offset centering**: Shift ranges to different positions

## Notes

- Pitchbend is 14-bit (0-16383), converted to/from 7-bit CC using MSB
- All transformations clamp output to valid MIDI range (0-127)
- Source CC must match for CC-based conversions
- Output callbacks are only called when enabled and mode matches

## License

Part of MidiCore project
