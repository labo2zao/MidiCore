# Strum Effect Module

## Overview

The Strum Effect module staggers MIDI chord notes to simulate guitar or harp strumming. Notes are progressively delayed based on the strum direction and timing configuration, creating a natural, expressive playing style.

## Features

- **Per-Track Configuration**: Independent settings for up to 4 MIDI tracks
- **Configurable Strum Time**: 0-200ms total duration to spread notes across
- **Multiple Directions**:
  - **Up**: Low to high notes (guitar upstroke)
  - **Down**: High to low notes (guitar downstroke)  
  - **Up-Down**: Alternates between up and down with each chord
  - **Random**: Randomizes note order for unpredictable effects
- **Velocity Ramping**: Optional velocity increase/decrease across the strum
- **MidiCore Compatible**: Uses stdint.h types and follows project conventions

## Usage

### Initialization

```c
#include "strum.h"

// Initialize the strum module (call once at startup)
strum_init();
```

### Basic Configuration

```c
// Enable strum on track 0
strum_set_enabled(0, 1);

// Set strum time to 40ms
strum_set_time(0, 40);

// Set direction to down (high to low)
strum_set_direction(0, STRUM_DIR_DOWN);
```

### Velocity Ramping

```c
// Enable velocity increase across strum
strum_set_velocity_ramp(0, STRUM_RAMP_INCREASE);

// Set ramp amount to 30% (velocity will vary by ±30%)
strum_set_ramp_amount(0, 30);
```

### Processing Notes

```c
// Example: Process a C major chord (C, E, G)
uint8_t chord_notes[] = {60, 64, 67};  // Must be sorted low to high
uint8_t chord_size = 3;
uint8_t delay_ms;
uint8_t new_velocity;

// Process each note in the chord
for (uint8_t i = 0; i < chord_size; i++) {
    strum_process_note(
        0,                    // track number
        chord_notes[i],       // note number
        100,                  // original velocity
        chord_notes,          // full chord array
        chord_size,           // number of notes in chord
        &delay_ms,            // output: delay for this note
        &new_velocity         // output: modified velocity
    );
    
    // Schedule note to play after delay_ms with new_velocity
    schedule_midi_note(chord_notes[i], new_velocity, delay_ms);
}
```

## API Reference

### Initialization

**`void strum_init(void)`**
- Initializes all tracks with default settings
- Default: disabled, 30ms time, down direction, no ramp

### Enable/Disable

**`void strum_set_enabled(uint8_t track, uint8_t enabled)`**
- Enable (1) or disable (0) strum effect for a track
- Track: 0-3

**`uint8_t strum_is_enabled(uint8_t track)`**
- Returns 1 if enabled, 0 if disabled

### Strum Time

**`void strum_set_time(uint8_t track, uint8_t time_ms)`**
- Set total strum duration in milliseconds
- Range: 0-200ms
- Values > 200 are clamped to 200

**`uint8_t strum_get_time(uint8_t track)`**
- Returns current strum time in milliseconds

### Direction

**`void strum_set_direction(uint8_t track, strum_direction_t direction)`**
- Set strum direction mode
- Values: `STRUM_DIR_UP`, `STRUM_DIR_DOWN`, `STRUM_DIR_UP_DOWN`, `STRUM_DIR_RANDOM`

**`strum_direction_t strum_get_direction(uint8_t track)`**
- Returns current direction mode

**`const char* strum_get_direction_name(strum_direction_t direction)`**
- Returns human-readable direction name

### Velocity Ramping

**`void strum_set_velocity_ramp(uint8_t track, strum_ramp_t ramp)`**
- Set velocity ramp mode
- Values: `STRUM_RAMP_NONE`, `STRUM_RAMP_INCREASE`, `STRUM_RAMP_DECREASE`

**`strum_ramp_t strum_get_velocity_ramp(uint8_t track)`**
- Returns current ramp mode

**`void strum_set_ramp_amount(uint8_t track, uint8_t amount)`**
- Set ramp amount (0-100%)
- Amount is percentage of velocity variation
- Example: 20% with velocity 100 → range 80-120

**`uint8_t strum_get_ramp_amount(uint8_t track)`**
- Returns current ramp amount

**`const char* strum_get_ramp_name(strum_ramp_t ramp)`**
- Returns human-readable ramp name

### Processing

**`void strum_process_note(...)`**
```c
void strum_process_note(uint8_t track, uint8_t note, uint8_t velocity,
                        const uint8_t* chord_notes, uint8_t chord_size,
                        uint8_t* delay_ms, uint8_t* new_velocity)
```
- Process a note through the strum effect
- **Inputs**:
  - `track`: Track number (0-3)
  - `note`: MIDI note number (0-127)
  - `velocity`: Original velocity (1-127)
  - `chord_notes`: Array of all notes in chord (sorted low to high)
  - `chord_size`: Number of notes (1-8)
- **Outputs**:
  - `delay_ms`: Calculated delay in milliseconds
  - `new_velocity`: Modified velocity after ramping

**`void strum_reset(uint8_t track)`**
- Reset strum state (useful for up-down mode)
- Call when changing patches or strum parameters

## Configuration Examples

### Guitar Downstroke (Folk Strum)
```c
strum_set_enabled(0, 1);
strum_set_time(0, 25);
strum_set_direction(0, STRUM_DIR_DOWN);
strum_set_velocity_ramp(0, STRUM_RAMP_INCREASE);
strum_set_ramp_amount(0, 15);
```

### Harp Arpeggio
```c
strum_set_enabled(0, 1);
strum_set_time(0, 80);
strum_set_direction(0, STRUM_DIR_UP);
strum_set_velocity_ramp(0, STRUM_RAMP_NONE);
```

### Mandolin Tremolo Effect
```c
strum_set_enabled(0, 1);
strum_set_time(0, 15);
strum_set_direction(0, STRUM_DIR_UP_DOWN);
strum_set_velocity_ramp(0, STRUM_RAMP_NONE);
```

### Unpredictable Texture
```c
strum_set_enabled(0, 1);
strum_set_time(0, 60);
strum_set_direction(0, STRUM_DIR_RANDOM);
strum_set_velocity_ramp(0, STRUM_RAMP_INCREASE);
strum_set_ramp_amount(0, 40);
```

## Implementation Details

### Delay Calculation

For a chord with N notes and strum time T:
- Each note gets delay: `T * note_position / (N - 1)`
- First note: 0ms delay
- Last note: T ms delay
- Intermediate notes: linearly interpolated

### Direction Modes

- **Up**: Notes played in array order (index 0 → N-1)
- **Down**: Notes played in reverse (index N-1 → 0)
- **Up-Down**: Alternates with each chord; uses state to track last direction
- **Random**: Each note gets random position (non-deterministic)

### Velocity Ramping

With ramp amount R (0-100%) and velocity V:
- Calculate max change: `ΔV = V * R / 100`
- **Increase**: Start at V-ΔV, end at V+ΔV
- **Decrease**: Start at V+ΔV, end at V-ΔV
- Velocity always clamped to valid MIDI range (1-127)

### Thread Safety

The module is **not thread-safe**. Ensure all functions are called from the same thread or add appropriate locking.

## Memory Usage

- **Static Memory**: ~24 bytes (4 tracks × 6 bytes per track)
- **Stack Usage**: Minimal (<32 bytes per function call)
- **No Dynamic Allocation**: All memory is static

## Performance

- **Processing Time**: O(N) where N is chord size (typically < 10 notes)
- **CPU Usage**: Negligible (simple arithmetic operations)
- **Typical Latency**: < 1µs per note on STM32F4

## Integration Notes

### With MIDI Router
```c
// In your MIDI processing callback
if (midi_event.type == NOTE_ON) {
    uint8_t delay, velocity;
    strum_process_note(track, note, velocity, chord, chord_size, 
                       &delay, &velocity);
    if (delay > 0) {
        schedule_delayed_note(note, velocity, delay);
    } else {
        send_note_immediately(note, velocity);
    }
}
```

### With Chord Module
```c
// When chord module generates notes
uint8_t chord_notes[6];
uint8_t count = chord_get_notes(root, type, chord_notes);

// Apply strum to generated chord
for (uint8_t i = 0; i < count; i++) {
    uint8_t delay, vel;
    strum_process_note(track, chord_notes[i], velocity, 
                       chord_notes, count, &delay, &vel);
    schedule_note(chord_notes[i], vel, delay);
}
```

## Limitations

- Maximum 8 notes per chord (defined by `STRUM_MAX_CHORD_NOTES`)
- Maximum 200ms strum time (defined by `STRUM_MAX_TIME_MS`)
- Single notes (chord_size = 1) pass through unmodified
- Random mode uses `rand()` - seed it appropriately for reproducibility

## License

Part of MidiCore project. See project LICENSE for details.
