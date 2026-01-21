# USB Descriptor Fix - Removing IAD to Match MIOS32

## Discovery

After multiple unsuccessful attempts to fix Windows error 0xC00000E5 by adjusting descriptor sizes, a critical difference was discovered:

**MIOS32 uses a 211-byte descriptor (0xD3) WITHOUT an IAD (Interface Association Descriptor).**

## The Issue

The MidiCore implementation included an 8-byte IAD that was added to satisfy Windows Composite Device Driver requirements. However:

1. MIOS32 works WITHOUT an IAD on the same hardware
2. The IAD may be causing Windows descriptor validation to fail
3. Documentation references consistently mention 211 bytes (matching NO IAD)

## The Fix

**Commit a56d1f0**: Removed IAD from descriptor

### Changes
- Removed 8-byte IAD from descriptor array
- Updated size from 219 bytes to **211 bytes (0xD3)**
- Updated size calculation macros
- Kept correct sizes: AC Header = 9 bytes, MS Header = 7 bytes

### New Descriptor Structure

```
Configuration Descriptor:          9 bytes
Audio Control Interface:            9 bytes
CS AC Header:                       9 bytes (has bInCollection + baInterfaceNr)
MIDI Streaming Interface:           9 bytes
CS MS Header:                       7 bytes
MIDI Jacks (4 ports × 33):        132 bytes
Endpoints (2 × 18):                36 bytes
----------------------------------------
Total:                            211 bytes (0xD3)
```

### Comparison

| Version | Size | IAD | Status |
|---------|------|-----|--------|
| MIOS32 (working) | 211 bytes (0xD3) | NO | ✅ Works on hardware |
| Previous MidiCore | 219 bytes (0xDB) | YES | ❌ Error 0xC00000E5 |
| **New MidiCore** | **211 bytes (0xD3)** | **NO** | 🧪 Testing |

## Rationale

### Why IAD Was Added Initially
The IAD was added based on documentation suggesting it's "REQUIRED for Windows Composite Device Driver (usbccgp) validation."

### Why It May Be Wrong
1. **MIOS32 doesn't use it** and works successfully
2. Composite devices CAN work without IAD if the device class is properly set
3. The IAD may be triggering additional Windows validation that fails
4. USB MIDI 1.0 spec doesn't require IAD for simple audio devices

### Device Class Configuration
The device descriptor has:
```c
bDeviceClass = 0x00      // Composite
bDeviceSubClass = 0x00
bDeviceProtocol = 0x00
```

For composite devices with audio, Windows can enumerate without IAD if:
- Device class is 0x00 (which it is)
- Configuration has proper interface descriptors (which it does)
- Interfaces have correct class/subclass values (which they do)

## Testing

This change makes the MidiCore descriptor **identical in structure to MIOS32**, which is proven to work on the same STM32F407VGT6 hardware.

**Expected result**: Windows should accept the descriptor without error 0xC00000E5.

## References

- USB Audio Device Class Specification v1.0
- USB MIDI Device Class Specification v1.0
- MIOS32 Implementation: https://github.com/midibox/mios32/
- Documentation files: MIOS32_USB_IMPLEMENTATION_GUIDE.md, USB_MIDI_MS_HEADER_BUG_FIX.md

---

**Status**: Testing required on hardware
**Commit**: a56d1f0
**Date**: 2026-01-21
