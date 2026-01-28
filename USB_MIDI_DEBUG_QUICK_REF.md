# USB MIDI RX Debug - Quick Reference

## Debug Message Locations (for Breakpoints)

### 1. [COMP-RX] EP:01 MIDI_OK
- **File:** `USB_DEVICE/App/usbd_composite.c`
- **Function:** `USBD_COMPOSITE_DataOut()`
- **Line:** **282**
- **Breakpoint:** `usbd_composite.c:282`

### 2. [COMP] Calling MIDI.DataOut
- **File:** `USB_DEVICE/App/usbd_composite.c`
- **Function:** `USBD_COMPOSITE_DataOut()`
- **Line:** **288**
- **Breakpoint:** `usbd_composite.c:288`

### 3. [RX-ISR] Cable:X CIN:XX
- **File:** `Services/usb_midi/usb_midi.c`
- **Function:** `usb_midi_rx_packet()`
- **Line:** **249**
- **Breakpoint:** `Services/usb_midi/usb_midi.c:249`

### 4. [RX-TASK] Processing N packet(s)
- **File:** `Services/usb_midi/usb_midi.c`
- **Function:** `usb_midi_process_rx_queue()`
- **Line:** **293**
- **Breakpoint:** `Services/usb_midi/usb_midi.c:293`

### 5. [RX] Cable:X YY YY YY (decoded MIDI)
- **File:** `App/tests/module_tests.c`
- **Function:** `module_test_usb_midi_print_packet()`
- **Line:** **7073**
- **Breakpoint:** `App/tests/module_tests.c:7073`

---

## Quick Diagnosis

### Symptom: Only [COMP-RX] EP:01 MIDI_OK appears

**Cause:** `usb_midi_process_rx_queue()` is NOT being called in test loop

**Fix:**
1. Open `App/tests/module_tests.c` (or `App/tests/app_test_usb_midi.c`)
2. Find the test loop (around line 7160 or 336)
3. Add at top of loop:
   ```c
   usb_midi_process_rx_queue();
   ```
4. Rebuild and reflash

### Expected Complete Output

```
[COMP-RX] EP:01 MIDI_OK
[COMP] Calling MIDI.DataOut
[COMP] MIDI.DataOut returned
[RX-ISR] Cable:0 CIN:09
[RX-TASK] Processing 1 packet(s)
[RX] Cable:0 90 3C 64 (Note On Ch:1 Note:60 Vel:100)
```

---

## Breakpoints for STM32CubeIDE Debugger

Copy and paste into "Add Breakpoint" dialog:

```
USB_DEVICE/App/usbd_composite.c:282
USB_DEVICE/App/usbd_composite.c:288
Services/usb_midi/usb_midi.c:249
Services/usb_midi/usb_midi.c:293
App/tests/module_tests.c:7073
```

---

## Critical Requirements

1. **Compiler Define Required:**
   ```c
   #define MODULE_TEST_USB_DEVICE_MIDI 1
   ```
   Set in: Project Properties → C/C++ Build → Settings → Preprocessor

2. **Test Loop Must Call:**
   ```c
   usb_midi_process_rx_queue();
   ```

3. **USB MIDI Must Be Initialized:**
   ```c
   usb_midi_init();  // Before MX_USB_DEVICE_Init()
   ```

---

## ISR vs Task Context

| Context | Functions | Debug Messages |
|---------|-----------|----------------|
| **ISR** | `USBD_COMPOSITE_DataOut()`, `usb_midi_rx_packet()` | `[COMP-RX]`, `[COMP]`, `[RX-ISR]` |
| **Task** | `usb_midi_process_rx_queue()`, `usb_midi_rx_debug_hook()` | `[RX-TASK]`, `[RX]` |

**Critical:** Transition from ISR to Task requires explicit call to `usb_midi_process_rx_queue()`!

---

## Full Documentation

For complete details, call chain diagrams, and debugging strategies, see:
**`USB_MIDI_RX_DEBUG_BREAKPOINTS.md`**
