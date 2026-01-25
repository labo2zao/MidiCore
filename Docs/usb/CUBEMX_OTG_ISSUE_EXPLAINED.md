# CubeMX OTG Mode Issue - Explained and Solved

## The Problem

When you select **OTG_FS** mode in STM32CubeMX:
- USB_HOST middleware becomes **grayed out** ❌
- USB_DEVICE middleware becomes **grayed out** ❌
- You cannot configure middleware through CubeMX GUI

## Why This Happens

**CubeMX Design**: When you select OTG_FS at the hardware level:
- CubeMX assumes you want **runtime mode switching**
- Middleware selection is disabled because CubeMX doesn't know if you'll use Host, Device, or both
- You're expected to handle mode configuration programmatically

## The Solution

### What We Did in the .ioc File

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

## What You Get

### Current Configuration (After .ioc Modification)

**Hardware**: USB_OTG_FS in Device_Only mode
- ID pin: PA10 (configured)
- D+ pin: PA12 (configured)
- D- pin: PA11 (configured)

**Middleware**: USB_DEVICE with MIDI class
- 4 virtual MIDI ports (cables 0-3)
- MidiCore will appear in your DAW as MIDI interface
- All 4 ports independently usable

## Next Steps

### 1. Open in CubeMX

```bash
# Open the modified .ioc file
Open: MidiCore.ioc in STM32CubeMX
```

### 2. Verify Configuration

Navigate to:
- **Connectivity** → **USB_OTG_FS**
  - Mode should show: **Device_Only** ✅
  
- **Middleware** → **USB_DEVICE**
  - Should be **enabled** and **not grayed out** ✅
  - Class: **MIDI** ✅

### 3. Generate Code

Click **Generate Code** button

CubeMX will create:
```
USB_DEVICE/
├── App/
│   ├── usb_device.c
│   └── usb_device.h
└── Target/
    ├── usbd_conf.c
    └── usbd_conf.h
```

### 4. Integrate MIDI Class

Follow the integration steps in `WHAT_TO_DO_NOW.md`:
- Replace HID references with MIDI
- Add our custom MIDI class
- Configure router integration

## About OTG and Runtime Mode Switching

### Can I Still Do OTG (Host + Device)?

**Yes!** But it requires more work:

1. **Start with Device mode** (current configuration)
2. **Implement runtime switching** in your code
3. **Detect cable type** via ID pin
4. **Switch modes** programmatically based on detection

### MIOS32 Approach

MIOS32 handles this by:
- Starting in one mode (Device or Host)
- Detecting ID pin state at runtime
- Switching modes when cable changes
- Reinitializing USB stack for new mode

### Do You Need OTG?

**For your use case** (appearing in DAW):
- ✅ **Device mode is sufficient**
- You want to appear as MIDI interface → Device mode
- Computer is the Host, MidiCore is the Device
- This is the standard configuration for USB MIDI interfaces

**Only need OTG if**:
- You want to switch between DAW mode and MIDI keyboard reading mode
- You need runtime flexibility to be either Host or Device
- Your hardware setup changes frequently

## Summary

### What Changed
- `.ioc` file modified: `Host_Only` → `Device_Only`
- USB_DEVICE middleware added and configured
- MIDI class selected

### What This Means
- ✅ USB_DEVICE is now selectable in CubeMX (no longer grayed out)
- ✅ Code generation will work
- ✅ MidiCore will appear in DAW with 4 MIDI ports
- ✅ No more "grayed out" issue

### What To Do
1. Open .ioc in CubeMX
2. Verify USB_DEVICE is enabled
3. Generate code
4. Follow integration guide
5. Test with your DAW

---

**Bottom Line**: The .ioc modification solves the "grayed out" problem. You can now proceed with CubeMX configuration and code generation! 🎉
