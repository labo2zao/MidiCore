# USB MIDI Descriptor Bug Fix - Error 0xC00000E5

## Problem
Windows reports error **0xC00000E5** (CONFIGURATION_DESCRIPTOR_VALIDATION_FAILURE):
```
L'appareil USB\VID_16C0&PID_0489\... a eu un problème de démarrage.
Problème : 0x0
État du problème : 0xC00000E5
```

## Root Cause
**2-byte shortfall in Configuration Descriptor wTotalLength**

The descriptor calculation was using `USB_DESC_SIZE_CS_INTERFACE = 7` for both:
1. **CS AC Header** - Actually **9 bytes** (has `bInCollection` + `baInterfaceNr` fields)
2. **CS MS Header** - Correctly **7 bytes** (just header fields)

This caused:
- Declared size: 217 bytes (0xD9)
- Actual size: 219 bytes (0xDB)
- **Mismatch: 2 bytes short**

Windows validates this field during USB enumeration. When the declared length doesn't match actual data, it rejects the descriptor.

## Solution
Split the descriptor size constant into two separate defines:
- `USB_DESC_SIZE_CS_AC_INTERFACE = 9` (for AC Header with collection info)
- `USB_DESC_SIZE_CS_MS_INTERFACE = 7` (for MS Header)

### File Changed
`USB_DEVICE/Class/MIDI/Src/usbd_midi.c`

**Before (WRONG):**
```c
#define USB_DESC_SIZE_CS_INTERFACE       7      /* Class-specific Interface Header */

#define USB_MIDI_CONFIG_DESC_SIZ         (USB_DESC_SIZE_CONFIGURATION + \
                                          USB_DESC_SIZE_IAD + \
                                          USB_DESC_SIZE_INTERFACE + \
                                          USB_DESC_SIZE_CS_INTERFACE + \  // ❌ 7 bytes
                                          USB_DESC_SIZE_INTERFACE + \
                                          USB_DESC_SIZE_CS_INTERFACE + \  // ❌ 7 bytes
                                          USB_MIDI_MS_TOTAL_LENGTH)
// Result: 217 bytes (2 bytes short!) ❌
```

**After (CORRECT):**
```c
#define USB_DESC_SIZE_CS_AC_INTERFACE    9      /* CS AC Interface Header (has bInCollection) */
#define USB_DESC_SIZE_CS_MS_INTERFACE    7      /* CS MS Interface Header */

#define USB_MIDI_CONFIG_DESC_SIZ         (USB_DESC_SIZE_CONFIGURATION + \
                                          USB_DESC_SIZE_IAD + \
                                          USB_DESC_SIZE_INTERFACE + \
                                          USB_DESC_SIZE_CS_AC_INTERFACE + \  // ✅ 9 bytes
                                          USB_DESC_SIZE_INTERFACE + \
                                          USB_DESC_SIZE_CS_MS_INTERFACE + \  // ✅ 7 bytes
                                          USB_MIDI_MS_TOTAL_LENGTH)
// Result: 219 bytes (CORRECT!) ✅
```

## Descriptor Breakdown (4-port MIDI interface)

### Configuration Descriptor (Total: 219 bytes / 0xDB)
```
9   bytes - Configuration Descriptor
8   bytes - Interface Association Descriptor (IAD)
9   bytes - Audio Control Interface
9   bytes - CS AC Header (has bInCollection + baInterfaceNr)  ← Was counted as 7!
9   bytes - MIDI Streaming Interface
7   bytes - CS MS Header
168 bytes - MIDI Jacks + Endpoints
------
219 bytes (0xDB) ✅
```

### MS_HEADER wTotalLength (168 bytes / 0xA8)
```
132 bytes - MIDI Jacks (4 ports × 33 bytes)
  24 bytes - External IN Jacks (4 × 6)
  36 bytes - Embedded IN Jacks (4 × 9)
  36 bytes - Embedded OUT Jacks (4 × 9)
  36 bytes - External OUT Jacks (4 × 9)
  
36 bytes - Endpoints
  9 bytes - Bulk OUT Endpoint (standard)
  9 bytes - CS Bulk OUT Endpoint
  9 bytes - Bulk IN Endpoint (standard)
  9 bytes - CS Bulk IN Endpoint
------
168 bytes (0xA8) ✅
```

## Why This Matches MIOS32
MIOS32 uses the correct 9-byte size for the AC Header in their USB implementation. Our hardware works with MIOS32 because they have the correct descriptor sizes.

## Validation
Run the validation tool to verify:
```bash
gcc Tools/validate_usb_descriptors.c -o validate_usb_descriptors
./validate_usb_descriptors
```

Expected output:
```
✅ Per-port jack size:  33 bytes (CORRECT)
✅ MS_HEADER wTotalLength: 168 bytes (CORRECT)
✅ Config wTotalLength:    219 bytes (CORRECT)

🎉 All descriptor sizes are CORRECT!
   This should fix Windows error 0xC00000E5
```

## Testing
After applying this fix:
1. ✅ Windows should enumerate device without error 0xC00000E5
2. ✅ Device Manager shows correct VID/PID (VID_16C0&PID_0489)
3. ✅ Device appears under "Audio, Video and Game Controllers"
4. ✅ No more "CONFIGURATION_DESCRIPTOR_VALIDATION_FAILURE"

## References
- **USB Device Class Definition for MIDI Devices v1.0**
  - Section 4.1: Standard Audio Control Interface Descriptor
  - Section 6.1.2.1: Class-specific MS Interface Header Descriptor
- **USB Audio Device Class Specification v1.0**
  - Section 4.3.1: Class-specific AC Interface Header Descriptor (9 bytes with bInCollection)
- **MIOS32 Reference**: https://github.com/midibox/mios32/

## Commit
- **Fix commit**: a85b221
- **Date**: 2026-01-21
- **Issue**: Windows error 0xC00000E5 (CONFIGURATION_DESCRIPTOR_VALIDATION_FAILURE)
- **Solution**: Split USB_DESC_SIZE_CS_INTERFACE into AC (9 bytes) and MS (7 bytes) variants
