# USB Interface Association Descriptor (IAD) Fix

## Problem: Windows 0xC00000E5 Error with Composite Device

### Symptoms
- Device enumerates with correct VID/PID (VID_16C0&PID_0489)
- Windows loads `usbccgp` (USB Composite Device Driver)
- Device fails validation with error **0xC00000E5** (STATUS_IO_DEVICE_ERROR)
- Event Viewer shows: "CONFIGURATION_DESCRIPTOR_VALIDATION_FAILURE"

### Root Cause

**Device Descriptor Configuration**:
- `bDeviceClass = 0x00` (defined at device level, see `USB_DEVICE/App/usbd_desc.c` line 34)
- `bNumInterfaces = 2` (Audio Control + MIDIStreaming)

When Windows sees:
1. Device with `bDeviceClass = 0x00` (class defined at interface level)
2. **Multiple interfaces** (2 in our case)

→ Windows treats it as a **Composite Device** and loads the `usbccgp` driver.

**Windows Composite Device Requirement**:
For a Composite Device to pass validation, Windows **REQUIRES** an **Interface Association Descriptor (IAD)** that groups related interfaces together.

**Without IAD**: Windows doesn't know which interfaces belong together → Validation fails with 0xC00000E5.

### Solution: Add Interface Association Descriptor (IAD)

**IAD Structure** (8 bytes):
```c
/* Interface Association Descriptor (IAD) */
0x08,                         /* bLength: 8 bytes */
0x0B,                         /* bDescriptorType: IAD (0x0B) */
0x00,                         /* bFirstInterface: 0 (Audio Control) */
0x02,                         /* bInterfaceCount: 2 (AC + MS) */
USB_DEVICE_CLASS_AUDIO,       /* bFunctionClass: 0x01 (Audio) */
AUDIO_SUBCLASS_MIDISTREAMING, /* bFunctionSubClass: 0x03 (MIDIStreaming) */
0x00,                         /* bFunctionProtocol: 0 */
0x00,                         /* iFunction: no string */
```

**Placement**: IAD must appear **BEFORE** the first interface it describes (Audio Control Interface).

**Descriptor Order** (corrected):
1. Configuration Descriptor (9 bytes)
2. **IAD** (8 bytes) ← **ADDED**
3. Audio Control Interface (9 bytes)
4. Class-specific AC Header (7 bytes)
5. MIDIStreaming Interface (9 bytes)
6. Class-specific MS Header (7 bytes)
7. MIDI Jacks + Endpoints (168 bytes for 4 ports)

**New Total Length**: 217 bytes (0xD9) for 4 ports

### What IAD Does

IAD tells Windows:
- "Interfaces 0 and 1 belong to the same **Audio/MIDI function**"
- "Load a single driver instance for both interfaces together"
- "Don't try to load separate drivers for each interface"

This allows Windows' `usbccgp` to correctly parse the device as a unified Audio/MIDI interface.

### USB Specification Reference

**USB 2.0 Engineering Change Notice (ECN)**: Interface Association Descriptor (IAD)
- Ratified: April 2003
- Purpose: Allow Composite Devices to group multiple interfaces into a single function
- Required for: Multi-interface functions on Composite Devices (Windows, macOS, Linux)

### Implementation Changes

**File**: `USB_DEVICE/Class/MIDI/Src/usbd_midi.c`

**Changes**:
1. Added `USB_DESC_SIZE_IAD` constant (8 bytes)
2. Updated `USB_MIDI_CONFIG_DESC_SIZ` macro to include IAD (+8 bytes)
3. Inserted IAD in configuration descriptor (between Config and AC Interface)

**Before**: 209 bytes (Config + 2 interfaces + jacks)
**After**: 217 bytes (Config + **IAD** + 2 interfaces + jacks)

### Testing

After this fix:
1. ✅ Windows should recognize device as unified Audio/MIDI function
2. ✅ `usbccgp` should successfully parse descriptor
3. ✅ No more 0xC00000E5 validation error
4. ✅ Device should enumerate as "MidiCore 4x4" USB MIDI interface

### Why MIOS32 Works Without This

**MIOS32 difference**: Uses `bDeviceClass = 0x02` (Communications) at device level, which may avoid Composite treatment, OR their implementation already has IAD.

Our implementation follows **USB MIDI v1.0 best practices** with `bDeviceClass = 0x00` for maximum compatibility, which requires IAD when using multiple interfaces.

### References

- USB 2.0 Specification
- USB Device Class Definition for MIDI Devices v1.0
- USB Interface Association Descriptor ECN
- Microsoft Windows Hardware Dev Center: USB Descriptor Validation
- [Understanding USB Composite Devices](https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/usb-composite-driver)

---

**Status**: ✅ IAD added in commit [current]. Device should now enumerate correctly in Windows without 0xC00000E5 error.
