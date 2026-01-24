# Envelope CC Module

ADSR Envelope Generator to CC output for MidiCore MIDI FX suite.

## Overview

The envelope_cc module generates ADSR (Attack/Decay/Sustain/Release) envelopes and outputs them as MIDI CC messages. Perfect for modulating synth parameters like filter cutoff, resonance, or any other CC-controllable parameter with envelope control.

## Features

- **ADSR Envelope Generator**
  - Attack time: 0-5000ms
  - Decay time: 0-5000ms
  - Sustain level: 0-127
  - Release time: 0-5000ms
- **Output to any CC number** (0-127)
- **Min/max value ranges** for output scaling
- **Per-track configuration** (4 tracks)
- **Trigger/Release functions** for note-based control
- **1ms tick** for smooth envelope calculation
- **Callback system** for CC output

## Usage Example

```c
#include "Services/envelope_cc/envelope_cc.h"

// Callback for CC output
void my_cc_callback(uint8_t track, uint8_t cc_number, uint8_t cc_value, uint8_t channel) {
    MIDI_SendCC(channel, cc_number, cc_value);
}

// Initialize
envelope_cc_init();
envelope_cc_set_callback(my_cc_callback);

// Configure track 0: Filter envelope on CC 74
envelope_cc_set_enabled(0, 1);
envelope_cc_set_channel(0, 0);              // MIDI channel 1
envelope_cc_set_cc_number(0, 74);           // CC 74 = Filter Cutoff
envelope_cc_set_attack(0, 50);              // 50ms attack
envelope_cc_set_decay(0, 200);              // 200ms decay
envelope_cc_set_sustain(0, 80);             // Sustain at 80
envelope_cc_set_release(0, 400);            // 400ms release
envelope_cc_set_min_value(0, 20);           // Start at 20
envelope_cc_set_max_value(0, 127);          // Peak at 127

// Trigger on note on
envelope_cc_trigger(0);

// Release on note off
envelope_cc_release(0);

// Call tick function every 1ms
envelope_cc_tick(current_time_ms);
```

## API Reference

### Initialization

- `envelope_cc_init()` - Initialize module
- `envelope_cc_set_callback()` - Set CC output callback

### Configuration

- `envelope_cc_set_enabled()` - Enable/disable per track
- `envelope_cc_set_channel()` - Set MIDI channel
- `envelope_cc_set_cc_number()` - Set CC number to modulate
- `envelope_cc_set_attack()` - Set attack time (0-5000ms)
- `envelope_cc_set_decay()` - Set decay time (0-5000ms)
- `envelope_cc_set_sustain()` - Set sustain level (0-127)
- `envelope_cc_set_release()` - Set release time (0-5000ms)
- `envelope_cc_set_min_value()` - Set minimum output value
- `envelope_cc_set_max_value()` - Set maximum output value

### Control

- `envelope_cc_trigger()` - Start envelope (begins attack)
- `envelope_cc_release()` - Release envelope (begins release)
- `envelope_cc_tick()` - 1ms tick processor (updates envelope)

### Status

- `envelope_cc_get_stage()` - Get current stage (IDLE/ATTACK/DECAY/SUSTAIN/RELEASE)
- `envelope_cc_get_value()` - Get current envelope value
- `envelope_cc_reset()` - Reset track state

## Envelope Stages

1. **IDLE** - Envelope is inactive
2. **ATTACK** - Rising from min to max value
3. **DECAY** - Falling from max to sustain level
4. **SUSTAIN** - Holding at sustain level
5. **RELEASE** - Falling from sustain to min value

## Integration

1. Include header in your project
2. Call `envelope_cc_init()` at startup
3. Set callback for CC output
4. Configure ADSR parameters per track
5. Call `envelope_cc_trigger()` on note on events
6. Call `envelope_cc_release()` on note off events
7. Call `envelope_cc_tick()` every 1ms from timer interrupt

## Common Applications

- **Filter envelope**: Modulate filter cutoff (CC 74)
- **Amplitude envelope**: Modulate expression (CC 11)
- **Resonance sweep**: Modulate resonance (CC 71)
- **Pan automation**: Modulate pan (CC 10)
- **Custom modulation**: Any CC-controllable parameter

## Notes

- All timing uses linear interpolation
- CC values are only sent when they change (reduces MIDI traffic)
- Zero attack/decay/release time = instant transition
- Sustain phase continues until release is called
- Default CC is 74 (filter cutoff)

## License

Part of MidiCore project
