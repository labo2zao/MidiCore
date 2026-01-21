# STM32 USB Device Library - Known Bugs & Post-CubeMX Regeneration Fixes

**⚠️ CRITICAL**: This document describes bugs in STM32's official USB Device Library that **MUST BE FIXED** after CubeMX code regeneration.

## Overview

The STM32 USB Device Library (in `Middlewares/ST/STM32_USB_Device_Library/`) contains two critical bugs that prevent USB MIDI enumeration. **These files are managed by CubeMX and will be overwritten** during regeneration.

---

## Bug #1: Uninitialized classId in Non-Composite Mode

### File
`Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c`

### Location
Function `USBD_Init()`, lines ~106-123

### Problem
```c
#ifdef USE_USBD_COMPOSITE
  pdev->classId = 0;          // ✅ Initialized in composite mode
  pdev->NumClasses = 0;
#else
  pdev->pClass[0] = NULL;
  pdev->pUserData[0] = NULL;
  // ❌ BUG: classId and NumClasses NEVER initialized!
  // Contains garbage values from RAM
#endif
```

### Impact
- Configuration descriptor retrieval fails (reads from `pClass[garbage]` instead of `pClass[0]`)
- Windows reports "Unknown Device" or VID_0000&PID_0000
- Device never reaches SET_CONFIGURATION state
- `USBD_MIDI_Init()` callback never called

### Fix Required After CubeMX Regeneration
Add after line with `pdev->pUserData[0] = NULL;`:
```c
  /* Initialize class tracking variables (CRITICAL FIX for non-composite mode) */
  pdev->NumClasses = 0;
  pdev->classId = 0;
```

---

## Bug #2: Incorrect MIDI Configuration Descriptor Length

### File
`Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Src/usbd_midi.c`

### Location
Lines ~64-70 (descriptor size calculations)

### Problem
```c
// ❌ WRONG: Assumes all MIDI IN Jacks are 6 bytes
#define USB_DESC_SIZE_JACK_IN 6
#define USB_MIDI_JACK_DESC_SIZE_PER_PORT ((USB_DESC_SIZE_JACK_IN * 2) + (USB_DESC_SIZE_JACK_OUT * 2))
// Result: (6*2) + (9*2) = 30 bytes/port ❌
```

**Reality**: 
- External IN Jack: 6 bytes ✅
- Embedded IN Jack: **9 bytes** ✅ (has `bNrInputPins`, `baSourceID`, `baSourcePin`)
- Embedded OUT Jack: 9 bytes ✅
- External OUT Jack: 9 bytes ✅
- **Actual per port: 33 bytes** (not 30!)

With 4 ports: 12 bytes short → Windows "CONFIGURATION_DESCRIPTOR_VALIDATION_FAILURE" (0xC00000E5)

### Fix Required After CubeMX Regeneration
Replace lines ~64-70:
```c
#define USB_DESC_SIZE_JACK_IN_EXTERNAL   6      /* MIDI IN Jack descriptor (External) */
#define USB_DESC_SIZE_JACK_IN_EMBEDDED   9      /* MIDI IN Jack descriptor (Embedded - has source pins) */
#define USB_DESC_SIZE_JACK_OUT           9      /* MIDI OUT Jack descriptor */

/* Calculate descriptor size - CORRECTED
 * Each port has 4 jacks:
 * - 1 External IN Jack: 6 bytes
 * - 1 Embedded IN Jack: 9 bytes (includes bNrInputPins, baSourceID, baSourcePin)
 * - 1 Embedded OUT Jack: 9 bytes
 * - 1 External OUT Jack: 9 bytes
 * Total per port: 33 bytes (was incorrectly 30)
 */
#define USB_MIDI_JACK_DESC_SIZE_PER_PORT (USB_DESC_SIZE_JACK_IN_EXTERNAL + \
                                          USB_DESC_SIZE_JACK_IN_EMBEDDED + \
                                          USB_DESC_SIZE_JACK_OUT + \
                                          USB_DESC_SIZE_JACK_OUT)
```

---

## Post-CubeMX Regeneration Checklist

After regenerating code with STM32CubeMX:

- [ ] **Bug #1**: Apply fix in `usbd_core.c` - Initialize `classId` and `NumClasses` in non-composite mode
- [ ] **Bug #2**: Apply fix in `usbd_midi.c` - Correct MIDI jack descriptor size calculation
- [ ] Rebuild project
- [ ] Test USB enumeration (should show correct VID/PID, not VID_0000&PID_0000)
- [ ] Verify Windows Device Manager shows device without "CONFIGURATION_DESCRIPTOR_VALIDATION_FAILURE"

---

## Alternative Solution: Custom USB Stack

To avoid modifying STM32 middleware (which is overwritten by CubeMX), consider implementing a custom USB MIDI stack like MIOS32:

**MIOS32 approach** (https://github.com/midibox/mios32):
- Complete custom USB implementation in `mios32/STM32F4xx/mios32_usb*.c`
- Does NOT use ST's USB Device Library
- Direct HAL USB API usage
- Protected from CubeMX regeneration

---

## Why These Bugs Exist

1. **Bug #1**: STM32 library assumes composite mode is default, non-composite mode was added later without proper initialization
2. **Bug #2**: Descriptor size calculation assumes all jack descriptors are same size, but Embedded IN jacks have additional source connection fields

Both bugs have been reported to ST but are not fixed in current CubeMX versions (as of 2026-01).

---

## Verification

After applying fixes, verify with:

1. **Windows Event Viewer**: Should NOT show "CONFIGURATION_DESCRIPTOR_VALIDATION_FAILURE"
2. **Device Manager**: Should show correct VID/PID (not VID_0000&PID_0000)
3. **USBTreeView** or **USBDeview**: Descriptor length should be 211 bytes (0xD3)
4. **Debugger**: `hUsbDeviceFS.pClass[0]` should be non-NULL and point to USBD_MIDI

---

## References

- USB MIDI Device Class Specification v1.0
- STM32F4xx Reference Manual (RM0090)
- MIOS32 USB Implementation: https://github.com/midibox/mios32/tree/master/drivers/STM32F4xx
