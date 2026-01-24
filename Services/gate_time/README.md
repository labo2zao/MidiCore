# Gate Time Module

Note Length/Gate Time Control for MidiCore MIDI FX suite.

## Overview

The gate_time module provides dynamic control over MIDI note lengths (gate time). It can shorten or extend note durations using three different modes: percentage-based, fixed milliseconds, or fixed MIDI ticks.

## Features

- **3 Gate Time Modes:**
  - Percent mode: 10-200% of original length
  - Fixed milliseconds: Absolute time in ms
  - Fixed ticks: Based on MIDI tick count
- **Per-track configuration** (4 tracks)
- **Min/max length limits** for boundary control
- **Note buffer** (32 notes per track)
- **1ms tick processing** for accurate timing
- **Callback system** for note on/off output

## Usage Example

```c
#include "Services/gate_time/gate_time.h"

// Callback for note output
void my_note_callback(uint8_t track, uint8_t note, uint8_t velocity, uint8_t channel) {
    if (velocity > 0) {
        // Note on
        MIDI_SendNoteOn(channel, note, velocity);
    } else {
        // Note off
        MIDI_SendNoteOff(channel, note, 0);
    }
}

// Initialize
gate_time_init();
gate_time_set_callback(my_note_callback);

// Configure track 0: 50% gate time
gate_time_set_enabled(0, 1);
gate_time_set_mode(0, GATE_TIME_MODE_PERCENT);
gate_time_set_value(0, 50);  // 50% of original length
gate_time_set_min_length(0, 10);   // Minimum 10ms
gate_time_set_max_length(0, 2000); // Maximum 2s

// Configure track 1: Fixed 100ms staccato
gate_time_set_enabled(1, 1);
gate_time_set_mode(1, GATE_TIME_MODE_FIXED_MS);
gate_time_set_value(1, 100);  // Always 100ms

// Process incoming notes
gate_time_process_note_on(0, 60, 100, 0, current_time_ms);

// Call tick function every 1ms
gate_time_tick(current_time_ms);
```

## API Reference

### Initialization

- `gate_time_init()` - Initialize module
- `gate_time_set_callback()` - Set note output callback

### Configuration

- `gate_time_set_enabled()` - Enable/disable per track
- `gate_time_set_mode()` - Set mode (PERCENT/FIXED_MS/FIXED_TICKS)
- `gate_time_set_value()` - Set gate time value
- `gate_time_set_min_length()` - Set minimum length limit
- `gate_time_set_max_length()` - Set maximum length limit

### Processing

- `gate_time_process_note_on()` - Process incoming note on
- `gate_time_process_note_off()` - Process incoming note off
- `gate_time_tick()` - 1ms tick processor (checks for notes to end)

### Utilities

- `gate_time_calculate_length()` - Calculate adjusted gate time
- `gate_time_reset()` - Reset track state
- `gate_time_get_stats()` - Get track statistics

## Integration

1. Include header in your project
2. Call `gate_time_init()` at startup
3. Set callback for note output
4. Configure per-track settings
5. Process note events through the module
6. Call `gate_time_tick()` every 1ms from timer interrupt

## Notes

- Buffer holds up to 32 simultaneous notes per track
- Minimum gate time is 1ms (enforced internally)
- Fixed tick mode assumes 120 BPM, 96 PPQN for conversion
- All active notes are sent note-off on reset

## License

Part of MidiCore project
