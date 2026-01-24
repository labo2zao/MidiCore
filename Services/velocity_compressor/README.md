# MIDI Velocity Compressor/Limiter Module

## Overview

The Velocity Compressor module provides dynamic range control for MIDI velocity values. It compresses or limits velocity dynamics by reducing the difference between soft and loud notes, making performances more consistent and controlled.

## Features

- **Per-track configuration** - Independent settings for 4 tracks
- **Configurable threshold** - Set where compression begins (1-127)
- **Multiple compression ratios** - 1:1, 2:1, 3:1, 4:1, 6:1, 8:1, 10:1, ∞:1 (limiter)
- **Makeup gain** - Compensate for level loss after compression (-20 to +40)
- **Soft/hard knee** - Gradual or immediate compression onset
- **Min/max velocity caps** - Hard limits on output velocity range
- **Bypass mode** - Disable processing while retaining settings
- **Gain reduction metering** - Monitor compression amount

## Compression Basics

### Threshold
The velocity level above which compression is applied. Notes below the threshold pass through unchanged.
- Typical range: 60-100
- Lower = more notes compressed
- Higher = only loud notes compressed

### Ratio
Determines how much compression is applied to velocities above the threshold.
- **1:1** - No compression (bypass)
- **2:1** - Gentle (for every 2 units over threshold, output 1 unit)
- **4:1** - Medium (common starting point)
- **8:1** - Heavy compression
- **∞:1** - Limiting (hard ceiling at threshold)

### Makeup Gain
Adds gain after compression to restore overall level.
- Range: -20 to +40 velocity units
- Use to compensate for reduced peaks
- Typical values: +5 to +15 for 4:1 compression

### Knee
Controls how gradually compression engages around the threshold.
- **Hard knee** - Immediate compression at threshold (aggressive)
- **Soft knee** - Gradual compression around threshold (smooth, musical)

### Min/Max Velocity Caps
Hard limits applied after compression.
- **Min** - Ensures notes are never too soft (useful for consistent sound)
- **Max** - Prevents excessively loud notes (protection)

## Usage Example

```c
#include "Services/velocity_compressor/velocity_compressor.h"

// Initialize module
velocity_compressor_init();

// Configure track 0 with moderate compression
velocity_compressor_set_enabled(0, 1);
velocity_compressor_set_threshold(0, 64);           // Compress above vel 64
velocity_compressor_set_ratio(0, COMP_RATIO_4_1);   // 4:1 compression
velocity_compressor_set_makeup_gain(0, 10);         // +10 makeup gain
velocity_compressor_set_knee(0, COMP_KNEE_SOFT);    // Smooth onset

// Set safety caps
velocity_compressor_set_min_velocity(0, 10);        // Never below 10
velocity_compressor_set_max_velocity(0, 120);       // Never above 120

// Process incoming MIDI velocities
uint8_t input_vel = 100;
uint8_t output_vel = velocity_compressor_process(0, input_vel);

// Monitor gain reduction for metering
uint8_t reduction = velocity_compressor_get_gain_reduction(0, input_vel);
```

## API Reference

### Initialization

```c
void velocity_compressor_init(void);
```
Initialize the module with default settings for all tracks.

### Enable/Disable

```c
void velocity_compressor_set_enabled(uint8_t track, uint8_t enabled);
uint8_t velocity_compressor_is_enabled(uint8_t track);
```
Enable or bypass compression for a track.

### Threshold

```c
void velocity_compressor_set_threshold(uint8_t track, uint8_t threshold);
uint8_t velocity_compressor_get_threshold(uint8_t track);
```
Set compression threshold (1-127).

### Ratio

```c
void velocity_compressor_set_ratio(uint8_t track, velocity_comp_ratio_t ratio);
velocity_comp_ratio_t velocity_compressor_get_ratio(uint8_t track);
```
Set compression ratio. Available ratios:
- `COMP_RATIO_1_1` - No compression
- `COMP_RATIO_2_1` - 2:1
- `COMP_RATIO_3_1` - 3:1
- `COMP_RATIO_4_1` - 4:1
- `COMP_RATIO_6_1` - 6:1
- `COMP_RATIO_8_1` - 8:1
- `COMP_RATIO_10_1` - 10:1
- `COMP_RATIO_INF` - Limiter (∞:1)

### Makeup Gain

```c
void velocity_compressor_set_makeup_gain(uint8_t track, int8_t gain);
int8_t velocity_compressor_get_makeup_gain(uint8_t track);
```
Set makeup gain (-20 to +40).

### Knee

```c
void velocity_compressor_set_knee(uint8_t track, velocity_comp_knee_t knee);
velocity_comp_knee_t velocity_compressor_get_knee(uint8_t track);
```
Set knee type:
- `COMP_KNEE_HARD` - Immediate compression
- `COMP_KNEE_SOFT` - Gradual compression

### Velocity Caps

```c
void velocity_compressor_set_min_velocity(uint8_t track, uint8_t min_vel);
uint8_t velocity_compressor_get_min_velocity(uint8_t track);

void velocity_compressor_set_max_velocity(uint8_t track, uint8_t max_vel);
uint8_t velocity_compressor_get_max_velocity(uint8_t track);
```
Set minimum and maximum output velocity limits (1-127).

### Processing

```c
uint8_t velocity_compressor_process(uint8_t track, uint8_t velocity);
```
Main processing function. Returns compressed velocity value.

### Gain Reduction

```c
uint8_t velocity_compressor_get_gain_reduction(uint8_t track, uint8_t velocity);
```
Calculate gain reduction for metering/visualization (0 = no reduction).

### Utility Functions

```c
const char* velocity_compressor_get_ratio_name(velocity_comp_ratio_t ratio);
const char* velocity_compressor_get_knee_name(velocity_comp_knee_t knee);
```
Get human-readable names for ratios and knee types.

### Reset

```c
void velocity_compressor_reset_track(uint8_t track);
void velocity_compressor_reset_all(void);
```
Reset settings to defaults.

## Common Use Cases

### 1. Taming Dynamic Performances
Reduce the difference between soft and loud notes for more consistent playback.
```c
velocity_compressor_set_threshold(0, 70);
velocity_compressor_set_ratio(0, COMP_RATIO_3_1);
velocity_compressor_set_makeup_gain(0, 8);
velocity_compressor_set_knee(0, COMP_KNEE_SOFT);
```

### 2. Velocity Limiting
Prevent excessively loud notes from triggering harsh sounds.
```c
velocity_compressor_set_threshold(0, 100);
velocity_compressor_set_ratio(0, COMP_RATIO_INF);  // Limiter
velocity_compressor_set_knee(0, COMP_KNEE_SOFT);
```

### 3. Velocity Normalization
Ensure all notes fall within a specific range.
```c
velocity_compressor_set_enabled(0, 1);
velocity_compressor_set_min_velocity(0, 40);
velocity_compressor_set_max_velocity(0, 110);
velocity_compressor_set_ratio(0, COMP_RATIO_1_1);  // Just caps, no compression
```

### 4. Gentle Smoothing
Reduce dynamics slightly without obvious compression.
```c
velocity_compressor_set_threshold(0, 80);
velocity_compressor_set_ratio(0, COMP_RATIO_2_1);  // Gentle
velocity_compressor_set_makeup_gain(0, 5);
velocity_compressor_set_knee(0, COMP_KNEE_SOFT);
```

### 5. Aggressive Compression
Heavily reduce dynamic range for consistent output.
```c
velocity_compressor_set_threshold(0, 50);
velocity_compressor_set_ratio(0, COMP_RATIO_8_1);
velocity_compressor_set_makeup_gain(0, 20);
velocity_compressor_set_knee(0, COMP_KNEE_HARD);
```

## Technical Details

### Signal Flow
1. Input velocity received (1-127)
2. Normalize to 0.0-1.0 range
3. Apply compression curve based on threshold and ratio
4. Apply makeup gain
5. Denormalize to 1-127 range
6. Apply min/max caps
7. Output compressed velocity

### Compression Algorithm
The module uses a logarithmic-inspired compression algorithm that operates on normalized velocity values:

**Hard Knee:**
```
if (input <= threshold):
    output = input
else:
    overshoot = input - threshold
    output = threshold + (overshoot / ratio)
```

**Soft Knee:**
Uses a smooth quadratic transition zone (±6 velocity units) around the threshold for musical compression onset.

### Memory Usage
- Per-track configuration: ~8 bytes
- Total static memory: ~32 bytes (4 tracks)
- No dynamic allocation
- No state memory (stateless processing)

### Performance
- Computational complexity: O(1)
- Floating-point math for precision
- Suitable for real-time MIDI processing
- Thread-safe (no shared state between calls)

## Integration

### Adding to MidiCore Project

1. Include the header in your MIDI processing code:
```c
#include "Services/velocity_compressor/velocity_compressor.h"
```

2. Initialize during system startup:
```c
velocity_compressor_init();
```

3. Process MIDI note velocities:
```c
void process_note_on(uint8_t track, uint8_t note, uint8_t velocity) {
    velocity = velocity_compressor_process(track, velocity);
    // Send compressed velocity to output
}
```

### Building

The module is self-contained and only requires:
- Standard C library (`<stdint.h>`, `<string.h>`)
- Math library (`<math.h>` for `powf`)
- Link with `-lm` flag

Standalone compilation:
```bash
gcc -DSTANDALONE_TEST -o test velocity_compressor_test.c velocity_compressor.c -lm
```

## Testing

Run the included test suite:
```bash
cd Services/velocity_compressor
gcc -DSTANDALONE_TEST -o test velocity_compressor_test.c velocity_compressor.c -lm
./test
```

The test suite validates:
- Initialization and defaults
- Bypass functionality
- Threshold behavior
- All compression ratios
- Makeup gain
- Soft/hard knee
- Min/max caps
- Limiter mode
- Gain reduction calculation
- Multi-track independence

## Troubleshooting

### Output too quiet after compression
- Increase makeup gain
- Lower threshold (compress less)
- Use lower ratio

### Compression too aggressive
- Use soft knee instead of hard knee
- Lower ratio (2:1 or 3:1)
- Raise threshold

### Not hearing any effect
- Check that compression is enabled
- Verify input velocities exceed threshold
- Ensure ratio is not 1:1

### Notes sound inconsistent
- Check min/max caps are reasonable
- Verify makeup gain isn't excessive
- Consider using soft knee for smoother response

## License

Part of the MidiCore project. See project license for details.

## Version History

- **1.0.0** (2024) - Initial release
  - Per-track configuration (4 tracks)
  - 8 compression ratios including limiter
  - Soft/hard knee
  - Makeup gain
  - Min/max velocity caps
  - Comprehensive test suite
