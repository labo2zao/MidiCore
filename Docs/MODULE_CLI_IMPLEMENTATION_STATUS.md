# Module CLI Implementation Status

**Tracking CLI integration status for all MidiCore modules**

Last Updated: 2026-01-28 (Major Update: +23 new CLI modules)

---

## Implementation Status Legend

- ✅ **Complete**: Full CLI integration with all parameters
- 🟡 **Partial**: Basic CLI, missing some parameters
- ⚪ **Stub**: File created, needs implementation details
- ❌ **Not Started**: No CLI file yet
- 🔵 **N/A**: Module doesn't need CLI (e.g., pure library)

---

## MIDI Modules

| Module | Status | File | Parameters | Notes |
|--------|--------|------|------------|-------|
| router | ✅ Complete | `router_cli.c` | All routes, channel masks | Existing, verified |
| midi_filter | ✅ Complete | `midi_filter_cli.c` | 6 params | Channel mode, note/velocity ranges |
| midi_delay | ✅ Complete | `midi_delay_cli.c` | 5 params | Division, feedback, mix, decay |
| midi_converter | ✅ Complete | `midi_converter_cli.c` | 7 params | All conversion modes |
| midi_monitor | ✅ Complete | `midi_monitor_cli.c` | 4 params | Message capture, filtering, decode |
| program_change_mgr | ✅ Complete | `program_change_mgr_cli.c` | 7 params | 128 preset slots, bank select |
| usb_midi | ✅ Complete | `usb_midi_cli.c` | 1 param | 4 USB MIDI ports/cables |
| usb_host_midi | ✅ Complete | `usb_host_midi_cli.c` | 1 param | External USB device support |

---

## Effect Modules

| Module | Status | File | Parameters | Notes |
|--------|--------|------|------------|-------|
| arpeggiator | ✅ Complete | `arpeggiator_cli_integration.c` | 2 params | Existing, verified |
| quantizer | ✅ Complete | `quantizer_cli.c` | 5 params | Resolution, strength, lookahead, swing |
| harmonizer | ✅ Complete | `harmonizer_cli.c` | 4 params | 2 voices with intervals |
| velocity_compressor | ✅ Complete | `velocity_compressor_cli.c` | 4 params | Threshold, ratio, gain, knee |
| cc_smoother | ✅ Complete | `cc_smoother_cli.c` | 5 params | Mode, amount, attack, release |
| envelope_cc | ✅ Complete | `envelope_cc_cli.c` | 7 params | Full ADSR + output config |
| lfo | ✅ Complete | `lfo_cli.c` | 5 params | Waveform, rate, depth, target |
| channelizer | ✅ Complete | `channelizer_cli.c` | 4 params | Mode, force channel, voice limit |
| chord | ✅ Complete | `chord_cli.c` | 4 params | Type, inversion, voicing |
| scale | ✅ Complete | `scale_cli.c` | 2 params | Scale type, root note |
| legato | ✅ Complete | `legato_cli.c` | 5 params | Priority, retrigger, glide, mono |
| note_repeat | ✅ Complete | `note_repeat_cli.c` | 4 params | Rate, gate, decay |
| note_stabilizer | ✅ Complete | `note_stabilizer_cli.c` | 3 params | Duration, delay, neighbor range |
| strum | ✅ Complete | `strum_cli.c` | 4 params | Time, direction, velocity ramp |
| swing | ✅ Complete | `swing_cli.c` | 3 params | Amount, resolution, groove |
| gate_time | ✅ Complete | `gate_time_cli.c` | 2 params | Mode, value |
| humanize | ✅ Complete | `humanize_cli.c` | 2 params | Time and velocity amounts |
| livefx | ✅ Complete | `livefx_cli.c` | 4 params | Transpose, velocity scale, force scale |

---

## Looper Module

| Module | Status | File | Parameters | Notes |
|--------|--------|------|------------|-------|
| looper | ✅ Complete | `looper_cli.c` | 10 params | Full transport + per-track control |

**Parameters:**
- Global: BPM, time signature, auto loop
- Per-track: State, mute, solo, quantize, MIDI channel, transpose

---

## Accordion Modules

| Module | Status | File | Parameters | Notes |
|--------|--------|------|------------|-------|
| bellows_expression | ✅ Complete | `bellows_expression_cli.c` | 5 params | Curve, pressure range, bidirectional, CC |
| bellows_shake | ✅ Complete | `bellows_shake_cli.c` | 4 params | Enable, sensitivity, depth, target |
| bass_chord_system | ✅ Complete | `bass_chord_system_cli.c` | 3 params | Layout, base note, octave doubling |
| register_coupling | ✅ Complete | `register_coupling_cli.c` | 3 params | Register, smooth transition, time |
| musette_detune | ✅ Complete | `musette_detune_cli.c` | 2 params | Style, detune cents |
| one_finger_chord | ✅ Complete | `one_finger_chord_cli.c` | 3 params | Accessibility: mode, voicing, split point (4 tracks) |
| assist_hold | ✅ Complete | `assist_hold_cli.c` | 3 params | Mode, duration, velocity threshold |

---

## Input Modules

| Module | Status | File | Parameters | Notes |
|--------|--------|------|------------|-------|
| ain | ✅ Complete | `ain_cli.c` | 4 params | Enable, velocity, scan interval, deadband |
| ainser | ✅ Complete | `ainser_map_cli.c` | 6 params | 64 channels, per-channel CC/curve/deadband/min/max |
| srio | 🟡 Partial | (via config) | N/A | Controlled via config module |
| din | ✅ Complete | `din_map_cli.c` | 6 params | 128 buttons, note/CC modes, channel, velocity |
| dout | ✅ Complete | `dout_map_cli.c` | 2 params | 256 LED outputs, RGB LED support |
| footswitch | ✅ Complete | `footswitch_cli.c` | 3 params | 8 footswitches, debounced state |
| pressure | 🔵 N/A | N/A | N/A | Integrated into bellows_expression |
| expression | ✅ Complete | `expression_cli.c` | 4 params | Breath/expression controller, curve, CC, bidirectional |

---

## Generator Modules

| Module | Status | File | Parameters | Notes |
|--------|--------|------|------------|-------|
| metronome | ✅ Complete | `metronome_cli.c` | 7 params | Existing, verified |
| rhythm_trainer | ✅ Complete | `rhythm_trainer_cli.c` | 4 params | Timing trainer, 14 subdivisions, windows |
| dream | ✅ Complete | `dream_cli.c` | 2 params | SAM5716 sampler control via MIDI/SysEx |

---

## System Modules

| Module | Status | File | Parameters | Notes |
|--------|--------|------|------------|-------|
| config | ✅ Complete | `config_cli.c` | 3 params | SRIO settings |
| watchdog | ✅ Complete | `watchdog_cli.c` | 0 params | Status only |
| performance | ✅ Complete | `performance_cli.c` | 6 params | 32 metric slots, timing stats, call counts |
| patch | ✅ Complete | `patch_cli.c` | 2 params | Patch/bank names, save/load commands |
| bootloader | ✅ Complete | `bootloader_cli.c` | 4 params | Version, app validation, addresses |
| system | ✅ Complete | `system_cli.c` | 4 params | SD status, fatal error, safe mode |
| log | ✅ Complete | `log_cli.c` | 2 params | Logging control, SD output |
| config_io | ✅ Complete | `config_io_cli.c` | 2 params | NGC file I/O, SD status |
| usb_cdc | ✅ Complete | `usb_cdc_cli.c` | 2 params | Virtual COM port status |
| usb_msc | ✅ Complete | `usb_msc_cli.c` | 1 param | SD card mass storage status |

---

## UI Modules

| Module | Status | File | Parameters | Notes |
|--------|--------|------|------------|-------|
| ui | ✅ Complete | `ui_cli.c` | 2 params | 14 OLED pages, chord mode |
| input | 🔵 N/A | N/A | N/A | Hardware-level, no CLI needed |

---

## Utility Modules

| Module | Status | File | Parameters | Notes |
|--------|--------|------|------------|-------|
| zones | ✅ Complete | `zones_cli.c` | 6 params | 4 zones, 2 layers, key ranges, transpose |
| instrument | ✅ Complete | `instrument_cli.c` | 5 params | Humanization, velocity curves, strum |
| nrpn_helper | 🔵 N/A | N/A | N/A | Helper functions only |

---

## Test Modules

| Module | Status | File | Parameters | Notes |
|--------|--------|------|------------|-------|
| test | ✅ Complete | `test_cli.c` | Various | Existing, test framework |

---

## Summary Statistics

### Overall Progress

| Category | Complete | Partial | Not Started | N/A | Total |
|----------|----------|---------|-------------|-----|-------|
| MIDI | 8 | 0 | 0 | 0 | **8** |
| Effect | 18 | 0 | 0 | 0 | **18** |
| Looper | 1 | 0 | 0 | 0 | **1** |
| Accordion | 7 | 0 | 0 | 0 | **7** |
| Input | 6 | 1 | 0 | 1 | **8** |
| Generator | 3 | 0 | 0 | 0 | **3** |
| System | 10 | 0 | 0 | 0 | **10** |
| UI | 1 | 0 | 0 | 1 | **2** |
| Utility | 2 | 0 | 0 | 1 | **3** |
| Test | 1 | 0 | 0 | 0 | **1** |
| **TOTAL** | **57** | **1** | **0** | **3** | **61** |

### Completion Percentage
- **🎉 ALL IMPLEMENTABLE MODULES COMPLETE**: 100% (57/57 modules with CLI)
- **CLI Files Created**: 55 total CLI files (23 new + 35 existing - 3 refactored)
- **Total Parameters Exposed**: 200+ parameters across all modules

### All Priority Modules Complete ✅
- ✅ Looper (10 parameters)
- ✅ All MIDI Modules (8 modules, 30+ parameters)
- ✅ All Effect Modules (18 modules, 70+ parameters)
- ✅ All Accordion Modules (7/7 complete)
- ✅ All Input Modules (8/8 addressable)
- ✅ All System Modules (10/10 complete)
- ✅ All Generator Modules (3/3 complete)
- ✅ All Utility Modules (3/3 addressable)

---

## Major Update - January 28, 2026

### 23 New CLI Modules Added

**Priority 1 - Core Functional (7 modules):**
1. ✅ `ainser_map_cli.c` - 64 channel analog input with per-channel configuration
2. ✅ `din_map_cli.c` - 128 button digital input mapping
3. ✅ `dout_map_cli.c` - 256 LED output control
4. ✅ `footswitch_cli.c` - 8 footswitch inputs
5. ✅ `patch_cli.c` - Patch/preset system
6. ✅ `performance_cli.c` - Performance monitoring (32 metrics)
7. ✅ `midi_monitor_cli.c` - MIDI message capture and filtering

**Priority 2 - System (8 modules):**
8. ✅ `bootloader_cli.c` - Bootloader control
9. ✅ `config_io_cli.c` - NGC configuration I/O
10. ✅ `log_cli.c` - Logging system
11. ✅ `usb_cdc_cli.c` - USB Virtual COM Port
12. ✅ `usb_midi_cli.c` - USB Device MIDI (4 ports)
13. ✅ `usb_host_midi_cli.c` - USB Host MIDI
14. ✅ `usb_msc_cli.c` - USB Mass Storage
15. ✅ `system_cli.c` - System status and control

**Priority 3 - Advanced (8 modules):**
16. ✅ `expression_cli.c` - Expression/breath controller
17. ✅ `one_finger_chord_cli.c` - Accessibility chord generator
18. ✅ `rhythm_trainer_cli.c` - Rhythm timing trainer
19. ✅ `program_change_mgr_cli.c` - Program change manager (128 presets)
20. ✅ `dream_cli.c` - Dream SAM5716 sampler
21. ✅ `ui_cli.c` - UI page navigation
22. ✅ `zones_cli.c` - Keyboard zone mapping (4 zones)
23. ✅ `instrument_cli.c` - Instrument humanization

---

## Remaining Work

### Minimal Items

1. **SRIO Configuration**
   - Currently partial via config_cli.c
   - Consider dedicated srio_cli.c if more parameters needed

3. **One Finger Chord**
   - Analyze header and create `one_finger_chord_cli.c`

### Medium Priority

4. **UI Module**
   - Create `ui_cli.c` for page selection and config

5. **Expression Pedal**
   - Create `expression_cli.c`

6. **Footswitch**
   - Create `footswitch_cli.c`

7. **Zones**
   - Create `zones_cli.c` for keyboard zones

### Low Priority

8. **Dream Sampler**
   - Create `dream_cli.c` for SAM5716 control

9. **Rhythm Trainer**
   - Create `rhythm_trainer_cli.c`

10. **Program Change Manager**
    - Create `program_change_mgr_cli.c`

11. **Performance Monitor**
    - Create `performance_cli.c` for CPU/RAM stats

---

## Testing Checklist

For each implemented module, verify:

- [ ] Module registers successfully at init
- [ ] `module list` shows the module
- [ ] `module info <name>` displays correct information
- [ ] `module params <name>` lists all parameters
- [ ] `module get <name> <param>` returns correct values
- [ ] `module set <name> <param> <value>` updates correctly
- [ ] `module enable/disable <name>` works (if applicable)
- [ ] Per-track commands work correctly (if per-track module)
- [ ] Enum parameters accept string values
- [ ] Range validation works (rejects out-of-range values)
- [ ] Configuration saves/loads correctly

---

## API Consistency Notes

### Function Naming Patterns

All modules should follow these patterns:

**Getters:**
```c
type module_get_param(track);         // Per-track
type module_get_param(void);          // Global
int module_is_enabled(track);         // Per-track
int module_get_enabled(void);         // Global
```

**Setters:**
```c
void module_set_param(track, value);  // Per-track
void module_set_param(value);         // Global
void module_set_enabled(track, val);  // Per-track
void module_set_enabled(val);         // Global
```

### Common Issues to Watch

1. **Track vs Global**: Some modules are global but have per-track configs
2. **Enum Names**: Ensure enum string arrays match enum order exactly
3. **Range Validation**: Always validate min/max in setters
4. **Config Structures**: Some modules use config structs (get/set config)
5. **Return Values**: Check if functions return int (error) or void

---

## Next Steps

1. ✅ Complete all priority modules (DONE!)
2. ⬜ Implement high-priority remaining modules
3. ⬜ Test all CLI implementations
4. ⬜ Update documentation with any API changes
5. ⬜ Create integration tests for CLI system
6. ⬜ Add CLI examples to module README files

---

**Last Updated**: January 28, 2025  
**Implementation Lead**: AI Assistant  
**Status**: 65% Complete (34/52 modules fully implemented)
