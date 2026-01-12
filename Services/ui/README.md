# UI v5 (Looper + Timeline + Piano-roll + Song + MIDI Monitor + SysEx + Config)

- SSD1322 framebuffer (4bpp) via `Hal/oled_ssd1322`.
- `ui_tick_20ms()` is called from `midi_io_task` every 20ms.
- Flush is limited to 10 Hz.

## Pages

### Core Looper Pages
- `UI_PAGE_LOOPER`: track overview (REC/PLAY/STOP/MUTE)
- `UI_PAGE_LOOPER_TL`: event dots + simple field edit (tick/note/vel)
- `UI_PAGE_LOOPER_PR`: piano-roll with durations (paired NOTE ON/OFF)

### New Pages (Phase 1 - 2026-01-12)
- `UI_PAGE_SONG`: scene arrangement view (4 tracks × 8 scenes)
- `UI_PAGE_MIDI_MONITOR`: real-time MIDI message display with decoding
- `UI_PAGE_SYSEX`: SysEx capture/display with hex viewer
- `UI_PAGE_CONFIG`: SD config file editor (SCS-style)

## Page Navigation

Page cycle via `BTN5`:
```
LOOPER → TIMELINE → PIANO ROLL → SONG → MIDI MONITOR → SYSEX → CONFIG → (back to LOOPER)
```

Page indicators in header:
- `LOOP` - Looper overview
- `TIME` - Timeline view  
- `PIANO` - Piano roll
- `SONG` - Song mode
- `MMON` - MIDI Monitor
- `SYSX` - SysEx viewer
- `CONF` - Config editor

## Page Details

### UI_PAGE_SONG (Song Mode)
**File**: `ui_page_song.c/h`

**Display**:
- Header: BPM and current scene (A-H)
- 4 tracks × 8 scenes grid
- Visual clip indicators (■ = has clip, □ = empty)
- Current selection highlight

**Controls**:
- BTN1: Play selected scene
- BTN2: Stop all tracks
- BTN3: Cycle track selection
- BTN4: Cycle scene selection
- ENC: Navigate scenes

### UI_PAGE_MIDI_MONITOR
**File**: `ui_page_midi_monitor.c/h`

**Display**:
- Header: LIVE/PAUSED status, event count
- Last 6 MIDI messages with timestamps
- Port indication (P0, P1, etc.)
- Decoded message info (NoteOn, CC, Bend, etc.)

**Controls**:
- BTN1: Pause/Resume capture
- BTN2: Clear buffer
- BTN3: Filter (placeholder)
- BTN4: Save to SD (placeholder)
- ENC: Scroll history

**Integration Note**: Call `ui_midi_monitor_capture(port, data, len, timestamp_ms)` from MIDI router to feed events.

### UI_PAGE_SYSEX
**File**: `ui_page_sysex.c/h`

**Display**:
- Header: Capture status, byte count
- Manufacturer ID decoding
- Hex viewer (16 bytes per row)
- Scrollable for large messages

**Controls**:
- BTN1: Send SysEx (placeholder)
- BTN2: Receive mode (placeholder)
- BTN3: Clear captured data
- BTN4: Save to SD (placeholder)
- ENC: Scroll hex view

**Integration Note**: Call `ui_sysex_capture(data, len)` from MIDI router when SysEx received.

### UI_PAGE_CONFIG
**File**: `ui_page_config.c/h`

**Display**:
- Header: Current category name
- Category indicator (1/4)
- VIEW/EDIT mode indicator
- Parameter list with values
- Selection highlight

**Categories**:
1. DIN Module (SRIO_DIN_ENABLE, SRIO_DIN_BYTES, DIN_INVERT_DEFAULT)
2. AINSER Module (AINSER_ENABLE, AINSER_I2C_ADDR, AINSER_SCAN_MS)
3. AIN Module (AIN_VELOCITY_ENABLE, AIN_CALIBRATE_AUTO)
4. System (placeholder)

**Controls**:
- BTN1: Save config to SD (placeholder)
- BTN2: Load config from SD (placeholder)
- BTN3: Toggle EDIT mode
- BTN4: Cycle categories
- ENC (VIEW mode): Navigate parameters
- ENC (EDIT mode): Modify parameter value

**SCS-Style Features**:
- Lightweight navigation (inspired by MIDIbox NG SCS)
- Context-sensitive controls
- Quick parameter editing
- Visual feedback

## Piano-roll controls

Piano-roll controls:
- NAV: ENC scroll, B1 track, B2 zoom, B3 select nearest, B4 edit
- EDIT: ENC change field, B3 next field (start/len/note/vel), B4 apply, B2 cancel, B1 delete note


Tools (Piano-roll NAV mode):
- BTN6: duplicate selected note (offset by quant step, default 1/16)
- BTN7/BTN8: transpose +1 / -1 semitone
- BTN9: humanize (small random tick/vel)

Quantize:
- If track quant is enabled, grid is drawn and START/LEN edits snap to it.


## Hardware mapping (v7)
- New layer: `Services/input`.
- Feed physical buttons via `input_feed_button(phys_id, pressed)`.
- Default mapping:
  - phys 0..8 -> logical BTN1..BTN9
  - phys 9 -> logical BTN5 (page cycle)
  - phys 10 -> SHIFT (long press 500ms)
- SHIFT layer maps phys 0..3 -> logical BTN6..BTN9 (dup/transpose+/transpose-/humanize).

You can later replace the demo with SRIO scan (74HC165/595) and an encoder decoder.


## Global config (SD)
- File: `/cfg/system.ngc` (key=value, comments #)
- Example: `Assets/sd_cfg/system.ngc`
- Toggles:
  - SRIO_ENABLE, SRIO_DIN_ENABLE, SRIO_DOUT_ENABLE
- Polarities:
  - DOUT_INVERT_DEFAULT, DIN_INVERT_DEFAULT
- Optional:
  - BIT_INV_<n>
  - RGB_LED_<i>_R/G/B mapping + RGB_*_INVERT

To disable DIN/DOUT at runtime (no rebuild): set SRIO_DIN_ENABLE=0 and/or SRIO_DOUT_ENABLE=0.


## Sections (v10)
`/cfg/system.ngc` supports Midibox-NG-like sections.
Example:
[SRIO]
ENABLE=1
DIN_BYTES=8

[UI]
SHIFT_HOLD_MS=500

Inside a section, keys are auto-prefixed (SRIO_ENABLE, UI_SHIFT_HOLD_MS...).
Flat keys still work.

## Module Configuration

Enable/disable pages at compile time via `Config/module_config.h`:
```c
#define MODULE_ENABLE_UI_PAGE_SONG 1           // Song mode page
#define MODULE_ENABLE_UI_PAGE_MIDI_MONITOR 1   // MIDI monitor page
#define MODULE_ENABLE_UI_PAGE_SYSEX 1          // SysEx page
#define MODULE_ENABLE_UI_PAGE_CONFIG 1         // Config editor page
```

## Integration Status

**Phase 1 Complete (2026-01-12)**:
- ✅ All four new pages implemented
- ✅ Integrated into UI navigation
- ✅ Build system updated
- ✅ Syntax validated

**Phase 2 - Next Steps**:
- ⏳ MIDI router integration (Monitor, SysEx)
- ⏳ SD card file I/O (Config, SysEx save)
- ⏳ Song mode backend (scene management in looper)
- ⏳ Hardware testing on STM32F407VGT6

## See Also

- `UI_LOOPA_IMPLEMENTATION.md` - Detailed UI design documentation
- `LOOPA_FEATURES_PLAN.md` - Full feature implementation roadmap
- `ui.h/ui.c` - Main UI controller
- `ui_gfx.h/ui_gfx.c` - Graphics primitives
