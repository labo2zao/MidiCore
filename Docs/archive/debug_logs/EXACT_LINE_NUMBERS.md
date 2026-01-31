# EXACT FILE AND LINE NUMBERS FOR BREAKPOINTS

## Simple Answer

### `[COMP-RX] EP:01 MIDI_OK`
- **File:** `USB_DEVICE/App/usbd_composite.c`
- **Line:** `282`

### `[RX-ISR] Cable:0 CIN:09`
- **File:** `Services/usb_midi/usb_midi.c`
- **Line:** `249`

### `[RX-TASK] Processing 1 packet(s)`
- **File:** `Services/usb_midi/usb_midi.c`
- **Line:** `293`

### `[RX] Cable:0 90 3C 64 (...)`
- **File:** `App/tests/module_tests.c`
- **Line:** `7073`

---

## Copy These Breakpoints

For STM32CubeIDE Debug Configuration:

```
USB_DEVICE/App/usbd_composite.c:282
Services/usb_midi/usb_midi.c:249
Services/usb_midi/usb_midi.c:293
App/tests/module_tests.c:7073
```

---

## Where the Fix Was Added

The fix that should make messages appear:

### File 1: `App/tests/module_tests.c`
- **Line:** `7164`
- **What:** Added `usb_midi_process_rx_queue();`

### File 2: `App/tests/app_test_usb_midi.c`
- **Line:** `339`
- **What:** Added `usb_midi_process_rx_queue();`

---

## Full Paths from Repository Root

```
/home/runner/work/MidiCore/MidiCore/USB_DEVICE/App/usbd_composite.c         (line 282)
/home/runner/work/MidiCore/MidiCore/Services/usb_midi/usb_midi.c            (line 249)
/home/runner/work/MidiCore/MidiCore/Services/usb_midi/usb_midi.c            (line 293)
/home/runner/work/MidiCore/MidiCore/App/tests/module_tests.c                (line 7073)
```

---

## What Each Line Does

| Line | File | What It Prints |
|------|------|----------------|
| 282 | usbd_composite.c | `snprintf(buf, sizeof(buf), "[COMP-RX] EP:%02X MIDI_OK\r\n", epnum);` |
| 249 | usb_midi.c | `snprintf(buf, sizeof(buf), "[RX-ISR] Cable:%d CIN:%02X\r\n", cable, cin);` |
| 293 | usb_midi.c | `snprintf(buf, sizeof(buf), "[RX-TASK] Processing %d packet(s)\r\n", count);` |
| 7073 | module_tests.c | `dbg_printf("[RX] Cable:%d %02X %02X %02X", cable, status, data1, data2);` |
