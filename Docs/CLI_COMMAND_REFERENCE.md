# MidiCore CLI Command Reference

**Comprehensive command-line interface documentation for all MidiCore modules**

Version: 1.0  
Last Updated: 2025-01-28

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [System Commands](#system-commands)
3. [Module Commands](#module-commands)
4. [Command Syntax](#command-syntax)
5. [MIDI Modules](#midi-modules)
6. [Effect Modules](#effect-modules)
7. [Input Modules](#input-modules)
8. [Generator Modules](#generator-modules)
9. [Looper Module](#looper-module)
10. [Accordion Modules](#accordion-modules)
11. [System Modules](#system-modules)
12. [Configuration & Persistence](#configuration--persistence)
13. [Quick Reference](#quick-reference)

---

## Getting Started

### Accessing the CLI

The CLI is available via:
- **USB CDC (Virtual COM Port)**: Primary interface
- **UART**: Hardware serial port (default 31250 baud for MIDI compatibility)
- **MIOS Studio Terminal**: Full SysEx-based terminal with color support

### Basic Usage

```bash
# Show all available commands
help

# Show help for specific command
help module

# List all modules
module list

# Get module information
module info <module_name>
```

---

## System Commands

### Built-in Commands

#### `help [command]`
Display help information for all commands or a specific command.

```bash
help                    # Show all commands
help module             # Show module command help
help config             # Show config command help
```

#### `version`
Display firmware version and build information.

```bash
version
# Output: MidiCore v1.2.3 (Build: Jan 28 2025, 14:30:15)
```

#### `uptime`
Show system uptime since boot.

```bash
uptime
# Output: Uptime: 0d 2h 15m 42s
```

#### `status`
Display system status (CPU, RAM, tasks, MIDI activity).

```bash
status
# Output:
# CPU: 45% | RAM: 85.2KB/128KB (66%) | Tasks: 12
# MIDI IN: 1234 msgs | MIDI OUT: 5678 msgs
# Router: 16 routes active
```

#### `reboot`
Reboot the system (requires confirmation).

```bash
reboot
# Prompt: Are you sure? Type 'yes' to confirm:
yes
```

#### `clear`
Clear the terminal screen.

```bash
clear
```

#### `list`
List available modules and components.

```bash
list                    # Alias for 'module list'
```

---

## Module Commands

### Core Module Syntax

All modules follow a consistent command structure:

```bash
module <action> <module_name> [arguments]
```

### Module Actions

#### `module list`
List all registered modules with their categories and states.

```bash
module list

# Output:
# MIDI Modules:
#   [✓] router              - MIDI routing matrix (16x16 nodes)
#   [✓] midi_filter         - MIDI message filtering (4 tracks)
#   [✓] midi_delay          - MIDI delay/echo (4 tracks)
#   
# Effect Modules:
#   [✓] arpeggiator         - MIDI arpeggiator
#   [ ] harmonizer          - MIDI harmonizer (4 tracks)
#   ...
```

#### `module info <name>`
Display detailed information about a module.

```bash
module info looper

# Output:
# Module: looper
# Category: Looper
# Description: Multi-track looper/sequencer (LoopA-inspired)
# Status: Enabled
# Tracks: 4
# Parameters: 10
#   - bpm (int, 20-300): Tempo
#   - time_sig_num (int, 1-16): Time signature numerator
#   - state (enum): Track state
#   - mute (bool): Mute track
#   ...
```

#### `module enable <name> [track]`
Enable a module or specific track.

```bash
module enable arpeggiator           # Enable global module
module enable midi_filter 0         # Enable track 0
```

#### `module disable <name> [track]`
Disable a module or specific track.

```bash
module disable harmonizer 1         # Disable track 1
```

#### `module status <name> [track]`
Check if module/track is enabled.

```bash
module status looper 0

# Output: Track 0: ENABLED (State: PLAY)
```

#### `module params <name>`
List all parameters for a module.

```bash
module params midi_filter

# Output:
# Parameters for midi_filter (track-based):
#   enabled (bool): Enable filter
#   channel_mode (enum): Channel filter mode
#     Values: ALL, ALLOW, BLOCK
#   min_note (int, 0-127): Minimum note to pass
#   max_note (int, 0-127): Maximum note to pass
#   ...
```

#### `module get <name> <param> [track]`
Get parameter value.

```bash
module get looper bpm                    # Global parameter
module get midi_filter enabled 0         # Track 0 parameter
module get arpeggiator pattern           # Global enum parameter
```

#### `module set <name> <param> <value> [track]`
Set parameter value.

```bash
module set looper bpm 120
module set midi_filter enabled true 0
module set arpeggiator pattern UP
module set harmonizer voice1_interval THIRD_UP 0
```

---

## Command Syntax

### Parameter Types

#### Boolean Parameters
Accepts: `true`, `false`, `1`, `0`, `on`, `off`, `yes`, `no`

```bash
module set midi_filter enabled true 0
module set looper mute false 1
```

#### Integer Parameters
Numeric values within specified range.

```bash
module set looper bpm 120              # Range: 20-300
module set midi_filter min_note 36 0   # Range: 0-127
module set envelope_cc attack_ms 100 0 # Range: 0-5000
```

#### Enum Parameters
Named values from a predefined set.

```bash
module set looper state REC 0
module set arpeggiator pattern UP_DOWN
module set quantizer resolution 1_16 0
module set chord type MINOR 0
```

### Track Indexing

For per-track modules, track index is 0-based (0-3 for 4 tracks).

```bash
module set looper state PLAY 0     # Track 0
module set looper state REC 1      # Track 1
module set harmonizer enabled true 2   # Track 2
```

### Error Handling

Commands return error messages for invalid input:

```bash
module set looper bpm 500
# Error: Value out of range (20-300)

module set arpeggiator pattern INVALID
# Error: Invalid enum value. Valid: UP, DOWN, UP_DOWN, RANDOM, AS_PLAYED

module set midi_filter enabled true 10
# Error: Invalid track index (0-3)
```

---

## MIDI Modules

### router
**MIDI routing matrix (16x16 nodes)**

```bash
# Enable/disable routes
router set <in_node> <out_node> <1|0>
router set 0 1 1                    # Route node 0 → 1

# Configure channel masks
router chanmask <in> <out> <mask>
router chanmask 0 1 all             # All channels
router chanmask 0 1 1               # Channel 1 only
router chanmask 0 1 1-8             # Channels 1-8

# Display routing matrix
router show

# Test routing
router test <in> <out>
router test 0 1                     # Test route 0 → 1
```

### midi_filter
**MIDI message filtering (4 tracks)**

```bash
# Enable/disable
module enable midi_filter 0
module disable midi_filter 1

# Set channel mode
module set midi_filter channel_mode ALL 0
module set midi_filter channel_mode ALLOW 1
module set midi_filter channel_mode BLOCK 2

# Set note range
module set midi_filter min_note 36 0
module set midi_filter max_note 96 0

# Set velocity range
module set midi_filter min_velocity 10 0
module set midi_filter max_velocity 120 0

# Get current settings
module get midi_filter channel_mode 0
module get midi_filter min_note 0
```

**Channel Modes:**
- `ALL`: Pass all channels
- `ALLOW`: Only pass specified channels
- `BLOCK`: Block specified channels

### midi_delay
**MIDI delay/echo with tempo sync (4 tracks)**

```bash
# Enable delay
module enable midi_delay 0

# Set time division
module set midi_delay division 1_8 0

# Set feedback and mix
module set midi_delay feedback 60 0      # 0-100%
module set midi_delay mix 30 0           # Wet/dry: 0-100%

# Set velocity decay
module set midi_delay velocity_decay 20 0  # Per repeat: 0-100%
```

**Time Divisions:**
`1_64`, `1_32`, `1_16`, `1_8`, `1_4`, `1_2`, `1_1`, `1_32T`, `1_16T`, `1_8T`, `1_4T`, `1_16_DOT`, `1_8_DOT`

### midi_converter
**Convert between MIDI message types (4 tracks)**

```bash
# Enable converter
module enable midi_converter 0

# Set conversion mode
module set midi_converter mode CC_TO_AT 0
module set midi_converter mode AT_TO_CC 1
module set midi_converter mode PB_TO_CC 2
module set midi_converter mode VEL_TO_CC 3

# Configure CC numbers
module set midi_converter source_cc 1 0      # Mod wheel
module set midi_converter dest_cc 11 0       # Expression

# Set scaling
module set midi_converter scale 100 0        # 0-200%
module set midi_converter offset 0 0         # -127 to 127
module set midi_converter invert false 0
```

**Conversion Modes:**
- `CC_TO_AT`: CC → Aftertouch
- `AT_TO_CC`: Aftertouch → CC
- `PB_TO_CC`: Pitch Bend → CC
- `CC_TO_PB`: CC → Pitch Bend
- `VEL_TO_CC`: Velocity → CC
- `CC_TO_CC`: CC → CC (with scaling)
- `NOTE_TO_CC`: Note → CC
- `CC_TO_NOTE`: CC → Note

---

## Effect Modules

### arpeggiator
**MIDI arpeggiator (global)**

```bash
# Enable/disable
module enable arpeggiator
module disable arpeggiator

# Set pattern
module set arpeggiator pattern UP
module set arpeggiator pattern DOWN
module set arpeggiator pattern UP_DOWN
module set arpeggiator pattern RANDOM
module set arpeggiator pattern AS_PLAYED

# Get status
module status arpeggiator
module get arpeggiator pattern
```

### quantizer
**Timing quantizer for MIDI notes (4 tracks)**

```bash
# Enable quantization
module enable quantizer 0

# Set grid resolution
module set quantizer resolution 1_16 0
module set quantizer resolution 1_8T 0    # Triplet
module set quantizer resolution 1_8_DOT 0 # Dotted

# Set strength (0-100%)
module set quantizer strength 75 0

# Set lookahead window
module set quantizer lookahead 50 0       # 0-500ms

# Set swing
module set quantizer swing 65 0           # 0-100% (50=straight)
```

**Resolutions:**
`1_4`, `1_8`, `1_16`, `1_32`, `1_8T`, `1_16T`, `1_4_DOT`, `1_8_DOT`

### harmonizer
**MIDI harmonizer - adds harmony notes (4 tracks)**

```bash
# Enable harmonizer
module enable harmonizer 0

# Configure voice 1
module set harmonizer voice1_enabled true 0
module set harmonizer voice1_interval THIRD_UP 0

# Configure voice 2
module set harmonizer voice2_enabled true 0
module set harmonizer voice2_interval FIFTH_UP 0

# Check settings
module get harmonizer voice1_interval 0
```

**Intervals:**
`UNISON`, `THIRD_UP`, `THIRD_DOWN`, `FIFTH_UP`, `FIFTH_DOWN`, `OCTAVE_UP`, `OCTAVE_DOWN`

### velocity_compressor
**Velocity dynamics compression (4 tracks)**

```bash
# Enable compressor
module enable velocity_compressor 0

# Set threshold
module set velocity_compressor threshold 80 0    # 1-127

# Set compression ratio
module set velocity_compressor ratio 4_1 0

# Set makeup gain
module set velocity_compressor makeup_gain 10 0  # 0-127

# Set knee type
module set velocity_compressor knee SOFT 0
```

**Ratios:** `1_1`, `2_1`, `3_1`, `4_1`, `8_1`, `INF_1` (limiter)  
**Knee Types:** `HARD`, `SOFT`

### cc_smoother
**MIDI CC smoother - eliminate zipper noise (4 tracks)**

```bash
# Enable smoother
module enable cc_smoother 0

# Set smoothing mode
module set cc_smoother mode MEDIUM 0

# Custom smoothing
module set cc_smoother mode CUSTOM 0
module set cc_smoother amount 128 0        # 0-255
module set cc_smoother attack 50 0         # 0-1000ms
module set cc_smoother release 200 0       # 0-1000ms
```

**Modes:** `OFF`, `LIGHT`, `MEDIUM`, `HEAVY`, `CUSTOM`

### envelope_cc
**ADSR envelope to CC output (4 tracks)**

```bash
# Enable envelope
module enable envelope_cc 0

# Set output
module set envelope_cc channel 0 0
module set envelope_cc cc_number 74 0      # Filter cutoff

# Set ADSR
module set envelope_cc attack_ms 10 0      # 0-5000ms
module set envelope_cc decay_ms 100 0
module set envelope_cc sustain 80 0        # 0-127
module set envelope_cc release_ms 500 0
```

### lfo
**Low Frequency Oscillator for modulation (4 tracks)**

```bash
# Enable LFO
module enable lfo 0

# Set waveform
module set lfo waveform SINE 0
module set lfo waveform TRIANGLE 0
module set lfo waveform SQUARE 0

# Set rate (Hz * 100, so 100 = 1.00 Hz)
module set lfo rate_hz 50 0               # 0.50 Hz

# Set depth
module set lfo depth 64 0                 # 0-127

# Set target
module set lfo target CC 0
module set lfo target PITCH 0
```

**Waveforms:** `SINE`, `TRIANGLE`, `SQUARE`, `SAW_UP`, `SAW_DOWN`, `RANDOM`  
**Targets:** `CC`, `PITCH`, `VELOCITY`, `TIMING`

### channelizer
**Intelligent channel mapping and voice management (4 tracks)**

```bash
# Set mode
module set channelizer mode FORCE 0
module set channelizer mode REMAP 0
module set channelizer mode ZONE 0

# Force all to single channel
module set channelizer force_channel 5 0

# Set voice limit
module set channelizer voice_limit 8 0    # 1-16
```

**Modes:** `BYPASS`, `FORCE`, `REMAP`, `ROTATE`, `ZONE`

### chord
**Chord trigger - single note to chord (4 tracks)**

```bash
# Enable chord trigger
module enable chord 0

# Set chord type
module set chord type MAJOR 0
module set chord type MINOR 0
module set chord type DOM7 0

# Set inversion
module set chord inversion 1 0            # 0-3

# Set voicing
module set chord voicing CLOSE 0
module set chord voicing DROP2 0
```

**Chord Types:**
`MAJOR`, `MINOR`, `DIM`, `AUG`, `MAJ7`, `MIN7`, `DOM7`, `SUS2`, `SUS4`

**Voicings:** `CLOSE`, `SPREAD`, `DROP2`, `DROP3`

### scale
**Scale quantization (global)**

```bash
# Set scale type
module set scale scale_type MAJOR
module set scale scale_type MINOR_HAR
module set scale scale_type PENTATONIC_MAJ

# Set root note (C=0)
module set scale root_note 0              # C
module set scale root_note 7              # G
```

**Scale Types:**
`CHROMATIC`, `MAJOR`, `MINOR_NAT`, `MINOR_HAR`, `MINOR_MEL`, `DORIAN`, `PHRYGIAN`, `LYDIAN`, `MIXOLYDIAN`, `LOCRIAN`, `PENTATONIC_MAJ`, `PENTATONIC_MIN`, `BLUES`, `WHOLE_TONE`

### legato
**Legato/mono/priority handling (4 tracks)**

```bash
# Enable legato mode
module enable legato 0

# Set note priority
module set legato priority LAST 0
module set legato priority HIGHEST 0

# Set retrigger mode
module set legato retrigger ON 0

# Set glide time
module set legato glide_time 100 0        # 0-2000ms

# Enable mono mode
module set legato mono_mode true 0
```

**Priority:** `LAST`, `HIGHEST`, `LOWEST`, `FIRST`  
**Retrigger:** `OFF`, `ON`

### note_repeat
**Note repeat/ratchet/stutter - MPC-style (4 tracks)**

```bash
# Enable repeat
module enable note_repeat 0

# Set repeat rate
module set note_repeat rate 1_16 0

# Set gate length
module set note_repeat gate 50 0          # 1-100%

# Set velocity decay
module set note_repeat velocity_decay 10 0 # 0-100%
```

**Rates:** `1_4`, `1_8`, `1_16`, `1_32`, `1_8T`, `1_16T`, `1_32T`

### note_stabilizer
**Stabilize note timing and velocity (4 tracks)**

```bash
# Enable stabilizer
module enable note_stabilizer 0

# Set minimum duration
module set note_stabilizer min_duration_ms 50 0  # 10-500ms

# Set retrigger delay
module set note_stabilizer retrigger_delay_ms 100 0  # 10-1000ms

# Set neighbor range
module set note_stabilizer neighbor_range 2 0    # 0-12 semitones
```

### strum
**Guitar-style strum effect (4 tracks)**

```bash
# Enable strum
module enable strum 0

# Set strum time
module set strum time 20 0                # 0-200ms

# Set direction
module set strum direction UP 0
module set strum direction DOWN 0
module set strum direction RANDOM 0

# Enable velocity ramp
module set strum velocity_ramp true 0
```

**Directions:** `UP`, `DOWN`, `UP_DOWN`, `RANDOM`

### swing
**Swing/groove timing (4 tracks)**

```bash
# Enable swing
module enable swing 0

# Set swing amount
module set swing amount 65 0              # 0-100% (50=straight)

# Set resolution
module set swing resolution 16TH 0

# Set groove preset
module set swing groove SHUFFLE 0
```

**Resolutions:** `8TH`, `16TH`, `32ND`  
**Grooves:** `STRAIGHT`, `SWING`, `SHUFFLE`, `HALF_TIME`, `DOUBLE_TIME`

### gate_time
**Note length/gate time control (4 tracks)**

```bash
# Enable gate control
module enable gate_time 0

# Set mode
module set gate_time mode FIXED 0
module set gate_time mode PERCENT 0

# Set value (meaning depends on mode)
module set gate_time value 100 0          # 0-1000
```

**Modes:** `FIXED` (ms), `PERCENT` (%), `ADD_SUBTRACT` (+/- ms)

### humanize
**Humanize timing and velocity (global)**

```bash
# Set timing variation
module set humanize time_amount 25        # 0-100%

# Set velocity variation
module set humanize velocity_amount 15    # 0-100%
```

### livefx
**Live FX system for real-time control (4 tracks)**

```bash
# Enable live FX
module enable livefx 0

# Set transpose
module set livefx transpose -12 0         # -12 to +12 semitones

# Set velocity scale
module set livefx velocity_scale 120 0    # 0-200%

# Force to scale
module set livefx force_scale true 0
```

---

## Input Modules

### ain
**Analog input - Hall sensor keyboard (global)**

```bash
# Enable AIN scanning
module set ain enable true

# Enable velocity sensing
module set ain velocity_enable true

# Set scan interval
module set ain scan_ms 5                  # 1-50ms

# Set ADC deadband
module set ain deadband 10                # 0-100
```

---

## Generator Modules

### metronome
**Metronome synchronized to looper BPM (global)**

```bash
# Enable metronome
module enable metronome

# Set output mode
module set metronome mode MIDI
module set metronome mode AUDIO

# Configure MIDI output
module set metronome midi_channel 9       # Drum channel
module set metronome accent_note 76       # High wood block
module set metronome regular_note 77      # Low wood block
module set metronome accent_velocity 100
module set metronome regular_velocity 80

# Get status
module status metronome
module get metronome mode
```

**Modes:** `OFF`, `MIDI`, `AUDIO`

---

## Looper Module

### looper
**Multi-track looper/sequencer - LoopA-inspired (4 tracks)**

#### Global Transport Commands

```bash
# Set tempo
module set looper bpm 120                 # 20-300 BPM

# Set time signature
module set looper time_sig_num 4          # 1-16
module set looper time_sig_den 4          # 2, 4, 8, or 16

# Enable auto-loop
module set looper auto_loop true

# Get transport settings
module get looper bpm
module get looper time_sig_num
module get looper time_sig_den
```

#### Per-Track Commands

```bash
# Transport control
module set looper state REC 0             # Start recording track 0
module set looper state PLAY 0            # Play track 0
module set looper state OVERDUB 0         # Overdub track 0
module set looper state OVERDUB_CC_ONLY 0 # Re-record CC only
module set looper state OVERDUB_NOTES_ONLY 0  # Re-record notes only
module set looper state STOP 0            # Stop track 0

# Track control
module set looper mute 0 true
module set looper mute 1 false
module set looper solo 1 true

# Set MIDI channel
module set looper midi_channel 0 5

# Set transpose
module set looper transpose 1 -12         # -127 to 127

# Quantization
module set looper quantize 0 1_16
module set looper quantize 1 1_8T
module set looper quantize 2 OFF

# Quick enable/disable
module enable looper 0                    # Set to PLAY
module disable looper 0                   # Set to STOP

# Check status
module status looper 0
module get looper state 0
module get looper mute 1
```

#### Looper States
- `STOP`: Track stopped, no playback
- `REC`: Initial recording of notes and CC
- `PLAY`: Playback only
- `OVERDUB`: Add more notes/CC to existing loop
- `OVERDUB_CC_ONLY`: Re-record CC automation only
- `OVERDUB_NOTES_ONLY`: Re-record MIDI notes only

#### Quantization Modes
`OFF`, `1_16`, `1_8`, `1_4`, `1_32T`, `1_16T`, `1_8T`, `1_2T`, `1_32Q`, `1_16Q`, `1_8Q`, `1_16S`, `1_8S`, `1_16SEPT`, `1_8SEPT`, `1_16_DOT`, `1_8_DOT`, `1_4_DOT`

---

## Accordion Modules

### bellows_expression
**Bellows pressure sensor (global)**

```bash
# Set expression curve
module set bellows_expression curve LINEAR
module set bellows_expression curve EXPONENTIAL
module set bellows_expression curve LOGARITHMIC

# Calibrate pressure range
module set bellows_expression min_pa 0    # 0-5000 Pa
module set bellows_expression max_pa 2000

# Enable bidirectional (push/pull)
module set bellows_expression bidirectional true

# Set CC assignments
module set bellows_expression expression_cc 11  # Expression (0-127)
```

**Curves:** `LINEAR`, `EXPONENTIAL`, `LOGARITHMIC`, `S_CURVE`

### bellows_shake
**Tremolo from bellows shaking (global)**

```bash
# Enable shake detection
module enable bellows_shake

# Set sensitivity
module set bellows_shake sensitivity 50   # 0-100

# Set tremolo depth
module set bellows_shake depth 64         # 0-127

# Set modulation target
module set bellows_shake target MOD_WHEEL
module set bellows_shake target VOLUME
module set bellows_shake target FILTER
```

**Targets:** `MOD_WHEEL`, `VOLUME`, `FILTER`, `BOTH`

### bass_chord_system
**Stradella bass for accordion (4 tracks)**

```bash
# Set bass layout
module set bass_chord_system layout STRADELLA_120 0
module set bass_chord_system layout STRADELLA_96 0
module set bass_chord_system layout FREE_BASS 0

# Set base note
module set bass_chord_system base_note 36 0  # 0-127

# Enable octave doubling
module set bass_chord_system octave_doubling true 0
```

**Layouts:** `STRADELLA_120`, `STRADELLA_96`, `STRADELLA_72`, `STRADELLA_48`, `FREE_BASS`

### register_coupling
**Accordion register switching (4 tracks)**

```bash
# Set register
module set register_coupling register MUSETTE 0
module set register_coupling register BANDONEON 0
module set register_coupling register CLARINET 0

# Enable smooth transitions
module set register_coupling smooth_transition true 0

# Set transition time
module set register_coupling transition_time 100 0  # 0-1000ms
```

**Registers:**
`MASTER`, `MUSETTE`, `BANDONEON`, `VIOLIN`, `CLARINET`, `BASSOON`, `PICCOLO`, `ORGAN`, `OBOE`, `FLUTE`

### musette_detune
**Classic accordion musette/chorus (4 tracks)**

```bash
# Set tuning style
module set musette_detune style FRENCH 0
module set musette_detune style ITALIAN 0
module set musette_detune style CUSTOM 0

# Set custom detune (cents)
module set musette_detune detune_cents 15 0  # 0-50 cents
```

**Styles:** `DRY`, `SCOTTISH`, `AMERICAN`, `FRENCH`, `ITALIAN`, `CUSTOM`

### assist_hold
**Auto-hold for motor disabilities (global)**

```bash
# Set hold mode
module set assist_hold mode TIMED

# Set hold duration (for timed mode)
module set assist_hold duration_ms 500    # 0-10000ms

# Set velocity threshold
module set assist_hold velocity_threshold 30  # 1-127
```

**Modes:** `OFF`, `PERMANENT`, `TIMED`

---

## System Modules

### config
**Global system configuration (global)**

```bash
# Enable SRIO subsystem
module set config srio_enable true

# Enable DIN scanning
module set config srio_din_enable true

# Enable DOUT output
module set config srio_dout_enable true
```

### watchdog
**System watchdog and health monitoring (global)**

```bash
# Check watchdog status
module status watchdog

# Watchdog is always-on, no parameters to configure
```

---

## Configuration & Persistence

### Saving Configuration

```bash
# Save all module settings to SD card
config save 0:/midicore.ini

# Save specific module
config save 0:/looper.ini looper
config save 0:/arpeggiator.ini arpeggiator
```

### Loading Configuration

```bash
# Load all settings
config load 0:/midicore.ini

# Load specific module
config load 0:/looper.ini looper
```

### Configuration File Format

Configuration files use INI format:

```ini
[looper]
bpm = 120
time_sig_num = 4
time_sig_den = 4
auto_loop = true

[looper.track0]
state = PLAY
mute = false
solo = false
quantize = 1_16
midi_channel = 0
transpose = 0

[arpeggiator]
enabled = true
pattern = UP_DOWN

[midi_filter.track0]
enabled = true
channel_mode = ALL
min_note = 36
max_note = 96
```

### Listing Configuration

```bash
# List all configuration entries
config list

# Get specific config value
config get arpeggiator.pattern
config get looper.bpm

# Set config value directly
config set arpeggiator.pattern DOWN
config set looper.bpm 140
```

---

## Quick Reference

### Common Command Patterns

```bash
# Enable/Disable
module enable <name> [track]
module disable <name> [track]
module status <name> [track]

# Get/Set Parameters
module get <name> <param> [track]
module set <name> <param> <value> [track]

# Module Information
module list
module info <name>
module params <name>

# Configuration
config save <filename> [module]
config load <filename> [module]
config get <key>
config set <key> <value>

# System
help [command]
version
uptime
status
reboot
clear
```

### Example Workflows

#### Setting up a looper session

```bash
# Configure transport
module set looper bpm 120
module set looper time_sig_num 4

# Setup tracks
module set looper midi_channel 0 0
module set looper quantize 1_16 0
module set looper midi_channel 5 1
module set looper quantize 1_8 1

# Start recording
module set looper state REC 0

# Switch to playback
module set looper state PLAY 0

# Overdub on track 2
module set looper state OVERDUB 2

# Save session
config save 0:/mysession.ini
```

#### Setting up MIDI effects chain

```bash
# Track 0: Note stabilizer → Quantizer → Harmonizer
module enable note_stabilizer 0
module set note_stabilizer min_duration_ms 50 0

module enable quantizer 0
module set quantizer resolution 1_16 0
module set quantizer strength 80 0

module enable harmonizer 0
module set harmonizer voice1_interval THIRD_UP 0
module set harmonizer voice1_enabled true 0

# Save chain
config save 0:/effects_chain.ini
```

#### Setting up accordion configuration

```bash
# Configure bellows
module set bellows_expression curve EXPONENTIAL
module set bellows_expression min_pa 100
module set bellows_expression max_pa 2000
module set bellows_expression bidirectional true

# Enable bellows shake
module enable bellows_shake
module set bellows_shake sensitivity 60
module set bellows_shake depth 50

# Configure bass system
module set bass_chord_system layout STRADELLA_120 0
module set bass_chord_system octave_doubling true 0

# Set register
module set register_coupling register MUSETTE 0

# Save accordion config
config save 0:/accordion.ini
```

---

## Command History and Auto-completion

### Command History
Use arrow keys to navigate command history:
- `↑` (Up Arrow): Previous command
- `↓` (Down Arrow): Next command

### Tab Completion
Press `Tab` to auto-complete:
- Command names
- Module names
- Parameter names
- Enum values

```bash
module set arp<TAB>                 # Completes to "arpeggiator"
module set looper st<TAB>           # Shows: "state", "status"
module set looper state <TAB>       # Shows: STOP, REC, PLAY, OVERDUB, ...
```

---

## Scripting and Automation

### Batch Commands
Execute multiple commands from a text file:

```bash
# Create script file on SD card: 0:/startup.txt
module enable arpeggiator
module set arpeggiator pattern UP
module set looper bpm 120
module set looper state PLAY 0

# Execute script
script run 0:/startup.txt
```

### Command Chaining
Chain multiple commands with semicolons:

```bash
module enable arpeggiator; module set arpeggiator pattern UP; module status arpeggiator
```

---

## Troubleshooting

### Command Not Found
```bash
# Error: Command 'xyz' not found
help                                # List all commands
```

### Module Not Found
```bash
# Error: Module 'xyz' not found
module list                         # List all modules
```

### Parameter Not Found
```bash
# Error: Parameter 'xyz' not found for module 'looper'
module params looper                # List all parameters
```

### Value Out of Range
```bash
# Error: Value out of range (20-300)
module params looper                # Check valid ranges
```

### Invalid Track Index
```bash
# Error: Invalid track index (0-3)
# Make sure you're using 0-based indexing (0, 1, 2, 3)
```

---

## Additional Resources

- **Module Inventory**: See `Docs/MODULE_INVENTORY.md` for detailed module specifications
- **Architecture**: See `ARCHITECTURE_DECISION.md` for system design
- **NGC Parsing**: See `Docs/NGC_TEXT_PARSING_REFERENCE.md` for advanced configuration

---

**End of CLI Command Reference**

For questions or issues, see the project README or open a GitHub issue.
