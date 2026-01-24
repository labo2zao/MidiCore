# CC Smoother - Quick Start Guide

## 1. Minimal Setup (5 lines of code)

```c
#include "Services/cc_smoother/cc_smoother.h"

// Initialize once at startup
cc_smoother_init();
cc_smoother_set_enabled(0, 1);              // Enable track 0
cc_smoother_set_mode(0, CC_SMOOTH_MODE_MEDIUM);  // Use medium smoothing

// Process incoming CC messages
uint8_t smoothed = cc_smoother_process(0, cc_number, input_value);

// Call from 1ms timer
cc_smoother_tick_1ms();
```

## 2. Real-World Example: Filter Cutoff Smoothing

```c
void setup_filter_smoothing(void) {
    cc_smoother_init();
    
    // Track 0: Lead synth with light smoothing
    cc_smoother_set_enabled(0, 1);
    cc_smoother_set_mode(0, CC_SMOOTH_MODE_LIGHT);
    
    // Don't smooth sustain pedal (CC 64)
    cc_smoother_set_cc_enabled(0, 64, 0);
}

void on_midi_cc(uint8_t channel, uint8_t cc_num, uint8_t value) {
    // Smooth the CC value
    uint8_t smoothed = cc_smoother_process(0, cc_num, value);
    
    // Send to synth
    synth_set_parameter(cc_num, smoothed);
}

// In your 1ms timer interrupt
void TIM2_IRQHandler(void) {
    cc_smoother_tick_1ms();
}
```

## 3. Common Configurations

### Configuration A: Performance Controller (Low Latency)
```c
cc_smoother_set_mode(track, CC_SMOOTH_MODE_LIGHT);
// Attack: 20ms, Release: 30ms
```

### Configuration B: Automation (Smooth)
```c
cc_smoother_set_mode(track, CC_SMOOTH_MODE_HEAVY);
// Attack: 100ms, Release: 200ms
```

### Configuration C: Custom (Fine Control)
```c
cc_smoother_set_mode(track, CC_SMOOTH_MODE_CUSTOM);
cc_smoother_set_amount(track, 75);        // 75% smoothing
cc_smoother_set_attack(track, 30);        // 30ms attack
cc_smoother_set_release(track, 150);      // 150ms release
cc_smoother_set_slew_limit(track, 50);    // 50 units/ms max
```

## 4. Which CCs to Smooth?

### ✅ SMOOTH THESE:
- CC 1 (Modulation Wheel)
- CC 7 (Volume)
- CC 10 (Pan)
- CC 11 (Expression)
- CC 71 (Resonance)
- CC 74 (Filter Cutoff)
- CC 73-77 (Envelope/LFO controls)

### ❌ DON'T SMOOTH THESE:
```c
// Binary on/off switches
cc_smoother_set_cc_enabled(track, 64, 0);  // Sustain pedal
cc_smoother_set_cc_enabled(track, 65, 0);  // Portamento on/off
cc_smoother_set_cc_enabled(track, 66, 0);  // Sostenuto
```

## 5. Troubleshooting

### Problem: CC values lag behind
**Solution:** Use lighter mode or reduce attack/release times
```c
cc_smoother_set_mode(track, CC_SMOOTH_MODE_LIGHT);
```

### Problem: Still hearing zipper noise
**Solution:** Use heavier smoothing
```c
cc_smoother_set_mode(track, CC_SMOOTH_MODE_HEAVY);
```

### Problem: Sustain pedal feels sluggish
**Solution:** Disable smoothing for CC 64
```c
cc_smoother_set_cc_enabled(track, 64, 0);
```

## 6. Integration Checklist

- [ ] Add `#include "Services/cc_smoother/cc_smoother.h"` to your code
- [ ] Call `cc_smoother_init()` at startup
- [ ] Enable and configure desired tracks
- [ ] Process CC messages through `cc_smoother_process()`
- [ ] Call `cc_smoother_tick_1ms()` from timer interrupt
- [ ] Disable smoothing for binary CCs (sustain, switches)

## 7. Performance Tips

1. **Only enable tracks you need** - saves CPU time
2. **Use lighter modes for real-time** - reduces latency
3. **Disable unused CCs** - reduces memory access
4. **Reset on patch change** - prevents glitches

```c
// When changing patches
cc_smoother_reset_track(track);
```

## Complete Working Example

See `cc_smoother_example.c` for 7 complete examples demonstrating all features.

## Need Help?

- Read the full README.md for detailed explanations
- Check IMPLEMENTATION_SUMMARY.md for technical details
- Review cc_smoother.h for complete API documentation
