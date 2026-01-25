# CC Smoother Module

## Overview

The CC Smoother module eliminates zipper noise and staircase effects in MIDI Control Change (CC) messages by applying intelligent smoothing algorithms. This is essential for smooth parameter changes in filter sweeps, volume automation, and other continuous controller modulations.

## Features

- **Exponential Moving Average (EMA)**: High-quality smoothing algorithm that provides natural-sounding transitions
- **Per-Track Configuration**: Independent settings for 4 tracks
- **All 128 CC Numbers**: Each CC number can be smoothed independently
- **Multiple Smoothing Modes**: Light, Medium, Heavy, and Custom presets
- **Attack/Release Times**: Separate control for increasing vs. decreasing values
- **Slew Rate Limiting**: Optional maximum rate of change limiting
- **Low Latency**: Efficient 1ms update rate
- **Selective Smoothing**: Enable/disable smoothing per CC number (e.g., exclude buttons/switches)

## Usage

### Basic Initialization

```c
#include "Services/cc_smoother/cc_smoother.h"

// Initialize the module
cc_smoother_init();

// Enable smoothing for track 0
cc_smoother_set_enabled(0, 1);

// Set smoothing mode
cc_smoother_set_mode(0, CC_SMOOTH_MODE_MEDIUM);
```

### Processing CC Messages

```c
// Process incoming CC message
uint8_t track = 0;
uint8_t cc_number = 74;  // Filter cutoff
uint8_t input_value = 100;

// Get smoothed value
uint8_t smoothed = cc_smoother_process(track, cc_number, input_value);

// Send smoothed CC value to MIDI output
send_midi_cc(channel, cc_number, smoothed);
```

### Periodic Updates

The module requires periodic calls to `cc_smoother_tick_1ms()` for continuous smoothing:

```c
// In your 1ms timer interrupt or main loop
void timer_1ms_handler(void) {
    cc_smoother_tick_1ms();
}
```

### Using Output Callback

For automatic output of smoothed values:

```c
void my_cc_output(uint8_t track, uint8_t cc_number, uint8_t value, uint8_t channel) {
    // Send smoothed CC to MIDI output
    midi_send_cc(channel, cc_number, value);
}

// Register callback
cc_smoother_set_output_callback(my_cc_output);
```

### Custom Configuration

```c
// Enable custom mode for precise control
cc_smoother_set_mode(0, CC_SMOOTH_MODE_CUSTOM);

// Set custom parameters
cc_smoother_set_amount(0, 75);        // 75% smoothing
cc_smoother_set_attack(0, 30);        // 30ms attack time
cc_smoother_set_release(0, 150);      // 150ms release time
cc_smoother_set_slew_limit(0, 50);    // Limit to 50 units/ms max change

// Disable smoothing for specific CCs (e.g., sustain pedal)
cc_smoother_set_cc_enabled(0, 64, 0);  // CC 64 (sustain) - no smoothing
```

## Smoothing Modes

| Mode | Attack Time | Release Time | Description | Use Case |
|------|-------------|--------------|-------------|----------|
| **Off** | - | - | No smoothing (pass-through) | Testing, debugging |
| **Light** | 20ms | 30ms | Fast response, minimal latency | Performance controllers, real-time |
| **Medium** | 50ms | 100ms | Balanced smoothing | General purpose, most scenarios |
| **Heavy** | 100ms | 200ms | Very smooth, slower response | Slow modulations, automation |
| **Custom** | User-defined | User-defined | Full manual control | Special requirements |

## Algorithm Details

### Exponential Moving Average (EMA)

The module uses an EMA filter with the following formula:

```
y[n] = α × target + (1 - α) × y[n-1]
```

Where:
- `α` (alpha) is the smoothing coefficient (0.0 to 1.0)
- `target` is the input CC value
- `y[n]` is the current smoothed output
- `y[n-1]` is the previous smoothed output

The coefficient α is calculated from the time constant:

```
α = 1 - exp(-Δt / τ)
```

Where:
- `Δt` is the update interval (1ms)
- `τ` (tau) is the time constant (attack or release time in ms)

### Attack vs. Release

The module automatically selects attack or release time constants based on the direction of change:
- **Attack**: Used when CC value is increasing
- **Release**: Used when CC value is decreasing

This mimics the behavior of analog synthesizer envelopes and sounds more natural for many parameters.

### Slew Rate Limiting

Optional slew rate limiting restricts the maximum rate of change:

```
max_change_per_ms = slew_limit
if (|change| > max_change):
    change = sign(change) × max_change
```

This prevents sudden jumps even with fast attack/release settings.

## Common CC Numbers

Here are some commonly smoothed CC numbers:

| CC # | Name | Typical Use |
|------|------|-------------|
| 1 | Modulation Wheel | Vibrato, modulation depth |
| 7 | Volume | Channel volume |
| 10 | Pan | Stereo position |
| 11 | Expression | Dynamic expression |
| 71 | Resonance | Filter resonance |
| 74 | Brightness/Cutoff | Filter cutoff frequency |
| 73 | Attack Time | Envelope attack |
| 75 | Decay Time | Envelope decay |
| 76 | Vibrato Rate | LFO rate |
| 77 | Vibrato Depth | LFO depth |

**Note**: Some CCs should NOT be smoothed:
- CC 64 (Sustain Pedal) - Binary on/off
- CC 120-127 (Channel Mode Messages) - Commands, not continuous values

## Performance Considerations

- **Memory Usage**: ~33KB RAM (4 tracks × 128 CCs × 64 bytes per state)
- **CPU Usage**: Minimal - approximately 1-2% on STM32F4 @ 168MHz
- **Update Rate**: 1ms recommended for smooth results
- **Latency**: 
  - Light mode: ~20-30ms
  - Medium mode: ~50-100ms
  - Heavy mode: ~100-200ms

## Best Practices

1. **Choose the Right Mode**:
   - Use Light mode for real-time performance controls
   - Use Medium mode for general automation
   - Use Heavy mode for slow, cinematic modulations

2. **Disable Smoothing for Binary CCs**:
   ```c
   cc_smoother_set_cc_enabled(track, 64, 0);  // Sustain pedal
   cc_smoother_set_cc_enabled(track, 65, 0);  // Portamento on/off
   ```

3. **Reset on Patch Changes**:
   ```c
   // When loading a new patch/preset
   cc_smoother_reset_track(track);
   ```

4. **Adjust Attack/Release for Different Parameters**:
   - Fast attack, slow release for volume/expression
   - Slow attack and release for filter sweeps
   - Fast attack and release for performance controls

## Example: Complete Setup

```c
#include "Services/cc_smoother/cc_smoother.h"

void setup_cc_smoothing(void) {
    // Initialize module
    cc_smoother_init();
    
    // Configure Track 0 (Lead Synth)
    cc_smoother_set_enabled(0, 1);
    cc_smoother_set_mode(0, CC_SMOOTH_MODE_LIGHT);
    cc_smoother_set_cc_enabled(0, 64, 0);  // No smoothing on sustain
    
    // Configure Track 1 (Pad)
    cc_smoother_set_enabled(1, 1);
    cc_smoother_set_mode(1, CC_SMOOTH_MODE_HEAVY);
    cc_smoother_set_attack(1, 200);   // Slow attack for pads
    cc_smoother_set_release(1, 300);  // Slow release
    
    // Configure Track 2 (Bass) - Custom settings
    cc_smoother_set_enabled(2, 1);
    cc_smoother_set_mode(2, CC_SMOOTH_MODE_CUSTOM);
    cc_smoother_set_amount(2, 60);
    cc_smoother_set_attack(2, 40);
    cc_smoother_set_release(2, 80);
    cc_smoother_set_slew_limit(2, 64);  // Moderate slew limiting
    
    // Set output callback
    cc_smoother_set_output_callback(my_midi_output);
}

// Process incoming CC
void on_midi_cc_received(uint8_t channel, uint8_t cc_number, uint8_t value) {
    uint8_t track = channel_to_track(channel);
    uint8_t smoothed = cc_smoother_process(track, cc_number, value);
    
    // Send to synth engine
    synth_set_cc(track, cc_number, smoothed);
}

// Timer callback (1ms)
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        cc_smoother_tick_1ms();
    }
}
```

## Troubleshooting

### Issue: CC values lag behind input
**Solution**: Decrease attack/release times or use a lighter smoothing mode

### Issue: Still hearing zipper noise
**Solution**: Increase smoothing amount or use a heavier mode

### Issue: Binary switches (sustain) sound weird
**Solution**: Disable smoothing for that specific CC number:
```c
cc_smoother_set_cc_enabled(track, 64, 0);
```

### Issue: High CPU usage
**Solution**: Reduce the number of active CCs or decrease update rate

## API Reference

See `cc_smoother.h` for complete API documentation with detailed parameter descriptions and return values.

## Integration with MidiCore

The CC Smoother integrates seamlessly with other MidiCore modules:

- **MIDI Router**: Process CCs before routing
- **MIDI Filter**: Filter before or after smoothing
- **Harmonizer**: Smooth CC for harmony voices
- **LFO Module**: Smooth LFO-generated CCs
- **Expression Module**: Smooth expression pedal input

## Version History

- **v1.0.0** (2024-01-24): Initial implementation
  - EMA smoothing algorithm
  - 5 smoothing modes
  - Attack/release times
  - Slew rate limiting
  - Per-track and per-CC configuration

## License

Part of the MidiCore project. See main project LICENSE file.

## Author

MidiCore Development Team

## See Also

- `Services/midi_filter/` - MIDI message filtering
- `Services/velocity/` - Velocity curve processing
- `Services/expression/` - Expression pedal handling
