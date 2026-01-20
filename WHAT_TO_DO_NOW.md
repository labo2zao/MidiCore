# WHAT TO DO NOW - Clear Action Plan

## Current Status

✅ **COMPLETED** (by Copilot):
- Created USB MIDI Device class with 4-port support
- Updated router configuration for 4 USB ports (8 total MIDI ports)
- Implemented Services/usb_midi with 4-port routing
- Improved Services/usb_host_midi with router integration
- Added MIDI constants and improved code quality

❌ **NOT DONE** (requires YOU to do):
- CubeMX configuration needs regeneration
- Build configuration needs include paths
- Testing on actual hardware

## Problem You're Facing Now

**Build Error**: 
```
fatal error: usbd_midi.h: No such file or directory
```

**Root Cause**: STM32CubeIDE doesn't know where to find the new MIDI class files.

---

# STEP-BY-STEP: What YOU Need to Do NOW

## Option A: Quick Fix (Add Include Paths Manually)

### Step 1: Add Include Path in STM32CubeIDE

1. **Right-click** on your project in Project Explorer
2. **Properties** → **C/C++ Build** → **Settings**
3. **Tool Settings** tab → **MCU GCC Compiler** → **Include paths**
4. **Click** the **Add** button (green +)
5. **Add** these paths ONE BY ONE:

```
../Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Inc
../Services
../Config
```

6. **Click** Apply and Close
7. **Clean** project: Project → Clean
8. **Build**: Project → Build All

### Step 2: Test Compilation

Build should now work. If it compiles successfully, you're done with the quick fix!

---

## Option B: Complete Setup (Recommended for Full USB Device Support)

This requires CubeMX and takes longer but gives you full USB Device functionality.

### Prerequisites

You need:
- ✅ STM32CubeMX installed
- ✅ MidiCore.ioc file (you have it)
- ✅ USB cable to connect to computer

### Step 1: Open CubeMX and Configure USB

1. **Open** `MidiCore.ioc` in **STM32CubeMX**

2. **Change USB Mode**:
   - Left panel → Connectivity → **USB_OTG_FS**
   - Mode: Change from **"Host_Only"** to **"OTG_FS"**
   - ✅ Enable VBUS sensing
   - ✅ ID pin: PA10 (already set)

3. **Add USB_DEVICE**:
   - Left panel → Middleware → **USB_DEVICE**
   - Check **✅ Enable**
   - Class for FS IP: Select **"Custom HID"** (we'll change this after)

4. **Generate Code**:
   - Click **"GENERATE CODE"** button (top right)
   - Click **"Open Folder"** when done

### Step 2: Replace HID with MIDI Class

After CubeMX generation:

1. **Edit**: `USB_DEVICE/App/usb_device.c`

Find this line (around line 23):
```c
#include "usbd_hid.h"
```
**Replace with**:
```c
#include "usbd_midi.h"
```

Find this line (around line 75):
```c
if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_HID) != USBD_OK)
```
**Replace with**:
```c
if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_MIDI) != USBD_OK)
```

2. **Edit**: `USB_DEVICE/Target/usbd_conf.c`

Add at the top (after other includes):
```c
#include "usbd_midi.h"
```

Find this line (search for "USBD_static_malloc"):
```c
static uint32_t mem[(sizeof(USBD_HID_HandleTypeDef) / 4) + 1];
```
**Replace with**:
```c
static uint32_t mem[(sizeof(USBD_MIDI_HandleTypeDef) / 4) + 1];
```

### Step 3: Add Include Paths (Same as Option A Step 1)

Follow Step 1 from Option A above.

### Step 4: Enable USB MIDI in Configuration

**Edit**: `Config/module_config.h`

Find these lines (around line 71-77):
```c
#ifndef MODULE_ENABLE_USB_MIDI
#define MODULE_ENABLE_USB_MIDI 0  // Change this to 1
#endif

#ifndef MODULE_ENABLE_USBH_MIDI
#define MODULE_ENABLE_USBH_MIDI 0  // Change this to 1 if you want Host mode
#endif
```

**Change to**:
```c
#ifndef MODULE_ENABLE_USB_MIDI
#define MODULE_ENABLE_USB_MIDI 1  // ✅ ENABLED
#endif

#ifndef MODULE_ENABLE_USBH_MIDI
#define MODULE_ENABLE_USBH_MIDI 1  // ✅ ENABLED (for OTG Host mode)
#endif
```

### Step 5: Initialize in main.c

**Edit**: `Core/Src/main.c`

Find `/* USER CODE BEGIN 2 */` (around line 153):

Add BEFORE the line `/* USER CODE END 2 */`:
```c
/* USER CODE BEGIN 2 */

#if MODULE_ENABLE_USB_MIDI
  usb_midi_init();
#endif

#if MODULE_ENABLE_USBH_MIDI
  usb_host_midi_init();
#endif

/* USER CODE END 2 */
```

### Step 6: Clean and Build

1. **Clean**: Project → Clean → Select your project → Clean
2. **Build**: Project → Build All (or Ctrl+B)
3. **Check**: Should compile without errors

---

## What You Get After This

### 8 MIDI Ports Total (Like MIOS32)

**Via USB (1 physical cable, 4 virtual ports)**:
- USB Port 1 (Cable 0)
- USB Port 2 (Cable 1)
- USB Port 3 (Cable 2)
- USB Port 4 (Cable 3)

**Via DIN MIDI (4 physical cables)**:
- DIN MIDI 1 (USART1)
- DIN MIDI 2 (USART2)
- DIN MIDI 3 (USART3)
- DIN MIDI 4 (UART5)

### Automatic USB Mode Switching

**Standard micro-USB cable** → **Device Mode**
- MidiCore appears in your DAW with 4 MIDI ports
- Works like any USB MIDI interface

**Micro-USB OTG adapter + MIDI keyboard** → **Host Mode**
- MidiCore reads USB MIDI keyboards
- Must power via separate USB Debug socket!

---

## Testing

### Test 1: Compilation

```bash
# Should build without errors
Project → Build All
```

Expected: **Build Finished. 0 errors.**

### Test 2: USB Device Mode (in DAW)

1. Flash firmware to board
2. Connect standard micro-USB cable to computer
3. Check Windows Device Manager (or Mac Audio MIDI Setup)
4. Should see **"MidiCore"** or **"STM32 Audio Device"**
5. Open your DAW (Ableton, FL Studio, etc.)
6. Should see **4 MIDI input ports** and **4 MIDI output ports**

### Test 3: USB Host Mode (read MIDI keyboard)

1. Disconnect from computer
2. Connect micro-USB OTG adapter to MidiCore
3. **Important**: Power MidiCore via USB Debug socket
4. Connect USB MIDI keyboard to OTG adapter
5. Play notes → Should see MIDI activity

---

## Summary: Your Action Checklist

### Minimum (Just Fix Build Error)

- [ ] Add include paths in STM32CubeIDE (Option A, Step 1)
- [ ] Clean and build project
- [ ] Verify compilation succeeds

### Complete (Full USB MIDI Support)

- [ ] Open MidiCore.ioc in CubeMX
- [ ] Change USB_OTG_FS mode to "OTG_FS"
- [ ] Enable USB_DEVICE middleware
- [ ] Generate code
- [ ] Edit usb_device.c (replace HID with MIDI)
- [ ] Edit usbd_conf.c (replace HID with MIDI)
- [ ] Add include paths
- [ ] Enable MODULE_ENABLE_USB_MIDI in module_config.h
- [ ] Add initialization in main.c
- [ ] Clean and build
- [ ] Flash and test

---

## Need Help?

### If Build Still Fails

Check:
1. Include paths added correctly? (check paths tab in build settings)
2. Files exist? (check Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/)
3. Clean project before building?

### If USB Not Working

Check:
1. CubeMX configuration: USB_OTG_FS mode = "OTG_FS"?
2. USB_DEVICE middleware enabled?
3. MODULE_ENABLE_USB_MIDI = 1?
4. usb_midi_init() called in main.c?

### If Device Not Recognized

Check:
1. USB cable is data cable (not charge-only)
2. Firmware flashed successfully?
3. Using correct USB port (OTG FS, not Debug)?

---

## Files Reference

**You MUST edit** (after CubeMX generation):
- `USB_DEVICE/App/usb_device.c`
- `USB_DEVICE/Target/usbd_conf.c`
- `Config/module_config.h`
- `Core/Src/main.c`

**Already done** (by Copilot):
- `Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/` (USB MIDI class)
- `Services/usb_midi/` (4-port routing)
- `Services/usb_host_midi/` (router integration)
- `Config/router_config.h` (4 USB ports + 4 DIN ports)

---

## TLDR - Fastest Path

1. **Add include path**: Right-click project → Properties → C/C++ Build → Settings → Include paths → Add: `../Middlewares/ST/STM32_USB_Device_Library/Class/MIDI/Inc`
2. **Clean and Build**
3. **Done** (for compilation)

For full USB MIDI: Follow "Option B" steps above.
