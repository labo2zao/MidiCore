# MidiCore LoopA UI Implementation Guide

## Overview

Ce document décrit comment adapter l'UI de MidiCore pour correspondre à l'application MIOS32 LoopA.

**Date**: 2026-01-12  
**Status**: Awaiting clarification from @labodezao

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

**Status**: ⏳ Waiting for requirements clarification  
**Ready to implement**: Yes (upon approval)  
**Estimated time**: 4-12 hours (depending on scope)
