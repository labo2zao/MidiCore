# Swing/Groove MIDI FX Module

## Overview

The Swing/Groove module provides sophisticated timing adjustments to add musical feel and human-like groove to MIDI sequences. It applies subtle timing shifts based on note position within the beat, creating various rhythmic feels from classic swing to shuffle and custom grooves.

## Features

- **Per-track configuration** (4 tracks max)
- **Swing amount** 0-100% (50% = no swing, >50% = swing late, <50% = swing early)
- **Multiple groove templates:**
  - Straight (50% timing, no swing)
  - Swing (classic 66% swing feel)
  - Shuffle (heavy 75% shuffle)
  - Triplet (triplet feel with 67% spacing)
  - Dotted (dotted 8th feel)
  - Half-Time (half-time shuffle pattern)
  - Custom (user-defined patterns)
- **Configurable resolution:** 8th notes, 16th notes, or 32nd notes
- **Swing depth control:** Percentage of beats affected by swing
- **Custom groove patterns:** Define up to 16-step timing patterns
- **Tempo-aware:** Automatically scales timing based on BPM

## API Reference

### Initialization

```c
void swing_init(uint16_t tempo);
```
Initialize the swing module with the specified tempo (BPM).

### Tempo Control

```c
void swing_set_tempo(uint16_t tempo);
uint16_t swing_get_tempo(void);
```
Update and query the current tempo (20-300 BPM).

### Enable/Disable

```c
void swing_set_enabled(uint8_t track, uint8_t enabled);
uint8_t swing_is_enabled(uint8_t track);
```
Enable or disable swing for a specific track (0-3).

### Swing Amount

```c
void swing_set_amount(uint8_t track, uint8_t amount);
uint8_t swing_get_amount(uint8_t track);
```
Set swing amount (0-100):
- **50** = No swing (straight timing)
- **>50** = Swing late (e.g., 66 for classic swing)
- **<50** = Swing early (reverse swing)

### Groove Templates

```c
void swing_set_groove(uint8_t track, swing_groove_t groove);
swing_groove_t swing_get_groove(uint8_t track);
const char* swing_get_groove_name(swing_groove_t groove);
```

Available groove types:
- `SWING_GROOVE_STRAIGHT` - No swing
- `SWING_GROOVE_SWING` - Classic swing (66%)
- `SWING_GROOVE_SHUFFLE` - Heavy shuffle (75%)
- `SWING_GROOVE_TRIPLET` - Triplet feel
- `SWING_GROOVE_DOTTED` - Dotted 8th feel
- `SWING_GROOVE_HALF_TIME` - Half-time shuffle
- `SWING_GROOVE_CUSTOM` - User-defined pattern

### Resolution

```c
void swing_set_resolution(uint8_t track, swing_resolution_t resolution);
swing_resolution_t swing_get_resolution(uint8_t track);
const char* swing_get_resolution_name(swing_resolution_t resolution);
```

Resolution types:
- `SWING_RESOLUTION_8TH` - Swing 8th notes
- `SWING_RESOLUTION_16TH` - Swing 16th notes
- `SWING_RESOLUTION_32ND` - Swing 32nd notes

### Depth Control

```c
void swing_set_depth(uint8_t track, uint8_t depth);
uint8_t swing_get_depth(uint8_t track);
```
Set depth percentage (0-100) - controls what percentage of beats are affected by swing.

### Timing Calculation

```c
int16_t swing_calculate_offset(uint8_t track, uint32_t tick_position, uint16_t ppqn);
int16_t swing_calculate_offset_ms(uint8_t track, uint32_t time_ms);
```
Calculate timing offset for a note:
- Returns offset in milliseconds
- Positive values = delay the note
- Negative values = play the note earlier

### Custom Patterns

```c
void swing_set_custom_pattern(uint8_t track, const uint8_t* pattern, uint8_t length);
void swing_get_custom_pattern(uint8_t track, uint8_t* pattern, uint8_t* length);
```
Define custom 16-step groove patterns where each value is 0-100 (50 = no offset).

### Reset

```c
void swing_reset(uint8_t track);
void swing_reset_all(void);
```
Reset swing state for tracks.

## Usage Examples

### Basic Swing Setup

```c
// Initialize with 120 BPM
swing_init(120);

// Enable swing on track 0
swing_set_enabled(0, 1);

// Set classic swing feel (66%)
swing_set_groove(0, SWING_GROOVE_SWING);
swing_set_amount(0, 66);

// Swing 16th notes
swing_set_resolution(0, SWING_RESOLUTION_16TH);

// Full depth (all beats affected)
swing_set_depth(0, 100);
```

### Applying Swing to Notes

#### Using Tick Position (MIDI sequencer)

```c
// When playing a note at tick position
uint32_t tick_pos = 480;  // Example position
uint16_t ppqn = 96;        // Pulses per quarter note

// Calculate timing offset
int16_t offset_ms = swing_calculate_offset(0, tick_pos, ppqn);

// Apply offset: delay note by offset_ms milliseconds
if (offset_ms > 0) {
    delay_note(offset_ms);
} else if (offset_ms < 0) {
    // Play note earlier (negative offset)
    advance_note(-offset_ms);
}
```

#### Using Real Time

```c
// When playing a note at specific time
uint32_t current_time_ms = get_current_time_ms();

// Calculate timing offset
int16_t offset_ms = swing_calculate_offset_ms(0, current_time_ms);

// Apply offset to note timing
schedule_note(note, velocity, current_time_ms + offset_ms);
```

### Custom Groove Pattern

```c
// Define a custom 8-step pattern
uint8_t custom_pattern[8] = {
    50,  // Step 1: no offset
    60,  // Step 2: slight delay
    50,  // Step 3: no offset
    70,  // Step 4: more delay
    50,  // Step 5: no offset
    55,  // Step 6: tiny delay
    50,  // Step 7: no offset
    65   // Step 8: delay
};

swing_set_custom_pattern(0, custom_pattern, 8);
swing_set_groove(0, SWING_GROOVE_CUSTOM);
swing_set_amount(0, 60);  // Scale the effect
```

### Shuffle Feel

```c
// Heavy shuffle on hi-hats (track 1)
swing_set_enabled(1, 1);
swing_set_groove(1, SWING_GROOVE_SHUFFLE);
swing_set_amount(1, 75);
swing_set_resolution(1, SWING_RESOLUTION_8TH);
```

### Subtle Humanization

```c
// Light swing for humanization
swing_set_enabled(2, 1);
swing_set_groove(2, SWING_GROOVE_SWING);
swing_set_amount(2, 55);  // Very subtle
swing_set_depth(2, 70);   // Only 70% of beats
swing_set_resolution(2, SWING_RESOLUTION_16TH);
```

## Technical Details

### Timing Calculation

The module calculates timing offsets based on:

1. **Position in beat:** Determines which subdivision is being played
2. **Groove pattern:** Each pattern defines timing for 16 subdivisions
3. **Swing amount:** Scales the effect intensity (50 = neutral)
4. **Depth:** Controls percentage of beats affected
5. **Resolution:** Determines subdivision size (8th, 16th, 32nd)

**Offset Range:** ±25% of subdivision length (tempo-dependent)

### Groove Patterns

Patterns are arrays of 16 values (0-100):
- **50** = No timing adjustment
- **>50** = Delay (push beat later)
- **<50** = Advance (pull beat earlier)

Example: Classic swing pattern
```
[50, 66, 50, 66, 50, 66, 50, 66, ...]
```
This delays every other subdivision by 16% (66-50).

### Performance Considerations

- Lightweight calculations suitable for real-time use
- No dynamic memory allocation
- Minimal CPU overhead per note
- Tempo changes take effect immediately

## Integration with MidiCore

The swing module is designed to integrate seamlessly with MidiCore's architecture:

1. **Initialize** during system startup
2. **Configure** per-track settings via UI or MIDI CC
3. **Apply timing offsets** in the MIDI output pipeline
4. **Update tempo** when receiving MIDI clock or tempo changes

## Common Use Cases

### Jazz/Swing Music
```c
swing_set_groove(track, SWING_GROOVE_SWING);
swing_set_amount(track, 66);
swing_set_resolution(track, SWING_RESOLUTION_8TH);
```

### Hip-Hop/R&B Shuffle
```c
swing_set_groove(track, SWING_GROOVE_SHUFFLE);
swing_set_amount(track, 70);
swing_set_resolution(track, SWING_RESOLUTION_16TH);
```

### Electronic Dance Music (Subtle Groove)
```c
swing_set_groove(track, SWING_GROOVE_SWING);
swing_set_amount(track, 52);  // Very subtle
swing_set_depth(track, 80);
swing_set_resolution(track, SWING_RESOLUTION_16TH);
```

### Triplet-Based Feel
```c
swing_set_groove(track, SWING_GROOVE_TRIPLET);
swing_set_amount(track, 67);
swing_set_resolution(track, SWING_RESOLUTION_8TH);
```

## Notes

- Swing is applied in real-time to note timing
- Does not modify note pitch, velocity, or duration
- Multiple tracks can have different swing settings
- Works with any tempo (20-300 BPM)
- Compatible with all MidiCore MIDI processing features

## Version History

- **1.0.0** - Initial release
  - Basic swing/groove functionality
  - 7 preset groove templates
  - Custom pattern support
  - Per-track configuration
