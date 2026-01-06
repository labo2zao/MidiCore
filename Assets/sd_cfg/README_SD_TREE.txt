Copy the folders inside Assets/sd_cfg/ to the root of your SD card.
Expected runtime paths:
- 0:/cfg/*.ngc
- 0:/patch/...
- 0:/dream/...




UI bindings:
- File: 0:/cfg/ui_bindings.ngc
- Section: [BINDINGS]
- Keys: PATCH_PREV, PATCH_NEXT, LOAD_APPLY, BANK_PREV, BANK_NEXT
- Value: DIN bit number (0..63). Use 65535 to disable.


Encoders:
- File: 0:/cfg/ui_encoders.ngc
- Section: [ENCODERS]
- Keys: SHIFT_DIN, ENC0_A, ENC0_B, ENC0_BTN, ENC0_MODE
- ENC0_MODE: NAV or UI


Encoders:
- File: 0:/cfg/ui_encoders.ngc
- Section: [ENCODERS]
- SHIFT_LONG_MS and SHIFT_LATCH control long-press latch behavior.
- ENC1_* allows a second encoder.


UI actions:
- File: 0:/cfg/ui_actions.ngc
- Keys: ENC0_* and ENC1_* with CW/CCW/SHIFT_CW/SHIFT_CCW/BTN/SHIFT_BTN
- Values: action names (see file header).


UI state persistence:
- File: 0:/cfg/ui_state.ngs
- Auto-loaded on boot, auto-saved after changes (page, chord_mode).


Chord bank config:
- File: 0:/cfg/chord_bank.ngc
- Used when CHORD_MODE=1. Supports multiple presets [CHORD0..] and a [MAP] by note class.


Patch banks:
- In 0:/patch/bankXX.ngc, [BANK] supports CHORD_BANK=<path> to select chord_bank file per bank.


Chord bank override per patch:
- In 0:/patch/patchXX.ngc, add [PATCH] CHORD_BANK=<path> to override bank/global chord bank.
- Priority: patch -> bank -> global.


DREAM init via patch:
- In patchXX.ngc, add [DREAM] SYSEX_FILE=<path> and optional OUT_NODE=USB_OUT/DIN_OUT1..4.
- SYSEX_FILE is a text file containing hex bytes (e.g., 'F0 ... F7').


DREAM init advanced:
- In [DREAM], you can use SYSEX_LIST=path1;path2;path3 (or comma).
- SEND_ONCE=1 prevents re-sending within the same boot; optionally set KEY=... to define the identity.


DREAM SysEx file formats:
- SYSEX_FILE/SYSEX_LIST entries can be either:
  - TEXT hex (e.g. 'F0 7D ... F7' with spaces/comments)
  - BINARY .syx (starting with 0xF0)
- Format is auto-detected.


Global safety:
- 0:/cfg/global.ngc [GLOBAL] SAFE_MODE=0/1
- Hold SHIFT during boot forces SAFE_MODE (SAFE*)


Atomic SD writes:
- ui_state.ngs and state.ngs are written via .tmp then rename, with a .bak backup (best-effort).
- In SAFE_MODE, auto-saving is disabled.


SD_REQUIRED:
- In 0:/cfg/global.ngc, set SD_REQUIRED=1 to refuse patch boot if SD is missing (UI shows 'SD REQUIRED').


Watchdog:
- Optional compile-time: add -DWATCHDOG_ENABLE=1 to enable IWDG.
- If a watchdog reset is detected, SAFE mode is forced and UI shows 'SAFE (WDT)'.


Panic handlers:
- HardFault/FreeRTOS stack overflow/malloc failed trigger SAFE mode + status line (PANIC ..).
- With WATCHDOG_ENABLE=1, reboot happens automatically (kick stops).


Boot reasons:
- Brownout reset forces SAFE mode and UI shows 'SAFE (BOR)'.
- SD mount is retried 3 times at boot.


SD read-only guard:
- After 3 SD write failures, firmware enters SD RO mode (no writes). UI shows 'SD RO'.
- Reads continue.


Logging:
- Firmware buffers log lines in RAM and appends to 0:/log.txt only when SD is writable (not SAFE, not SD RO).


Instrument expressivity (B):
- File: 0:/cfg/instrument.ngc
- Sections: [HUMAN], [VELOCITY], [CHORD_COND], [CHORD_STRUM]


C (Performance mapping):
- 0:/cfg/zones.ngc : splits/layers/channels/transpose/vel scaling


C+ additions:
- 0:/cfg/expression.ngc : map expression sensor to MIDI CC (buffered/smoothed)
- zones.ngc supports L2_ENABLE and STACK.


C+ pressure sensor:
- 0:/cfg/pressure.ngc : I2C pressure polling and scaling to 0..4095 feeding expression.


Debug:
- On boot, firmware scans configured I2C bus (pressure.ngc) and logs found addresses.

Expression advanced keys in 0:/cfg/expression.ngc: DEADBAND_CC, HYST_CC, CURVE, CURVE_PARAM, BIDIR, CC_PUSH, CC_PULL.

Calibration:
- 0:/cfg/calibration.ngc : one-shot boot calibration (ATM0, PMIN/PMAX, RAW_MIN/MAX).

Calibration hot-reload:
- After calibration, firmware updates pressure (ATM0/PMIN/PMAX) and expression (RAW_MIN/MAX) in RAM immediately.

Expression:
- ZERO_DEADBAND_PA: neutral zone around 0Pa to avoid push/pull flips.
Calibration:
- CAL_KEEP_FILES: keep .bak files (1) or remove them (0) after atomic swaps.
