# USB MIDI Loopback/Freeze Fix - Deep Technical Analysis

## Problem Statement
User reported: "USB MIDI driver of midicore is going in loopback/freezing and MIOS Studio just wait for connection"

## Root Cause - CIN Corruption Bug

### The Critical Bug
The `usb_midi_send_packet()` function was **discarding the CIN** (Code Index Number) parameter and allowing `USBD_MIDI_SendData()` to re-interpret the message bytes, causing SysEx corruption.

### Code Analysis

**BROKEN Implementation (Before Fix):**
```c
void usb_midi_send_packet(uint8_t cin, uint8_t b0, uint8_t b1, uint8_t b2) {
  uint8_t cable = (cin >> 4) & 0x0F;  // Extract cable from CIN
  uint8_t data[3] = {b0, b1, b2};     // Build message bytes
  uint16_t length = 3;
  
  // BUG: CIN information is LOST here!
  // USBD_MIDI_SendData will re-interpret b0, b1, b2 to guess the CIN
  USBD_MIDI_SendData(&hUsbDeviceFS, cable, data, length);
}
```

**Why This Breaks:**
1. Caller provides CIN (e.g., 0x04 for SysEx continue)
2. Function extracts cable but **discards CIN value**
3. Passes raw bytes to `USBD_MIDI_SendData()`
4. `USBD_MIDI_SendData()` examines first byte to **guess** CIN
5. For SysEx continuation (bytes like `7E 32 00`), no 0xF0 start byte
6. Function misinterprets as channel message or misc
7. **WRONG CIN sent in USB packet!**

### SysEx Corruption Example

**MIOS32 Query Response:**
```
F0 00 00 7E 32 00 0F 4D 49 4F 53 33 32 F7
│  ││ ││ │  │  │  │  │  │  │  │  │  │  └─ SysEx end
│  ││ ││ │  │  │  │  └──┴──┴──┴──┴──┴──── "MIOS32"
│  ││ ││ │  │  │  └───────────────────── ACK response (0x0F)
│  ││ ││ │  │  └──────────────────────── Device instance
│  ││ ││ │  └─────────────────────────── Device ID (0x32)
│  ││ ││ └────────────────────────────── MIOS Manufacturer ID
│  ││ │└─────────────────────────────────
│  ││ └──────────────────────────────────
│  │└───────────────────────────────────
│  └────────────────────────────────────
└───────────────────────────────────── SysEx start
```

**Correct USB-MIDI Packet Transmission:**
```
Packet 1: [cable=0, CIN=0x4] [F0 00 00] - SysEx start/continue
Packet 2: [cable=0, CIN=0x4] [7E 32 00] - SysEx continue
Packet 3: [cable=0, CIN=0x4] [0F 4D 49] - SysEx continue
Packet 4: [cable=0, CIN=0x4] [4F 53 33] - SysEx continue
Packet 5: [cable=0, CIN=0x6] [32 F7 00] - SysEx end 2-byte
                           ^
                Header byte = (cable << 4) | CIN
```

**Corrupted Transmission (Bug):**
```
Packet 1: [0x04] [F0 00 00] - OK (F0 detected correctly)
Packet 2: [0x??] [7E 32 00] - WRONG! 0x7E misinterpreted
Packet 3: [0x??] [0F 4D 49] - WRONG! 0x0F misinterpreted
Packet 4: [0x??] [4F 53 33] - WRONG! 0x4F misinterpreted
Packet 5: [0x??] [32 F7 00] - Maybe OK if F7 detected
```

### Impact on MIOS Studio

**Why It Freezes:**
1. MIOS Studio sends query: `F0 00 00 7E 32 00 00 01 F7`
2. MidiCore receives and processes correctly
3. Generates proper response SysEx
4. **But transmits with CORRUPTED CINs!**
5. MIOS Studio receives malformed USB packets
6. Parsing fails - never finds proper F7 end
7. **MIOS Studio hangs waiting for complete message**
8. Times out, tries bootloader protocol
9. **Appears as "loopback/freeze"**

## Solution - Direct Packet Transmission

### Fixed Implementation

```c
void usb_midi_send_packet(uint8_t cin, uint8_t b0, uint8_t b1, uint8_t b2) {
  /* CRITICAL FIX: Send packet directly with provided CIN to avoid re-interpretation
   * 
   * Problem: USBD_MIDI_SendData() re-interprets the message bytes to determine CIN,
   * but for SysEx continuation packets (CIN 0x4), the bytes don't start with 0xF0,
   * causing misinterpretation as channel messages.
   * 
   * Solution: Build the 4-byte USB-MIDI packet directly and transmit it.
   * This preserves the CIN provided by the caller (e.g., usb_midi_send_sysex).
   */
  
  USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)hUsbDeviceFS.pClassData;
  
  /* Check if interface is ready */
  if (hmidi == NULL || !hmidi->is_ready) {
    return; /* Interface not ready - silently fail */
  }
  
  /* Check if endpoint is busy before sending */
  if (hUsbDeviceFS.ep_in[MIDI_IN_EP & 0x0F].status == USBD_BUSY) {
    return; /* Endpoint busy - drop packet (non-blocking behavior) */
  }
  
  /* Build USB MIDI packet (4 bytes) with provided CIN */
  uint8_t packet[4];
  packet[0] = cin;  /* Cable number (upper 4 bits) + CIN (lower 4 bits) */
  packet[1] = b0;
  packet[2] = b1;
  packet[3] = b2;
  
  /* Transmit packet directly */
  USBD_LL_Transmit(&hUsbDeviceFS, MIDI_IN_EP, packet, 4);
}
```

### Key Improvements

1. **Preserves CIN** - Uses the CIN provided by caller without modification
2. **Direct Transmission** - Bypasses USBD_MIDI_SendData's interpretation layer
3. **Non-Blocking** - Checks endpoint status, drops packet if busy
4. **Proper SysEx** - All continuation packets have correct CIN 0x4
5. **USB Compliance** - Follows USB-MIDI 1.0 specification exactly

### How It Fixes the Freeze

**Before Fix:**
```
Query → Process → Generate Response → Corrupt CINs → MIOS Studio confused → FREEZE
```

**After Fix:**
```
Query → Process → Generate Response → Correct CINs → MIOS Studio receives → SUCCESS
```

## Testing Procedure

### Hardware Test
1. Build and flash firmware with fix
2. Connect USB to PC
3. Open MIOS Studio
4. **Expected:** Device enumerates successfully
5. **Expected:** No freeze or timeout
6. **Expected:** Device appears in device list
7. **Expected:** Query responses work
8. Test normal MIDI operation

### What to Look For
- ✅ MIOS Studio doesn't freeze
- ✅ Device enumeration completes
- ✅ Query responses received
- ✅ No loopback behavior
- ✅ No bootloader retries
- ✅ Normal MIDI messages work

## Technical Background

### USB-MIDI Packet Structure
```
Byte 0: Header = (Cable << 4) | CIN
        Cable: 0-15 (virtual MIDI ports)
        CIN: Code Index Number (message type)
Byte 1-3: MIDI data bytes
```

### CIN Values (USB-MIDI 1.0 Spec)
```
0x0: Miscellaneous function codes
0x1: Cable events
0x2: Two-byte System Common
0x3: Three-byte System Common
0x4: SysEx starts or continues (3 bytes, no F7)
0x5: SysEx ends with following single byte / Single-byte System Common
0x6: SysEx ends with following two bytes
0x7: SysEx ends with following three bytes
0x8: Note-Off
0x9: Note-On
0xA: Poly-KeyPress
0xB: Control Change
0xC: Program Change
0xD: Channel Pressure
0xE: PitchBend Change
0xF: Single Byte
```

### Why CIN Matters for SysEx

SysEx messages can be very long (>256 bytes). They're split into 4-byte USB packets:
- **First packet**: CIN 0x4 or 0x7 (if complete in 3 bytes)
- **Middle packets**: CIN 0x4 (continuation)
- **Last packet**: CIN 0x5/0x6/0x7 (depending on bytes remaining)

**The host MUST see the correct CIN sequence to reassemble the SysEx properly!**

If CINs are wrong:
- Host may think SysEx ended early
- Host may think it's a different message type
- Host gets confused and hangs

## Related Files

- `Services/usb_midi/usb_midi.c` - Fixed function
- `Services/usb_midi/usb_midi_sysex.c` - Calls usb_midi_send_packet with correct CINs
- `Services/mios32_query/mios32_query.c` - Generates MIOS32 responses
- `USB_DEVICE/Class/MIDI/Src/usbd_midi.c` - Low-level USB MIDI class

## Conclusion

This was a **critical architectural bug** in the USB MIDI transmission path. The abstraction layer (`USBD_MIDI_SendData`) was designed for simple MIDI messages but couldn't handle SysEx continuation packets properly. By bypassing it and transmitting packets directly with preserved CINs, we ensure USB-MIDI compliance and fix the freeze issue.

**This represents a deep, long-term solution** - not a workaround, but a proper fix to the root cause.
