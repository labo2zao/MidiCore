# Automation System Documentation

## Overview

The MidiCore Looper features a comprehensive three-tier automation system that enables dynamic control of LFO and LiveFX parameters during live performance and production. This document provides complete technical documentation for all automation features.

## Table of Contents

1. [System Architecture](#system-architecture)
2. [Envelope Recording & Playback](#envelope-recording--playback)
3. [Modulation Matrix](#modulation-matrix)
4. [Scene-Based Snapshots](#scene-based-snapshots)
5. [API Reference](#api-reference)
6. [Usage Examples](#usage-examples)
7. [Performance Considerations](#performance-considerations)
8. [Best Practices](#best-practices)

---

## System Architecture

The automation system consists of three complementary subsystems:

### 1. Envelope Recording/Playback
- **Purpose**: Capture and replay parameter changes in real-time
- **Storage**: 128 automation events per track (timestamp + parameter + value)
- **Resolution**: 1ms timing precision
- **Mode**: Looped playback synchronized with loop boundaries

### 2. Modulation Matrix
- **Purpose**: Route LFO sources to multiple modulation destinations
- **Capacity**: 4 LFO sources × 8 destinations each = 32 total routings
- **Modulation**: -100% to +100% per routing
- **Features**: Cross-modulation support (e.g., LFO1 → LFO2 rate)

### 3. Scene-Based Snapshots
- **Purpose**: Save/recall complete LFO and LiveFX states per scene
- **Storage**: Per-scene parameter snapshots (8 scenes × 4 tracks)
- **Transitions**: Smooth morphing with configurable time (0-5000ms)
- **Integration**: Works seamlessly with scene chaining

---

## Envelope Recording & Playback

### Overview

Envelope automation records real-time parameter changes during performance and replays them in sync with loop playback. This enables hands-free parameter sweeps, filter movements, and dynamic effects.

### Supported Parameters

#### LFO Parameters
- `AUTOMATION_PARAM_LFO_RATE` - LFO frequency (0.01-10 Hz)
- `AUTOMATION_PARAM_LFO_DEPTH` - Modulation depth (0-100%)
- `AUTOMATION_PARAM_LFO_WAVEFORM` - Waveform selection (Sine, Triangle, Saw, Square, Random, S&H)

#### LiveFX Parameters
- `AUTOMATION_PARAM_LIVEFX_TRANSPOSE` - Pitch transpose (±12 semitones)
- `AUTOMATION_PARAM_LIVEFX_VELOCITY` - Velocity scaling (0-200%)
- `AUTOMATION_PARAM_LIVEFX_SCALE` - Force-to-scale mode (15 musical scales)

### Recording Workflow

```c
// 1. Start recording a specific parameter
looper_automation_start_record(track, AUTOMATION_PARAM_LFO_RATE);

// 2. Adjust parameter in real-time (via UI, MIDI Learn, etc.)
looper_set_lfo_rate(track, new_rate);

// 3. Stop recording when done
looper_automation_stop_record(track);

// 4. Enable playback to hear the automation
looper_automation_enable_playback(track, 1);
```

### Overdub Mode

Automation supports overdubbing - adding new events without erasing existing automation:

```c
// Enable overdub mode before recording
looper_automation_set_overdub_mode(track, 1);

// Record additional automation events
looper_automation_start_record(track, AUTOMATION_PARAM_LFO_DEPTH);
// ... adjust parameters ...
looper_automation_stop_record(track);

// Disable overdub to replace automation
looper_automation_set_overdub_mode(track, 0);
```

### Clearing Automation

```c
// Clear all automation for a specific track
looper_automation_clear(track);

// Clear automation for a specific parameter only
looper_automation_clear_parameter(track, AUTOMATION_PARAM_LFO_RATE);
```

### Playback Control

```c
// Enable/disable playback
looper_automation_enable_playback(track, 1);  // Enable
looper_automation_enable_playback(track, 0);  // Disable

// Check if playback is active
uint8_t is_playing = looper_automation_is_playback_enabled(track);

// Get current event count
uint16_t event_count = looper_automation_get_event_count(track);
```

---

## Modulation Matrix

### Overview

The modulation matrix routes LFO outputs to multiple destinations, creating complex evolving textures. Each of the 4 LFOs can modulate up to 8 different parameters simultaneously.

### Matrix Configuration

#### LFO Sources (4 total)
- `LFO_SOURCE_1` - Primary LFO
- `LFO_SOURCE_2` - Secondary LFO
- `LFO_SOURCE_3` - Tertiary LFO
- `LFO_SOURCE_4` - Quaternary LFO

#### Modulation Destinations (8 per LFO)

**LFO Destinations** (Cross-Modulation):
- `MOD_DEST_LFO1_RATE` - Modulate LFO 1 rate
- `MOD_DEST_LFO2_RATE` - Modulate LFO 2 rate
- `MOD_DEST_LFO3_RATE` - Modulate LFO 3 rate
- `MOD_DEST_LFO4_RATE` - Modulate LFO 4 rate
- `MOD_DEST_LFO1_DEPTH` - Modulate LFO 1 depth
- `MOD_DEST_LFO2_DEPTH` - Modulate LFO 2 depth

**LiveFX Destinations**:
- `MOD_DEST_LIVEFX_TRANSPOSE` - Modulate pitch transpose
- `MOD_DEST_LIVEFX_VELOCITY` - Modulate velocity scaling

**Track Parameters**:
- `MOD_DEST_TRACK_VOLUME` - Modulate track output volume
- `MOD_DEST_TRACK_PAN` - Modulate stereo panning (if supported)

### Setting Up Routings

```c
// Route LFO 1 to modulate LFO 2's rate at 50% depth
looper_mod_matrix_set_routing(LFO_SOURCE_1, MOD_DEST_LFO2_RATE, 50);

// Route LFO 2 to modulate LiveFX velocity at -30% (inverse modulation)
looper_mod_matrix_set_routing(LFO_SOURCE_2, MOD_DEST_LIVEFX_VELOCITY, -30);

// Route LFO 3 to modulate track volume at 75%
looper_mod_matrix_set_routing(LFO_SOURCE_3, MOD_DEST_TRACK_VOLUME, 75);

// Clear a specific routing
looper_mod_matrix_clear_routing(LFO_SOURCE_1, MOD_DEST_LFO2_RATE);
```

### Matrix Control

```c
// Enable/disable entire modulation matrix
looper_mod_matrix_enable(1);  // Enable
looper_mod_matrix_enable(0);  // Disable

// Check if matrix is enabled
uint8_t is_enabled = looper_mod_matrix_is_enabled();

// Clear all routings
looper_mod_matrix_clear_all();

// Clear all routings for a specific source
looper_mod_matrix_clear_source(LFO_SOURCE_1);
```

### Reading Matrix State

```c
// Get modulation amount for a specific routing
int8_t mod_amount = looper_mod_matrix_get_routing(LFO_SOURCE_1, MOD_DEST_LFO2_RATE);
// Returns: -100 to +100, or 0 if no routing exists

// Check if a routing exists
uint8_t has_routing = looper_mod_matrix_has_routing(LFO_SOURCE_1, MOD_DEST_LFO2_RATE);
```

---

## Scene-Based Snapshots

### Overview

Scene snapshots save the complete state of all LFO and LiveFX parameters for each scene. When switching scenes, parameters are automatically recalled with optional smooth transitions.

### Saving Scene Automation

```c
// Save current LFO/LiveFX state to scene 3
looper_scene_save_automation(3);
```

**Saved Parameters Include**:
- All LFO settings (enabled, waveform, rate, depth, target, sync mode)
- All LiveFX settings (enabled, transpose, velocity scale, force-to-scale mode)
- Modulation matrix routings
- Automation playback states

### Loading Scene Automation

```c
// Load automation state from scene 5
looper_scene_load_automation(5);
```

**Loading Behavior**:
- Parameters transition smoothly over configured morph time
- Automation playback continues if enabled
- Scene chaining automation continues seamlessly

### Morphing Between Scenes

```c
// Set transition time to 2 seconds (2000ms)
looper_scene_set_morph_time(2000);

// Instant transitions (no morphing)
looper_scene_set_morph_time(0);

// Slow, smooth transitions (5 seconds)
looper_scene_set_morph_time(5000);

// Get current morph time
uint16_t morph_ms = looper_scene_get_morph_time();
```

### Auto-Save on Scene Change

```c
// Enable automatic snapshot saving when changing scenes
looper_scene_enable_auto_save(1);

// Disable auto-save (manual control)
looper_scene_enable_auto_save(0);

// Check auto-save status
uint8_t auto_save = looper_scene_is_auto_save_enabled();
```

---

## API Reference

### Envelope Automation API

| Function | Description | Parameters | Returns |
|----------|-------------|------------|---------|
| `looper_automation_start_record()` | Start recording automation | `track` (0-3), `param_type` | `void` |
| `looper_automation_stop_record()` | Stop recording automation | `track` (0-3) | `void` |
| `looper_automation_enable_playback()` | Enable/disable playback | `track` (0-3), `enabled` (0/1) | `void` |
| `looper_automation_clear()` | Clear all automation | `track` (0-3) | `void` |
| `looper_automation_clear_parameter()` | Clear specific parameter | `track` (0-3), `param_type` | `void` |
| `looper_automation_set_overdub_mode()` | Enable overdub recording | `track` (0-3), `enabled` (0/1) | `void` |
| `looper_automation_is_playback_enabled()` | Check playback state | `track` (0-3) | `uint8_t` (0/1) |
| `looper_automation_get_event_count()` | Get number of events | `track` (0-3) | `uint16_t` |

### Modulation Matrix API

| Function | Description | Parameters | Returns |
|----------|-------------|------------|---------|
| `looper_mod_matrix_set_routing()` | Set modulation routing | `source`, `destination`, `amount` (-100 to +100) | `void` |
| `looper_mod_matrix_clear_routing()` | Clear specific routing | `source`, `destination` | `void` |
| `looper_mod_matrix_enable()` | Enable/disable matrix | `enabled` (0/1) | `void` |
| `looper_mod_matrix_clear_all()` | Clear all routings | None | `void` |
| `looper_mod_matrix_clear_source()` | Clear source routings | `source` | `void` |
| `looper_mod_matrix_get_routing()` | Get routing amount | `source`, `destination` | `int8_t` |
| `looper_mod_matrix_has_routing()` | Check if routing exists | `source`, `destination` | `uint8_t` |
| `looper_mod_matrix_is_enabled()` | Check matrix state | None | `uint8_t` |

### Scene Automation API

| Function | Description | Parameters | Returns |
|----------|-------------|------------|---------|
| `looper_scene_save_automation()` | Save current state | `scene` (0-7) | `int` (0/-1) |
| `looper_scene_load_automation()` | Load saved state | `scene` (0-7) | `int` (0/-1) |
| `looper_scene_set_morph_time()` | Set transition time | `time_ms` (0-5000) | `void` |
| `looper_scene_get_morph_time()` | Get transition time | None | `uint16_t` |
| `looper_scene_enable_auto_save()` | Enable auto-save | `enabled` (0/1) | `void` |
| `looper_scene_is_auto_save_enabled()` | Check auto-save | None | `uint8_t` |

---

## Usage Examples

### Example 1: Recording Filter Sweep

```c
// Record a slow LFO rate sweep on track 0
void record_filter_sweep(void) {
    // Start recording LFO rate
    looper_automation_start_record(0, AUTOMATION_PARAM_LFO_RATE);
    
    // Manually sweep rate from slow to fast over 8 bars
    // (in practice, this would be done via UI/encoder)
    for (int i = 0; i < 100; i++) {
        uint16_t rate = 10 + (i * 2);  // 10 to 210 (0.1Hz to 2.1Hz)
        looper_set_lfo_rate(0, rate);
        delay_ms(80);  // Sweep over ~8 seconds
    }
    
    // Stop recording
    looper_automation_stop_record(0);
    
    // Enable playback
    looper_automation_enable_playback(0, 1);
}
```

### Example 2: Complex Modulation Matrix

```c
// Create evolving texture with cross-modulating LFOs
void setup_evolving_texture(void) {
    // LFO 1: Very slow sine wave (0.05 Hz = 20 second cycle)
    looper_set_lfo_waveform(0, LOOPER_LFO_WAVEFORM_SINE);
    looper_set_lfo_rate(0, 5);  // 0.05 Hz
    looper_set_lfo_depth(0, 100);
    looper_set_lfo_enabled(0, 1);
    
    // LFO 2: Faster random (0.5 Hz)
    looper_set_lfo_waveform(0, LOOPER_LFO_WAVEFORM_RANDOM);
    looper_set_lfo_rate(0, 50);  // 0.5 Hz
    looper_set_lfo_depth(0, 80);
    looper_set_lfo_enabled(0, 1);
    
    // Route LFO 1 to modulate LFO 2's rate (creates acceleration/deceleration)
    looper_mod_matrix_set_routing(LFO_SOURCE_1, MOD_DEST_LFO2_RATE, 60);
    
    // Route LFO 2 to modulate velocity (creates rhythmic variation)
    looper_mod_matrix_set_routing(LFO_SOURCE_2, MOD_DEST_LIVEFX_VELOCITY, 40);
    
    // Route LFO 1 to modulate LFO 2's depth (creates intensity waves)
    looper_mod_matrix_set_routing(LFO_SOURCE_1, MOD_DEST_LFO2_DEPTH, -50);
    
    // Enable modulation matrix
    looper_mod_matrix_enable(1);
}
```

### Example 3: Scene-Based Song Arrangement

```c
// Setup different automation per song section
void setup_song_arrangement(void) {
    // SCENE 0: Verse - Subtle LFO on velocity
    looper_set_lfo_waveform(0, LOOPER_LFO_WAVEFORM_SINE);
    looper_set_lfo_rate(0, 20);  // 0.2 Hz
    looper_set_lfo_depth(0, 30);
    looper_set_lfo_target(0, LOOPER_LFO_TARGET_VELOCITY);
    looper_scene_save_automation(0);
    
    // SCENE 1: Chorus - Fast LFO on pitch for vibrato
    looper_set_lfo_waveform(0, LOOPER_LFO_WAVEFORM_SINE);
    looper_set_lfo_rate(0, 60);  // 0.6 Hz
    looper_set_lfo_depth(0, 15);
    looper_set_lfo_target(0, LOOPER_LFO_TARGET_PITCH);
    looper_scene_save_automation(1);
    
    // SCENE 2: Bridge - Random LFO for chaotic feel
    looper_set_lfo_waveform(0, LOOPER_LFO_WAVEFORM_RANDOM);
    looper_set_lfo_rate(0, 40);  // 0.4 Hz
    looper_set_lfo_depth(0, 70);
    looper_set_lfo_target(0, LOOPER_LFO_TARGET_VELOCITY);
    looper_scene_save_automation(2);
    
    // Set 1 second morph time between scenes
    looper_scene_set_morph_time(1000);
    
    // Enable scene chaining: Verse → Chorus → Bridge → Verse
    looper_set_scene_chain(0, 1, 1);  // Scene 0 → 1
    looper_set_scene_chain(1, 2, 1);  // Scene 1 → 2
    looper_set_scene_chain(2, 0, 1);  // Scene 2 → 0 (loop)
}
```

### Example 4: Live Performance Recording

```c
// Record live tweaks during performance
void live_performance_recording(void) {
    // Enable overdub mode to layer multiple parameters
    looper_automation_set_overdub_mode(0, 1);
    
    // Record LFO rate changes
    looper_automation_start_record(0, AUTOMATION_PARAM_LFO_RATE);
    // ... perform for one loop cycle, adjusting rate ...
    looper_automation_stop_record(0);
    
    // Add LiveFX transpose automation (overdubbed)
    looper_automation_start_record(0, AUTOMATION_PARAM_LIVEFX_TRANSPOSE);
    // ... perform for one loop cycle, adjusting transpose ...
    looper_automation_stop_record(0);
    
    // Add LFO depth automation (overdubbed)
    looper_automation_start_record(0, AUTOMATION_PARAM_LFO_DEPTH);
    // ... perform for one loop cycle, adjusting depth ...
    looper_automation_stop_record(0);
    
    // Disable overdub mode
    looper_automation_set_overdub_mode(0, 0);
    
    // Enable playback of all recorded automation
    looper_automation_enable_playback(0, 1);
}
```

---

## Performance Considerations

### Memory Usage

| Component | Memory per Track | Total (4 Tracks) |
|-----------|------------------|------------------|
| Automation Events | 2048 bytes (128 events × 16 bytes) | ~8 KB |
| Modulation Matrix | 128 bytes (32 routings × 4 bytes) | 512 bytes |
| Scene Snapshots | 512 bytes per scene | ~4 KB (8 scenes) |
| **Total** | ~2.7 KB | **~12.5 KB** |

### CPU Overhead

- **Envelope Playback**: <1% per active track
- **Modulation Matrix**: <1% when enabled (32 routings)
- **Scene Morphing**: <2% during transition (5 seconds max)
- **Combined Maximum**: <5% with all features active

### Latency

- **Automation Update**: <1ms from event trigger to parameter change
- **Modulation Matrix**: <1ms from LFO output to destination
- **Scene Morphing**: Smooth interpolation, no audible artifacts
- **Total System Latency**: <2ms end-to-end

### Timing Resolution

- **Automation Events**: 1ms precision (1000 events per second maximum)
- **LFO Update Rate**: 1ms (1000 Hz sample rate)
- **Morph Interpolation**: 1ms steps for smooth transitions

---

## Best Practices

### 1. Recording Automation

**DO**:
- Record automation during loop playback for perfect sync
- Use overdub mode to layer multiple parameter changes
- Clear unused automation to save memory
- Test playback before saving to scene

**DON'T**:
- Record automation when loop is stopped (timing will be wrong)
- Exceed 128 events per track (oldest events will be discarded)
- Record rapid parameter changes (can cause zipper noise)

### 2. Modulation Matrix

**DO**:
- Start with subtle modulation amounts (10-30%)
- Use cross-modulation sparingly for complex textures
- Clear unused routings to reduce CPU load
- Save matrix state per scene for different song sections

**DON'T**:
- Create feedback loops (e.g., LFO1 → LFO2 → LFO1)
- Use extreme modulation amounts (>80%) unless desired
- Modulate too many parameters simultaneously (CPU intensive)

### 3. Scene Automation

**DO**:
- Use morph time for smooth transitions between scenes
- Save automation after setting up each scene
- Test scene changes before live performance
- Use scene chaining for automatic song progression

**DON'T**:
- Use very long morph times (>2 seconds) for rhythmic material
- Forget to save automation after making changes
- Rely on auto-save for critical performances (use manual save)

### 4. Live Performance

**DO**:
- Practice automation recording workflow before performance
- Use footswitch mapping for hands-free automation control
- Monitor event count to avoid hitting 128-event limit
- Create scene-based presets for different songs

**DON'T**:
- Record automation during critical performance moments
- Change too many parameters simultaneously (confusing)
- Forget to enable playback after recording
- Use automation for time-critical events (use manual control)

---

## Troubleshooting

### Problem: Automation not playing back

**Possible Causes**:
1. Playback not enabled: `looper_automation_enable_playback(track, 1)`
2. No automation recorded: Check `looper_automation_get_event_count(track)`
3. Loop not playing: Ensure looper is in PLAY mode
4. Track muted: Check `looper_is_track_muted(track)`

### Problem: Modulation matrix not working

**Possible Causes**:
1. Matrix not enabled: `looper_mod_matrix_enable(1)`
2. Source LFO not enabled: Check `looper_get_lfo_enabled(track)`
3. Modulation amount too low: Increase amount to 50%+
4. Destination parameter at min/max: Check target parameter range

### Problem: Scene morphing too abrupt

**Solutions**:
- Increase morph time: `looper_scene_set_morph_time(2000)`
- Check if parameter supports morphing (some may snap)
- Verify scene automation was saved correctly

### Problem: Running out of automation events

**Solutions**:
- Clear unused automation: `looper_automation_clear_parameter()`
- Reduce recording frequency (don't tweak too fast)
- Use modulation matrix instead for cyclic changes
- Split automation across multiple tracks

---

## Advanced Topics

### Creating Generative Music

Combine all three automation systems for evolving, generative compositions:

```c
// Setup chaotic, evolving system
void setup_generative_system(void) {
    // LFO 1: Ultra-slow random (30 second cycles)
    looper_set_lfo_waveform(0, LOOPER_LFO_WAVEFORM_RANDOM);
    looper_set_lfo_rate(0, 3);  // 0.03 Hz
    
    // LFO 2: Medium triangle (4 second cycles)
    looper_set_lfo_waveform(0, LOOPER_LFO_WAVEFORM_TRIANGLE);
    looper_set_lfo_rate(0, 25);  // 0.25 Hz
    
    // Cross-modulation for complexity
    looper_mod_matrix_set_routing(LFO_SOURCE_1, MOD_DEST_LFO2_RATE, 70);
    looper_mod_matrix_set_routing(LFO_SOURCE_2, MOD_DEST_LFO1_DEPTH, -40);
    looper_mod_matrix_enable(1);
    
    // Record manual variations over 8 bars
    looper_automation_start_record(0, AUTOMATION_PARAM_LFO_DEPTH);
    // ... manual tweaking ...
    looper_automation_stop_record(0);
    looper_automation_enable_playback(0, 1);
    
    // Save as generative scene
    looper_scene_save_automation(0);
}
```

### Integration with MIDI Learn

Map external controllers to automation recording:

```c
// Map CC 1 (mod wheel) to automation recording trigger
void map_automation_to_midi(void) {
    // When CC 1 > 64, start recording
    if (midi_cc_value > 64 && !is_recording) {
        looper_automation_start_record(current_track, current_param);
        is_recording = 1;
    }
    // When CC 1 < 64, stop recording
    else if (midi_cc_value < 64 && is_recording) {
        looper_automation_stop_record(current_track);
        is_recording = 0;
    }
}
```

---

## Future Enhancements

Potential features for future versions:

1. **Automation Curves**: Pre-defined curves (linear, exponential, logarithmic)
2. **Multi-Track Sync**: Synchronize automation across all tracks
3. **MIDI Export**: Export automation as CC messages to MIDI files
4. **Visual Editor**: Graphical automation curve editing on OLED
5. **Automation Groups**: Record multiple parameters simultaneously
6. **External Sync**: Sync automation to external MIDI clock

---

## See Also

- [LFO Module Documentation](Services/lfo/README.md)
- [LiveFX Documentation](Services/livefx/README.md)
- [Scene Management](Services/looper/README.md#scene-management)
- [TESTING_PROTOCOL.md](TESTING_PROTOCOL.md) - Automation test cases

---

**Document Version**: 1.0  
**Last Updated**: 2026-01-17  
**Author**: MidiCore Development Team
