# MIDI DIN Testing Guide

## Table of Contents
- [Overview](#overview)
- [Quick Start Examples](#quick-start-examples)
- [Hardware Requirements](#hardware-requirements)
- [Building and Flashing](#building-and-flashing)
- [Running the Test](#running-the-test)
- [LiveFX Features](#livefx-features)
- [New Advanced Features](#new-advanced-features)
- [MIDI Learn Command Reference](#midi-learn-command-reference)
- [Debug Output Format](#debug-output-format)
- [Troubleshooting](#troubleshooting)
- [Technical Details](#technical-details)

---

## Overview

The `MODULE_TEST_MIDI_DIN` test provides a comprehensive demonstration of MIDI processing capabilities in MidiCore, combining:

1. **MIDI I/O** - Bidirectional MIDI communication over DIN ports
2. **LiveFX Transform** - Real-time MIDI message transformation
3. **MIDI Learn** - Dynamic parameter mapping via CC messages
4. **Advanced Features** - Channel filtering, presets, velocity curves, and more

This test allows you to:
- Receive MIDI from a controller via DIN IN
- Apply real-time effects (transpose, velocity scale, force-to-scale)
- Control effects parameters using MIDI CC messages
- Send transformed MIDI to DIN OUT for monitoring

---

## Quick Start Examples

### Example 1: Basic MIDI Echo (LiveFX Disabled)

**Setup:**
1. Flash firmware with `MODULE_TEST_MIDI_DIN` enabled
2. Connect MIDI keyboard to DIN IN1
3. Connect DIN OUT1 to synth
4. Open serial monitor at 115200 baud

**Test:**
1. Play C4 (MIDI note 60) on keyboard
2. You should see on serial monitor:
   ```
   [RX] DIN1: 90 3C 64 NOTE_ON Ch:1 Note:60 Vel:100
   ```
3. Synth should play C4 (unchanged)

**Result:** MIDI passes through unmodified ✓

### Example 2: Transpose Up by 5 Semitones

**Setup:** Same as Example 1

**Test:**
1. Send CC 20 with value 127 (enable LiveFX)
   ```
   MIDI: B0 14 7F
   ```
   Monitor shows: `[LEARN] LiveFX ENABLED`

2. Send CC 22 (transpose up) five times:
   ```
   MIDI: B0 16 7F  (×5 times)
   ```
   Monitor shows:
   ```
   [LEARN] Transpose: +1
   [LEARN] Transpose: +2
   [LEARN] Transpose: +3
   [LEARN] Transpose: +4
   [LEARN] Transpose: +5
   ```

3. Play C4 (note 60) on keyboard
4. Monitor shows:
   ```
   [RX] DIN1: 90 3C 64 NOTE_ON Ch:1 Note:60 Vel:100
   [TX] DIN OUT1 (transformed): 90 41 64 Note:60→65 Vel:100→100
   ```
5. Synth plays F4 (note 65 = 60 + 5)

**Result:** Notes transposed up 5 semitones ✓

### Example 3: Increase Velocity by 50%

**Setup:** Same as Example 1, LiveFX enabled

**Test:**
1. Send CC 25 (velocity up) five times for +50%
   ```
   MIDI: B0 19 7F (×5 times)
   ```
   Monitor shows:
   ```
   [LEARN] Velocity Scale: 110%
   [LEARN] Velocity Scale: 120%
   [LEARN] Velocity Scale: 130%
   [LEARN] Velocity Scale: 140%
   [LEARN] Velocity Scale: 150%
   ```

2. Play C4 at velocity 80
3. Monitor shows:
   ```
   [RX] DIN1: 90 3C 50 NOTE_ON Ch:1 Note:60 Vel:80
   [TX] DIN OUT1 (transformed): 90 3C 78 Note:60→60 Vel:80→120
   ```
4. Synth plays C4 at velocity 120 (80 × 1.5 = 120)

**Result:** Notes 50% louder ✓

### Example 4: Force to C Major Scale

**Setup:** Same as Example 1, LiveFX enabled, transpose reset to 0

**Test:**
1. Set scale type to Major (1):
   ```
   MIDI: B0 1C 01
   ```
   Monitor: `[LEARN] Scale Type: 1`

2. Set scale root to C (0):
   ```
   MIDI: B0 1D 00
   ```
   Monitor: `[LEARN] Scale Root: 0`

3. Enable force-to-scale:
   ```
   MIDI: B0 1B 7F
   ```
   Monitor: `[LEARN] Force-to-Scale: ON`

4. Play C# (note 61) - it snaps to D (note 62)
   Monitor:
   ```
   [RX] DIN1: 90 3D 64 NOTE_ON Ch:1 Note:61 Vel:100
   [TX] DIN OUT1 (transformed): 90 3E 64 Note:61→62 Vel:100→100
   ```

5. Play D# (note 63) - it snaps to E (note 64)
   Monitor:
   ```
   [RX] DIN1: 90 3F 64 NOTE_ON Ch:1 Note:63 Vel:100
   [TX] DIN OUT1 (transformed): 90 40 64 Note:63→64 Vel:100→100
   ```

**Result:** All notes quantized to C Major scale (C, D, E, F, G, A, B) ✓

**C Major Scale Notes:**
- C (60) → C (60) ✓
- C# (61) → D (62)
- D (62) → D (62) ✓
- D# (63) → E (64)
- E (64) → E (64) ✓
- F (65) → F (65) ✓
- F# (66) → G (67)
- G (67) → G (67) ✓
- G# (68) → A (69)
- A (69) → A (69) ✓
- A# (70) → B (71)
- B (71) → B (71) ✓

### Example 5: Combine All Effects

**Setup:** Same as Example 1

**Test:**
1. Enable LiveFX:
   ```
   MIDI: B0 14 7F
   ```

2. Transpose up by 2 semitones (2× CC 22):
   ```
   MIDI: B0 16 7F
   MIDI: B0 16 7F
   ```

3. Increase velocity by 20% (2× CC 25):
   ```
   MIDI: B0 19 7F
   MIDI: B0 19 7F
   ```

4. Enable C Major force-to-scale:
   ```
   MIDI: B0 1C 01  (Major scale)
   MIDI: B0 1D 00  (C root)
   MIDI: B0 1B 7F  (Enable)
   ```

5. Play C4 (note 60) at velocity 100
6. Processing:
   - Transpose: 60 + 2 = 62 (D)
   - Force-to-scale: 62 (D) → 62 (D) ✓ (D is in C Major)
   - Velocity: 100 × 1.2 = 120

7. Monitor shows:
   ```
   [RX] DIN1: 90 3C 64 NOTE_ON Ch:1 Note:60 Vel:100
   [TX] DIN OUT1 (transformed): 90 3E 78 Note:60→62 Vel:100→120
   ```

8. Synth plays D4 at velocity 120

**Result:** All effects applied in sequence ✓

---

## Hardware Requirements

- **STM32F407VGT6** microcontroller (or compatible)
- **MIDI DIN IN/OUT** interface (UART-based)
- **MIDI Controller** (keyboard, pad controller, etc.)
- **MIDI Device** (synth, DAW, or MIDI monitor)
- **UART Debug Connection** (115200 baud, 8N1)

### Wiring

**MIDI DIN Connections:**
```
Controller → DIN IN1 (UART3 RX, PB11)
DIN OUT1 (UART3 TX, PB10) → Synth/DAW
```

**Debug UART:**
```
Debug UART2 (PA2/PA3) → USB-Serial adapter @ 115200 baud
```

---

## Building and Flashing

### Using STM32CubeIDE

1. Open project in STM32CubeIDE
2. Go to Project Properties → C/C++ Build → Settings → MCU GCC Compiler → Preprocessor
3. Add define: `MODULE_TEST_MIDI_DIN`
4. Build and flash the project

### Using Command Line (if Makefile available)

```bash
make CFLAGS+="-DMODULE_TEST_MIDI_DIN"
```

### Optional: Select UART Port

If you need to use a different UART for DIN1:

```bash
make CFLAGS+="-DMODULE_TEST_MIDI_DIN -DTEST_MIDI_DIN_UART_PORT=2"
```

---

## Running the Test

### Test Sequence

The test starts with LiveFX **disabled** by default. MIDI messages pass through unmodified.

#### Step 1: Verify MIDI I/O

1. Play notes on your MIDI controller
2. Notes should pass through to DIN OUT1 unchanged
3. Debug UART shows received messages:
   ```
   [RX] DIN1: 90 3C 64 NOTE_ON Ch:1 Note:60 Vel:100
   ```

#### Step 2: Enable LiveFX

1. Send **CC 20** with value **127** (Channel 1)
2. Debug output confirms:
   ```
   [RX] DIN1: B0 14 7F CC Ch:1 CC:20 Val:127
   [LEARN] LiveFX ENABLED
   ```

#### Step 3: Test Effects

Follow the examples above to test transpose, velocity scaling, and force-to-scale features.

---

## LiveFX Features

### Transpose
- **Range**: -12 to +12 semitones
- **Default**: 0
- **Effect**: Shifts all notes up or down by semitones
- **Use case**: Change key without rerecording

### Velocity Scale
- **Range**: 0% to 200% (0-255 internal, 128 = 100%)
- **Default**: 100%
- **Effect**: Multiplies note velocity by scale factor
- **Use case**: Adjust dynamics, make notes louder/softer

### Force-to-Scale
- **Scale Types**: 0-11 (Chromatic, Major, Minor, etc.)
- **Scale Root**: 0-11 (C to B)
- **Default**: Disabled
- **Effect**: Quantizes notes to nearest scale degree
- **Use case**: Ensure melody stays in key, create modal effects

---

## New Advanced Features

### Feature 1: MIDI Channel Filtering

**Purpose**: Process MIDI messages from specific channels or all channels.

**CC Command:**
- **CC 30**: Set channel filter
  - Values 0-15: Process only that channel (0 = Channel 1)
  - Value 127: Process ALL channels

**Usage Examples:**

**Filter Channel 1 Only (Default):**
```
Send: CC 30, value 0
Output: [LEARN] Channel Filter: Channel 1
```

**Process All Channels:**
```
Send: CC 30, value 127
Output: [LEARN] Channel Filter: ALL channels
```

### Feature 2: Save/Load Presets to SD Card

**Purpose**: Persist LiveFX settings across power cycles.

**CC Commands:**
- **CC 40**: Save preset (value 0-7 = slot number)
- **CC 41**: Load preset (value 0-7 = slot number)

**Saved Parameters:**
- Transpose (-12 to +12)
- Velocity scale (0-255)
- Scale type (0-14)
- Scale root (0-11)
- Scale enable (on/off)
- Velocity curve (0-2)
- Note range min/max (0-127)

**Usage Example:**
```bash
# Save Current Settings to Slot 0
Send: CC 40, value 0
Output: [LEARN] Preset 0 saved to SD

# Load Preset from Slot 3
Send: CC 41, value 3
Output: [LEARN] Preset 3 loaded from SD
```

### Feature 3: Scale Name Display

**Purpose**: Show human-readable scale names instead of numeric indices.

**Automatic Operation**: Activates automatically when changing scale type or root.

**Example Output:**
```
Send: CC 28, value 1
Output: [LEARN] Scale Type: Major (index 1)
        [INFO] Current scale: C Major
```

**Supported Scale Names:**
| Index | Scale Name |
|-------|------------|
| 0 | Chromatic |
| 1 | Major |
| 2 | Natural Minor |
| 3 | Harmonic Minor |
| 4 | Melodic Minor |
| 5 | Dorian |
| 6 | Phrygian |
| 7 | Lydian |
| 8 | Mixolydian |
| 9 | Locrian |
| 10 | Major Pentatonic |
| 11 | Minor Pentatonic |
| 12 | Blues |
| 13 | Whole Tone |
| 14 | Diminished |

### Feature 4: MIDI Message Statistics

**Purpose**: Track and display processing activity.

**Tracked Metrics:**
- **Notes Processed**: Total note on/off messages received
- **Notes Transformed**: Notes that were modified by LiveFX
- **CC Received**: Total control change messages (MIDI learn)

**Display Format:**
```
Stats: Notes: 245 | Transformed: 178 | CC: 23
```

### Feature 5: Velocity Curves

**Purpose**: Apply non-linear velocity scaling.

**CC Command:**
- **CC 50**: Set velocity curve type
  - Value 0: Linear (default)
  - Value 1: Exponential
  - Value 2: Logarithmic

**Curve Types:**

**Linear (Default):**
- Behavior: Direct 1:1 mapping
- Use Case: No velocity modification

**Exponential:**
- Behavior: Softer at low velocities, stronger at high velocities
- Use Case: Add emphasis to forte playing

**Logarithmic:**
- Behavior: Stronger at low velocities, softer at high velocities
- Use Case: Boost quiet notes, compress loud notes

**Velocity Comparison Table:**

| Input | Linear | Exponential | Logarithmic |
|-------|--------|-------------|-------------|
| 20 | 20 | 3 | 50 |
| 40 | 40 | 13 | 71 |
| 60 | 60 | 28 | 87 |
| 80 | 80 | 50 | 100 |
| 100 | 100 | 79 | 112 |
| 120 | 120 | 113 | 123 |
| 127 | 127 | 127 | 127 |

### Feature 6: Note Range Limiting

**Purpose**: Filter notes by MIDI note number range.

**CC Commands:**
- **CC 53**: Set minimum note (0-127)
- **CC 54**: Set maximum note (0-127)

**Usage Example - Bass Range (24-60):**
```
Send: CC 53, value 24  // E0
Send: CC 54, value 60  // C4
Output: [LEARN] Note Range Min: 24
        [LEARN] Note Range Max: 60
Effect: Only notes E0 to C4 are processed
```

**Filter Behavior:**
Notes outside the range are:
- Not processed by LiveFX
- Not sent to MIDI OUT
- Logged to debug UART

---

## MIDI Learn Command Reference

All commands use **MIDI Channel 1** (status byte 0xB0).

### Original Features (CC 20-29)
| CC | Function | Values |
|----|----------|--------|
| 20 | LiveFX Enable | >64=ON, ≤64=OFF |
| 21 | Transpose Down | Any (decrements) |
| 22 | Transpose Up | Any (increments) |
| 23 | Transpose Reset | Any (sets to 0) |
| 24 | Velocity Scale Down | Any (-10%) |
| 25 | Velocity Scale Up | Any (+10%) |
| 26 | Velocity Scale Reset | Any (100%) |
| 27 | Force-to-Scale Toggle | >64=ON, ≤64=OFF |
| 28 | Scale Type | 0-14 |
| 29 | Scale Root | 0-11 |

### New Features (CC 30, 40-41, 50, 53-54)
| CC | Function | Values |
|----|----------|--------|
| 30 | Channel Filter | 0-15, 127=ALL |
| 40 | Save Preset | 0-7 (slot) |
| 41 | Load Preset | 0-7 (slot) |
| 50 | Velocity Curve | 0=Linear, 1=Exp, 2=Log |
| 53 | Note Range Min | 0-127 |
| 54 | Note Range Max | 0-127 |

---

## Debug Output Format

### Received Messages
```
[RX] DIN1: 90 3C 64 NOTE_ON Ch:1 Note:60 Vel:100
```

### Transformed Messages
```
[TX] DIN OUT1 (transformed): 90 3D 78 Note:60→61 Vel:100→120
```

### MIDI Learn Events
```
[LEARN] LiveFX ENABLED
[LEARN] Transpose: +2
[LEARN] Velocity Scale: 120%
[LEARN] Force-to-Scale: ON
```

### Status Updates (every 10 seconds)
```
--- LiveFX Status ---
Enabled: YES | Transpose: +2 | Velocity: 120% | Scale: ON
---------------------
```

### Enhanced Status Display
```
╔══════════════════════════════════════════════════════════════╗
║                     LiveFX Status Report                     ║
╚══════════════════════════════════════════════════════════════╝
Enabled: YES | Transpose: +2 | Velocity: 120% | Curve: Exp
Scale: C Major | Channel: ALL
Note Range: 36-96 (LIMITED)
──────────────────────────────────────────────────────────────
Stats: Notes: 245 | Transformed: 178 | CC: 23
══════════════════════════════════════════════════════════════
```

---

## Troubleshooting

### No MIDI Input
- Check MIDI cable connections
- Verify UART port configuration (PB11 for UART3)
- Check MIDI controller is sending on Channel 1
- Monitor debug UART for activity

### No MIDI Output
- Check DIN OUT1 connection (PB10 for UART3)
- Verify router is enabled (`MODULE_ENABLE_ROUTER=1`)
- Check receiving device is listening

### LiveFX Not Working
- Verify `MODULE_ENABLE_LIVEFX=1` in Config/module_config.h
- Send CC 20 (value 127) to enable LiveFX
- Check debug output for [LEARN] messages

### Transformed Notes Incorrect
- Send CC 23 to reset transpose
- Send CC 26 to reset velocity scale
- Send CC 27 (value 0) to disable force-to-scale
- Verify parameters in status output

### Preset Save/Load Issues

**Problem**: `[ERROR] Failed to save preset 0`
**Solutions:**
- Verify SD card is mounted
- Create `/presets/` directory on SD card
- Check `MODULE_ENABLE_PATCH=1` in module_config.h
- Verify SD card has write permissions

---

## Technical Details

### Module Dependencies

This test requires the following modules to be enabled:

- `MODULE_ENABLE_MIDI_DIN` - MIDI DIN I/O
- `MODULE_ENABLE_ROUTER` - MIDI message routing
- `MODULE_ENABLE_LIVEFX` - Real-time effects
- `MODULE_ENABLE_SCALE` - Scale quantization (for force-to-scale)

### Code Architecture

```
MIDI IN → midi_din_tick() → Parse Messages → Check CC (MIDI Learn)
                                           ↓
                              Apply LiveFX Transform (if enabled)
                                           ↓
                              midi_din_send() → MIDI OUT
```

### Performance

- **Latency**: < 1ms typical (depends on UART speed and processing)
- **Max Notes**: Limited by UART buffer size
- **CC Processing**: Immediate parameter updates
- **Status Updates**: Every 10 seconds (non-blocking)
- **Velocity Curve**: ~10 µs per note (sqrtf calculation)
- **Note Range Check**: <1 µs per note (comparison)
- **Channel Filter**: <1 µs per message (comparison)

### Memory Usage
- **Statistics**: 12 bytes (3× uint32_t counters)
- **State Variables**: ~20 bytes (curve, channel, range)
- **Total Added**: ~32 bytes

### Storage Requirements
- **SD Card**: ~500 bytes per preset file
- **8 Presets**: ~4 KB total

---

## MIDI Message Reference

### Status Bytes (Channel 1)
- `90` = Note On, Channel 1
- `80` = Note Off, Channel 1
- `B0` = Control Change, Channel 1

### CC Numbers (Hex → Decimal)
- `14` (20) = LiveFX Enable
- `15` (21) = Transpose Down
- `16` (22) = Transpose Up
- `17` (23) = Transpose Reset
- `18` (24) = Velocity Down
- `19` (25) = Velocity Up
- `1A` (26) = Velocity Reset
- `1B` (27) = Force-to-Scale Toggle
- `1C` (28) = Scale Type
- `1D` (29) = Scale Root
- `1E` (30) = Channel Filter
- `28` (40) = Save Preset
- `29` (41) = Load Preset
- `32` (50) = Velocity Curve
- `35` (53) = Note Range Min
- `36` (54) = Note Range Max

### Common Values
- `00` = 0
- `40` = 64 (threshold for on/off toggles)
- `7F` = 127 (maximum)

---

## Testing with MIDI Monitoring Tools

### Windows: MIDI Monitor (MIDI-OX)
1. Configure MIDI IN/OUT ports
2. View raw MIDI bytes in hex
3. Send test messages manually

### macOS: MIDI Monitor
1. Select MIDI device
2. Enable "Display events numerically"
3. View incoming/outgoing messages

### Linux: aseqdump / amidi
```bash
# Monitor MIDI port
aseqdump -p 14:0

# Send CC 20 (enable LiveFX)
amidi -p hw:1,0 -S 'B0 14 7F'
```

### Web-based: WebMIDI
Use online MIDI monitors to visualize messages in real-time.

---

## Summary

The MODULE_TEST_MIDI_DIN test provides comprehensive MIDI processing with:

✅ **MIDI I/O** - Bidirectional communication  
✅ **LiveFX** - Real-time transformation  
✅ **MIDI Learn** - Dynamic parameter mapping  
✅ **Advanced Features** - Channel filtering, presets, curves, note range  
✅ **Statistics** - Processing activity tracking  
✅ **Status Display** - Real-time monitoring  

This test is essential for validating MIDI hardware and developing musical workflows in MidiCore.

---

*Document Version*: 1.0  
*Last Updated*: January 2026  
*Status*: Production Ready
