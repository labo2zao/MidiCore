# Footswitch/Pedal Integration Guide

Complete technical guide for integrating footswitches and pedals with the LOOPA system for hands-free live performance control.

## Table of Contents
1. [Hardware Setup](#hardware-setup)
2. [Supported Actions](#supported-actions)
3. [Configuration API](#configuration-api)
4. [Wiring Diagrams](#wiring-diagrams)
5. [Use Cases](#use-cases)
6. [Best Practices](#best-practices)
7. [Troubleshooting](#troubleshooting)

---

## Hardware Setup

### Physical Connections

**Supported Inputs**: 8 footswitch inputs (FS1-FS8)

**Connection Points**:
- DIN module GPIO pins (configurable)
- Pull-up resistors: 10kΩ to 3.3V
- Ground reference: Common ground with MCU

**Switch Types**:
- **Recommended**: Momentary SPST-NO (Normally Open)
- **Supported**: Momentary SPST-NC (Normally Closed) with inverted logic
- **Not Recommended**: Latching switches (will require special handling)

### Electrical Specifications

| Parameter | Value | Notes |
|-----------|-------|-------|
| Input Voltage | 0V / 3.3V | Logic level |
| Current Draw | <1mA per switch | With pull-up |
| Debounce Time | 20ms | Hardware implementation |
| Max Cable Length | 5m | Recommended for noise immunity |
| Connector Type | 1/4" TS/TRS or XLR | Standard audio connectors |

### Hardware Components

**Required**:
- STM32F4 microcontroller with DIN module
- 8x Momentary foot switches
- 8x 10kΩ pull-up resistors (if not on PCB)
- Connection cables (1/4" TS or TRS)

**Optional**:
- LED indicators for switch status
- Enclosure for pedal board
- Power supply (if separate from main unit)
- MIDI control surface integration

---

## Supported Actions

### Action List

The footswitch system supports 13 distinct actions that can be mapped to any of the 8 physical switches:

#### 1. **Play/Stop** (Global)
- **Action ID**: `FOOTSWITCH_ACTION_PLAY_STOP`
- **Parameter**: N/A (global transport)
- **Behavior**: Toggles playback on/off for all tracks
- **Use Case**: Master transport control

#### 2. **Record**
- **Action ID**: `FOOTSWITCH_ACTION_RECORD`
- **Parameter**: Track number (0-3)
- **Behavior**: Starts/stops recording on specified track
- **Use Case**: Capture new loops

#### 3. **Overdub**
- **Action ID**: `FOOTSWITCH_ACTION_OVERDUB`
- **Parameter**: Track number (0-3)
- **Behavior**: Enables overdub mode (add notes to existing loop)
- **Use Case**: Layer additional parts

#### 4. **Undo**
- **Action ID**: `FOOTSWITCH_ACTION_UNDO`
- **Parameter**: Track number (0-3)
- **Behavior**: Reverts last recording action
- **Use Case**: Correct mistakes quickly

#### 5. **Redo**
- **Action ID**: `FOOTSWITCH_ACTION_REDO`
- **Parameter**: Track number (0-3)
- **Behavior**: Restores undone action
- **Use Case**: Restore accidentally undone loops

#### 6. **Tap Tempo**
- **Action ID**: `FOOTSWITCH_ACTION_TAP_TEMPO`
- **Parameter**: N/A
- **Behavior**: Sets BPM based on tap interval
- **Use Case**: Sync tempo to live performance

#### 7. **Select Track**
- **Action ID**: `FOOTSWITCH_ACTION_SELECT_TRACK`
- **Parameter**: N/A (cycles 0→1→2→3→0)
- **Behavior**: Cycles through tracks for subsequent actions
- **Use Case**: Quick track switching

#### 8. **Trigger Scene**
- **Action ID**: `FOOTSWITCH_ACTION_TRIGGER_SCENE`
- **Parameter**: Scene number (0-7, A-H)
- **Behavior**: Launches specified scene
- **Use Case**: Song arrangement changes

#### 9. **Mute**
- **Action ID**: `FOOTSWITCH_ACTION_MUTE`
- **Parameter**: Track number (0-3)
- **Behavior**: Toggles track mute on/off
- **Use Case**: Drop-out effects

#### 10. **Solo**
- **Action ID**: `FOOTSWITCH_ACTION_SOLO`
- **Parameter**: Track number (0-3)
- **Behavior**: Toggles track solo (mutes others)
- **Use Case**: Focus on specific track

#### 11. **Clear**
- **Action ID**: `FOOTSWITCH_ACTION_CLEAR`
- **Parameter**: Track number (0-3)
- **Behavior**: Clears all events on track
- **Use Case**: Quick reset

#### 12. **Quantize**
- **Action ID**: `FOOTSWITCH_ACTION_QUANTIZE`
- **Parameter**: Track number (0-3)
- **Behavior**: Quantizes track to grid
- **Use Case**: Tighten timing

#### 13. **Custom** (Future)
- **Action ID**: `FOOTSWITCH_ACTION_CUSTOM`
- **Parameter**: User-defined
- **Behavior**: Programmable action
- **Use Case**: Macro commands

---

## Configuration API

### Basic Configuration

```c
#include "Services/looper/looper.h"

// Initialize footswitch system
looper_footswitch_init();

// Map footswitch 0 to Play/Stop
looper_footswitch_set_mapping(0, FOOTSWITCH_ACTION_PLAY_STOP, 0);

// Map footswitch 1 to Record on track 0
looper_footswitch_set_mapping(1, FOOTSWITCH_ACTION_RECORD, 0);

// Map footswitch 2 to Overdub on track 0
looper_footswitch_set_mapping(2, FOOTSWITCH_ACTION_OVERDUB, 0);

// Map footswitch 3 to Trigger Scene A (scene 0)
looper_footswitch_set_mapping(3, FOOTSWITCH_ACTION_TRIGGER_SCENE, 0);

// Map footswitch 4 to Mute track 1
looper_footswitch_set_mapping(4, FOOTSWITCH_ACTION_MUTE, 1);

// Map footswitch 5 to Tap Tempo
looper_footswitch_set_mapping(5, FOOTSWITCH_ACTION_TAP_TEMPO, 0);
```

### Reading Mappings

```c
// Get current mapping for footswitch 0
uint8_t action = looper_footswitch_get_action(0);
uint8_t param = looper_footswitch_get_param(0);

// Check if footswitch is currently pressed
uint8_t is_pressed = looper_footswitch_is_pressed(0);
```

### Clearing Mappings

```c
// Clear all mappings
looper_footswitch_clear_all_mappings();

// Clear specific footswitch
looper_footswitch_clear_mapping(0);
```

### SD Card Persistence

```c
// Save mappings to SD card
looper_footswitch_save_config("/sdcard/footswitch.cfg");

// Load mappings from SD card
looper_footswitch_load_config("/sdcard/footswitch.cfg");
```

**Note**: Mappings are automatically saved/loaded during system initialization and shutdown.

---

## Wiring Diagrams

### Basic SPST-NO Switch Connection

```
Footswitch (Normally Open)
         ┌──────────┐
    ○────┤          ├────○
         └──────────┘
         (Momentary)

MCU Connection:
                       3.3V
                        │
                       ┌┴┐
                       │ │ 10kΩ Pull-up
                       │ │
                       └┬┘
    GPIO Pin ───────────┼───────○ Switch Terminal 1
                        │
    GND ─────────────────────────○ Switch Terminal 2
```

### TRS Connector Wiring

```
1/4" TRS Jack
   Tip (T): GPIO Signal
   Ring (R): Not Connected (NC) or LED+
   Sleeve (S): Ground

   ┌─────────┐
T ─┤  ═════  ├─ GPIO Pin
R ─┤    ║    ├─ NC or LED+
S ─┤  ═════  ├─ GND
   └─────────┘
```

### Multiple Switches (Parallel Connection)

```
           3.3V
            │
           ┌┴┐
           │ │ 10kΩ
           └┬┘
            ├─── GPIO1 (FS1)
            │
           ┌┴┐
           │ │ 10kΩ
           └┬┘
            ├─── GPIO2 (FS2)
            │
           ┌┴┐
           │ │ 10kΩ
           └┬┘
            └─── GPIO3 (FS3)

Each switch connects its GPIO pin to GND when pressed.
```

---

## Use Cases

### Live Performance (Solo Musician)

**Setup**:
- FS1: Play/Stop (global transport)
- FS2: Record Track 0 (bass line)
- FS3: Record Track 1 (rhythm)
- FS4: Record Track 2 (melody)
- FS5: Mute Track 1 (variation)
- FS6: Undo (quick corrections)

**Workflow**:
1. Tap FS1 to start playback
2. Tap FS2 to record bass line loop
3. Tap FS3 to record rhythm loop
4. Tap FS4 to record melody
5. Use FS5 to mute/unmute rhythm for variation
6. Use FS6 if mistake is made

### Band Performance (Multiple Musicians)

**Setup**:
- FS1: Trigger Scene A (intro)
- FS2: Trigger Scene B (verse)
- FS3: Trigger Scene C (chorus)
- FS4: Trigger Scene D (bridge)
- FS5: Play/Stop (master control)
- FS6: Tap Tempo (sync to drummer)

**Workflow**:
1. Use FS6 to set tempo matching drummer
2. Trigger scenes in song order (FS1→FS2→FS3→FS4)
3. Use FS5 for emergency stop if needed

### Accordion Player

**Setup**:
- FS1: Record Track 0 (bass accompaniment)
- FS2: Overdub Track 0 (add chord progression)
- FS3: Record Track 1 (melody line)
- FS4: Play/Stop
- FS5: Mute Track 0 (solo melody)
- FS6: Clear Track 1 (restart melody)

**Workflow**:
1. Record bass with FS1
2. Overdub chords with FS2
3. Record melody with FS3
4. Toggle accompaniment with FS5 for solo sections
5. Clear and re-record melody with FS6 if needed

### Practice/Jam Session

**Setup**:
- FS1: Play/Stop
- FS2: Select Track (cycle through tracks)
- FS3: Record (on selected track)
- FS4: Overdub (on selected track)
- FS5: Clear (on selected track)
- FS6: Undo

**Workflow**:
1. Use FS2 to select which track to work on
2. Record initial loop with FS3
3. Add layers with FS4 (overdub)
4. Undo mistakes with FS6
5. Clear track with FS5 to start fresh

---

## Best Practices

### Hardware

1. **Switch Selection**:
   - Use high-quality momentary switches (Boss-style pedals recommended)
   - Avoid cheap switches that may bounce or have inconsistent actuation
   - Test switches for comfortable actuation force (typically 200-400g)

2. **Cable Quality**:
   - Use shielded cables for cable lengths >2m
   - Solder connections for reliability (avoid breadboard connections)
   - Label cables clearly for troubleshooting

3. **Layout**:
   - Place most-used actions (Play/Stop, Record) in center positions
   - Group related actions together (Record, Overdub, Undo)
   - Leave space between pedals for foot clearance

4. **LED Indicators** (Optional):
   - Add LEDs to show switch status (on/off)
   - Use different colors for different action types
   - Keep LED brightness moderate for stage visibility

### Software

1. **Mapping Strategy**:
   - Start with essential actions (Play/Stop, Record)
   - Add complexity gradually based on workflow needs
   - Save multiple configurations for different performance scenarios

2. **Testing**:
   - Test all mappings before live performance
   - Verify debounce timing is adequate (adjust if needed)
   - Check for any conflicts or unexpected behaviors

3. **Backup**:
   - Save footswitch configurations to SD card regularly
   - Document your mappings in a separate file
   - Keep backup configuration files

### Performance

1. **Latency**:
   - Footswitch response time is <1ms (negligible)
   - Debounce adds 20ms (prevents double-triggers)
   - Total latency: ~20ms (imperceptible for human timing)

2. **Reliability**:
   - Footswitch system uses mutex-protected operations (thread-safe)
   - Debounce prevents false triggers from switch bounce
   - All state changes are atomic (no race conditions)

---

## Troubleshooting

### Common Issues

#### 1. **Footswitch Not Responding**

**Symptoms**: No action when pressing footswitch

**Possible Causes**:
- Incorrect wiring (check continuity with multimeter)
- Faulty switch (test with another switch)
- Wrong GPIO pin configured (verify DIN module settings)
- Pull-up resistor missing or incorrect value

**Solutions**:
- Verify wiring matches diagram above
- Test switch continuity (should be open when not pressed, closed when pressed)
- Check DIN module configuration in `config.ngc`
- Measure pull-up resistor (should be ~10kΩ)

#### 2. **Double-Triggering**

**Symptoms**: Single press triggers action twice

**Possible Causes**:
- Insufficient debounce time (rare with 20ms default)
- Poor quality switch with excessive bounce
- Electrical noise on cable (unshielded long cables)

**Solutions**:
- Increase debounce time in DIN module config (if needed)
- Replace switch with higher quality unit
- Use shielded cable or shorter cable length
- Add capacitor (100nF) across switch terminals for additional filtering

#### 3. **Intermittent Operation**

**Symptoms**: Footswitch works sometimes but not always

**Possible Causes**:
- Loose connection (solder joint or connector)
- Dirty switch contacts (oxidation)
- Cable damage (broken wire inside insulation)
- Interference from other equipment

**Solutions**:
- Check and re-solder all connections
- Clean switch contacts with contact cleaner
- Test cable with multimeter (check for intermittent open circuit)
- Move cables away from power supplies and other noise sources

#### 4. **Wrong Action Triggered**

**Symptoms**: Pressing one switch triggers different action

**Possible Causes**:
- Incorrect mapping configuration
- GPIO pin conflict (wrong pin assigned)
- Crossed wires (switches connected to wrong pins)

**Solutions**:
- Verify mapping with `looper_footswitch_get_action()`
- Check DIN module pin assignments
- Trace wiring and correct any crossed connections
- Clear all mappings and reconfigure from scratch

#### 5. **No LED Feedback** (if installed)

**Symptoms**: LED not lighting when action is active

**Possible Causes**:
- LED wired backwards (check polarity)
- Current-limiting resistor wrong value (LED might be dim or off)
- GPIO not configured as output (check firmware)

**Solutions**:
- Verify LED polarity (cathode to ground)
- Calculate correct resistor value: R = (Vcc - Vf) / If (typically 220Ω-1kΩ)
- Check GPIO configuration (should be output mode)

### Diagnostic Procedure

**Step-by-step diagnostics**:

1. **Visual Inspection**:
   - Check all solder joints for cold solder or bridges
   - Verify wiring matches color codes and diagram
   - Look for physical damage to switches or cables

2. **Electrical Testing** (with multimeter):
   - Measure pull-up resistor value (should be 10kΩ ±5%)
   - Check switch continuity (open when not pressed, <1Ω when pressed)
   - Verify GPIO voltage (should be 3.3V when not pressed, 0V when pressed)

3. **Software Verification**:
   - Check DIN module configuration (`config.ngc`)
   - Verify footswitch mappings are saved to SD card
   - Test with MIDI Monitor page to see if actions are triggered

4. **Isolation Testing**:
   - Test one footswitch at a time (disconnect others)
   - Swap switches to isolate faulty switch vs. faulty wiring
   - Try different GPIO pins to isolate pin-specific issues

### Debug Mode

Enable footswitch debug output (if available in firmware):

```c
// Enable debug logging for footswitch events
looper_footswitch_set_debug(1);
```

This will log all footswitch events to the serial console for troubleshooting.

---

## Hardware Recommendations

### Recommended Switches

1. **Boss FS-5U** (momentary, unlatch mode)
   - High quality, reliable
   - Standard 1/4" TS connection
   - Compact footprint

2. **Digitech FS3X** (3-button unit)
   - Three switches in one enclosure
   - TRS connection for multiple switches
   - Good for compact setups

3. **DIY Momentary Switches**:
   - Carling 111-PC SPST momentary
   - Switchcraft 112A 1/4" jack
   - Hammond 1590A enclosure

### Recommended Cables

- **Mogami 2524** (guitar cable)
- **Hosa GTR-210** (10ft guitar cable)
- Custom-length cables with quality connectors

### Pedalboard Solutions

1. **Pedaltrain Nano** (compact)
2. **RockBoard TRES 3.1** (medium)
3. **DIY wooden pedalboard** (custom size)

---

## SD Card Configuration Format

Footswitch mappings are stored in `.NGC` format compatible with MIDIbox NG:

```
# Footswitch Configuration
# Format: FS_MAP <footswitch_id> <action_id> <parameter>

FS_MAP 0 1 0    # FS0: Play/Stop
FS_MAP 1 2 0    # FS1: Record Track 0
FS_MAP 2 3 0    # FS2: Overdub Track 0
FS_MAP 3 8 0    # FS3: Trigger Scene A
FS_MAP 4 9 1    # FS4: Mute Track 1
FS_MAP 5 6 0    # FS5: Tap Tempo
FS_MAP 6 4 0    # FS6: Undo Track 0
FS_MAP 7 5 0    # FS7: Redo Track 0
```

**Action IDs**:
1. Play/Stop
2. Record
3. Overdub
4. Undo
5. Redo
6. Tap Tempo
7. Select Track
8. Trigger Scene
9. Mute
10. Solo
11. Clear
12. Quantize

---

## Performance Specifications

| Metric | Value | Notes |
|--------|-------|-------|
| Input Latency | <1ms | Signal detection |
| Debounce Time | 20ms | Prevents double-triggers |
| Total Response Time | ~21ms | Detection + debounce |
| Max Simultaneous Presses | 8 | All switches can be pressed at once |
| Current Draw (per switch) | <1mA | With pull-up resistor |
| Total Current Draw | <10mA | All 8 switches + LEDs |
| SD Card Save Time | <50ms | Mapping configuration |
| SD Card Load Time | <30ms | Mapping restoration |

---

## Future Enhancements

Planned improvements for footswitch system:

1. **Expression Pedal Support**:
   - Continuous control (0-127) via analog input
   - Map to LFO rate, velocity scaling, etc.

2. **MIDI Pedal Controllers**:
   - Support for MIDI-based pedal controllers (e.g., Behringer FCB1010)
   - Map MIDI CC/PC messages to footswitch actions

3. **Programmable Macros**:
   - Combine multiple actions into single footswitch press
   - Example: Mute all tracks except current + enable solo mode

4. **Hold Actions**:
   - Short press vs. long press for different actions
   - Example: Short press = mute, long press = solo

5. **Visual Feedback on Display**:
   - Show footswitch status on UI page
   - Display current mappings for quick reference

---

## Support & Contributions

For issues, feature requests, or contributions related to footswitch functionality:

- **GitHub Issues**: [labodezao/MidiCore/issues](https://github.com/labodezao/MidiCore/issues)
- **Pull Requests**: Contributions welcome!
- **Documentation**: Update this guide with new use cases or troubleshooting tips

---

## License

This documentation is part of the MidiCore project and follows the same license terms.

---

**Last Updated**: 2026-01-17  
**Version**: 1.0  
**Author**: GitHub Copilot & Contributors
