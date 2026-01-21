# Test Execution Tracking

**Project**: MidiCore - Complete LOOPA Feature Implementation
**Test Protocol**: TESTING_PROTOCOL.md
**Test Start Date**: _____________
**Test Completion Date**: _____________
**Tester**: _____________
**Hardware Configuration**: _____________

---

## Test Environment Setup

### Hardware Requirements
- [ ] STM32F4 MCU (or equivalent) with firmware flashed
- [ ] SSD1322 OLED display (256×64, grayscale) connected and functional
- [ ] SD card reader with formatted SD card (FAT32)
- [ ] MIDI IN/OUT ports connected
- [ ] DIN module (optional, for button matrix)
- [ ] AINSER64 module (optional, for analog inputs)
- [ ] Footswitch inputs (optional, 8 switches)
- [ ] XGZP6847D pressure sensor (optional, for breath controller)

### Software Requirements
- [ ] Firmware built and flashed successfully
- [ ] SD card contains config files (config.ngc, templates)
- [ ] MIDI controller or keyboard for testing
- [ ] MIDI monitor software on PC (optional, for verification)

### Initial System Check
- [ ] Power on - system boots without errors
- [ ] Display shows main looper page
- [ ] MIDI IN/OUT functional (loopback test)
- [ ] SD card detected and mounted
- [ ] Button/encoder inputs responsive

**Notes**: _____________________________________________________________

---

## Phase 1: UI Pages Testing

### Section A: Song Mode Page (ui_page_song.c/h)

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| P1-A-01 | Navigate to Song Mode page | ☐ Pass ☐ Fail | |
| P1-A-02 | Verify 4×8 scene grid display | ☐ Pass ☐ Fail | |
| P1-A-03 | Verify empty cells show as unfilled | ☐ Pass ☐ Fail | |
| P1-A-04 | Record clip on Track 1, Scene A | ☐ Pass ☐ Fail | |
| P1-A-05 | Verify cell A1 shows as filled | ☐ Pass ☐ Fail | |
| P1-A-06 | Navigate between scenes with encoder | ☐ Pass ☐ Fail | |
| P1-A-07 | Trigger scene playback with button | ☐ Pass ☐ Fail | |
| P1-A-08 | Verify current scene highlighted | ☐ Pass ☐ Fail | |
| P1-A-09 | BPM display updates correctly | ☐ Pass ☐ Fail | |
| P1-A-10 | Playback state indicator shows correctly | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

**Issues Found**: 
_____________________________________________________________________________
_____________________________________________________________________________

---

### Section B: MIDI Monitor Page (ui_page_midi_monitor.c/h)

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| P1-B-01 | Navigate to MIDI Monitor page | ☐ Pass ☐ Fail | |
| P1-B-02 | Send NoteOn - verify displayed | ☐ Pass ☐ Fail | |
| P1-B-03 | Verify timestamp format (mm:ss.ms) | ☐ Pass ☐ Fail | |
| P1-B-04 | Verify Note name decoded (C4, D#5, etc) | ☐ Pass ☐ Fail | |
| P1-B-05 | Send CC message - verify decoded | ☐ Pass ☐ Fail | |
| P1-B-06 | Send PitchBend - verify decoded | ☐ Pass ☐ Fail | |
| P1-B-07 | Pause capture - verify messages stop | ☐ Pass ☐ Fail | |
| P1-B-08 | Resume capture - verify messages resume | ☐ Pass ☐ Fail | |
| P1-B-09 | Clear buffer - verify display cleared | ☐ Pass ☐ Fail | |
| P1-B-10 | Scroll up/down through message history | ☐ Pass ☐ Fail | |
| P1-B-11 | Verify circular buffer (old messages drop) | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### Section C: SysEx Viewer Page (ui_page_sysex.c/h)

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| P1-C-01 | Navigate to SysEx Viewer page | ☐ Pass ☐ Fail | |
| P1-C-02 | Send short SysEx (10 bytes) | ☐ Pass ☐ Fail | |
| P1-C-03 | Verify hex display (16 bytes/row) | ☐ Pass ☐ Fail | |
| P1-C-04 | Verify manufacturer ID decoded (1-byte) | ☐ Pass ☐ Fail | |
| P1-C-05 | Send SysEx with 3-byte mfr ID | ☐ Pass ☐ Fail | |
| P1-C-06 | Verify 3-byte mfr ID decoded | ☐ Pass ☐ Fail | |
| P1-C-07 | Send long SysEx (>128 bytes) | ☐ Pass ☐ Fail | |
| P1-C-08 | Verify truncation warning displayed | ☐ Pass ☐ Fail | |
| P1-C-09 | Scroll through long SysEx data | ☐ Pass ☐ Fail | |
| P1-C-10 | Verify length display accurate | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### Section D: Config Editor Page (ui_page_config.c/h)

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| P1-D-01 | Navigate to Config Editor page | ☐ Pass ☐ Fail | |
| P1-D-02 | Verify VIEW mode displays all modules | ☐ Pass ☐ Fail | |
| P1-D-03 | Enter EDIT mode | ☐ Pass ☐ Fail | |
| P1-D-04 | Navigate to DIN module settings | ☐ Pass ☐ Fail | |
| P1-D-05 | Change DIN enabled setting | ☐ Pass ☐ Fail | |
| P1-D-06 | Verify parameter value updates | ☐ Pass ☐ Fail | |
| P1-D-07 | Save configuration to SD card | ☐ Pass ☐ Fail | |
| P1-D-08 | Power cycle device | ☐ Pass ☐ Fail | |
| P1-D-09 | Verify settings loaded from SD card | ☐ Pass ☐ Fail | |
| P1-D-10 | Test all 43 config parameters | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### Section E: LiveFX Control Page (ui_page_livefx.c/h)

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| P1-E-01 | Navigate to LiveFX page | ☐ Pass ☐ Fail | |
| P1-E-02 | Select Track 1 | ☐ Pass ☐ Fail | |
| P1-E-03 | Adjust transpose (+6 semitones) | ☐ Pass ☐ Fail | |
| P1-E-04 | Play notes - verify transposed | ☐ Pass ☐ Fail | |
| P1-E-05 | Adjust velocity scaling (150%) | ☐ Pass ☐ Fail | |
| P1-E-06 | Play notes - verify velocity scaled | ☐ Pass ☐ Fail | |
| P1-E-07 | Enable Force-to-Scale (C Major) | ☐ Pass ☐ Fail | |
| P1-E-08 | Play chromatic - verify quantized | ☐ Pass ☐ Fail | |
| P1-E-09 | Test all 15 scale types | ☐ Pass ☐ Fail | |
| P1-E-10 | Disable LiveFX - verify bypass | ☐ Pass ☐ Fail | |
| P1-E-11 | Switch between VIEW/EDIT modes | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### Section F: Rhythm Trainer Page (ui_page_rhythm.c/h)

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| P1-F-01 | Navigate to Rhythm Trainer page | ☐ Pass ☐ Fail | |
| P1-F-02 | Verify measure bars displayed | ☐ Pass ☐ Fail | |
| P1-F-03 | Verify subdivision grid visible | ☐ Pass ☐ Fail | |
| P1-F-04 | Start rhythm trainer (1/4 notes) | ☐ Pass ☐ Fail | |
| P1-F-05 | Play on-beat notes - verify "Perfect" | ☐ Pass ☐ Fail | |
| P1-F-06 | Play slightly early - verify "Good/Early" | ☐ Pass ☐ Fail | |
| P1-F-07 | Play off-beat - verify "Off" | ☐ Pass ☐ Fail | |
| P1-F-08 | Test MUTE mode (silence off-beat) | ☐ Pass ☐ Fail | |
| P1-F-09 | Test WARNING mode (alert sound) | ☐ Pass ☐ Fail | |
| P1-F-10 | Change difficulty to HARD | ☐ Pass ☐ Fail | |
| P1-F-11 | Verify stricter timing thresholds | ☐ Pass ☐ Fail | |
| P1-F-12 | Test all 14 subdivisions | ☐ Pass ☐ Fail | |
| P1-F-13 | Test triplets (1/8T, 1/16T) | ☐ Pass ☐ Fail | |
| P1-F-14 | Test dotted notes (1/4., 1/8., 1/16.) | ☐ Pass ☐ Fail | |
| P1-F-15 | Test polyrhythms (5, 7, 8, 11, 13-tuplets) | ☐ Pass ☐ Fail | |
| P1-F-16 | Verify statistics tracking | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

## Phase 2: LiveFX System & Router Integration

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| P2-01 | Record loop with LiveFX enabled | ☐ Pass ☐ Fail | |
| P2-02 | Verify LiveFX applied during recording | ☐ Pass ☐ Fail | |
| P2-03 | Change LiveFX settings during playback | ☐ Pass ☐ Fail | |
| P2-04 | Verify real-time transformation | ☐ Pass ☐ Fail | |
| P2-05 | Test router MIDI_IN → LiveFX → Looper | ☐ Pass ☐ Fail | |
| P2-06 | Test router Looper → LiveFX → MIDI_OUT | ☐ Pass ☐ Fail | |
| P2-07 | Scene management - save scene | ☐ Pass ☐ Fail | |
| P2-08 | Scene management - load scene | ☐ Pass ☐ Fail | |
| P2-09 | Step playback - manual cursor control | ☐ Pass ☐ Fail | |
| P2-10 | Metronome - start with count-in | ☐ Pass ☐ Fail | |
| P2-11 | Metronome - BPM synchronization | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

## Phase 3: Advanced Features Testing

### Section A: SD Card Configuration (43 Parameters)

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| P3-A-01 | Load config.ngc from SD card | ☐ Pass ☐ Fail | |
| P3-A-02 | Verify all 43 parameters loaded | ☐ Pass ☐ Fail | |
| P3-A-03 | Modify DIN module settings (7 params) | ☐ Pass ☐ Fail | |
| P3-A-04 | Save modified config | ☐ Pass ☐ Fail | |
| P3-A-05 | Test AINSER module config (3 params) | ☐ Pass ☐ Fail | |
| P3-A-06 | Test AIN module config (5 params) | ☐ Pass ☐ Fail | |
| P3-A-07 | Test MIDI settings (2 params) | ☐ Pass ☐ Fail | |
| P3-A-08 | Test Pressure module (9 params) | ☐ Pass ☐ Fail | |
| P3-A-09 | Test Expression module (15 params) | ☐ Pass ☐ Fail | |
| P3-A-10 | Test Calibration module (5 params) | ☐ Pass ☐ Fail | |
| P3-A-11 | Load config_minimal.ngc | ☐ Pass ☐ Fail | |
| P3-A-12 | Load config_full.ngc | ☐ Pass ☐ Fail | |
| P3-A-13 | Verify .NGC format compatibility | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### Section B: Beatloop Visual Enhancements

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| P3-B-01 | Start recording - verify loop markers appear | ☐ Pass ☐ Fail | |
| P3-B-02 | Verify loop start marker (2px line + triangle) | ☐ Pass ☐ Fail | |
| P3-B-03 | Verify loop end marker (2px line + triangle) | ☐ Pass ☐ Fail | |
| P3-B-04 | Verify shading between markers | ☐ Pass ☐ Fail | |
| P3-B-05 | Start playback - verify playhead visible | ☐ Pass ☐ Fail | |
| P3-B-06 | Verify playhead moves in real-time | ☐ Pass ☐ Fail | |
| P3-B-07 | Verify loop length display in header | ☐ Pass ☐ Fail | |
| P3-B-08 | Verify playback state indicator | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### Section C: Scene Chaining/Automation

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| P3-C-01 | Enable scene chain A→B | ☐ Pass ☐ Fail | |
| P3-C-02 | Record loop on Track 0, Scene A | ☐ Pass ☐ Fail | |
| P3-C-03 | Start playback - verify auto-trigger to B | ☐ Pass ☐ Fail | |
| P3-C-04 | Verify seamless transition | ☐ Pass ☐ Fail | |
| P3-C-05 | Test chain B→C→D | ☐ Pass ☐ Fail | |
| P3-C-06 | Disable chain - verify manual mode | ☐ Pass ☐ Fail | |
| P3-C-07 | Test circular chain (D→A) | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### Section D: MIDI File Export

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| P3-D-01 | Record loop on Track 1 | ☐ Pass ☐ Fail | |
| P3-D-02 | Export track to MIDI file | ☐ Pass ☐ Fail | |
| P3-D-03 | Verify .mid file created on SD card | ☐ Pass ☐ Fail | |
| P3-D-04 | Load .mid file in DAW (Reaper) | ☐ Pass ☐ Fail | |
| P3-D-05 | Verify tempo embedded correctly | ☐ Pass ☐ Fail | |
| P3-D-06 | Verify time signature correct | ☐ Pass ☐ Fail | |
| P3-D-07 | Verify MIDI events accurate | ☐ Pass ☐ Fail | |
| P3-D-08 | Export multi-track (all 4 tracks) | ☐ Pass ☐ Fail | |
| P3-D-09 | Verify SMF Format 1 structure | ☐ Pass ☐ Fail | |
| P3-D-10 | Export scene to MIDI file | ☐ Pass ☐ Fail | |
| P3-D-11 | Test DAW compatibility (Ableton, Logic) | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

## Enhancement Features Testing

### E1: Tempo Tap Function

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| E1-01 | Activate tempo tap mode | ☐ Pass ☐ Fail | |
| E1-02 | Tap 4 times at 120 BPM | ☐ Pass ☐ Fail | |
| E1-03 | Verify BPM detected as ~120 | ☐ Pass ☐ Fail | |
| E1-04 | Test at various tempos (60-240 BPM) | ☐ Pass ☐ Fail | |
| E1-05 | Verify ±0.5 BPM accuracy | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### E2: Undo/Redo System

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| E2-01 | Record loop on Track 1 | ☐ Pass ☐ Fail | |
| E2-02 | Overdub additional notes | ☐ Pass ☐ Fail | |
| E2-03 | Undo - verify overdub removed | ☐ Pass ☐ Fail | |
| E2-04 | Redo - verify overdub restored | ☐ Pass ☐ Fail | |
| E2-05 | Multiple undos (test 10-level stack) | ☐ Pass ☐ Fail | |
| E2-06 | Verify memory usage (~24KB per level) | ☐ Pass ☐ Fail | |
| E2-07 | Test undo/redo across tracks | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### E3: Loop Quantization

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| E3-01 | Record unquantized loop | ☐ Pass ☐ Fail | |
| E3-02 | Apply 1/4 note quantization | ☐ Pass ☐ Fail | |
| E3-03 | Verify events aligned to grid | ☐ Pass ☐ Fail | |
| E3-04 | Test all 5 resolutions (1/4 to 1/64) | ☐ Pass ☐ Fail | |
| E3-05 | Verify undo/redo integration | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### E4: MIDI Clock Sync

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| E4-01 | Send MIDI clock at 120 BPM | ☐ Pass ☐ Fail | |
| E4-02 | Verify looper syncs to external clock | ☐ Pass ☐ Fail | |
| E4-03 | Change clock tempo - verify follow | ☐ Pass ☐ Fail | |
| E4-04 | Verify ±0.1 BPM accuracy | ☐ Pass ☐ Fail | |
| E4-05 | Test jitter filtering | ☐ Pass ☐ Fail | |
| E4-06 | Stop clock - verify looper stops | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### E5: Track Mute/Solo Controls

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| E5-01 | Record loops on all 4 tracks | ☐ Pass ☐ Fail | |
| E5-02 | Mute Track 2 - verify silent | ☐ Pass ☐ Fail | |
| E5-03 | Unmute Track 2 - verify audible | ☐ Pass ☐ Fail | |
| E5-04 | Solo Track 1 - verify only T1 audible | ☐ Pass ☐ Fail | |
| E5-05 | Test exclusive solo mode | ☐ Pass ☐ Fail | |
| E5-06 | Verify zero latency switching | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### E6: Copy/Paste Functionality

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| E6-01 | Record loop on Track 1 | ☐ Pass ☐ Fail | |
| E6-02 | Copy Track 1 | ☐ Pass ☐ Fail | |
| E6-03 | Paste to Track 2 | ☐ Pass ☐ Fail | |
| E6-04 | Verify Track 2 identical to Track 1 | ☐ Pass ☐ Fail | |
| E6-05 | Copy entire Scene A | ☐ Pass ☐ Fail | |
| E6-06 | Paste to Scene B | ☐ Pass ☐ Fail | |
| E6-07 | Verify all tracks copied | ☐ Pass ☐ Fail | |
| E6-08 | Test clipboard persistence | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### E7: Global Transpose

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| E7-01 | Record loops on all tracks | ☐ Pass ☐ Fail | |
| E7-02 | Set global transpose +12 semitones | ☐ Pass ☐ Fail | |
| E7-03 | Verify all tracks transposed | ☐ Pass ☐ Fail | |
| E7-04 | Test negative transpose (-24) | ☐ Pass ☐ Fail | |
| E7-05 | Verify note clamping (0-127) | ☐ Pass ☐ Fail | |
| E7-06 | Measure operation time (<15ms) | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### E8: Randomizer

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| E8-01 | Record quantized loop | ☐ Pass ☐ Fail | |
| E8-02 | Apply velocity randomization (±32) | ☐ Pass ☐ Fail | |
| E8-03 | Verify velocity variation | ☐ Pass ☐ Fail | |
| E8-04 | Apply timing randomization (±6 ticks) | ☐ Pass ☐ Fail | |
| E8-05 | Verify groove/shuffle effect | ☐ Pass ☐ Fail | |
| E8-06 | Apply note skip (50% probability) | ☐ Pass ☐ Fail | |
| E8-07 | Verify sparse pattern created | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### E9: Humanizer

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| E9-01 | Record mechanical loop | ☐ Pass ☐ Fail | |
| E9-02 | Apply humanizer (intensity 75%) | ☐ Pass ☐ Fail | |
| E9-03 | Verify subtle velocity variation | ☐ Pass ☐ Fail | |
| E9-04 | Verify groove-aware timing shifts | ☐ Pass ☐ Fail | |
| E9-05 | Verify on-beat notes less affected | ☐ Pass ☐ Fail | |
| E9-06 | Compare to randomizer (more musical) | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### E10: Arpeggiator

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| E10-01 | Enable arpeggiator on Track 1 | ☐ Pass ☐ Fail | |
| E10-02 | Test UP pattern | ☐ Pass ☐ Fail | |
| E10-03 | Test DOWN pattern | ☐ Pass ☐ Fail | |
| E10-04 | Test UPDOWN pattern | ☐ Pass ☐ Fail | |
| E10-05 | Test RANDOM pattern | ☐ Pass ☐ Fail | |
| E10-06 | Test CHORD pattern | ☐ Pass ☐ Fail | |
| E10-07 | Adjust gate length (10-95%) | ☐ Pass ☐ Fail | |
| E10-08 | Test octave range (1-4 octaves) | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### E11: Footswitch Mapping

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| E11-01 | Connect footswitch to input 1 | ☐ Pass ☐ Fail | |
| E11-02 | Assign Play/Stop action | ☐ Pass ☐ Fail | |
| E11-03 | Test footswitch triggers action | ☐ Pass ☐ Fail | |
| E11-04 | Verify debounce (20ms) functional | ☐ Pass ☐ Fail | |
| E11-05 | Test all 13 actions | ☐ Pass ☐ Fail | |
| E11-06 | Test all 8 footswitch inputs | ☐ Pass ☐ Fail | |
| E11-07 | Save mappings - power cycle - verify loaded | ☐ Pass ☐ Fail | |
| E11-08 | Measure response latency (<1ms) | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### E12: MIDI Learn

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| E12-01 | Activate MIDI learn mode | ☐ Pass ☐ Fail | |
| E12-02 | Send CC #7 from controller | ☐ Pass ☐ Fail | |
| E12-03 | Verify CC captured | ☐ Pass ☐ Fail | |
| E12-04 | Map CC to track volume | ☐ Pass ☐ Fail | |
| E12-05 | Test learned mapping functional | ☐ Pass ☐ Fail | |
| E12-06 | Learn 32 different mappings | ☐ Pass ☐ Fail | |
| E12-07 | Test channel filtering | ☐ Pass ☐ Fail | |
| E12-08 | Test 10-second timeout | ☐ Pass ☐ Fail | |
| E12-09 | Measure processing latency (<1ms) | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

### E13: Quick-Save Slots

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| E13-01 | Record session (all tracks, BPM, etc) | ☐ Pass ☐ Fail | |
| E13-02 | Save to Slot 1 with name "Test1" | ☐ Pass ☐ Fail | |
| E13-03 | Measure save time (<200ms) | ☐ Pass ☐ Fail | |
| E13-04 | Clear all tracks | ☐ Pass ☐ Fail | |
| E13-05 | Load Slot 1 | ☐ Pass ☐ Fail | |
| E13-06 | Verify all state restored | ☐ Pass ☐ Fail | |
| E13-07 | Measure load time (<300ms) | ☐ Pass ☐ Fail | |
| E13-08 | Test all 8 slots | ☐ Pass ☐ Fail | |
| E13-09 | Power cycle - verify slots persist | ☐ Pass ☐ Fail | |

**Section Summary**: _____ tests passed, _____ tests failed

---

## Code Quality Testing

### CQ1: Memory Optimization

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| CQ1-01 | Verify configurable undo depth | ☐ Pass ☐ Fail | |
| CQ1-02 | Set depth to 3 - verify ~72KB usage | ☐ Pass ☐ Fail | |
| CQ1-03 | Set depth to 10 - verify ~240KB usage | ☐ Pass ☐ Fail | |
| CQ1-04 | Test system stability at various depths | ☐ Pass ☐ Fail | |

---

### CQ2: API Consistency

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| CQ2-01 | Verify all `looper_set_*` functions exist | ☐ Pass ☐ Fail | |
| CQ2-02 | Verify all `looper_get_*` functions exist | ☐ Pass ☐ Fail | |
| CQ2-03 | Verify return value standards documented | ☐ Pass ☐ Fail | |
| CQ2-04 | Test boundary validation (track 0-3) | ☐ Pass ☐ Fail | |
| CQ2-05 | Test boundary validation (scene 0-7) | ☐ Pass ☐ Fail | |
| CQ2-06 | Verify no duplicate functions | ☐ Pass ☐ Fail | |

---

### CQ3: Thread Safety

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| CQ3-01 | Concurrent access to randomizer | ☐ Pass ☐ Fail | |
| CQ3-02 | Verify volatile seed variable | ☐ Pass ☐ Fail | |
| CQ3-03 | Test mutex-protected operations | ☐ Pass ☐ Fail | |
| CQ3-04 | Stress test with rapid API calls | ☐ Pass ☐ Fail | |

---

### CQ4: Documentation

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| CQ4-01 | Verify all functions have @brief tags | ☐ Pass ☐ Fail | |
| CQ4-02 | Verify all parameters have @param tags | ☐ Pass ☐ Fail | |
| CQ4-03 | Verify all functions have @return tags | ☐ Pass ☐ Fail | |
| CQ4-04 | Generate Doxygen docs - verify complete | ☐ Pass ☐ Fail | |

---

## Integration Testing

### INT1: Multi-Feature Scenarios

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| INT1-01 | LiveFX + Looper + MIDI Export | ☐ Pass ☐ Fail | |
| INT1-02 | Scene Chain + Mute/Solo + Quick-Save | ☐ Pass ☐ Fail | |
| INT1-03 | Arpeggiator + Humanizer + Quantization | ☐ Pass ☐ Fail | |
| INT1-04 | MIDI Learn + Footswitch + Tempo Tap | ☐ Pass ☐ Fail | |
| INT1-05 | All UI pages + All enhancements active | ☐ Pass ☐ Fail | |

---

### INT2: Stress Testing

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| INT2-01 | Max events (512) per track | ☐ Pass ☐ Fail | |
| INT2-02 | All 4 tracks at max capacity | ☐ Pass ☐ Fail | |
| INT2-03 | Rapid scene switching (1 per second) | ☐ Pass ☐ Fail | |
| INT2-04 | Continuous recording for 30 minutes | ☐ Pass ☐ Fail | |
| INT2-05 | High MIDI throughput (>1000 msg/sec) | ☐ Pass ☐ Fail | |

---

## Regression Testing

**Test Date**: _____________

| Test ID | Test Case | Status | Notes |
|---------|-----------|--------|-------|
| REG-01 | Load old session files (pre-v1.0) | ☐ Pass ☐ Fail | |
| REG-02 | Verify backward compatibility | ☐ Pass ☐ Fail | |
| REG-03 | Test with minimal config | ☐ Pass ☐ Fail | |
| REG-04 | Test without optional modules | ☐ Pass ☐ Fail | |

---

## Performance Benchmarks

**Test Date**: _____________

| Metric | Target | Measured | Status |
|--------|--------|----------|--------|
| Looper latency (recording) | <5ms | ______ ms | ☐ Pass ☐ Fail |
| Looper latency (playback) | <5ms | ______ ms | ☐ Pass ☐ Fail |
| LiveFX processing latency | <5ms | ______ ms | ☐ Pass ☐ Fail |
| MIDI Learn processing | <1ms | ______ ms | ☐ Pass ☐ Fail |
| Footswitch response | <1ms | ______ ms | ☐ Pass ☐ Fail |
| Quick-Save (save) | <200ms | ______ ms | ☐ Pass ☐ Fail |
| Quick-Save (load) | <300ms | ______ ms | ☐ Pass ☐ Fail |
| Global transpose | <15ms | ______ ms | ☐ Pass ☐ Fail |
| MIDI export (4 tracks) | <500ms | ______ ms | ☐ Pass ☐ Fail |
| Scene switching | <50ms | ______ ms | ☐ Pass ☐ Fail |

---

## Issue Tracking

### Issue Template

**Issue #**: ___________
**Date Found**: ___________
**Test ID**: ___________
**Severity**: ☐ Critical ☐ High ☐ Medium ☐ Low
**Category**: ☐ Bug ☐ Performance ☐ Usability

**Description**:
_____________________________________________________________________________

**Steps to Reproduce**:
1. _______________________________________________________________________
2. _______________________________________________________________________
3. _______________________________________________________________________

**Expected Behavior**:
_____________________________________________________________________________

**Actual Behavior**:
_____________________________________________________________________________

**Workaround** (if any):
_____________________________________________________________________________

**Status**: ☐ Open ☐ In Progress ☐ Fixed ☐ Won't Fix

---

## Test Summary

**Total Test Cases**: 300+
**Tests Executed**: _______
**Tests Passed**: _______
**Tests Failed**: _______
**Tests Skipped**: _______
**Pass Rate**: _______%

**Critical Issues**: _______
**High Priority Issues**: _______
**Medium Priority Issues**: _______
**Low Priority Issues**: _______

---

## Sign-Off

**Testing Completed By**: _______________________________________
**Date**: _______________________
**Signature**: _______________________________________

**Production Ready**: ☐ Yes ☐ No

**Comments**:
_____________________________________________________________________________
_____________________________________________________________________________
_____________________________________________________________________________
_____________________________________________________________________________

---

## Recommendations for Next Release

1. _______________________________________________________________________
2. _______________________________________________________________________
3. _______________________________________________________________________
4. _______________________________________________________________________
5. _______________________________________________________________________

---

**End of Test Execution Document**
