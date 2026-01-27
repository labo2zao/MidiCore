# MidiCore Module Inventory

This document provides a comprehensive inventory of all modules in the MidiCore firmware,
their functions, configurable parameters, and CLI access methods.

## Table of Contents

1. [System Modules](#system-modules)
2. [MIDI Modules](#midi-modules)
3. [Input Modules](#input-modules)
4. [Effect Modules](#effect-modules)
5. [Generator Modules](#generator-modules)
6. [Looper Module](#looper-module)
7. [Accordion-Specific Modules](#accordion-specific-modules)
8. [UI Modules](#ui-modules)
9. [CLI Usage Examples](#cli-usage-examples)

---

## System Modules

### bootloader
**Category:** System  
**Description:** USB MIDI Bootloader for firmware updates  
**Functions:**
- `bootloader_init()` - Initialize bootloader
- `bootloader_check_application()` - Check if valid application exists
- `bootloader_jump_to_application()` - Jump to application code
- `bootloader_request_entry()` - Request entry into bootloader mode
- `bootloader_process_sysex()` - Process firmware update SysEx

**Parameters:** None (read-only system module)

### config
**Category:** System  
**Description:** Global system configuration  
**Functions:**
- Configuration loading from SD card
- SRIO, DIN, DOUT settings
- UI configuration
- Safe mode settings

**Parameters:**
- `srio_enable` (bool) - Enable SRIO subsystem
- `srio_din_enable` (bool) - Enable DIN scanning
- `srio_dout_enable` (bool) - Enable DOUT output
- `ui_shift_hold_ms` (int) - Shift button hold time in ms

### watchdog
**Category:** System  
**Description:** System watchdog and health monitoring  
**Functions:**
- Watchdog timer management
- System health checks
- Automatic reset on hang

---

## MIDI Modules

### router
**Category:** MIDI  
**Description:** MIDI routing matrix (16x16 nodes)  
**Functions:**
- `router_init()` - Initialize router
- `router_send()` - Send MIDI message through router
- `router_set_route()` - Configure routing

**Parameters:**
- Per-node routing configuration
- Enable/disable per route

### midi_filter
**Category:** MIDI / Effect  
**Description:** MIDI message filtering and routing control  
**Functions:**
- `midi_filter_init()` - Initialize filter
- `midi_filter_set_enabled()` - Enable/disable filter
- `midi_filter_process()` - Filter MIDI messages

**Parameters:**
- `enabled` (bool) - Enable filter
- `message_types` (bitmask) - Filter message types (note, CC, etc.)
- `channel_mode` (enum) - Channel filter mode
- `min_note` (int 0-127) - Minimum note to pass
- `max_note` (int 0-127) - Maximum note to pass

### midi_delay
**Category:** MIDI / Effect  
**Description:** MIDI delay/echo with tempo sync  
**Functions:**
- `midi_delay_init()` - Initialize delay
- `midi_delay_set_enabled()` - Enable/disable
- `midi_delay_set_tempo()` - Set tempo
- `midi_delay_tick_1ms()` - Process delay

**Parameters:**
- `enabled` (bool) - Enable delay
- `division` (enum) - Time division (1/4, 1/8, 1/16, etc.)
- `feedback` (int 0-100) - Feedback amount in %
- `mix` (int 0-100) - Wet/dry mix in %

### midi_converter
**Category:** MIDI / Effect  
**Description:** Convert between MIDI message types  
**Functions:**
- `midi_converter_init()` - Initialize converter
- `midi_converter_set_mode()` - Set conversion mode

**Parameters:**
- `mode` (enum) - Conversion mode (CC_TO_AFTERTOUCH, etc.)
- `source_cc` (int 0-127) - Source CC number
- `target_cc` (int 0-127) - Target CC number

---

## Input Modules

### ain
**Category:** Input  
**Description:** Analog input (Hall sensor keyboard)  
**Functions:**
- `ain_init()` - Initialize AIN
- `ain_tick_5ms()` - 5ms tick for scanning
- `ain_pop_event()` - Get next key event

**Parameters:**
- `enable` (bool) - Enable AIN scanning
- `velocity_enable` (bool) - Enable velocity sensing
- `scan_ms` (int) - Scan interval in ms
- `deadband` (int) - ADC deadband

### ainser
**Category:** Input  
**Description:** AINSER64 analog input (64 channels via SPI)  
**Functions:**
- `ainser_map_init_defaults()` - Initialize mappings
- `ainser_map_process()` - Process ADC values

**Parameters:**
- Per-channel configuration:
  - `cc` (int 0-127) - MIDI CC number
  - `channel` (int 0-15) - MIDI channel
  - `curve` (enum) - Response curve
  - `invert` (bool) - Invert value
  - `min` (int 0-4095) - Min ADC value
  - `max` (int 0-4095) - Max ADC value
  - `threshold` (int) - Change threshold

### srio
**Category:** Input/Output  
**Description:** Shift register I/O (74HC165/595)  
**Functions:**
- DIN (Digital Input) scanning
- DOUT (Digital Output) control
- Configurable scan rate

**Parameters:**
- `scan_ms` (int) - Scan interval in ms
- `din_bytes` (int) - Number of DIN bytes
- `dout_bytes` (int) - Number of DOUT bytes

### din
**Category:** Input  
**Description:** Digital input mapping (buttons to MIDI)  
**Functions:**
- `din_map_init_defaults()` - Initialize mappings
- `din_map_process_event()` - Process button event

**Parameters:**
- Per-button configuration:
  - `enabled` (bool) - Enable button
  - `type` (enum) - Note, CC, or other
  - `channel` (int 0-15) - MIDI channel
  - `number` (int 0-127) - Note/CC number
  - `velocity` (int 1-127) - Note velocity

### footswitch
**Category:** Input  
**Description:** Footswitch input (8 switches)  
**Functions:**
- `footswitch_init()` - Initialize footswitches
- `footswitch_scan()` - Scan inputs
- `footswitch_is_pressed()` - Check state

**Parameters:**
- Per-footswitch action mapping

### pressure / bellows_expression
**Category:** Input / Accordion  
**Description:** Bellows pressure sensor (I2C XGZP6847D)  
**Functions:**
- `bellows_init()` - Initialize bellows sensor
- `bellows_set_curve()` - Set expression curve
- `bellows_set_pressure_range()` - Calibrate range

**Parameters:**
- `curve` (enum) - Linear, Log, Exponential
- `min_pa` (int) - Minimum pressure in Pa
- `max_pa` (int) - Maximum pressure in Pa
- `bidirectional` (bool) - Enable push/pull detection

---

## Effect Modules

### arpeggiator
**Category:** Effect / Generator  
**Description:** MIDI arpeggiator  
**Functions:**
- `arp_init()` - Initialize arpeggiator
- `arp_set_enabled()` - Enable/disable
- `arp_set_pattern()` - Set pattern

**Parameters:**
- `enabled` (bool) - Enable arpeggiator
- `pattern` (enum) - UP, DOWN, UP_DOWN, RANDOM, AS_PLAYED
- `rate` (enum) - Time division
- `octaves` (int 1-4) - Number of octaves

### note_repeat
**Category:** Effect  
**Description:** Note repeat/ratchet/stutter (MPC-style)  
**Functions:**
- `note_repeat_init()` - Initialize
- `note_repeat_set_enabled()` - Enable/disable
- `note_repeat_set_rate()` - Set repeat rate

**Parameters:**
- `enabled` (bool) - Enable repeat
- `rate` (enum) - Repeat rate division
- `gate` (int 1-100) - Gate length %
- `accent` (int 0-127) - Accent velocity

### quantizer
**Category:** Effect  
**Description:** Timing quantizer for MIDI notes  
**Functions:**
- `quantizer_init()` - Initialize
- `quantizer_set_enabled()` - Enable/disable
- `quantizer_set_grid()` - Set quantization grid

**Parameters:**
- `enabled` (bool) - Enable quantization
- `grid` (enum) - Grid resolution (1/4, 1/8, 1/16, etc.)
- `strength` (int 0-100) - Quantization strength %
- `swing` (int 0-100) - Swing amount %

### humanize
**Category:** Effect  
**Description:** Humanize timing and velocity  
**Functions:**
- `humanize_init()` - Initialize
- `humanize_time_ms()` - Get time offset
- `humanize_vel_delta()` - Get velocity offset

**Parameters:**
- `time_amount` (int 0-100) - Timing variation amount
- `velocity_amount` (int 0-100) - Velocity variation amount

### swing
**Category:** Effect  
**Description:** Swing/groove timing  
**Functions:**
- Apply swing to MIDI timing

**Parameters:**
- `amount` (int 0-100) - Swing amount %

### channelizer
**Category:** Effect / MIDI  
**Description:** Intelligent channel mapping and voice management  
**Functions:**
- `channelizer_init()` - Initialize
- Zone-based channel routing
- Voice stealing

**Parameters:**
- Per-zone configuration:
  - `enabled` (bool) - Enable zone
  - `note_min` (int 0-127) - Min note
  - `note_max` (int 0-127) - Max note
  - `output_channel` (int 0-15) - Output channel
  - `transpose` (int -127 to 127) - Transpose semitones

### harmonizer
**Category:** Effect  
**Description:** MIDI harmonizer (adds harmony notes)  
**Functions:**
- `harmonizer_init()` - Initialize
- `harmonizer_set_enabled()` - Enable/disable
- `harmonizer_set_voice_interval()` - Set interval

**Parameters:**
- `enabled` (bool) - Enable harmonizer
- Per-voice (up to 4):
  - `interval` (enum) - THIRD, FIFTH, SEVENTH, OCTAVE
  - `enabled` (bool) - Enable voice

### chord
**Category:** Effect  
**Description:** Chord trigger - single note to chord  
**Functions:**
- `chord_init()` - Initialize
- `chord_set_enabled()` - Enable/disable
- `chord_set_type()` - Set chord type

**Parameters:**
- `enabled` (bool) - Enable chord trigger
- `type` (enum) - MAJOR, MINOR, DIMINISHED, AUGMENTED, etc.
- `inversion` (int 0-3) - Chord inversion
- `voicing` (enum) - CLOSE, SPREAD, DROP2

### strum
**Category:** Effect  
**Description:** Guitar-style strum effect  
**Functions:**
- Strum chords with timing spread

**Parameters:**
- `enabled` (bool) - Enable strum
- `delay_ms` (int) - Delay between notes in ms
- `direction` (enum) - UP, DOWN, ALTERNATE

### musette_detune
**Category:** Effect / Accordion  
**Description:** Classic accordion musette/chorus  
**Functions:**
- `musette_init()` - Initialize
- `musette_set_style()` - Set tuning style

**Parameters:**
- `style` (enum) - DRY, SCOTTISH, AMERICAN, FRENCH, ITALIAN
- `voices` (enum) - Voice configuration (L-M-M-H)
- `detune_cents` (int) - Detune amount in cents

### gate_time
**Category:** Effect  
**Description:** Note length/gate time control  
**Functions:**
- `gate_time_init()` - Initialize
- `gate_time_set_enabled()` - Enable/disable
- `gate_time_set_mode()` - Set mode

**Parameters:**
- `enabled` (bool) - Enable gate control
- `mode` (enum) - FIXED, PERCENT, ADD_SUBTRACT
- `value` (int) - Gate value (depends on mode)

### legato
**Category:** Effect  
**Description:** Legato/mono/priority handling  
**Functions:**
- `legato_init()` - Initialize
- `legato_set_mode()` - Set priority mode

**Parameters:**
- `mode` (enum) - LOWEST, HIGHEST, LAST
- `retrigger` (enum) - ALWAYS, NEVER, LEGATO_ONLY
- `portamento_time_ms` (int) - Glide time

### velocity_compressor
**Category:** Effect  
**Description:** Velocity dynamics compression  
**Functions:**
- Apply compression to velocity

**Parameters:**
- `threshold` (int 1-127) - Compression threshold
- `ratio` (float) - Compression ratio
- `makeup_gain` (int) - Output gain

### cc_smoother
**Category:** Effect  
**Description:** MIDI CC smoother (eliminate zipper noise)  
**Functions:**
- `cc_smoother_init()` - Initialize
- `cc_smoother_set_enabled()` - Enable/disable

**Parameters:**
- `enabled` (bool) - Enable smoothing
- `mode` (enum) - LIGHT, MEDIUM, HEAVY, CUSTOM
- `amount` (int 0-255) - Smoothing amount

### envelope_cc
**Category:** Effect / Generator  
**Description:** ADSR envelope to CC output  
**Functions:**
- `envelope_cc_init()` - Initialize
- `envelope_cc_set_enabled()` - Enable/disable

**Parameters:**
- `enabled` (bool) - Enable envelope
- `channel` (int 0-15) - Output channel
- `cc_number` (int 0-127) - CC to modulate
- `attack_ms` (int) - Attack time
- `decay_ms` (int) - Decay time
- `sustain` (int 0-127) - Sustain level
- `release_ms` (int) - Release time

### lfo
**Category:** Effect / Generator  
**Description:** Low Frequency Oscillator for modulation  
**Functions:**
- `lfo_init()` - Initialize
- `lfo_set_enabled()` - Enable/disable
- `lfo_tick_1ms()` - Process LFO

**Parameters:**
- `enabled` (bool) - Enable LFO
- `waveform` (enum) - SINE, TRIANGLE, SQUARE, SAW, RANDOM
- `rate_hz` (float) - LFO rate in Hz
- `depth` (int 0-127) - Modulation depth
- `target` (enum) - Modulation target (CC, pitch, etc.)

---

## Looper Module

### looper
**Category:** Looper  
**Description:** Multi-track looper/sequencer (LoopA-inspired)  
**Functions:**
- `looper_init()` - Initialize looper
- Transport control (play, stop, record)
- Track management (4 tracks)
- Scene management (8 scenes)
- Pattern editing
- Undo/redo
- Quantization
- Clipboard operations

**Parameters:**
- `bpm` (int 20-300) - Tempo
- `time_signature_num` (int) - Time signature numerator
- `time_signature_den` (int) - Time signature denominator
- Per-track:
  - `mute` (bool) - Mute track
  - `solo` (bool) - Solo track
  - `quantize` (bool) - Enable quantization
  - `midi_channel` (int 0-15) - MIDI channel

**Sub-modules:**
- Pattern editor
- Piano roll UI
- Automation

---

## Accordion-Specific Modules

### bass_chord_system
**Category:** Accordion  
**Description:** Stradella bass for accordion  
**Functions:**
- `bass_chord_init()` - Initialize
- `bass_chord_set_layout()` - Set bass layout

**Parameters:**
- `layout` (enum) - STRADELLA, FREE_BASS, BELGIAN
- `base_note` (int 0-127) - Starting note
- `octave_doubling` (bool) - Enable octave doubling

### one_finger_chord
**Category:** Accordion / Effect  
**Description:** One-finger chord accompaniment  
**Functions:**
- Single key triggers full chord

**Parameters:**
- `enabled` (bool) - Enable one-finger chords
- `chord_type` (enum) - Chord types to generate

### bellows_shake
**Category:** Accordion / Effect  
**Description:** Tremolo from bellows shaking  
**Functions:**
- `bellows_shake_init()` - Initialize
- `bellows_shake_set_enabled()` - Enable/disable

**Parameters:**
- `enabled` (bool) - Enable shake detection
- `sensitivity` (int 0-100) - Detection sensitivity
- `depth` (int 0-127) - Tremolo depth
- `target` (enum) - MOD_WHEEL, VOLUME, FILTER

### register_coupling
**Category:** Accordion  
**Description:** Accordion register switching  
**Functions:**
- Register combination management

**Parameters:**
- Register button mappings

### assist_hold
**Category:** Accordion / Accessibility  
**Description:** Auto-hold for motor disabilities  
**Functions:**
- `assist_hold_init()` - Initialize
- `assist_hold_set_mode()` - Set hold mode

**Parameters:**
- `mode` (enum) - OFF, PERMANENT, TIMED
- `duration_ms` (int) - Hold duration (for timed mode)
- `velocity_threshold` (int 1-127) - Min velocity to hold

---

## Generator Modules

### metronome
**Category:** Generator  
**Description:** Metronome synchronized to looper  
**Functions:**
- `metronome_init()` - Initialize
- `metronome_set_enabled()` - Enable/disable

**Parameters:**
- `enabled` (bool) - Enable metronome
- `mode` (enum) - MIDI, AUDIO
- `midi_channel` (int 0-15) - Output channel
- `accent_note` (int 0-127) - Downbeat note
- `regular_note` (int 0-127) - Regular beat note
- `accent_velocity` (int 1-127) - Downbeat velocity
- `regular_velocity` (int 1-127) - Beat velocity

### rhythm_trainer
**Category:** Generator / Accessibility  
**Description:** Rhythm training assistance  
**Functions:**
- Practice mode with rhythm guidance

**Parameters:**
- Training patterns
- Difficulty level

---

## UI Modules

### ui
**Category:** UI  
**Description:** User interface system (OLED + encoders)  
**Functions:**
- Page management
- Menu navigation
- Encoder input
- Button handling

**Sub-modules:**
- ui_page_looper
- ui_page_midi_monitor
- ui_page_config
- ui_page_livefx
- ui_page_sysex

### input
**Category:** UI / Input  
**Description:** Input event processing (buttons, encoders)  
**Functions:**
- `input_init()` - Initialize input system
- `input_tick()` - Process inputs
- `input_feed_button()` - Handle button event
- `input_feed_encoder()` - Handle encoder event

**Parameters:**
- `debounce_ms` (int) - Button debounce time
- `shift_hold_ms` (int) - Shift long-press time

---

## Other Modules

### patch
**Category:** System  
**Description:** Patch/preset management on SD card  
**Functions:**
- Load/save patches
- Bank management

### dream
**Category:** Output  
**Description:** Dream SAM5716 sampler control  
**Functions:**
- SysEx communication with sampler

### scale
**Category:** Effect  
**Description:** Scale quantization  
**Functions:**
- Force notes to scale

**Parameters:**
- `scale_type` (enum) - Major, Minor, Dorian, etc.
- `root_note` (int 0-11) - Root note (C=0)

### livefx
**Category:** Effect  
**Description:** Live FX system for real-time control  
**Functions:**
- `livefx_init()` - Initialize
- `livefx_set_transpose()` - Set transpose

**Parameters:**
- `transpose` (int -12 to 12) - Transpose semitones
- `velocity_scale` (int 0-200) - Velocity scale %
- `force_scale` (bool) - Force to scale

### zones
**Category:** MIDI  
**Description:** Keyboard zone management  
**Functions:**
- Split keyboard into zones

### expression
**Category:** Input / MIDI  
**Description:** Expression pedal input  
**Functions:**
- Map expression pedal to MIDI CC

### nrpn_helper
**Category:** MIDI  
**Description:** NRPN message generation/parsing  
**Functions:**
- Build and parse NRPN messages

### note_stabilizer
**Category:** Effect  
**Description:** Stabilize note timing and velocity  
**Functions:**
- Reduce jitter in note events

---

## CLI Usage Examples

### List All Modules
```
module list
```

### Get Module Information
```
module info looper
module info arpeggiator
```

### Enable/Disable Module
```
module enable arpeggiator
module disable arpeggiator 0    # Disable for track 0
```

### Get Module Status
```
module status looper
module status harmonizer 1      # Status for track 1
```

### List Module Parameters
```
module params arpeggiator
module params midi_filter
```

### Get Parameter Value
```
module get arpeggiator pattern
module get looper bpm
module get midi_filter enabled 0
```

### Set Parameter Value
```
module set arpeggiator pattern UP
module set arpeggiator enabled true
module set looper bpm 120
module set midi_filter enabled true 0
module set harmonizer interval FIFTH 1
```

### Configuration Persistence
```
config save 0:/midicore.ini     # Save to SD card
config load 0:/midicore.ini     # Load from SD card
config list                     # List all config entries
config get arpeggiator.pattern
config set arpeggiator.pattern DOWN
```

### System Commands
```
help                    # Show all commands
help module             # Show help for module command
version                 # Show firmware version
uptime                  # Show system uptime
status                  # Show system status
reboot                  # Reboot system
clear                   # Clear screen
```

---

## Notes for UI Integration

When integrating these CLI commands into the UI menu system with rotary encoders:

1. **Module Selection Menu:**
   - Use `module list` to populate a scrollable menu
   - Group by category
   - Show enabled/disabled status

2. **Parameter Editing:**
   - Use `module params <name>` to get parameters
   - Show parameter type and range
   - Map rotary encoder to value adjustment
   - Use `module set` to apply changes

3. **Quick Access:**
   - Common parameters (BPM, transpose, etc.) in top-level menu
   - Module enable/disable as toggle buttons
   - Parameter presets for quick recall

4. **Visual Feedback:**
   - Show current values on OLED
   - Indicate when parameter is being edited
   - Confirm saves to SD card

---

## Module Integration Checklist

When adding CLI support to a new module:

1. ☐ Create module descriptor in module_registry
2. ☐ Define all parameters with types and ranges
3. ☐ Implement getter/setter functions for each parameter
4. ☐ Register module at init time
5. ☐ Test CLI commands (get, set, enable, disable)
6. ☐ Add to this documentation
7. ☐ Update UI menus if applicable
