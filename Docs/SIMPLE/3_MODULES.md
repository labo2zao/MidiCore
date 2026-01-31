# 🎹 MidiCore Modules

**All 60+ modules, organized by category**

---

## 🎵 MIDI Modules

### router
**Routes MIDI messages between ports**

| Setting | Values | Description |
|---------|--------|-------------|
| route | 0-15 | Enable/disable routes |
| chanmask | all, 1-16 | Filter channels |

```
router set 0 1 1            ← Route port 0 → 1
router chanmask 0 1 all     ← Allow all channels
```

---

### midi_filter
**Filter MIDI messages by type/channel/note**

| Setting | Values | Description |
|---------|--------|-------------|
| enabled | true/false | On/off |
| channel_mode | ALL, ALLOW, BLOCK | Channel filter |
| min_note | 0-127 | Lowest note |
| max_note | 0-127 | Highest note |

```
module enable midi_filter 0
module set midi_filter min_note 36 0
```

---

### midi_delay
**Echo MIDI notes with tempo sync**

| Setting | Values | Description |
|---------|--------|-------------|
| enabled | true/false | On/off |
| division | 1_4, 1_8, 1_16, etc. | Time division |
| feedback | 0-100 | Repeat amount % |
| mix | 0-100 | Wet/dry % |

---

### midi_converter
**Convert between MIDI message types**

| Setting | Values | Description |
|---------|--------|-------------|
| mode | CC_TO_AT, AT_TO_CC, etc. | Conversion type |
| source_cc | 0-127 | Input CC |
| dest_cc | 0-127 | Output CC |

---

## 🎛️ Effect Modules

### arpeggiator
**Play notes in sequence**

| Setting | Values | Description |
|---------|--------|-------------|
| enabled | true/false | On/off |
| pattern | UP, DOWN, UP_DOWN, RANDOM | Direction |
| rate | 1_4, 1_8, 1_16 | Speed |
| octaves | 1-4 | Range |

```
module enable arpeggiator
module set arpeggiator pattern UP
```

---

### harmonizer
**Add harmony notes**

| Setting | Values | Description |
|---------|--------|-------------|
| enabled | true/false | On/off |
| voice1_interval | THIRD_UP, FIFTH_UP, etc. | Interval |
| voice1_enabled | true/false | Voice on/off |

```
module enable harmonizer 0
module set harmonizer voice1_interval THIRD_UP 0
```

---

### quantizer
**Snap timing to grid**

| Setting | Values | Description |
|---------|--------|-------------|
| enabled | true/false | On/off |
| resolution | 1_8, 1_16, etc. | Grid size |
| strength | 0-100 | Amount % |
| swing | 0-100 | Swing % |

---

### velocity_compressor
**Control dynamics**

| Setting | Values | Description |
|---------|--------|-------------|
| threshold | 1-127 | Compress above this |
| ratio | 2_1, 4_1, 8_1 | Compression ratio |
| makeup_gain | 0-127 | Output boost |

---

### chord
**Single note → full chord**

| Setting | Values | Description |
|---------|--------|-------------|
| type | MAJOR, MINOR, DIM, AUG, etc. | Chord type |
| inversion | 0-3 | Inversion |
| voicing | CLOSE, SPREAD, DROP2 | Voicing |

---

### scale
**Force notes to a scale**

| Setting | Values | Description |
|---------|--------|-------------|
| scale_type | MAJOR, MINOR_NAT, DORIAN, etc. | Scale |
| root_note | 0-11 | Root (C=0, C#=1, D=2...) |

---

### legato
**Mono/legato playing**

| Setting | Values | Description |
|---------|--------|-------------|
| priority | LAST, HIGHEST, LOWEST | Note priority |
| retrigger | ON, OFF | Retrigger mode |
| glide_time | 0-2000 | Portamento ms |

---

### strum
**Guitar-style strum**

| Setting | Values | Description |
|---------|--------|-------------|
| time | 0-200 | Strum time ms |
| direction | UP, DOWN, RANDOM | Direction |

---

### humanize
**Add human feel**

| Setting | Values | Description |
|---------|--------|-------------|
| time_amount | 0-100 | Timing variation % |
| velocity_amount | 0-100 | Velocity variation % |

---

### cc_smoother
**Smooth CC changes**

| Setting | Values | Description |
|---------|--------|-------------|
| mode | LIGHT, MEDIUM, HEAVY | Amount |
| amount | 0-255 | Custom amount |

---

### lfo
**Low frequency modulation**

| Setting | Values | Description |
|---------|--------|-------------|
| waveform | SINE, TRIANGLE, SQUARE | Shape |
| rate_hz | 0-1000 | Speed (×100) |
| depth | 0-127 | Amount |
| target | CC, PITCH | Destination |

---

### envelope_cc
**ADSR envelope to CC**

| Setting | Values | Description |
|---------|--------|-------------|
| cc_number | 0-127 | Output CC |
| attack_ms | 0-5000 | Attack time |
| decay_ms | 0-5000 | Decay time |
| sustain | 0-127 | Sustain level |
| release_ms | 0-5000 | Release time |

---

## 🔴 Looper Module

### looper
**Record and playback MIDI (4 tracks)**

| Setting | Values | Description |
|---------|--------|-------------|
| bpm | 20-300 | Tempo |
| time_sig_num | 1-16 | Time signature top |
| time_sig_den | 2,4,8,16 | Time signature bottom |
| state | STOP, REC, PLAY, OVERDUB | Track state |
| mute | true/false | Mute track |
| solo | true/false | Solo track |
| quantize | 1_8, 1_16, OFF | Quantize |
| midi_channel | 0-15 | MIDI channel |
| transpose | -127 to 127 | Transpose |

```
module set looper bpm 120
module set looper state REC 0
module set looper state PLAY 0
```

---

## 🪗 Accordion Modules

### bellows_expression
**Bellows pressure to expression**

| Setting | Values | Description |
|---------|--------|-------------|
| curve | LINEAR, EXPONENTIAL, LOG | Response curve |
| min_pa | 0-5000 | Min pressure |
| max_pa | 0-5000 | Max pressure |
| bidirectional | true/false | Push/pull |
| expression_cc | 0-127 | Output CC |

---

### bellows_shake
**Tremolo from bellows shaking**

| Setting | Values | Description |
|---------|--------|-------------|
| sensitivity | 0-100 | Detection |
| depth | 0-127 | Tremolo depth |
| target | MOD_WHEEL, VOLUME, FILTER | Destination |

---

### bass_chord_system
**Stradella bass system**

| Setting | Values | Description |
|---------|--------|-------------|
| layout | STRADELLA_120, FREE_BASS | Layout |
| base_note | 0-127 | Starting note |
| octave_doubling | true/false | Double octave |

---

### register_coupling
**Register switching**

| Setting | Values | Description |
|---------|--------|-------------|
| register | MUSETTE, BANDONEON, CLARINET | Register |
| smooth_transition | true/false | Smooth change |
| transition_time | 0-1000 | Time ms |

---

### musette_detune
**Classic musette/chorus**

| Setting | Values | Description |
|---------|--------|-------------|
| style | FRENCH, ITALIAN, SCOTTISH | Style |
| detune_cents | 0-50 | Detune amount |

---

### assist_hold
**Auto-hold for accessibility**

| Setting | Values | Description |
|---------|--------|-------------|
| mode | OFF, PERMANENT, TIMED | Hold mode |
| duration_ms | 0-10000 | Hold time |
| velocity_threshold | 1-127 | Min velocity |

---

## 🎚️ Input Modules

### ain
**Analog inputs (Hall sensors)**

| Setting | Values | Description |
|---------|--------|-------------|
| enable | true/false | On/off |
| velocity_enable | true/false | Velocity sensing |
| scan_ms | 1-50 | Scan interval |
| deadband | 0-100 | Noise rejection |

---

### ainser
**AINSER64 (64 analog channels)**

Per-channel settings:
- cc, channel, curve, invert, min, max, threshold

---

### din
**Digital inputs (buttons)**

Per-button settings:
- enabled, type, channel, number, velocity

---

### srio
**Shift register I/O**

| Setting | Values | Description |
|---------|--------|-------------|
| scan_ms | 1-50 | Scan interval |
| din_bytes | 1-32 | Input bytes |
| dout_bytes | 1-32 | Output bytes |

---

### footswitch
**Footswitch inputs (8)**

Map footswitches to actions:
- Play/Stop, Record, Overdub, Undo, Redo, etc.

---

## 🔧 Generator Modules

### metronome
**Metronome (sync to looper)**

| Setting | Values | Description |
|---------|--------|-------------|
| mode | OFF, MIDI, AUDIO | Output mode |
| midi_channel | 0-15 | MIDI channel |
| accent_note | 0-127 | Downbeat note |
| regular_note | 0-127 | Beat note |

---

## ⚙️ System Modules

### config
**System configuration**

| Setting | Values | Description |
|---------|--------|-------------|
| srio_enable | true/false | Enable SRIO |
| srio_din_enable | true/false | Enable DIN |
| srio_dout_enable | true/false | Enable DOUT |

---

### watchdog
**System health monitoring**

Always on. No settings.

---

## 📊 Module Categories Summary

| Category | Count | Examples |
|----------|-------|----------|
| MIDI | 4 | router, midi_filter, midi_delay |
| Effect | 15 | arpeggiator, harmonizer, quantizer |
| Looper | 1 | looper (4 tracks) |
| Accordion | 6 | bellows, bass_chord, musette |
| Input | 5 | ain, ainser, din, srio |
| Generator | 1 | metronome |
| System | 2 | config, watchdog |
| **Total** | **34+** | Plus sub-modules |

---

**Need details?** Use `module info <name>` and `module params <name>`
