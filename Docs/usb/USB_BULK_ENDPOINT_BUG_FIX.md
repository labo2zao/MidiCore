# CRITICAL FIX: Invalid Bulk Endpoint Descriptors

## The Real Problem

**Windows error 0xC00000E5 was caused by invalid Bulk endpoint descriptors!**

### What Was Wrong

The Standard Bulk Endpoint descriptors were **9 bytes instead of 7 bytes**.

They incorrectly included:
- `bRefresh` (byte 8)
- `bSynchAddress` (byte 9)

These fields are **ONLY for Isochronous and Interrupt endpoints**, NOT Bulk endpoints!

### USB 2.0 Specification

**Section 9.6.6: Endpoint Descriptor**

For **Bulk endpoints**, the descriptor is **7 bytes**:
1. bLength (1 byte) = 0x07
2. bDescriptorType (1 byte) = 0x05
3. bEndpointAddress (1 byte)
4. bmAttributes (1 byte) = 0x02 (Bulk)
5. wMaxPacketSize (2 bytes)
6. bInterval (1 byte)

**TOTAL: 7 bytes**

For **Isochronous/Interrupt** endpoints, additional fields exist:
7. bRefresh (1 byte) - **NOT for Bulk!**
8. bSynchAddress (1 byte) - **NOT for Bulk!**

**TOTAL for Isoch/Interrupt: 9 bytes**

### The Bug

**File**: `USB_DEVICE/Class/MIDI/Src/usbd_midi.c`

**Lines 372-381 (OUT Endpoint) - BEFORE:**
```c
/* Standard Bulk OUT Endpoint Descriptor */
0x09,                                  /* bLength ❌ WRONG! */
USB_DESC_TYPE_ENDPOINT,
MIDI_OUT_EP,
0x02,                                  /* bmAttributes: Bulk */
LOBYTE(MIDI_DATA_FS_MAX_PACKET_SIZE),
HIBYTE(MIDI_DATA_FS_MAX_PACKET_SIZE),
0x00,                                  /* bInterval */
0x00,                                  /* bRefresh ❌ INVALID FOR BULK! */
0x00,                                  /* bSynchAddress ❌ INVALID FOR BULK! */
```

**Lines 393-402 (IN Endpoint) - BEFORE:**
```c
/* Standard Bulk IN Endpoint Descriptor */
0x09,                                  /* bLength ❌ WRONG! */
USB_DESC_TYPE_ENDPOINT,
MIDI_IN_EP,
0x02,                                  /* bmAttributes: Bulk */
LOBYTE(MIDI_DATA_FS_MAX_PACKET_SIZE),
HIBYTE(MIDI_DATA_FS_MAX_PACKET_SIZE),
0x00,                                  /* bInterval */
0x00,                                  /* bRefresh ❌ INVALID FOR BULK! */
0x00,                                  /* bSynchAddress ❌ INVALID FOR BULK! */
```

### The Fix

**AFTER (CORRECT):**
```c
/* Standard Bulk OUT Endpoint Descriptor */
0x07,                                  /* bLength: 7 bytes for Bulk ✅ */
USB_DESC_TYPE_ENDPOINT,
MIDI_OUT_EP,
0x02,                                  /* bmAttributes: Bulk */
LOBYTE(MIDI_DATA_FS_MAX_PACKET_SIZE),
HIBYTE(MIDI_DATA_FS_MAX_PACKET_SIZE),
0x00,                                  /* bInterval */
/* bRefresh and bSynchAddress removed - not valid for Bulk! */
```

**Same fix applied to IN endpoint.**

### Impact on Descriptor Sizes

The 4 extra invalid bytes (2 per endpoint) changed all calculations:

**BEFORE (WRONG):**
- Endpoints: 36 bytes (2 × 9-byte std + 2 × 9-byte CS)
- MS_HEADER wTotalLength: 168 bytes
- Configuration wTotalLength: 219 bytes (0xDB)

**AFTER (CORRECT):**
- Endpoints: 32 bytes (2 × **7-byte** std + 2 × 9-byte CS)
- MS_HEADER wTotalLength: **164 bytes** (0xA4)
- Configuration wTotalLength: **215 bytes** (0xD7)

### Why Windows Rejected It

Windows parses the descriptor byte-by-byte according to the USB spec:

1. Reads `bLength = 0x09` for Bulk endpoint
2. Expects 9 bytes, but Bulk should only be 7
3. Reads 2 extra bytes (`bRefresh`, `bSynchAddress`)
4. Now out of sync with the rest of the descriptor
5. Fails validation → **Error 0xC00000E5** (CONFIGURATION_DESCRIPTOR_VALIDATION_FAILURE)

### Previous Fix Was Incomplete

**Commit a85b221** fixed the AC Header size (9 bytes not 7), bringing the size from 217 to 219 bytes.

But it **missed** the Bulk endpoint error, leaving 4 invalid bytes that Windows rejected.

### Validation

After this fix:
```
✅ Per-port jack size:  33 bytes
✅ MS_HEADER wTotalLength: 164 bytes
✅ Config wTotalLength:    215 bytes
✅ Bulk endpoints: 7 bytes each (per USB 2.0 spec)
```

### References

- **USB 2.0 Specification**, Section 9.6.6: Endpoint Descriptor
- **USB Device Class Definition for MIDI Devices v1.0**
- **MIOS32 Implementation**: Uses 7-byte Bulk endpoint descriptors (correct)

---

**Status**: ✅ FIXED in commit 148eddc
**Expected Result**: Windows should now enumerate device without error 0xC00000E5
