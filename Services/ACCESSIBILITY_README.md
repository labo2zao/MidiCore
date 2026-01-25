# MIDI FX for Accessibility

Comprehensive MIDI processing modules designed to make music creation accessible to people with disabilities.

## Overview

These modules address specific challenges faced by musicians with:
- Limited mobility or motor disabilities
- Tremors or involuntary movements
- Difficulty maintaining key pressure
- Visual impairments
- Cognitive or learning disabilities

## Modules

### 1. One-Finger Chord (`one_finger_chord/`)

**For:** People who can only press one key at a time or have limited hand mobility.

**Features:**
- Play full chords with single notes
- 4 operating modes:
  - **Auto**: Automatically generates chord from melody
  - **Split Keyboard**: Left hand sets chord, right hand plays melody
  - **Single Note**: Each note triggers a full chord
  - **Disabled**: Normal pass-through
- Configurable voicing styles:
  - Simple (Root + 5th) - easiest to hear
  - Triad (Root + 3rd + 5th)
  - Seventh (adds 7th)
  - Full (all chord tones)
- Optional bass note generation
- Adjustable chord velocity relative to melody

**Use Cases:**
- Single-finger keyboard playing
- Limited hand mobility
- One-handed playing
- Simplified music education

**Example:**
```c
// Setup one-finger chord in split mode
ofc_init();
ofc_set_mode(0, OFC_MODE_SPLIT_KEYBOARD);
ofc_set_voicing(0, OFC_VOICING_TRIAD);
ofc_set_split_point(0, 60);  // C4 split
ofc_set_bass_enabled(0, 1);
ofc_set_output_callback(my_note_output);

// Now: left hand (below C4) = chord, right hand = melody
ofc_process_note(0, 48, 80, 0);  // C3 - sets C chord
ofc_process_note(0, 72, 100, 0); // C5 - plays melody + C chord
```

---

### 2. Assist Hold (`assist_hold/`)

**For:** People with difficulty maintaining pressure on keys or muscle fatigue.

**Features:**
- Automatically sustains notes
- 5 hold modes:
  - **Latch**: Toggle on/off with same note
  - **Timed**: Hold for fixed duration (100-10000ms)
  - **Next Note**: Hold until next note is played
  - **Infinite**: Hold forever until manually released
  - **Disabled**: Normal operation
- Mono/poly mode options
- Configurable hold duration
- Velocity threshold for activation
- Statistics tracking (number of held notes)

**Use Cases:**
- Muscle weakness or fatigue
- Difficulty maintaining key pressure
- Tremor-related key bouncing
- Hands-free sustain

**Example:**
```c
// Setup timed hold for 2 seconds
assist_hold_init();
assist_hold_set_mode(0, HOLD_MODE_TIMED);
assist_hold_set_duration_ms(0, 2000);
assist_hold_set_mono_mode(0, 0);  // Polyphonic
assist_hold_set_output_callback(my_note_output);

// Note will sustain for 2 seconds automatically
assist_hold_process_note(0, 60, 100, 0, get_time_ms());

// Or use latch mode for toggle behavior
assist_hold_set_mode(0, HOLD_MODE_LATCH);
// Press once = note on, press again = note off
```

---

### 3. Note Stabilizer (`note_stabilizer/`)

**For:** People with tremors, spasms, or unintended movements.

**Features:**
- Filters unintended notes
- Minimum note duration enforcement (10-500ms)
- Retrigger delay (10-1000ms) - prevents rapid repeated notes
- Neighboring key filter (±1-12 semitones) - prevents accidental adjacent key presses
- Velocity stabilization threshold
- Note averaging for smooth velocity
- Statistics (filtered vs. passed notes)

**Use Cases:**
- Essential tremor
- Parkinson's disease
- Cerebral palsy
- Muscle spasms
- Unintended movements

**Example:**
```c
// Setup stabilizer for tremor filtering
note_stab_init();
note_stab_set_enabled(0, 1);
note_stab_set_min_duration_ms(0, 100);      // Ignore notes < 100ms
note_stab_set_retrigger_delay_ms(0, 150);   // 150ms between same note
note_stab_set_neighbor_range(0, 1);         // Filter ±1 semitone
note_stab_set_averaging_enabled(0, 1);
note_stab_set_output_callback(my_note_output);

// Process notes - unintended quick hits filtered
note_stab_process_note(0, 60, 100, 0, get_time_ms());

// Get statistics to monitor effectiveness
uint32_t filtered, passed;
note_stab_get_stats(0, &filtered, &passed);
printf("Filtered: %u, Passed: %u\n", filtered, passed);
```

---

## Integration with MidiCore

All modules follow MidiCore architecture:

```c
// Initialize all accessibility modules
void accessibility_init(void) {
    ofc_init();
    assist_hold_init();
    note_stab_init();
}

// Chain modules for comprehensive assistance
void process_accessible_note(uint8_t note, uint8_t velocity, uint8_t channel) {
    uint32_t timestamp = HAL_GetTick();
    
    // Stage 1: Stabilize input (filter tremors)
    note_stab_process_note(0, note, velocity, channel, timestamp);
    
    // Stage 2: Apply hold assist
    // (connected via callback from stabilizer)
    
    // Stage 3: Generate chord accompaniment
    // (connected via callback from hold assist)
}
```

## Configuration Examples

### For Essential Tremor

```c
// Aggressive tremor filtering
note_stab_set_min_duration_ms(0, 150);
note_stab_set_retrigger_delay_ms(0, 200);
note_stab_set_neighbor_range(0, 2);  // ±2 semitones
```

### For Limited Mobility

```c
// One-finger chord with hold assist
ofc_set_mode(0, OFC_MODE_SINGLE_NOTE_CHORD);
assist_hold_set_mode(0, HOLD_MODE_NEXT_NOTE);
```

### For Muscle Fatigue

```c
// Long hold duration, easy release
assist_hold_set_mode(0, HOLD_MODE_TIMED);
assist_hold_set_duration_ms(0, 5000);  // 5 seconds
assist_hold_set_mono_mode(0, 1);       // One note at a time
```

## Memory Usage

Each module per track:
- One-Finger Chord: ~80 bytes
- Assist Hold: ~200 bytes
- Note Stabilizer: ~700 bytes (includes 128-note history)

Total for 4 tracks: ~4KB

## Performance

All modules:
- Real-time safe (no dynamic allocation)
- <10μs processing time per note
- Suitable for embedded systems (STM32F4+)
- No floating-point operations

## Accessibility Best Practices

1. **Start Simple**: Enable one module at a time
2. **Customize**: Adjust thresholds based on individual needs
3. **Monitor**: Use statistics to verify effectiveness
4. **Combine**: Chain modules for comprehensive assistance
5. **Test**: Validate with actual user feedback

## Future Enhancements

Potential additions:
- Visual feedback (LED indicators for held notes)
- Adaptive learning (auto-adjust thresholds)
- Profiles (save/load user configurations)
- Voice control integration
- Eye-tracking support
- Switch control interface

## Medical Conditions Addressed

These modules can assist with:
- **Motor Disorders**: Parkinson's, essential tremor, dystonia, cerebral palsy
- **Muscular Conditions**: Muscular dystrophy, ALS, multiple sclerosis
- **Neuromuscular**: Myasthenia gravis, Guillain-Barré syndrome
- **Injuries**: Stroke recovery, nerve damage, arthritis
- **Other**: Fibromyalgia, chronic fatigue, age-related decline

## Licensing & Medical Disclaimer

These modules are assistive technology tools, not medical devices. They do not diagnose, treat, or cure any medical condition. Users should consult healthcare professionals for medical advice.

## Contributing

To add new accessibility features:
1. Identify specific disability challenge
2. Research existing solutions
3. Design module following MidiCore patterns
4. Test with users who have the condition
5. Document use cases and limitations

## Support

For questions about accessibility features or suggestions for new modules, please open an issue on GitHub.

---

**Remember**: Music is for everyone. These tools aim to break down barriers and make music creation accessible to all.
