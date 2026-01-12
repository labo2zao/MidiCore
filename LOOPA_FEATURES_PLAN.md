# MidiCore LoopA-Inspired Features Implementation Plan

## Overview

Plan d'implémentation des fonctionnalités inspirées de LoopA pour MidiCore.

**Date**: 2026-01-12  
**Requester**: @labodezao  
**Status**: Implementation in progress

---

## Requirements Summary

### Core Features Requested

1. **UI inspirée LoopA** (pas clone exact)
   - 4 tracks (déjà configuré: `LOOPER_TRACKS 4`)
   - OLED SSD1322 256×64 (déjà en place)
   - Layout cohérent

2. **Mode Song**
   - Arrangement/séquençage de clips
   - Timeline de scènes

3. **MIDI Monitor**
   - Affichage temps réel des messages MIDI
   - IN/OUT monitoring

4. **Mode SysEx**
   - Capture/envoi SysEx
   - Affichage formaté

5. **LiveFX**
   - Transposition
   - Humanization
   - Velocity scaling
   - Force-to-scale

6. **Beatloop**
   - Loop region indicator
   - Visual playhead

7. **Force-to-Scale**
   - Quantization mélodique
   - Scale selection

8. **Export MIDI**
   - Sauvegarde .mid format SMF
   - Track export

9. **Mode Config (NEW)**
   - Éditeur de fichiers de configuration SD
   - Édition des paramètres DIN module
   - Édition des paramètres AINSER module
   - Édition des paramètres AIN module
   - Sauvegarde sur carte SD

10. **Step Playback (NEW)**
   - Déplacement curseur avant/arrière avec footswitches
   - Playback pas à pas (step-by-step)
   - Similaire au step recording
   - Navigation manuelle dans timeline
   - Intégration contrôle pédale

---

## UI Design Philosophy

### Inspiration: MIDIbox NG Standard Control Surface (SCS)

**Reference**: http://www.ucapps.de/midibox_ng_manual_scs.html

Le SCS (Standard Control Surface) de MIDIbox NG offre une UI légère avec:
- **Minimal buttons**: 4-6 boutons (Left, Right, Up, Down, Shift, Select)
- **Small display**: LCD 2×16 ou petit OLED
- **Menu hierarchy**: Navigation par menu/sous-menu
- **Context-sensitive**: Actions changent selon le contexte
- **Quick access**: Fonctions principales à 1-2 boutons

### MidiCore UI Approach

**Inspiration SCS pour MidiCore**:
- ✅ UI légère et rapide
- ✅ Navigation hiérarchique (pages → sous-pages)
- ✅ Boutons context-sensitive
- ✅ Affichage compact mais informatif
- ✅ OLED 256×64 (plus grand que SCS LCD 2×16)

**Avantages pour MidiCore**:
- Interface accessible sans souris/clavier
- Édition live pendant performance
- Faible consommation de ressources CPU
- Compatible hardware MBHP

---

## Implementation Phases

### Phase 1: UI Pages (Priority 1) ✅ Starting

#### 1.1 Enhanced Looper Overview
- [x] Keep existing 4-track display
- [ ] Add visual indicators (boxes, states)
- [ ] Improve layout LoopA-style
- [ ] Add scene indicator

#### 1.2 Song Mode Page (NEW)
- [ ] Create `ui_page_song.c/h`
- [ ] Scene arrangement view
- [ ] Clip matrix (4 tracks × N scenes)
- [ ] Scene playback controls

#### 1.3 MIDI Monitor Page (NEW)
- [ ] Create `ui_page_midi_monitor.c/h`
- [ ] Real-time MIDI message display
- [ ] IN/OUT ports visualization
- [ ] Message filtering

#### 1.4 SysEx Page (NEW)
- [ ] Create `ui_page_sysex.c/h`
- [ ] SysEx capture/display
- [ ] Hex viewer
- [ ] Send/receive controls

#### 1.5 Config Editor Page (NEW)
- [ ] Create `ui_page_config.c/h`
- [ ] File browser for SD config files
- [ ] Text editor for .ngc files
- [ ] Parameter editor (structured view)
- [ ] DIN/AINSER/AIN module configuration
- [ ] Save/reload config files

### Phase 2: Looper Features (Priority 2)

#### 2.1 LiveFX System
- [ ] Create `Services/livefx/livefx.c/h`
- [ ] Transpose (-12 to +12 semitones)
- [ ] Velocity scale (0-200%)
- [ ] Apply to track/all
- [ ] Integration with humanize

#### 2.2 Force-to-Scale
- [ ] Create `Services/scale/scale.c/h`
- [ ] Scale definitions (Major, Minor, etc.)
- [ ] Note quantization to scale
- [ ] Real-time application
- [ ] UI for scale selection

#### 2.3 Beatloop Enhancement
- [ ] Visual loop region on timeline
- [ ] Playhead indicator

#### 2.4 Step Playback (NEW)
- [ ] Add cursor navigation functions
- [ ] Forward/backward step functions
- [ ] Footswitch integration
- [ ] Pause on step mode
- [ ] Visual cursor highlight
- [ ] Step size configuration (beat/bar/tick)
- [ ] Loop start/end markers
- [ ] Visual feedback during loop

#### 2.4 Song Mode Logic
- [ ] Scene structure in looper
- [ ] Scene chaining
- [ ] Scene playback engine
- [ ] Save/load song arrangements

### Phase 3: Advanced Features (Priority 3)

#### 3.1 Step Playback System
- [ ] Add to `Services/looper/looper.c`
- [ ] Cursor position tracking
- [ ] Step forward/backward functions
- [ ] Footswitch event handlers
- [ ] Integration with UI timeline
- [ ] Configuration: step size (beat/16th/32nd)

#### 3.2 MIDI Export

#### 3.1 MIDI File Export
- [ ] Create `Services/midi_export/midi_export.c/h`
- [ ] SMF (Standard MIDI File) format
- [ ] Type 1 (multi-track)
- [ ] Tempo map export
- [ ] Save to SD card

### Phase 4: Integration & Polish (Priority 4)

#### 4.1 UI Navigation
- [ ] Page switching logic
- [ ] Button mappings
- [ ] Encoder assignments
- [ ] Status bar

#### 4.2 Module Configuration
- [ ] Add enable flags for new modules
- [ ] Update `module_config.h`
- [ ] Dependency validation

---

## File Structure

### New Files to Create

```
Services/
├── livefx/
│   ├── livefx.c
│   ├── livefx.h
│   └── README.md
├── scale/
│   ├── scale.c
│   ├── scale.h
│   └── scale_definitions.h
├── midi_export/
│   ├── midi_export.c
│   ├── midi_export.h
│   └── smf_format.h
├── midi_monitor/
│   ├── midi_monitor.c
│   └── midi_monitor.h
├── config_editor/
│   ├── config_editor.c
│   ├── config_editor.h
│   └── ngc_parser.c
└── ui/
    ├── ui_page_song.c
    ├── ui_page_song.h
    ├── ui_page_midi_monitor.c
    ├── ui_page_midi_monitor.h
    ├── ui_page_sysex.c
    ├── ui_page_sysex.h
    ├── ui_page_config.c
    └── ui_page_config.h
```

### Files to Modify

```
Services/ui/ui.h              # Add new page enums
Services/ui/ui.c              # Add page handlers
Services/looper/looper.h      # Add scene support
Services/looper/looper.c      # Implement scenes
Config/module_config.h        # Add new module flags
App/app_init.c               # Initialize new modules
```

---

## Detailed Implementation

### 1. LiveFX Module

**File**: `Services/livefx/livefx.c`

```c
typedef struct {
  int8_t transpose;        // -12 to +12 semitones
  uint8_t vel_scale;       // 0-200% (128 = 100%)
  uint8_t force_scale;     // Scale index
  uint8_t scale_root;      // Root note (C=0)
  uint8_t enabled;
} livefx_t;

void livefx_init(void);
void livefx_set_transpose(int8_t semitones);
void livefx_set_velocity_scale(uint8_t percent);
void livefx_set_force_scale(uint8_t scale_idx, uint8_t root);
void livefx_apply(router_msg_t* msg);
```

### 2. Scale Quantization

**File**: `Services/scale/scale.c`

```c
typedef enum {
  SCALE_CHROMATIC = 0,
  SCALE_MAJOR,
  SCALE_MINOR_NAT,
  SCALE_MINOR_HARM,
  SCALE_MINOR_MEL,
  SCALE_DORIAN,
  SCALE_PHRYGIAN,
  SCALE_LYDIAN,
  SCALE_MIXOLYDIAN,
  SCALE_PENTATONIC_MAJ,
  SCALE_PENTATONIC_MIN,
  SCALE_BLUES
} scale_type_t;

uint8_t scale_quantize_note(uint8_t note, scale_type_t scale, uint8_t root);
const char* scale_name(scale_type_t scale);
```

### 3. Song Mode

**File**: `Services/looper/looper.h` (additions)

```c
#define LOOPER_SCENES 8

typedef struct {
  uint8_t scene_chain[16];  // Scene sequence
  uint8_t chain_len;
  uint8_t current_scene;
} song_mode_t;

void looper_song_init(void);
void looper_song_set_scene(uint8_t scene_idx);
void looper_song_play_chain(void);
```

### 4. MIDI Monitor

**File**: `Services/midi_monitor/midi_monitor.c`

```c
#define MONITOR_BUFFER_SIZE 64

typedef struct {
  uint32_t timestamp_ms;
  uint8_t port;      // IN1/OUT1/etc
  uint8_t len;
  uint8_t data[4];
} midi_event_t;

void midi_monitor_init(void);
void midi_monitor_capture(uint8_t port, const uint8_t* data, uint8_t len);
uint32_t midi_monitor_get_events(midi_event_t* out, uint32_t max);
void midi_monitor_clear(void);
```

### 5. MIDI Export (SMF)

**File**: `Services/midi_export/midi_export.c`

```c
int midi_export_track(uint8_t track_idx, const char* filename);
int midi_export_all_tracks(const char* filename);
```

**Format**: Standard MIDI File Type 1
- Header chunk (MThd)
- Track chunks (MTrk)
- Tempo map
- Time signature
- Note events

### 6. Step Playback (NEW)

**File**: `Services/looper/looper.c` (extensions)

```c
typedef enum {
  STEP_SIZE_TICK,    // 1 tick (PPQN/24)
  STEP_SIZE_16TH,    // 1/16 note (PPQN/4)
  STEP_SIZE_8TH,     // 1/8 note (PPQN/2)
  STEP_SIZE_BEAT,    // 1 beat (PPQN)
  STEP_SIZE_BAR      // 1 bar (PPQN*4)
} step_size_t;

// Step playback functions
void looper_step_playback_enable(uint8_t track_idx, bool enable);
void looper_step_forward(uint8_t track_idx);   // Move cursor forward
void looper_step_backward(uint8_t track_idx);  // Move cursor backward
void looper_step_set_size(step_size_t size);   // Configure step size
uint32_t looper_step_get_cursor(uint8_t track_idx);  // Get cursor position

// Footswitch integration
void looper_on_footswitch_fwd(void);  // Callback for forward footswitch
void looper_on_footswitch_bwd(void);  // Callback for backward footswitch
```

**Fonctionnalités**:
- Navigation manuelle dans timeline avec footswitches
- Playback pas à pas (pause automatique entre steps)
- Step size configurable (tick/16th/8th/beat/bar)
- Highlight visuel du curseur sur timeline UI
- Compatible avec step recording existant

---

## UI Layout Designs

### Song Mode Page

```
┌─────────────────────────────────────────────────┐
│ SONG MODE      BPM:120  Scene: A→B→C→A         │
├──────────┬──────────┬──────────┬──────────┬────┤
│ Scene A  │ Scene B  │ Scene C  │ Scene D  │... │
├──────────┼──────────┼──────────┼──────────┼────┤
│ T1: ■4br │ T1: ■8br │ T1: □--  │ T1: ■4br │    │
│ T2: ■4br │ T2: □--  │ T2: ■4br │ T2: ■8br │    │
│ T3: □--  │ T3: ■4br │ T3: ■4br │ T3: □--  │    │
│ T4: ■8br │ T4: ■4br │ T4: □--  │ T4: ■4br │    │
└──────────┴──────────┴──────────┴──────────┴────┘
│ PLAY │STOP│EDIT│CHAIN│SAVE│LOAD│           │
```

### MIDI Monitor Page

```
┌─────────────────────────────────────────────────┐
│ MIDI MONITOR           IN ▼  OUT ▲  Clear      │
├─────────────────────────────────────────────────┤
│ [12:34:56.123] IN1  90 3C 64  (Note On C3)     │
│ [12:34:56.456] OUT1 B0 07 7F  (CC 7 = 127)     │
│ [12:34:56.789] IN2  80 3C 00  (Note Off C3)    │
│ [12:34:57.012] IN1  F0 7E 7F 09 01 F7 (SysEx)  │
│ [12:34:57.234] OUT1 90 40 5A  (Note On E3)     │
│ ...                                              │
└─────────────────────────────────────────────────┘
│ PAUSE│CLR │FILT│SAVE│PORT│MODE│           │
```

### SysEx Page

```
┌─────────────────────────────────────────────────┐
│ SYSEX MODE     Captured: 24 bytes              │
├─────────────────────────────────────────────────┤
│ F0 7E 7F 09 01 F7                              │
│ ^ManID ^Dev ^Cmd                               │
│                                                  │
│ Hex View:                                       │
│ 00: F0 7E 7F 09 01 F7 00 00                    │
│ 08: 00 00 00 00 00 00 00 00                    │
│                                                  │
└─────────────────────────────────────────────────┘
│ SEND │RCV │CLR │SAVE│LOAD│EDIT│           │
```

### Config Editor Page (NEW)

**Inspired by MIDIbox NG SCS (Standard Control Surface)**

```
┌─────────────────────────────────────────────────┐
│ CONFIG EDITOR   File: global.ngc               │
├─────────────────────────────────────────────────┤
│ [DIN Module Configuration]                      │
│ SRIO_DIN_ENABLE      = 1                       │
│ SRIO_DIN_BYTES       = 8                       │
│ DIN_INVERT_DEFAULT   = 0                       │
│                                                  │
│ [AINSER Module Configuration]                   │
│ AINSER_ENABLE        = 1                       │
│ AINSER_I2C_ADDR      = 0x48                    │
│ AINSER_SCAN_MS       = 5                       │
│                                                  │
│ [AIN Module Configuration]                      │
│ AIN_VELOCITY_ENABLE  = 1                       │
│ AIN_CALIBRATE_AUTO   = 1                       │
└─────────────────────────────────────────────────┘
│ SAVE │LOAD│EDIT│FILES│↑   │↓   │           │
```

**SCS-Style Lightweight Navigation**:
- Menu hierarchy: Main → Category → Parameter
- 4-6 button navigation (cursor keys + shift)
- Context-sensitive button labels
- Quick parameter editing (increment/decrement)
- Visual feedback (highlight, cursor)

### LiveFX Controls

```
┌─────────────────────────────────────────────────┐
│ LIVEFX         Track: 1    [ENABLED]           │
├─────────────────────────────────────────────────┤
│ Transpose:  +2 semitones   [-12 ... +12]       │
│ Velocity:   110%           [0% ... 200%]       │
│ Scale:      C Major        [12 scales]         │
│ Force Scale: [X] ON                             │
│                                                  │
│ Humanize:   [X] Time ±3ms  [ ] Vel ±5         │
└─────────────────────────────────────────────────┘
│ APPLY│RESET│SCALE│SAVE│LOAD│     │           │
```

---

## Module Configuration Updates

### `Config/module_config.h` additions

```c
/** @brief Enable LiveFX module */
#ifndef MODULE_ENABLE_LIVEFX
#define MODULE_ENABLE_LIVEFX 1
#endif

/** @brief Enable Scale quantization */
#ifndef MODULE_ENABLE_SCALE
#define MODULE_ENABLE_SCALE 1
#endif

/** @brief Enable Song mode */
#ifndef MODULE_ENABLE_SONG_MODE
#define MODULE_ENABLE_SONG_MODE 1
#endif

/** @brief Enable MIDI Monitor */
#ifndef MODULE_ENABLE_MIDI_MONITOR
#define MODULE_ENABLE_MIDI_MONITOR 1
#endif

/** @brief Enable Config Editor UI page */
#ifndef MODULE_ENABLE_CONFIG_EDITOR
#define MODULE_ENABLE_CONFIG_EDITOR 1
#endif

/** @brief Enable SCS-style lightweight UI navigation */
#ifndef MODULE_ENABLE_SCS_UI_STYLE
#define MODULE_ENABLE_SCS_UI_STYLE 1
#endif

/** @brief Enable Step Playback (NEW) */
#ifndef MODULE_ENABLE_STEP_PLAYBACK
#define MODULE_ENABLE_STEP_PLAYBACK 1
#endif
```

---

## Implementation Timeline

### Immediate (Commit 1)
- [x] Create implementation plan document
- [x] Update UI_LOOPA_IMPLEMENTATION.md

### Week 1 (Commits 2-4)
- [ ] LiveFX module skeleton
- [ ] Scale module with quantization
- [ ] Enhanced looper overview page
- [ ] **Step playback functions** (NEW)

### Week 2 (Commits 5-7)
- [ ] Song mode logic
- [ ] Song mode UI page
- [ ] MIDI monitor capture logic
- [ ] **Footswitch integration for step playback** (NEW)

### Week 3 (Commits 8-10)
- [ ] MIDI monitor UI page
- [ ] SysEx UI page
- [ ] Beatloop visual enhancements
- [ ] **Step playback UI indicators** (NEW)

### Week 4 (Commits 11-13)
- [ ] MIDI export (SMF format)
- [ ] Config editor (NEW)
- [ ] Integration testing
- [ ] Documentation updates

---

## Testing Strategy

### Unit Tests
- Scale quantization accuracy
- LiveFX transpose/velocity
- MIDI file format validation
- Song mode scene chaining

### Integration Tests
- LiveFX → Router pipeline
- Monitor capture → Display
- Looper → MIDI export
- Song mode playback

### UI Tests
- Page navigation
- Visual feedback
- Button/encoder response
- Display refresh rate

---

## Documentation Updates

### Files to Update
- `README_MODULE_CONFIG.md` - Add new modules
- `MIOS32_COMPATIBILITY.md` - Note LoopA features
- `UI_LOOPA_IMPLEMENTATION.md` - Implementation status

### New Documentation
- `Services/livefx/README.md`
- `Services/scale/README.md`
- `Services/midi_export/README.md`

---

## Compatibility Notes

### MIOS32 LoopA Features
- ✅ LiveFX (transpose, velocity, force-to-scale)
- ✅ Beatloop visualization
- ✅ Song mode (scene arrangement)
- ⚠️ MIDI export (not in original LoopA)
- ⚠️ MIDI monitor (enhanced feature)

### MidiCore Enhancements
- 4 tracks (vs 6 in LoopA)
- MIDI monitor page
- SysEx dedicated page
- MIDI file export

---

## Status Tracking

| Feature | Status | Files | Commit |
|---------|--------|-------|--------|
| Implementation Plan | ✅ Done | This file | TBD |
| LiveFX Module | ⏳ Pending | livefx.c/h | TBD |
| Scale Module | ⏳ Pending | scale.c/h | TBD |
| Song Mode | ⏳ Pending | looper.c, ui_page_song.c | TBD |
| MIDI Monitor | ⏳ Pending | midi_monitor.c, ui_page_midi_monitor.c | TBD |
| SysEx Page | ⏳ Pending | ui_page_sysex.c | TBD |
| Beatloop UI | ⏳ Pending | ui_page_looper_timeline.c | TBD |
| MIDI Export | ⏳ Pending | midi_export.c | TBD |

---

**Next Step**: Begin implementation with LiveFX and Scale modules (most fundamental)
