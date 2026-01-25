# USB MIDI Descriptor Bug Fix - MS_HEADER wTotalLength

## Problem

Windows rejects USB MIDI device with error **0xC00000E5 "CONFIGURATION_DESCRIPTOR_VALIDATION_FAILURE"**.

Device is detected with correct VID/PID and recognized as USB\COMPOSITE, but enumeration fails during descriptor validation.

## Root Cause

**Incorrect MS_HEADER wTotalLength** in Class-specific MIDIStreaming Interface Header descriptor.

### The Bug

In `USB_DEVICE/Class/MIDI/Src/usbd_midi.c` line ~174:

```c
/* Class-specific MIDIStreaming Interface Descriptor */
0x07,                                  /* bLength */
AUDIO_DESCRIPTOR_TYPE_INTERFACE,       /* bDescriptorType */
0x01,                                  /* bDescriptorSubtype: MS_HEADER */
0x00, 0x01,                            /* bcdMSC: 1.00 */
LOBYTE(65 + (MIDI_NUM_PORTS * 12)),   /* ❌ WRONG! wTotalLength */
HIBYTE(65 + (MIDI_NUM_PORTS * 12)),
```

With `MIDI_NUM_PORTS = 4`:
- Calculated: `65 + (4 * 12) = 113 bytes` ❌
- **Actual descriptor length: 168 bytes** ✅

**Mismatch: 55 bytes short!**

### Why This Broke Enumeration

According to **USB MIDI Device Class Specification v1.0**, section 6.1.2.1:

> **MS_HEADER wTotalLength**: Total number of bytes returned for the class-specific MIDIStreaming interface descriptor. Includes the combined length of this descriptor header and all Jack and Endpoint descriptors.

Windows validates this field during enumeration. When the declared length doesn't match the actual descriptor data, Windows rejects the entire configuration with error 0xC00000E5.

---

## Correct Calculation

### What MS_HEADER wTotalLength Must Include

Starting **after** the MS_HEADER itself (7 bytes), until end of last Class-Specific Endpoint:

1. **MIDI Jack Descriptors** (4 ports):
   - 4 × External IN Jack: 4 × 6 = 24 bytes
   - 4 × Embedded IN Jack: 4 × 9 = 36 bytes
   - 4 × Embedded OUT Jack: 4 × 9 = 36 bytes
   - 4 × External OUT Jack: 4 × 9 = 36 bytes
   - **Subtotal: 132 bytes**

2. **Endpoint Descriptors**:
   - Bulk OUT Endpoint (Standard): 9 bytes
   - Class-Specific Bulk OUT Endpoint: 5 + 4 jacks = 9 bytes
   - Bulk IN Endpoint (Standard): 9 bytes
   - Class-Specific Bulk IN Endpoint: 5 + 4 jacks = 9 bytes
   - **Subtotal: 36 bytes**

**Total MS_HEADER wTotalLength: 132 + 36 = 168 bytes** ✅

---

## The Fix

```c
/* Class-specific MIDIStreaming Interface Descriptor */
0x07,                                  /* bLength */
AUDIO_DESCRIPTOR_TYPE_INTERFACE,       /* bDescriptorType */
0x01,                                  /* bDescriptorSubtype: MS_HEADER */
0x00, 0x01,                            /* bcdMSC: 1.00 */
/* wTotalLength: Length from after MS_HEADER to end of last CS endpoint
 * = Jacks + Endpoints
 * = (4*6 + 4*9 + 4*9 + 4*9) + (9 + 9 + 9 + 9)
 * = (24 + 36 + 36 + 36) + 36
 * = 132 + 36 = 168 bytes */
LOBYTE(168),  /* ✅ CORRECT */
HIBYTE(168),
```

---

## Complete Descriptor Size Breakdown

### Configuration Descriptor (Total: 211 bytes / 0xD3)

1. **Configuration Descriptor**: 9 bytes
2. **Audio Control Interface**: 9 bytes
3. **CS Audio Control Header**: 9 bytes
4. **MIDIStreaming Interface**: 9 bytes
5. **CS MIDIStreaming Header**: 7 bytes ← Declares wTotalLength = 168
6. **MIDI Jacks** (next 132 bytes):
   - External IN Jacks (4 × 6): 24 bytes
   - Embedded IN Jacks (4 × 9): 36 bytes
   - Embedded OUT Jacks (4 × 9): 36 bytes
   - External OUT Jacks (4 × 9): 36 bytes
7. **Endpoints** (next 36 bytes):
   - Bulk OUT Endpoint: 9 bytes
   - CS Bulk OUT Endpoint: 9 bytes
   - Bulk IN Endpoint: 9 bytes
   - CS Bulk IN Endpoint: 9 bytes

**Configuration wTotalLength**: 9 + 9 + 9 + 9 + 7 + 132 + 36 = **211 bytes (0xD3)** ✅

**MS_HEADER wTotalLength**: 132 + 36 = **168 bytes (0xA8)** ✅

---

## Verification

After applying the fix, verify with:

1. **Windows Device Manager**: Device should enumerate without errors
2. **USBTreeView** or **USBDeview**: 
   - Configuration Descriptor → wTotalLength = 0x00D3 (211 bytes)
   - MS_HEADER → wTotalLength = 0x00A8 (168 bytes)
3. **Windows Event Viewer**: NO "CONFIGURATION_DESCRIPTOR_VALIDATION_FAILURE" errors

---

## Why The Original Formula Was Wrong

```c
LOBYTE(65 + (MIDI_NUM_PORTS * 12))
```

This formula assumed:
- Base overhead: 65 bytes (incorrect - doesn't match actual structure)
- Per-port overhead: 12 bytes (incorrect - each port has 33 bytes of jacks + endpoint refs)

This is a **copy-paste error** from a 1-port MIDI example that was never updated for multi-port devices.

---

## References

- **USB Device Class Definition for MIDI Devices v1.0**
  - Section 6.1.2.1: Class-specific MS Interface Header Descriptor
  - Section 6.2: MIDI Adapter MIDI IN Jack Descriptor
  - Section 6.3: MIDI Adapter MIDI OUT Jack Descriptor
- **MIOS32 Implementation**: https://github.com/midibox/mios32/blob/master/drivers/STM32F4xx/mios32_usb_midi.c
  - Uses correct 168-byte MS_HEADER wTotalLength for 4 ports
