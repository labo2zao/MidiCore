# Timing Quantizer Module

## Overview

The Timing Quantizer module provides precise MIDI note timing correction by snapping note events to a rhythmic grid. It supports per-track configuration with multiple grid resolutions, adjustable quantization strength, and intelligent handling of early and late notes.

## Features

- **Per-Track Configuration**: Independent settings for up to 4 tracks
- **Multiple Grid Resolutions**: 1/4, 1/8, 1/16, 1/32, 1/64 notes plus triplet variations
- **Adjustable Quantize Strength**: 0-100% (100% = hard snap, lower values = groove feel)
- **Look-Ahead Window**: Buffer early notes for precise quantization
- **Flexible Late Note Handling**: Snap forward, backward, nearest, or disable
- **Swing Support**: Add swing feel to quantized timing
- **Tempo Sync**: Automatically adjusts to BPM changes
- **Statistics Tracking**: Monitor quantization performance per track

## Grid Resolutions

| Resolution | Description | Ticks (PPQN=96) |
|-----------|-------------|-----------------|
| `QUANTIZER_RES_QUARTER` | Quarter note | 96 |
| `QUANTIZER_RES_8TH` | Eighth note | 48 |
| `QUANTIZER_RES_8TH_TRIPLET` | Eighth triplet | 32 |
| `QUANTIZER_RES_16TH` | Sixteenth note | 24 |
| `QUANTIZER_RES_16TH_TRIPLET` | Sixteenth triplet | 16 |
| `QUANTIZER_RES_32ND` | 32nd note | 12 |
| `QUANTIZER_RES_32ND_TRIPLET` | 32nd triplet | 8 |
| `QUANTIZER_RES_64TH` | 64th note | 6 |

## Late Note Handling Modes

- **Nearest**: Snap to closest grid point (default)
- **Forward**: Always snap to next grid point
- **Backward**: Always snap to previous grid point
- **Off**: Don't quantize late notes (pass through)

## Usage Examples

### Basic Initialization

```c
#include "Services/quantizer/quantizer.h"

// Initialize with 120 BPM and 96 PPQN
quantizer_init(120, 96);

// Enable quantizer for track 0
quantizer_set_enabled(0, 1);

// Set to 16th note grid
quantizer_set_resolution(0, QUANTIZER_RES_16TH);

// Set 100% quantize strength (hard snap)
quantizer_set_strength(0, 100);
```

### Soft Quantization (Groove Feel)

```c
// Enable track 1 with softer quantization
quantizer_set_enabled(1, 1);
quantizer_set_resolution(1, QUANTIZER_RES_16TH);

// 70% strength preserves some human timing
quantizer_set_strength(1, 70);

// Add slight swing feel
quantizer_set_swing(1, 60);  // 60% = subtle swing
```

### Processing Notes with Timing

```c
// Process a note-on event
uint8_t note = 60;  // Middle C
uint8_t velocity = 100;
uint8_t channel = 0;
uint32_t time_ms = 1234;  // Current time in milliseconds

// Add note to quantization buffer
if (quantizer_process_note_on(0, note, velocity, channel, time_ms)) {
    // Note successfully buffered for quantization
}

// Later, check for notes ready to play
quantizer_note_t ready_notes[QUANTIZER_MAX_NOTES_PER_TRACK];
uint8_t count = quantizer_get_ready_notes(0, current_time_ms, ready_notes);

for (uint8_t i = 0; i < count; i++) {
    // Play the quantized note
    midi_send_note_on(ready_notes[i].channel, 
                      ready_notes[i].note, 
                      ready_notes[i].velocity);
}
```

### Using Tick-Based Timing

```c
// Process note using MIDI ticks instead of milliseconds
uint32_t tick_position = 384;  // 1 bar at PPQN=96

if (quantizer_process_note_on_ticks(0, note, velocity, channel, tick_position)) {
    // Note buffered
}

// Calculate quantized tick position directly
uint32_t original_ticks = 387;  // Slightly off grid
uint32_t quantized_ticks = quantizer_calculate_ticks(0, original_ticks);
// Result: 384 (snapped to grid)
```

### Direct Time Calculation (No Buffering)

```c
// Calculate quantized time without buffering
uint32_t original_time = 1237;  // Slightly off grid
uint32_t quantized_time = quantizer_calculate_time(0, original_time);

// Get the timing offset applied
int32_t offset = quantizer_get_offset(0, original_time);
// Positive = note delayed, Negative = note advanced
```

### Look-Ahead Configuration

```c
// Set 100ms look-ahead window for early notes
quantizer_set_lookahead(0, 100);

// Notes arriving up to 100ms before the grid point
// will be held and played at the correct time
```

### Late Note Handling

```c
// Always push late notes forward to next grid
quantizer_set_late_mode(0, QUANTIZER_LATE_SNAP_FORWARD);

// Or snap backward to maintain tight timing
quantizer_set_late_mode(1, QUANTIZER_LATE_SNAP_BACKWARD);

// Or find nearest grid (default)
quantizer_set_late_mode(2, QUANTIZER_LATE_SNAP_NEAREST);
```

### Grid Information

```c
// Get grid interval
uint32_t interval_ms = quantizer_get_grid_interval_ms(0);
uint32_t interval_ticks = quantizer_get_grid_interval_ticks(0);

// Find next/previous grid points
uint32_t current_time = 1234;
uint32_t next_grid = quantizer_get_next_grid(0, current_time);
uint32_t prev_grid = quantizer_get_prev_grid(0, current_time);

// Check if time is on grid
uint16_t tolerance = 5;  // 5ms tolerance
if (quantizer_is_on_grid(0, current_time, tolerance)) {
    // Time is close to grid point
}
```

### Statistics and Monitoring

```c
// Get quantization statistics
uint8_t buffered;
uint32_t total_quantized;
int32_t avg_offset;

quantizer_get_stats(0, &buffered, &total_quantized, &avg_offset);

printf("Track 0: %d notes buffered, %d total quantized\n", 
       buffered, total_quantized);
printf("Average offset: %d ms\n", avg_offset);
```

### Tempo Changes

```c
// Update tempo (recalculates all grid timing)
quantizer_set_tempo(140);  // Change to 140 BPM

// Get current tempo
uint16_t tempo = quantizer_get_tempo();
```

### Multiple Tracks Example

```c
// Configure different quantization per track
void setup_multi_track_quantizer(void) {
    quantizer_init(120, 96);
    
    // Track 0: Tight 16th note quantization (drums)
    quantizer_set_enabled(0, 1);
    quantizer_set_resolution(0, QUANTIZER_RES_16TH);
    quantizer_set_strength(0, 100);  // Hard snap
    quantizer_set_late_mode(0, QUANTIZER_LATE_SNAP_NEAREST);
    
    // Track 1: Soft 8th note quantization (bass)
    quantizer_set_enabled(1, 1);
    quantizer_set_resolution(1, QUANTIZER_RES_8TH);
    quantizer_set_strength(1, 80);   // Slight groove
    quantizer_set_swing(1, 55);      // Subtle swing
    
    // Track 2: Triplet feel (hi-hats)
    quantizer_set_enabled(2, 1);
    quantizer_set_resolution(2, QUANTIZER_RES_16TH_TRIPLET);
    quantizer_set_strength(2, 90);
    
    // Track 3: Disabled (manual timing)
    quantizer_set_enabled(3, 0);
}
```

### Reset and Cleanup

```c
// Reset single track (clears buffer, keeps settings)
quantizer_reset(0);

// Reset all tracks
quantizer_reset_all();
```

## Integration with MidiCore

The quantizer module integrates seamlessly with other MidiCore services:

```c
// Example: Quantize + Swing
quantizer_init(120, 96);
swing_init(120);

// Configure quantizer for tight grid
quantizer_set_enabled(0, 1);
quantizer_set_resolution(0, QUANTIZER_RES_16TH);
quantizer_set_strength(0, 100);

// Add swing feel on top
swing_set_enabled(0, 1);
swing_set_amount(0, 65);  // Swing amount

// Process note through both
uint32_t time = quantizer_calculate_time(0, original_time);
int16_t swing_offset = swing_calculate_offset_ms(0, time);
uint32_t final_time = time + swing_offset;
```

## Performance Considerations

- **Buffer Size**: Each track can buffer up to 16 notes
- **CPU Usage**: Minimal - simple integer math operations
- **Memory**: ~200 bytes per track
- **Latency**: Look-ahead window adds configurable latency (0-500ms)

## Configuration Tips

### For Tight Electronic Music
```c
quantizer_set_resolution(track, QUANTIZER_RES_16TH);
quantizer_set_strength(track, 100);
quantizer_set_late_mode(track, QUANTIZER_LATE_SNAP_NEAREST);
quantizer_set_lookahead(track, 50);
```

### For Natural Groove
```c
quantizer_set_resolution(track, QUANTIZER_RES_8TH);
quantizer_set_strength(track, 60);  // Preserve some human feel
quantizer_set_swing(track, 60);     // Add swing
quantizer_set_late_mode(track, QUANTIZER_LATE_SNAP_NEAREST);
```

### For Live Performance
```c
quantizer_set_resolution(track, QUANTIZER_RES_16TH);
quantizer_set_strength(track, 80);
quantizer_set_lookahead(track, 100);  // More forgiving
quantizer_set_late_mode(track, QUANTIZER_LATE_SNAP_FORWARD);  // Stay in time
```

## API Reference

See `quantizer.h` for complete API documentation including:
- Initialization and configuration functions
- Note processing and buffering
- Grid calculation utilities
- Statistics and monitoring
- Name string helpers

## Thread Safety

The quantizer module is **not thread-safe**. If using in a multi-threaded environment, external synchronization is required.

## Limitations

- Maximum 4 tracks (expandable via `QUANTIZER_MAX_TRACKS`)
- Maximum 16 notes buffered per track
- Tempo range: 20-300 BPM
- Look-ahead window: 0-500ms
- Note-off events are not quantized (only note-on)

## License

Part of MidiCore - see main project LICENSE file.
