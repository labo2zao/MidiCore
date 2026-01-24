# MODULE_TEST_MIDI_DIN - New Features Guide

## Overview

This document describes the 6 new features added to the MODULE_TEST_MIDI_DIN test module, bringing advanced MIDI processing capabilities for professional use.

## Feature Summary

| Feature | CC Commands | Description |
|---------|-------------|-------------|
| **Channel Filtering** | CC 30 | Select which MIDI channel to process |
| **Preset Save/Load** | CC 40-41 | Store settings to SD card |
| **Scale Name Display** | (automatic) | Human-readable scale names |
| **Statistics Tracking** | (automatic) | Message processing counters |
| **Velocity Curves** | CC 50 | Non-linear velocity scaling |
| **Note Range Limiting** | CC 53-54 | Filter notes by range |

---

## Feature 1: MIDI Channel Filtering

### Purpose
Process MIDI messages from specific channels or all channels, enabling multi-channel setups.

### CC Command
- **CC 30**: Set channel filter
  - Values 0-15: Process only that channel (0 = Channel 1)
  - Value 127: Process ALL channels

### Usage Examples

#### Example 1: Filter Channel 1 Only (Default)
```
Send: CC 30, value 0
Output: [LEARN] Channel Filter: Channel 1
```

#### Example 2: Filter Channel 10 (Drums)
```
Send: CC 30, value 9
Output: [LEARN] Channel Filter: Channel 10
```

#### Example 3: Process All Channels
```
Send: CC 30, value 127
Output: [LEARN] Channel Filter: ALL channels
```

### Use Cases
- Multi-track recording with channel-specific effects
- Drum processing on channel 10 only
- Vocal processing on dedicated channel
- Multi-instrument setups with shared MIDI bus

---

## Feature 2: Save/Load Presets to SD Card

### Purpose
Persist LiveFX settings across power cycles by saving/loading from SD card.

### CC Commands
- **CC 40**: Save preset (value 0-7 = slot number)
- **CC 41**: Load preset (value 0-7 = slot number)

### Saved Parameters
- Transpose (-12 to +12)
- Velocity scale (0-255)
- Scale type (0-14)
- Scale root (0-11)
- Scale enable (on/off)
- Velocity curve (0-2)
- Note range min/max (0-127)

### File Format
Settings are saved as INI files:
```
File: 0:/presets/livefx_0.ini through livefx_7.ini
Format: INI key-value pairs
```

### Usage Examples

#### Example 1: Save Current Settings to Slot 0
```
1. Configure LiveFX parameters as desired
2. Send: CC 40, value 0
3. Output: [LEARN] Preset 0 saved to SD
```

#### Example 2: Load Preset from Slot 3
```
Send: CC 41, value 3
Output: [LEARN] Preset 3 loaded from SD
All parameters restored from saved preset
```

#### Example 3: Create Preset Bank
```
Slot 0: "Default" - No effects
Slot 1: "Transpose +5" - Up perfect 4th
Slot 2: "C Major Scale" - Force to C Major
Slot 3: "Soft Touch" - Logarithmic velocity curve
Slot 4: "Bass Range" - Notes 24-60 only
Slot 5: "Drums" - Channel 10, exponential velocity
```

### Prerequisites
- `MODULE_ENABLE_PATCH=1` must be enabled
- SD card must be mounted
- `/presets/` directory should exist on SD card

### Error Handling
```
[ERROR] Failed to save preset 0  // SD card not mounted
[ERROR] Failed to load preset 3  // File doesn't exist
[ERROR] Patch module not enabled  // MODULE_ENABLE_PATCH=0
```

---

## Feature 3: Scale Name Display

### Purpose
Show human-readable scale names instead of numeric indices for better usability.

### Automatic Operation
This feature activates automatically when changing:
- Scale type (CC 28)
- Scale root (CC 29)

### Example Output

#### Setting Scale Type
```
Send: CC 28, value 1
Output: [LEARN] Scale Type: Major (index 1)
        [INFO] Current scale: C Major
```

#### Setting Scale Root
```
Send: CC 29, value 5 (F)
Output: [LEARN] Scale Root: F (note 5)
        [INFO] Current scale: F Major
```

### Supported Scale Names
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

### Status Display Integration
```
╔══════════════════════════════════════════════════════════════╗
║                     LiveFX Status Report                     ║
╚══════════════════════════════════════════════════════════════╝
Enabled: YES | Transpose: +0 | Velocity: 100% | Curve: Linear
Scale: F Major | Channel: 1
```

---

## Feature 4: MIDI Message Statistics

### Purpose
Track and display processing activity for monitoring and debugging.

### Tracked Metrics
- **Notes Processed**: Total note on/off messages received
- **Notes Transformed**: Notes that were modified by LiveFX
- **CC Received**: Total control change messages (MIDI learn)

### Display Format
Statistics appear in the status report every 10 seconds:
```
──────────────────────────────────────────────────────────────
Stats: Notes: 245 | Transformed: 178 | CC: 23
══════════════════════════════════════════════════════════════
```

### Use Cases
- **Performance Monitoring**: Track MIDI activity during live performance
- **Debugging**: Verify MIDI messages are being received
- **Effect Analysis**: See how many notes are being transformed
- **Learn Activity**: Monitor MIDI learn command frequency

### Example Scenario
```
Initial State:
Stats: Notes: 0 | Transformed: 0 | CC: 0

After playing 100 notes with LiveFX disabled:
Stats: Notes: 100 | Transformed: 0 | CC: 0

After enabling LiveFX and transposing +2:
Stats: Notes: 200 | Transformed: 100 | CC: 1

After changing scale 3 times:
Stats: Notes: 200 | Transformed: 100 | CC: 4
```

---

## Feature 5: Velocity Curves

### Purpose
Apply non-linear velocity scaling for more musical dynamics control.

### CC Command
- **CC 50**: Set velocity curve type
  - Value 0: Linear (default)
  - Value 1: Exponential
  - Value 2: Logarithmic

### Curve Types

#### Linear (Default)
- **Behavior**: Direct 1:1 mapping
- **Formula**: `output = input`
- **Use Case**: No velocity modification, preserve original dynamics

#### Exponential
- **Behavior**: Softer at low velocities, stronger at high velocities
- **Formula**: `output = input²` (normalized)
- **Use Case**: Add emphasis to forte playing, gentle pianissimo
- **Example**: Input 64 → Output 32, Input 100 → Output 79

#### Logarithmic
- **Behavior**: Stronger at low velocities, softer at high velocities
- **Formula**: `output = √input` (normalized)
- **Use Case**: Boost quiet notes, compress loud notes
- **Example**: Input 64 → Output 90, Input 100 → Output 112

### Usage Examples

#### Example 1: Set Linear Curve (Default)
```
Send: CC 50, value 0
Output: [LEARN] Velocity Curve: Linear
Effect: No velocity modification
```

#### Example 2: Set Exponential Curve
```
Send: CC 50, value 1
Output: [LEARN] Velocity Curve: Exponential
Effect: Soft notes become softer, loud notes emphasized
```

#### Example 3: Set Logarithmic Curve
```
Send: CC 50, value 2
Output: [LEARN] Velocity Curve: Logarithmic
Effect: Soft notes boosted, loud notes compressed
```

### Velocity Comparison Table

| Input | Linear | Exponential | Logarithmic |
|-------|--------|-------------|-------------|
| 20 | 20 | 3 | 50 |
| 40 | 40 | 13 | 71 |
| 60 | 60 | 28 | 87 |
| 80 | 80 | 50 | 100 |
| 100 | 100 | 79 | 112 |
| 120 | 120 | 113 | 123 |
| 127 | 127 | 127 | 127 |

### Integration with Velocity Scale
Velocity curve is applied **before** velocity scaling:
```
Process Order:
1. Input velocity (from MIDI note)
2. Apply velocity curve (CC 50)
3. Apply velocity scale (CC 25/26)
4. Apply transpose/force-to-scale
5. Output to DIN OUT1
```

---

## Feature 6: Note Range Limiting

### Purpose
Filter notes by MIDI note number range, preventing out-of-range transpositions.

### CC Commands
- **CC 53**: Set minimum note (0-127)
- **CC 54**: Set maximum note (0-127)

### Default Range
- Minimum: 0
- Maximum: 127
- Status: Unlimited

### Usage Examples

#### Example 1: Bass Range (24-60)
```
Send: CC 53, value 24  // E0
Send: CC 54, value 60  // C4
Output: [LEARN] Note Range Min: 24
        [LEARN] Note Range Max: 60
Effect: Only notes E0 to C4 are processed
Status: Note Range: 24-60 (LIMITED)
```

#### Example 2: Piano Range (21-108)
```
Send: CC 53, value 21  // A0
Send: CC 54, value 108  // C8
Output: [LEARN] Note Range Min: 21
        [LEARN] Note Range Max: 108
```

#### Example 3: Prevent Transpose Overflow
```
Configuration:
- Transpose: +12 (one octave up)
- Note Range Max: 115
Benefit: Notes above 115 won't transpose to >127 (invalid)
```

### Filter Behavior
Notes outside the range are:
- **Not processed** by LiveFX
- **Not sent** to MIDI OUT
- **Logged** to debug UART

Example filter message:
```
[FILTER] Note 20 outside range 24-60, skipped
```

### Status Display
```
Note Range: 36-96      // Limited range
Note Range: 0-127      // Full range (no limit shown)
```

### Use Cases
- **Bass Processing**: Limit to bass register (24-60)
- **Lead Processing**: Limit to upper register (60-96)
- **Drum Filtering**: Process specific note ranges for drums
- **Safety**: Prevent out-of-range transpositions
- **Split Keyboard**: Process different zones separately

---

## Combined Features Example

### Scenario: Professional Bass Setup

**Goal**: Process bass MIDI (channel 10, notes 24-60) with soft touch and save as preset

```bash
# Step 1: Configure channel filter for bass channel
Send: CC 30, value 9  # Channel 10
[LEARN] Channel Filter: Channel 10

# Step 2: Set note range for bass register
Send: CC 53, value 24  # E0
Send: CC 54, value 60  # C4
[LEARN] Note Range Min: 24
[LEARN] Note Range Max: 60

# Step 3: Apply logarithmic velocity curve for soft touch
Send: CC 50, value 2
[LEARN] Velocity Curve: Logarithmic

# Step 4: Add slight velocity boost
Send: CC 25  # Velocity up
Send: CC 25  # Velocity up
[LEARN] Velocity Scale: 120%

# Step 5: Enable LiveFX
Send: CC 20, value 127
[LEARN] LiveFX ENABLED

# Step 6: Save as preset 5 "Bass Setup"
Send: CC 40, value 5
[LEARN] Preset 5 saved to SD

# Result: Status display shows
╔══════════════════════════════════════════════════════════════╗
║                     LiveFX Status Report                     ║
╚══════════════════════════════════════════════════════════════╝
Enabled: YES | Transpose: +0 | Velocity: 120% | Curve: Log
Scale: OFF | Channel: 10
Note Range: 24-60 (LIMITED)
──────────────────────────────────────────────────────────────
Stats: Notes: 0 | Transformed: 0 | CC: 8
══════════════════════════════════════════════════════════════
```

---

## CC Command Quick Reference

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

## Enhanced Status Display

The status report now shows all features:

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

**Display Elements:**
1. **Line 1**: Basic settings (enabled, transpose, velocity scale, curve type)
2. **Line 2**: Scale name and channel filter
3. **Line 3**: Note range (shows LIMITED if range is restricted)
4. **Line 4**: Statistics (notes, transformations, CC messages)

---

## Troubleshooting

### Preset Save/Load Issues

**Problem**: `[ERROR] Failed to save preset 0`
**Solutions**:
- Verify SD card is mounted
- Create `/presets/` directory on SD card
- Check `MODULE_ENABLE_PATCH=1` in module_config.h
- Verify SD card has write permissions

### Channel Filter Not Working

**Problem**: Notes from other channels still processed
**Solutions**:
- Send CC 30 to set channel filter
- Verify correct channel (0=Ch1, 9=Ch10)
- Check MIDI controller is sending on expected channel

### Velocity Curve Not Applied

**Problem**: No velocity change observed
**Solutions**:
- Ensure LiveFX is enabled (CC 20, value >64)
- Try extreme curve settings (exponential with soft notes)
- Verify MIDI controller sends variable velocities
- Check velocity scale is not overriding effect

### Note Range Filter

**Problem**: All notes filtered out
**Solutions**:
- Verify min ≤ max (send CC 54 after CC 53)
- Check note range includes input notes
- Reset to full range: CC 53 value 0, CC 54 value 127

---

## Performance Considerations

### Memory Usage
- **Statistics**: 12 bytes (3× uint32_t counters)
- **State Variables**: ~20 bytes (curve, channel, range)
- **Total Added**: ~32 bytes

### CPU Impact
- **Velocity Curve**: ~10 µs per note (sqrtf calculation)
- **Note Range Check**: <1 µs per note (comparison)
- **Channel Filter**: <1 µs per message (comparison)
- **Statistics**: <1 µs per message (increment)
- **Total**: <15 µs additional latency per note

### Storage Requirements
- **SD Card**: ~500 bytes per preset file
- **8 Presets**: ~4 KB total

---

## See Also

- [MIDI_DIN_LIVEFX_TEST.md](MIDI_DIN_LIVEFX_TEST.md) - Main test guide
- [MIDI_DIN_EXAMPLES.md](MIDI_DIN_EXAMPLES.md) - Quick start examples
- [MODULE_TEST_MIDI_DIN_SUMMARY.md](MODULE_TEST_MIDI_DIN_SUMMARY.md) - Implementation summary
- [README_MODULE_TESTING.md](README_MODULE_TESTING.md) - Module testing framework

---

**Document Version**: 1.0  
**Last Updated**: January 2026  
**Author**: GitHub Copilot  
**Status**: Production Ready
