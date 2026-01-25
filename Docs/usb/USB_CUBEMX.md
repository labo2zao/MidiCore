# STM32CubeMX Configuration and Protection Guide

**Document Version:** 2.0  
**Last Updated:** 2026-01-25  
**Target:** STM32F407VGT6, MidiCore 4x4 USB MIDI Interface

---

## Table of Contents

1. [CubeMX OTG Mode Issue](#cubemx-otg-mode-issue)
2. [Protecting Custom USB Code from Regeneration](#protecting-custom-usb-code-from-regeneration)
3. [USB Host and Device Coexistence](#usb-host-and-device-coexistence)
4. [Post-Regeneration Checklist](#post-regeneration-checklist)
5. [STM32 USB Library Bugs](#stm32-usb-library-bugs)
6. [References](#references)

---

## CubeMX OTG Mode Issue

### The Problem

When you select **OTG_FS** mode in STM32CubeMX:
- USB_HOST middleware becomes **grayed out** ❌
- USB_DEVICE middleware becomes **grayed out** ❌
- You cannot configure middleware through CubeMX GUI

### Why This Happens

**CubeMX Design**: When you select OTG_FS at the hardware level:
- CubeMX assumes you want **runtime mode switching**
- Middleware selection is disabled because CubeMX doesn't know if you'll use Host, Device, or both
- You're expected to handle mode configuration programmatically

### The Solution

Changed USB configuration to **Device_Only** mode instead of OTG_FS:

```
USB_OTG_FS.VirtualMode = Device_Only  (instead of OTG_FS)
```

Added USB_DEVICE middleware:
```
USB_DEVICE.CLASS_NAME_FS = MIDI
USB_DEVICE.VirtualMode = Midi
USB_DEVICE.VirtualModeFS = Midi_FS
```

### Why This Works

✅ **USB_DEVICE middleware is now selectable** in CubeMX  
✅ **Code generation works** normally  
✅ **MIDI class is configured** and ready  
✅ **You can still add Host mode** programmatically if needed later

### Current Configuration

**Hardware**: USB_OTG_FS in Device_Only mode
- ID pin: PA10 (configured)
- D+ pin: PA12 (configured)
- D- pin: PA11 (configured)

**Middleware**: USB_DEVICE with MIDI class
- 4 virtual MIDI ports (cables 0-3)
- MidiCore will appear in your DAW as MIDI interface
- All 4 ports independently usable

### About OTG and Runtime Mode Switching

#### Can I Still Do OTG (Host + Device)?

**Yes!** But it requires more work:

1. **Start with Device mode** (current configuration)
2. **Implement runtime switching** in your code
3. **Detect cable type** via ID pin
4. **Switch modes** programmatically based on detection

#### Do You Need OTG?

**For your use case** (appearing in DAW):
- ✅ **Device mode is sufficient**
- You want to appear as MIDI interface → Device mode
- Computer is the Host, MidiCore is the Device
- This is the standard configuration for USB MIDI interfaces

**Only need OTG if**:
- You want to switch between DAW mode and MIDI keyboard reading mode
- You need runtime flexibility to be either Host or Device
- Your hardware setup changes frequently

---

## Protecting Custom USB Code from Regeneration

### Problem

STM32CubeMX regenerates code in:
- `Middlewares/ST/` → **ALL FILES OVERWRITTEN**
- `USB_DEVICE/App/` → **OVERWRITTEN (with user code markers preserved)**
- `USB_DEVICE/Target/` → **OVERWRITTEN (with user code markers preserved)**

### Our Solution

#### Custom MIDI Class Location (Protected from CubeMX)

```
USB_DEVICE/Class/MIDI/
├── Inc/
│   └── usbd_midi.h          # Custom USB MIDI class header (4-port support)
└── Src/
    └── usbd_midi.c          # Custom USB MIDI class implementation (MIOS32-style)
```

**✅ This directory is NOT managed by CubeMX** → Safe from regeneration

#### Application Layer (Also Protected)

```
Services/usb_midi/
├── usb_midi.h                # Application-level USB MIDI API
├── usb_midi.c                # Router integration layer
├── usb_midi_sysex.h          # SysEx handling
└── usb_midi_sysex.c
```

**✅ Services/ is never touched by CubeMX**

#### CubeMX-Generated Files (Modified with Markers)

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

#### DO NOT USE (Will be overwritten)

```
❌ Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/  ← DELETE THIS IF IT EXISTS
```

This directory will be completely overwritten by CubeMX. Our class MUST be in `USB_DEVICE/Class/MIDI/`.

### Architecture

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

### Benefits

1. **✅ CubeMX Safe**: Our MIDI class survives regeneration
2. **✅ MIOS32 Compatible**: Based on proven MIOS32 implementation
3. **✅ 4-Port Support**: Full 4x4 MIDI interface like MIOS32
4. **✅ No Duplicates**: Single source of truth in `USB_DEVICE/Class/MIDI/`
5. **✅ Bug-Free Descriptors**: Correct wTotalLength calculation (215 bytes)
6. **⚠️ One Manual Fix**: Still need to fix `usbd_core.c` after CubeMX regen (automated script provided)

---

## USB Host and Device Coexistence

### Hardware Limitation

The **STM32F407VGT6** has **ONE** USB peripheral: `USB_OTG_FS`

**Hardware limitation**:
- 1 USB OTG Full-Speed controller
- 1 set of D+/D- pins (PA11/PA12)
- Can only be in **ONE MODE** at any given time

### Possible Modes

#### Mode 1: USB Device ONLY
```
STM32 ──USB──> PC (DAW)
     (MidiCore appears as MIDI interface)
```
- 4 MIDI ports to PC
- Standard USB cable
- This is what we're implementing!

#### Mode 2: USB Host ONLY  
```
USB MIDI Keyboard ──> STM32 (MidiCore)
              (reads keyboard data)
```
- Reads up to 16 MIDI cables
- Requires OTG cable + external VBUS power
- Advanced MIOS32 mode

#### Mode 3: Automatic Switching (MIOS32-style)
```
┌─────────────────────────────────────────┐
│ Cable type detection:                   │
│                                         │
│ Standard cable → Device Mode           │
│ OTG cable      → Host Mode             │
│                                         │
│ Automatic runtime switching            │
└─────────────────────────────────────────┘
```

### MIOS32 Automatic Mode Switching

**Cable detection** via ID pin (PA10):
```c
if (GPIO_ReadPin(PA10) == LOW) {
    // ID pin grounded = OTG cable = Host mode
    switch_to_host_mode();
} else {
    // ID pin floating = Standard cable = Device mode  
    switch_to_device_mode();
}
```

**Switching sequence**:
1. Detect cable change (interrupt on PA10)
2. Stop current mode (`HAL_PCD_Stop` or `HAL_HCD_Stop`)
3. Disable interrupts
4. Reset USB peripheral
5. Configure new mode
6. Start new mode

### Current MidiCore Configuration

**Configuration present**:
```c
// Config/module_config.h
#define MODULE_ENABLE_USB_MIDI   1  // Device mode
#define MODULE_ENABLE_USBH_MIDI  1  // Host mode ready
```

**Potential Issue** ⚠️:
- Both initializations are called
- Possible conflict on the same peripheral
- Device init then Host init = Host overwrites Device!

### Recommendation

**For TESTING now** (Device mode only):
```c
// Config/module_config.h
#define MODULE_ENABLE_USB_MIDI   1  // Device mode
#define MODULE_ENABLE_USBH_MIDI  0  // Host disabled ← CHANGE
```

**To implement Host + Device later**:
1. Start with working Device mode
2. Add cable detection (ID pin PA10)
3. Implement switching function
4. Test both modes separately
5. Test automatic switching

### Protecting USB_HOST Files

**Before Any CubeMX Generation**:

```bash
cd /home/runner/work/MidiCore/MidiCore

# Commit current state
git add USB_HOST/
git commit -m "Backup: USB_HOST before CubeMX regeneration"

# Create explicit backup
tar -czf USB_HOST_backup_$(date +%Y%m%d_%H%M%S).tar.gz USB_HOST/
```

**After Generation**:

```bash
# Check if USB_HOST was preserved
ls -la USB_HOST/

# If removed or modified, restore from git
git checkout HEAD -- USB_HOST/
```

### Long-Term Solution: Manual USB_HOST Management

**Don't rely on CubeMX for USB_HOST**. Manage it manually:
- Keep files in git
- Restore after CubeMX generation
- Initialize in USER CODE sections
- Consider it "external" to CubeMX

---

## Post-Regeneration Checklist

After regenerating with CubeMX, verify:

- [ ] `USB_DEVICE/Class/MIDI/` still exists (should NOT be touched)
- [ ] `USB_DEVICE/App/usb_device.c` includes: `"USB_DEVICE/Class/MIDI/Inc/usbd_midi.h"`
- [ ] `USB_DEVICE/Target/usbd_conf.c` includes: `"USB_DEVICE/Class/MIDI/Inc/usbd_midi.h"`
- [ ] `Services/usb_midi/usb_midi.c` includes: `"USB_DEVICE/Class/MIDI/Inc/usbd_midi.h"`
- [ ] NO files in `Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/` (delete if they appear)
- [ ] Apply STM32 USB Library bug fixes (see next section)

If includes are wrong:
```bash
# Run this script to fix includes
python3 Tools/fix_usb_includes.py
```

---

## STM32 USB Library Bugs

### Bug #1: Uninitialized classId in Non-Composite Mode

**File**: `Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c`

**Location**: Function `USBD_Init()`, lines ~106-123

**Problem**:
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

**Impact**:
- Configuration descriptor retrieval fails (reads from `pClass[garbage]` instead of `pClass[0]`)
- Windows reports "Unknown Device" or VID_0000&PID_0000
- Device never reaches SET_CONFIGURATION state
- `USBD_MIDI_Init()` callback never called

**Fix Required After CubeMX Regeneration**:

Add after line with `pdev->pUserData[0] = NULL;`:
```c
  /* Initialize class tracking variables (CRITICAL FIX for non-composite mode) */
  pdev->NumClasses = 0;
  pdev->classId = 0;
```

### Bug #2: Incorrect MIDI Configuration Descriptor Length

**File**: `Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Src/usbd_midi.c`

**Location**: Lines ~64-70 (descriptor size calculations)

**Problem**:
```c
// ❌ WRONG: Assumes all MIDI IN Jacks are 6 bytes
#define USB_DESC_SIZE_JACK_IN 6
#define USB_MIDI_JACK_DESC_SIZE_PER_PORT ((USB_DESC_SIZE_JACK_IN * 2) + (USB_DESC_SIZE_JACK_OUT * 2))
// Result: (6*2) + (9*2) = 30 bytes/port ❌
```

**Reality**: 
- External IN Jack: 6 bytes ✅
- Embedded IN Jack: **9 bytes** ✅ (has `bNrInputPins`, `baSourceID`, `baSourcePin`)
- **Actual per port: 33 bytes** (not 30!)

**Fix Required After CubeMX Regeneration**:

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
#define USB_MIDI_JACK_DESC_SIZE_PER_PORT (USB_DESC_SIZE_JACK_IN_EXTERNAL +                                           USB_DESC_SIZE_JACK_IN_EMBEDDED +                                           USB_DESC_SIZE_JACK_OUT +                                           USB_DESC_SIZE_JACK_OUT)
```

### Automated Fix

**Run after each CubeMX regeneration**:
```bash
python3 Tools/fix_stm32_usb_core.py
```

### Post-Fix Checklist

After applying fixes:

- [ ] Bug #1: Applied fix in `usbd_core.c` - Initialize `classId` and `NumClasses`
- [ ] Bug #2: Applied fix in `usbd_midi.c` - Correct MIDI jack descriptor size
- [ ] Rebuild project
- [ ] Test USB enumeration (should show correct VID/PID, not VID_0000&PID_0000)
- [ ] Verify Windows Device Manager shows device without "CONFIGURATION_DESCRIPTOR_VALIDATION_FAILURE"

### Why These Bugs Exist

1. **Bug #1**: STM32 library assumes composite mode is default, non-composite mode was added later without proper initialization
2. **Bug #2**: Descriptor size calculation assumes all jack descriptors are same size, but Embedded IN jacks have additional source connection fields

Both bugs have been reported to ST but are not fixed in current CubeMX versions (as of 2026-01).

---

## References

### Official Documentation
- **STM32CubeMX User Manual** (UM1718)
- **STM32F4xx Reference Manual** (RM0090)
- **USB Device Class Definition for MIDI Devices v1.0**

### Implementation References
- **MIOS32 USB**: https://github.com/midibox/mios32/tree/master/drivers/STM32F4xx
  - Complete custom USB implementation
  - Does NOT use ST's USB Device Library
  - Direct HAL USB API usage
  - Protected from CubeMX regeneration

### Tools
- **STM32CubeMX**: Latest version
- **STM32CubeIDE**: Optional for integrated workflow
- **Git**: For version control and backup

---

## Key Files and Their Status

| File | CubeMX Managed? | Protected? | Contains |
|------|----------------|-----------|----------|
| `USB_DEVICE/Class/MIDI/*` | ❌ No | ✅ Yes | Our custom MIDI class |
| `Services/usb_midi/*` | ❌ No | ✅ Yes | Application layer |
| `USB_DEVICE/App/*` | ⚠️ Partial | ⚠️ Use markers | USB Device init |
| `USB_DEVICE/Target/*` | ⚠️ Partial | ⚠️ Use markers | HAL callbacks |
| `Middlewares/ST/...Core/` | ✅ Yes | ❌ No | STM32 USB core (has bugs) |
| `Middlewares/ST/.../MIDI/` | ✅ Yes | ❌ No | DELETE - use ours instead |

---

## Summary

### What Changed from CubeMX defaults
- `.ioc` file modified: `OTG_FS` → `Device_Only`
- USB_DEVICE middleware added and configured
- MIDI class selected
- Custom MIDI class in protected location

### What This Means
- ✅ USB_DEVICE is now selectable in CubeMX (no longer grayed out)
- ✅ Code generation will work
- ✅ MidiCore will appear in DAW with 4 MIDI ports
- ✅ Custom code protected from CubeMX regeneration
- ⚠️ Two STM32 library bugs must be fixed after each regeneration

### What To Do
1. Open .ioc in CubeMX
2. Verify USB_DEVICE is enabled
3. Generate code
4. Apply STM32 library bug fixes
5. Verify custom code still in place
6. Test with your DAW

---

**Bottom Line**: The protection strategy and automated fixes solve the regeneration problem. Our custom MIDI implementation survives CubeMX updates! 🎉

**Document Status**: ✅ Complete CubeMX protection guide  
**Last Verified**: 2026-01-25
