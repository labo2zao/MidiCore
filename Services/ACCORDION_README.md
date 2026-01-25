# MIDI FX for Accordion

Comprehensive MIDI processing modules designed specifically for accordion makers and players.

## Overview

These modules implement accordion-specific MIDI features including bellows expression, musette tuning, Stradella bass system, register coupling, bellows shake detection, and more. Designed for both traditional acoustic-MIDI hybrid accordions and pure MIDI accordions.

## Modules (6 Total)

### 1. Bellows Expression (`bellows_expression/`)

**Advanced bellows pressure to MIDI expression mapping**

**Features:**
- Multiple expression curves:
  - **Linear**: Direct pressure-to-MIDI mapping
  - **Exponential**: More sensitive at low pressure (easier pp playing)
  - **Logarithmic**: More sensitive at high pressure (better ff control)
  - **S-Curve**: Smooth at extremes, responsive in middle
- Bidirectional support (push/pull different characteristics)
- Pressure calibration (min/max Pa settings)
- Smoothing/anti-jitter (0-100%)
- Attack/release times for natural feel
- Dual CC output (Expression CC11 + Breath CC2)

**Use Cases:**
- Professional MIDI accordion with pressure sensor
- Bellows expression control
- Dynamic performance
- Realistic accordion sound reproduction

**Example:**
```c
// Setup bellows with S-curve for natural response
bellows_init();
bellows_set_curve(0, BELLOWS_CURVE_S_CURVE);
bellows_set_pressure_range(0, -500, 500);  // ±500 Pa
bellows_set_smoothing(0, 30);              // 30% smoothing
bellows_set_bidirectional(0, 1);           // Different push/pull
bellows_set_attack_release(0, 10, 50);     // 10ms attack, 50ms release
bellows_set_output_callback(my_cc_output);

// Process pressure readings from sensor
int32_t pressure = read_pressure_sensor();
bellows_process_pressure(0, pressure, 0);
```

---

### 2. Musette Detune (`musette_detune/`)

**Classic accordion musette/chorus effect**

**Features:**
- Traditional musette styles:
  - **Dry**: Single voice (no detune)
  - **Light**: Subtle detune ±2-5 cents (chamber orchestra)
  - **French**: Classic musette ±10-15 cents (bal-musette)
  - **Italian**: Italian style ±8-12 cents
  - **American**: Swing style ±5-8 cents
  - **Extreme**: Heavy musette ±20+ cents (folk/cajun)
  - **Custom**: User-defined detune
- Voice configurations (L-M-M-H pattern):
  - **1 voice**: M only (dry/bassoon)
  - **2 voices L-M**: Bassoon register
  - **2 voices M-H**: Violin register
  - **3 voices L-M-H**: Full musette
  - **4 voices L-L-M-H**: Super musette
- Per-voice level control (0-100%)
- Stereo spread control (mono to wide stereo)
- Pitchbend-based detuning

**Use Cases:**
- Authentic accordion sound
- Musette/bal-musette styles
- Italian/French/American accordion tones
- Register simulation

**Example:**
```c
// Setup French musette with 3 voices
musette_init();
musette_set_style(0, MUSETTE_STYLE_FRENCH);
musette_set_voices(0, MUSETTE_VOICES_3_LMH);
musette_set_stereo_spread(0, 60);          // Moderate stereo width
musette_set_voice_level(0, 0, 90);         // L voice 90%
musette_set_voice_level(0, 1, 100);        // M voice 100%
musette_set_voice_level(0, 2, 95);         // H voice 95%
musette_set_output_callback(my_note_output);

// Process notes - automatically creates musette effect
musette_process_note(0, 60, 100, 0);  // C4 with French musette
```

---

### 3. Bass Chord System (`bass_chord_system/`)

**Stradella bass system for accordion left hand**

**Features:**
- Multiple bass layouts:
  - **120-bass**: Standard 6-row system
  - **96-bass**: 5-row system
  - **72-bass**: 4-row system
  - **48-bass**: Compact system
  - **Free bass**: Chromatic (no chord generation)
- Stradella chord types:
  - Counter-bass (root + 5th)
  - Fundamental bass (root + octave)
  - Major chord
  - Minor chord
  - Dominant 7th
  - Diminished 7th
- Configurable voicing density (sparse/normal/dense)
- Octave doubling for bass notes
- Independent bass/chord velocity control
- Circle of fifths layout

**Use Cases:**
- Standard accordion bass system
- Left-hand accompaniment
- Bass button mapping
- Chord generation from single buttons

**Example:**
```c
// Setup 120-bass Stradella system
bass_chord_init();
bass_chord_set_layout(0, BASS_LAYOUT_120);
bass_chord_set_base_note(0, 36);           // C2 base
bass_chord_set_octave_doubling(0, 1);      // Double bass notes
bass_chord_set_voicing_density(0, 1);      // Normal density
bass_chord_set_bass_velocity(0, 110);      // Bass 110%
bass_chord_set_chord_velocity(0, 90);      // Chords 90%
bass_chord_set_output_callback(my_note_output);

// Process bass buttons
// Button 0-19 = counter-bass row
// Button 20-39 = bass row
// Button 40-59 = major chord row
// etc.
bass_chord_process_button(0, 42, 100, 0);  // Major D chord
```

---

### 4. Register Coupling (`register_coupling/`)

**Automatic voice combination management and register switching**

**Features:**
- 16 standard accordion registers:
  - **Master**: All reeds (L-L-M-M-H) - full sound
  - **Musette**: L-M-H - classic wet musette
  - **Bandoneon**: L-M - dry, dark
  - **Accordion**: M-M - slightly wet
  - **Violin**: M-H - bright
  - **Clarinet**: M - single reed, clear
  - **Bassoon**: L - single reed, dark
  - **Piccolo**: H - single reed, bright
  - **Organ**: M-M-H - organ-like
  - **Harmonium**: L-M-M-H - full, slightly wet
  - **Oboe**: L-H - hollow
  - **Flute**: M-H - airy
  - **Custom 1-4**: User-defined combinations
- Smooth register transitions (crossfade)
- Configurable transition time (10-500ms)
- Register memory (remembers last register per key range)
- Foot switch support (next/previous register)
- Reed set management (L1, L2, M1, M2, H)

**Use Cases:**
- Professional accordion register simulation
- Voice combination management
- Smooth transitions between registers
- Custom reed configurations

**Example:**
```c
// Setup register system
reg_coupling_init();
reg_coupling_set_register(0, REG_MUSETTE);  // Start with musette
reg_coupling_set_smooth_transition(0, 1);
reg_coupling_set_transition_time(0, 80);    // 80ms crossfade
reg_coupling_set_output_callback(my_reed_output);

// Process notes - automatically applies register configuration
reg_coupling_process_note(0, 60, 100, 0);

// Change register (foot switch)
reg_coupling_next_register(0);  // Cycle to next register

// Get current configuration
reed_set_config_t config;
reg_coupling_get_reed_config(0, REG_CLARINET, &config);
```

---

### 5. Bellows Shake (`bellows_shake/`)

**Detects and generates tremolo from bellows shaking**

**Features:**
- Automatic shake/vibrato detection
- Frequency range detection (2-20 Hz)
- Configurable sensitivity (0-100%)
- Multiple tremolo targets:
  - **Volume**: Modulate expression (traditional tremolo)
  - **Pitch**: Modulate pitch (vibrato)
  - **Filter**: Modulate filter cutoff
  - **Both**: Volume + pitch simultaneously
- Depth control (0-100%)
- Real-time frequency measurement
- Zero-crossing detection algorithm

**Use Cases:**
- Authentic accordion tremolo
- Bellows shake technique
- Vibrato generation
- Expressive performance

**Example:**
```c
// Setup shake detection
bellows_shake_init();
bellows_shake_set_enabled(0, 1);
bellows_shake_set_sensitivity(0, 60);      // Moderate sensitivity
bellows_shake_set_depth(0, 70);            // 70% tremolo depth
bellows_shake_set_target(0, SHAKE_TARGET_VOLUME);
bellows_shake_set_freq_range(0, 4, 12);   // 4-12 Hz range
bellows_shake_set_cc_callback(my_cc_output);

// Process pressure - automatically detects shake
bellows_shake_process_pressure(0, pressure, timestamp, 0);

// Check if shake detected
if (bellows_shake_is_detected(0)) {
    uint8_t freq = bellows_shake_get_frequency(0);
    printf("Shake detected at %d Hz\n", freq);
}
```

---

## Integration with Existing MidiCore Features

These accordion modules work seamlessly with existing MidiCore features:

### Bellows + Expression Module
```c
// Chain bellows pressure to expression module
bellows_process_pressure(0, pressure, 0);
// Automatically outputs to expression CC module if configured
```

### Musette + Harmonizer
```c
// Combine musette detuning with harmonic generation
musette_process_note(0, note, velocity, channel);
harmonizer_generate(0, note, velocity, out_notes, out_vels);
```

### Bass System + Chord Module
```c
// Use bass system with additional chord voicing options
bass_chord_process_button(0, button, velocity, channel);
chord_set_voicing(0, CHORD_VOICING_DROP2);
```

## Complete Accordion Configuration Example

```c
void setup_professional_accordion(void) {
    // 1. Bellows expression with natural feel
    bellows_init();
    bellows_set_curve(0, BELLOWS_CURVE_S_CURVE);
    bellows_set_pressure_range(0, -600, 600);
    bellows_set_smoothing(0, 25);
    bellows_set_bidirectional(0, 1);
    
    // 2. Bellows shake detection for tremolo
    bellows_shake_init();
    bellows_shake_set_enabled(0, 1);
    bellows_shake_set_sensitivity(0, 60);
    bellows_shake_set_depth(0, 70);
    bellows_shake_set_target(0, SHAKE_TARGET_VOLUME);
    
    // 3. Register coupling for voice management
    reg_coupling_init();
    reg_coupling_set_register(0, REG_MUSETTE);
    reg_coupling_set_smooth_transition(0, 1);
    reg_coupling_set_transition_time(0, 80);
    
    // 4. French musette on right hand
    musette_init();
    musette_set_style(0, MUSETTE_STYLE_FRENCH);
    musette_set_voices(0, MUSETTE_VOICES_3_LMH);
    musette_set_stereo_spread(0, 55);
    
    // 5. Stradella bass on left hand
    bass_chord_init();
    bass_chord_set_layout(0, BASS_LAYOUT_120);
    bass_chord_set_octave_doubling(0, 1);
    bass_chord_set_voicing_density(0, 1);
    
    // 6. Optional: Add swing for musette
    swing_init();
    swing_set_groove_type(0, GROOVE_TYPE_SWING);
    swing_set_amount(0, 55);
}
```

## Hardware Integration

### Pressure Sensor (XGZP6847D I2C)

Already supported in MidiCore:
```c
#include "Services/pressure/pressure_i2c.h"

int32_t pressure_pa;
pressure_read_pa(&pressure_pa);
bellows_process_pressure(0, pressure_pa, 0);
```

### Button Matrix (Stradella Bass)

Use existing SRIO or DIN modules:
```c
// Map bass buttons to MIDI notes
void on_bass_button(uint8_t button, uint8_t pressed) {
    bass_chord_process_button(0, button, pressed ? 100 : 0, 0);
}
```

### Analog Keys (Right Hand)

Use existing AINSER64 module with accordion profile:
```c
#include "Services/ainser/ainser_profile_accordion_v1.h"
ainser_profile_accordion_v1_apply();
```

## Register Simulation

Accordion registers can be simulated by switching musette configurations:

```c
typedef enum {
    REG_BASSOON = 0,      // M only
    REG_CLARINET,         // M-H
    REG_PICCOLO,          // H only
    REG_VIOLIN,           // M-M detuned
    REG_MUSETTE,          // L-M-H
    REG_MASTER            // L-L-M-H
} accordion_register_t;

void set_accordion_register(accordion_register_t reg) {
    switch (reg) {
        case REG_BASSOON:
            musette_set_voices(0, MUSETTE_VOICES_1);
            musette_set_style(0, MUSETTE_STYLE_DRY);
            break;
        case REG_MUSETTE:
            musette_set_voices(0, MUSETTE_VOICES_3_LMH);
            musette_set_style(0, MUSETTE_STYLE_FRENCH);
            break;
        // ... etc
    }
}
```

## Memory Usage

Per track:
- Bellows Expression: ~50 bytes
- Musette Detune: ~30 bytes
- Bass Chord System: ~20 bytes
- Register Coupling: ~80 bytes
- Bellows Shake: ~300 bytes (includes pressure history)

Total for 4 tracks: ~1.9 KB

## Performance

All modules:
- Real-time safe (no dynamic allocation)
- <20μs processing per event
- Suitable for embedded systems (STM32F4+)
- No floating-point operations

## Accordion Types Supported

- **Piano Accordion**: Full support (120-bass Stradella, musette, bellows)
- **Button Accordion** (Chromatic): Supported (requires button mapping)
- **Diatonic Accordion**: Partial support (requires custom note mapping)
- **Bayan**: Supported with free bass layout
- **Concertina**: Bellows expression applicable

## Configuration Presets

### French Bal-Musette
```c
musette_set_style(0, MUSETTE_STYLE_FRENCH);
musette_set_voices(0, MUSETTE_VOICES_3_LMH);
swing_set_amount(0, 58);  // Slight swing
```

### Italian Folk
```c
musette_set_style(0, MUSETTE_STYLE_ITALIAN);
musette_set_voices(0, MUSETTE_VOICES_3_LMH);
bellows_set_curve(0, BELLOWS_CURVE_EXPONENTIAL);
```

### American Swing
```c
musette_set_style(0, MUSETTE_STYLE_AMERICAN);
musette_set_voices(0, MUSETTE_VOICES_2_MH);
swing_set_groove_type(0, GROOVE_TYPE_SWING);
```

### Classical (Dry)
```c
musette_set_style(0, MUSETTE_STYLE_DRY);
musette_set_voices(0, MUSETTE_VOICES_1);
bellows_set_curve(0, BELLOWS_CURVE_LINEAR);
```

## Future Enhancements

Potential additions:
- Register coupling (automatic voice combinations)
- Cassotto simulation (reed chamber resonance)
- Free bass converter (Stradella → chromatic)
- Bellows shake/tremolo detection
- Reed response simulation
- Bellows noise generation

## For Accordion Makers

### Sensor Recommendations
- **Pressure**: XGZP6847D (already supported)
- **Buttons**: Hall effect or optical switches
- **Keys**: Velocity-sensitive FSR or optical

### Calibration Process
1. Record min/max bellows pressure during playing
2. Adjust `bellows_set_pressure_range()` accordingly
3. Test different curves for player preference
4. Fine-tune musette detune amounts

### Quality Checklist
- [ ] Bellows response feels natural
- [ ] Musette sounds authentic
- [ ] Bass chords are well-balanced
- [ ] No latency or jitter
- [ ] All registers function correctly

## Support

For accordion-specific questions or custom development:
- Open GitHub issue with "Accordion:" prefix
- Include your accordion type and configuration
- Provide sensor specifications if applicable

---

**Built for accordion makers, by accordion enthusiasts.**
