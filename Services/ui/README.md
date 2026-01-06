# UI v4 (Looper Overview + Timeline + Piano-roll)

- SSD1322 framebuffer (4bpp) via `Hal/oled_ssd1322`.
- `ui_tick_20ms()` is called from `midi_io_task` every 20ms.
- Flush is limited to 10 Hz.

Pages:
- `UI_PAGE_LOOPER`: track overview (REC/PLAY/STOP/MUTE)
- `UI_PAGE_LOOPER_TL`: event dots + simple field edit (tick/note/vel)
- `UI_PAGE_LOOPER_PR`: piano-roll with durations (paired NOTE ON/OFF)

Page cycle:
- `BTN5` cycles: Overview -> Timeline -> Piano-roll -> Overview

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
