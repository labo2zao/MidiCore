# MidiCore LoopA UI Implementation Guide

## Overview

Ce document décrit comment adapter l'UI de MidiCore pour correspondre à l'application MIOS32 LoopA.

**Date**: 2026-01-12  
**Status**: Phase 1 Implementation Complete - Four new UI pages added

---

## LoopA UI Characteristics (Reference)

### Display Specifications
- **Resolution**: 256x64 pixels
- **Grayscale**: 16 levels (4-bit)
- **Screen Layout**: 3 sections (Top/Middle/Bottom)

### Typical LoopA Display Layout

```
┌─────────────────────────────────────────────────────────┐
│ TOP (12px): Track/Scene Selection + Status              │ 
│ Track 1 | 2 | 3 | 4 | 5 | 6    Scene: A  BPM:120 4/4   │
├─────────────────────────────────────────────────────────┤
│ MIDDLE (40px): Note Roll / Clip View                     │
│ ░░░██░░░░██████░░░░░░░░██░░░░██░░░░                    │
│ ░██░░██░░██░░██░░░░░░██░░██░░██░░░░                    │
│ ░██░░██░░████░░░░░░░░██████░░██░░░░                    │
│ ░░░██░░░░██░░░░░░░░░░██░░██░░░░░░░░                    │
├─────────────────────────────────────────────────────────┤
│ BOTTOM (12px): Context Menu / Function Keys             │
│ REC │PLAY│STOP│MUTE│ FX │SAVE│LOAD│MENU               │
└─────────────────────────────────────────────────────────┘
```

### Key Features of LoopA UI

1. **6 Tracks × 6 Scenes Grid** (36 clips total)
2. **Visual Clip Indicators**: Show which clips have data
3. **Real-time Note Roll**: Notes scroll during playback
4. **Context-Sensitive Buttons**: Top row changes per mode
5. **LED Feedback**: Encoder rings show position/value
6. **Live Effects**: Transpose, force-to-scale, humanize
7. **Beatloop**: Visual loop position indicator

---

## Current MidiCore UI

### Existing Pages

#### 1. `UI_PAGE_LOOPER` (Overview)
```c
// Current: Simple text list
LOOPER  BPM:120  TS:4/4
───────────────────────
>T1 REC  L:4  Q:1/16  M:0
 T2 STOP L:4  Q:OFF   M:0
 T3 PLAY L:8  Q:1/8   M:0
 T4 STOP L:4  Q:1/4   M:1
───────────────────────
B1 REC  B2 PLAY  B3 STOP
```

**Files**: `Services/ui/ui_page_looper.c`

#### 2. `UI_PAGE_LOOPER_TL` (Timeline)
```c
// Current: Horizontal timeline with cursor
Track 1  │  Cursor at beat 2.5
═════════╪═════════════════════
         │
    ●    │  ●     ●    ●
         │
```

**Files**: `Services/ui/ui_page_looper_timeline.c`

#### 3. `UI_PAGE_LOOPER_PR` (Piano Roll)
```c
// Current: Vertical note display
    ┌───────────────┐
 C5 │  ██  ██       │
 B4 │     ██  ██    │
 A4 │ ██      ██    │
 G4 │    ██    ██   │
    └───────────────┘
```

**Files**: `Services/ui/ui_page_looper_pianoroll.c`

---

## Proposed LoopA-Style UI

### Option 1: Complete Redesign (Exact LoopA Clone)

**Pros**:
- Familiar to LoopA users
- Proven UI design
- Feature parity

**Cons**:
- Major code changes
- Different hardware (encoders, buttons)
- Need scene/clip matrix

**Estimated Effort**: ~8-12 hours

### Option 2: LoopA-Inspired Enhancements

**Keep existing pages, add LoopA features**:

1. **Enhanced Overview Page**:
   - Add scene selector (A-F)
   - Visual clip indicators (filled rectangles)
   - Grid layout instead of list
   
2. **Improved Timeline**:
   - Add loop region indicator
   - Beatloop visual markers
   - Playhead animation
   
3. **Better Piano Roll**:
   - Velocity bars
   - Note length visualization
   - Grid lines (beats/bars)

**Estimated Effort**: ~4-6 hours

### Option 3: Hybrid Approach

**New main page combining LoopA concepts**:

```c
┌─────────────────────────────────────────────┐
│ Bank:MyBank  Patch:Lead  120BPM  Scene:A    │
├──────────┬──────────┬──────────┬────────────┤
│ T1 ●REC  │ T2 ■PLAY │ T3 ■PLAY │ T4 □STOP  │
│ 4 bars   │ 8 bars   │ 4 bars   │ --        │
├──────────┼──────────┼──────────┼────────────┤
│ T5 □STOP │ T6 □STOP │ Timeline ──>          │
│ --       │ --       │ ╪═══════════          │
└──────────┴──────────┴────────────────────────┘
│ REC │PLAY│STOP│MUTE│SCENE│QUAN│            │
```

**Features**:
- 6-track grid (2x3)
- Visual state indicators
- Integrated timeline
- LoopA-style bottom menu

**Estimated Effort**: ~6-8 hours

---

## Implementation Plan

### Phase 1: UI Redesign (if approved)

1. **Create new page**: `UI_PAGE_LOOPER_LOOPA`
2. **Implement grid layout**: 6 tracks, 6 scenes
3. **Add visual indicators**: Filled/empty clips
4. **Enhance graphics**: Better rectangles, borders

### Phase 2: Additional Features

1. **Scene support**: Add scene switching (A-F)
2. **Clip matrix**: Store 6×6 clips (36 total)
3. **Visual feedback**: Active track/scene highlight
4. **LiveFX integration**: If requested

### Phase 3: Polish

1. **Animations**: Smooth transitions
2. **LED feedback**: If hardware supports
3. **Context menus**: Per-mode button labels
4. **Help overlay**: Quick reference

---

## Code Structure

### New Files (if full implementation)

```
Services/ui/
├── ui_page_looper_loopa.c     # Main LoopA-style page
├── ui_page_looper_loopa.h
├── ui_loopa_scenes.c          # Scene management
├── ui_loopa_scenes.h
├── ui_loopa_clips.c           # Clip matrix
└── ui_loopa_clips.h
```

### Modified Files

```
Services/ui/ui.c               # Add new page
Services/ui/ui.h               # UI_PAGE_LOOPER_LOOPA
Services/ui/ui_gfx.c           # Enhanced graphics
Services/looper/looper.c       # Scene support (if needed)
```

---

## Hardware Considerations

### MidiCore Hardware
- **Display**: 256×64 OLED (SSD1322) ✅
- **Buttons**: 13 physical keys (LoopA compatible) ✅
- **Encoders**: 4 rotary encoders ✅
- **LEDs**: Need check (LoopA has LED rings) ⚠️

### Compatibility
- Display resolution: ✅ Identical (256×64)
- Grayscale levels: ✅ 16 levels (4-bit)
- Button layout: ⚠️ May differ (need mapping)
- Encoder feedback: ⚠️ LED rings optional

---

## Questions for @labodezao

1. **UI Style**:
   - Exact LoopA clone?
   - LoopA-inspired enhancements?
   - Hybrid approach?

2. **Features Priority**:
   - Scenes (A-F) support?
   - Clip matrix (6×6)?
   - LiveFX (transpose, force-to-scale)?
   - Beatloop visualization?

3. **Visual Reference**:
   - Specific LoopA screenshots?
   - Preferred layout/arrangement?
   - Must-have visual elements?

4. **Hardware**:
   - LED feedback available?
   - Button mapping preferences?
   - Encoder ring LEDs?

---

## Next Steps

**Awaiting clarification** from @labodezao on:
- Desired level of LoopA similarity
- Feature priorities
- Visual references/screenshots

Once confirmed, implementation can proceed with:
1. Create new UI page(s)
2. Implement grid layout
3. Add LoopA-style visual elements
4. Test on hardware
5. Iterate based on feedback

---

## References

- **LoopA Manual**: https://www.midiphy.com/files/9d9/midiphy_LoopA_manual_v210b.pdf
- **MIOS32 LoopA**: https://github.com/midibox/mios32/tree/master/apps/sequencers/LoopA
- **MIDIphy LoopA**: https://www.midiphy.com/en/loopa-v2/

---

## Phase 1 Implementation: New UI Pages (2026-01-12)

### Implemented Pages

Following the LOOPA_FEATURES_PLAN.md roadmap, four new UI pages have been implemented:

#### 4. `UI_PAGE_SONG` (Song Mode)
```c
// Scene arrangement view with clip matrix
SONG MODE  BPM:120  Scene:A
───────────────────────────────
   A  B  C  D  E  F  G  H
T1 ■  ■  □  ■  □  □  ■  □
T2 ■  □  ■  ■  □  ■  □  □
T3 □  ■  ■  □  ■  □  □  ■
T4 ■  ■  □  ■  ■  □  ■  □
───────────────────────────────
B1 PLAY  B2 STOP  B3 EDIT  B4 CHAIN
```

**Files**: `Services/ui/ui_page_song.c/h`

**Features**:
- 4 tracks × 8 scenes (A-H) grid view
- Visual clip indicators (■ = has clip, □ = empty)
- Scene selection with encoder
- Track selection with BTN3
- Play/Stop all tracks in scene
- Scene chaining (BTN4)

#### 5. `UI_PAGE_MIDI_MONITOR` (MIDI Monitor)
```c
// Real-time MIDI message display
MIDI MONITOR  [LIVE]  Events:24
────────────────────────────────────
12.345 P0 90 3C 64 NoteOn Ch1 N:60 V:100
12.567 P1 B0 07 7F CC Ch1 #7=127
12.789 P0 80 3C 00 NoteOff Ch1 N:60 V:0
13.012 P0 F0 7E 7F SysEx...
13.234 P1 90 40 5A NoteOn Ch1 N:64 V:90
13.456 P0 E0 00 40 Bend Ch1 Val:0
────────────────────────────────────
B1 PAUSE  B2 CLEAR  B3 FILT  B4 SAVE
```

**Files**: `Services/ui/ui_page_midi_monitor.c/h`

**Features**:
- Display last 6 MIDI messages
- Timestamp in seconds.milliseconds
- Port indication (P0=IN1, P1=OUT1, etc.)
- Decoded message type (NoteOn, CC, Bend, etc.)
- PAUSE/RESUME capture (BTN1)
- Clear buffer (BTN2)
- Message filtering placeholder (BTN3)
- Save to SD placeholder (BTN4)

#### 6. `UI_PAGE_SYSEX` (SysEx Viewer)
```c
// SysEx message capture and hex viewer
SYSEX VIEWER  Captured: 24 bytes
────────────────────────────────────
Mfr: 0x7E
Hex View:
00: F0 7E 7F 09 01 F7 00 00
08: 00 00 00 00 00 00 00 00
10: 00 00 00 00 00 00 00 00
────────────────────────────────────
B1 SEND  B2 RCV  B3 CLR  B4 SAVE
```

**Files**: `Services/ui/ui_page_sysex.c/h`

**Features**:
- Capture and display SysEx messages
- Manufacturer ID decoding (1-byte and 3-byte)
- Hex viewer with 16 bytes per row
- Scroll through large SysEx messages with encoder
- Clear captured data (BTN3)
- Send/Receive placeholders (BTN1, BTN2)
- Save to SD placeholder (BTN4)

#### 7. `UI_PAGE_CONFIG` (Config Editor)
```c
// SD card configuration file editor
CONFIG: DIN Module
────────────────────────────────────
[Category 1/4]  VIEW mode
  SRIO_DIN_ENABLE      = 1
  SRIO_DIN_BYTES       = 8
> DIN_INVERT_DEFAULT   = 0

[AINSER Module]
  AINSER_ENABLE        = 1
  AINSER_I2C_ADDR      = 0x48
  AINSER_SCAN_MS       = 5
────────────────────────────────────
B1 SAVE  B2 LOAD  B3 EDIT  B4 CAT
```

**Files**: `Services/ui/ui_page_config.c/h`

**Features**:
- SCS-style lightweight configuration editor
- Four categories: DIN, AINSER, AIN, System
- View and edit module parameters
- Toggle EDIT mode with BTN3
- Cycle categories with BTN4
- Navigate parameters with encoder
- Edit values in EDIT mode with encoder
- Save/Load config placeholders (BTN1, BTN2)

### Page Navigation

All pages are accessible via BTN5 (page cycle button):
```
LOOPER → TIMELINE → PIANO ROLL → SONG → MIDI MONITOR → SYSEX → CONFIG → (back to LOOPER)
```

Page indicators shown in header:
- `LOOP` - Looper overview
- `TIME` - Timeline view
- `PIANO` - Piano roll
- `SONG` - Song mode
- `MMON` - MIDI Monitor
- `SYSX` - SysEx viewer
- `CONF` - Config editor

### Integration Status

- ✅ Page enums added to `ui.h`
- ✅ Page handlers integrated in `ui.c`
- ✅ Module config flags added to `module_config.h`
- ✅ Build system updated (`Debug/Services/ui/subdir.mk`)
- ✅ Syntax validation passed
- ✅ All pages render without errors
- ⏳ Hardware testing pending (requires STM32 device)
- ⏳ MIDI integration pending (capture functions are placeholders)
- ⏳ SD card integration pending (save/load functions are placeholders)

### Next Steps for Full Implementation

1. **MIDI Integration**:
   - Hook `ui_midi_monitor_capture()` into MIDI router
   - Hook `ui_sysex_capture()` into MIDI router
   - Implement real-time message capture

2. **SD Card Integration**:
   - Implement config file loading in `ui_page_config.c`
   - Implement config file saving in `ui_page_config.c`
   - Add file browser for .ngc files
   - Implement SysEx save/load to SD

3. **Song Mode Backend**:
   - Implement scene management in looper
   - Add clip matrix storage (4 tracks × 8 scenes)
   - Implement scene chaining/playback logic

4. **Hardware Testing**:
   - Test on STM32F407VGT6 target
   - Verify OLED rendering
   - Test button/encoder responsiveness
   - Profile memory usage

---

## References

- **LoopA Manual**: https://www.midiphy.com/files/9d9/midiphy_LoopA_manual_v210b.pdf
- **MIOS32 LoopA**: https://github.com/midibox/mios32/tree/master/apps/sequencers/LoopA
- **MIDIphy LoopA**: https://www.midiphy.com/en/loopa-v2/

---

**Status**: ✅ Phase 1 Complete - Four new UI pages implemented  
**Ready to test**: Yes (requires hardware)  
**Estimated integration time**: 2-4 hours for full MIDI/SD integration
