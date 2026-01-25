# MidiCore USB MIDI Implementation - CubeMX Regeneration Protection

**⚠️ IMPORTANT**: This document explains how our USB MIDI implementation is protected from STM32CubeMX code regeneration.

## Problem

STM32CubeMX regenerates code in:
- `Middlewares/ST/` → **ALL FILES OVERWRITTEN**
- `USB_DEVICE/App/` → **OVERWRITTEN (with user code markers preserved)**
- `USB_DEVICE/Target/` → **OVERWRITTEN (with user code markers preserved)**

## Our Solution

### Custom MIDI Class Location (Protected from CubeMX)

```
USB_DEVICE/Class/MIDI/
├── Inc/
│   └── usbd_midi.h          # Custom USB MIDI class header (4-port support)
└── Src/
    └── usbd_midi.c          # Custom USB MIDI class implementation (MIOS32-style)
```

**✅ This directory is NOT managed by CubeMX** → Safe from regeneration

### Application Layer (Also Protected)

```
Services/usb_midi/
├── usb_midi.h                # Application-level USB MIDI API
├── usb_midi.c                # Router integration layer
├── usb_midi_sysex.h          # SysEx handling
└── usb_midi_sysex.c
```

**✅ Services/ is never touched by CubeMX**

### CubeMX-Generated Files (Modified with Markers)

Files that CubeMX regenerates but preserves user code sections:

- `USB_DEVICE/App/usb_device.c` → Uses `/* USER CODE BEGIN/END */` markers
- `USB_DEVICE/Target/usbd_conf.c` → Uses `/* USER CODE BEGIN/END */` markers

**Our changes**:
```c
// In USB_DEVICE/App/usb_device.c
#include "USB_DEVICE/Class/MIDI/Inc/usbd_midi.h"  // ← Points to our protected class

// In USB_DEVICE/Target/usbd_conf.c  
#include "USB_DEVICE/Class/MIDI/Inc/usbd_midi.h"  // ← Points to our protected class
```

### DO NOT USE (Will be overwritten)

```
❌ Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/  ← DELETE THIS IF IT EXISTS
```

This directory will be completely overwritten by CubeMX. Our class MUST be in `USB_DEVICE/Class/MIDI/`.

---

## Post-CubeMX Regeneration Checklist

After regenerating with CubeMX, verify:

- [ ] `USB_DEVICE/Class/MIDI/` still exists (should NOT be touched)
- [ ] `USB_DEVICE/App/usb_device.c` includes: `"USB_DEVICE/Class/MIDI/Inc/usbd_midi.h"`
- [ ] `USB_DEVICE/Target/usbd_conf.c` includes: `"USB_DEVICE/Class/MIDI/Inc/usbd_midi.h"`
- [ ] `Services/usb_midi/usb_midi.c` includes: `"USB_DEVICE/Class/MIDI/Inc/usbd_midi.h"`
- [ ] NO files in `Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/` (delete if they appear)

If includes are wrong (pointing to `"usbd_midi.h"` instead of `"USB_DEVICE/Class/MIDI/Inc/usbd_midi.h"`):
```bash
# Run this script to fix includes
python3 Tools/fix_usb_includes.py
```

---

## STM32 Middleware Bugs (Still Present)

Even with our protected class, there's still ONE bug in STM32's core library that CubeMX will regenerate:

### Bug: Uninitialized classId in usbd_core.c

**File**: `Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c`

After CubeMX regeneration, you MUST apply this fix:

```c
// Around line 120, in USBD_Init() function:
#else
  pdev->pClass[0] = NULL;
  pdev->pUserData[0] = NULL;
  
  /* USER CODE BEGIN: Initialize class tracking variables */
  pdev->NumClasses = 0;
  pdev->classId = 0;
  /* USER CODE END */
#endif
```

**Run after each CubeMX regen**:
```bash
python3 Tools/fix_stm32_usb_core.py
```

See `Docs/STM32_USB_LIBRARY_BUGS.md` for details.

---

## Architecture

```
Application Layer
  ↓
Services/usb_midi/          ← Router integration (protected)
  ↓
USB_DEVICE/Class/MIDI/      ← Our custom MIDI class (protected)
  ↓
Middlewares/.../Core/       ← STM32 USB Device core (CubeMX managed, needs one fix)
  ↓
HAL USB PCD Driver          ← STM32 HAL (CubeMX managed)
```

---

## Key Files and Their Status

| File | CubeMX Managed? | Protected? | Contains |
|------|----------------|-----------|----------|
| `USB_DEVICE/Class/MIDI/*` | ❌ No | ✅ Yes | Our custom MIDI class |
| `Services/usb_midi/*` | ❌ No | ✅ Yes | Application layer |
| `USB_DEVICE/App/*` | ⚠️ Partial | ⚠️ Use markers | USB Device init |
| `USB_DEVICE/Target/*` | ⚠️ Partial | ⚠️ Use markers | HAL callbacks |
| `Middlewares/ST/...Core/` | ✅ Yes | ❌ No | STM32 USB core (has 1 bug) |
| `Middlewares/ST/.../MIDI/` | ✅ Yes | ❌ No | DELETE - use ours instead |

---

## Benefits of This Approach

1. **✅ CubeMX Safe**: Our MIDI class survives regeneration
2. **✅ MIOS32 Compatible**: Based on proven MIOS32 implementation
3. **✅ 4-Port Support**: Full 4x4 MIDI interface like MIOS32
4. **✅ No Duplicates**: Single source of truth in `USB_DEVICE/Class/MIDI/`
5. **✅ Bug-Free Descriptors**: Correct wTotalLength calculation (211 bytes)
6. **⚠️ One Manual Fix**: Still need to fix `usbd_core.c` after CubeMX regen (automated script provided)

---

## References

- MIOS32 USB: https://github.com/midibox/mios32/tree/master/drivers/STM32F4xx
- USB MIDI Spec: https://www.usb.org/sites/default/files/midi10.pdf
- STM32F4 RM0090: Reference Manual (USB OTG sections)
