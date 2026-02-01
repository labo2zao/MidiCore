# USB MIDI SysEx Buffer Handling Fix

## Overview
This fix improves the robustness of USB MIDI SysEx message handling in MidiCore to prevent potential crashes when receiving large or malformed SysEx messages from MIOS Studio and other Digital Audio Workstations (DAWs).

## Problem Statement
When connecting MidiCore to MIOS Studio, the USB MIDI driver needed improved defensive programming for SysEx message handling, particularly for:
- Large SysEx messages approaching the 256-byte buffer limit
- Malformed or incomplete SysEx messages
- Consistency across different SysEx packet types (CIN 0x4, 0x5, 0x6, 0x7)

## Solution
The fix implements three key improvements:

### 1. Consistent Boundary Checking
**Before:**
- CIN 0x5 (1-byte end): `if (buf->pos < USB_MIDI_SYSEX_BUFFER_SIZE)`
- CIN 0x6 (2-byte end): `if (buf->pos + 2 <= USB_MIDI_SYSEX_BUFFER_SIZE)`
- CIN 0x7 (3-byte end): `if (buf->pos + 3 <= USB_MIDI_SYSEX_BUFFER_SIZE)`

**After:**
- CIN 0x5 (1-byte end): `if (buf->pos + 1 <= USB_MIDI_SYSEX_BUFFER_SIZE)` ✓
- CIN 0x6 (2-byte end): `if (buf->pos + 2 <= USB_MIDI_SYSEX_BUFFER_SIZE)` (unchanged)
- CIN 0x7 (3-byte end): `if (buf->pos + 3 <= USB_MIDI_SYSEX_BUFFER_SIZE)` (unchanged)

All handlers now use the same semantic pattern: "Is there room for N bytes?"

### 2. Explicit Overflow Handling
**CIN 0x4 (SysEx continue):**
```c
if (buf->active) {
  if (buf->pos + 3 <= USB_MIDI_SYSEX_BUFFER_SIZE) {
    // Copy 3 bytes
  } else {
    // NEW: Explicit overflow handling
    buf->pos = 0;
    buf->active = 0;
  }
}
```

### 3. Guaranteed Buffer Cleanup
**All end packet handlers (CIN 0x5/0x6/0x7):**
```c
if (buf->active) {
  if (buf->pos + N <= USB_MIDI_SYSEX_BUFFER_SIZE) {
    // Copy N bytes and process
  }
  // ALWAYS reset buffer state (even on overflow)
  buf->pos = 0;
  buf->active = 0;
}
```

## Technical Details

### Buffer Structure
```c
#define USB_MIDI_SYSEX_BUFFER_SIZE 256

typedef struct {
  uint8_t buffer[USB_MIDI_SYSEX_BUFFER_SIZE];  // 256 bytes (indices 0-255)
  uint16_t pos;                                  // Next write position / current length
  uint8_t active;                                // SysEx reception active flag
  uint8_t padding;
} sysex_buffer_t;
```

### Valid States
- `pos` can range from 0 to 256
- Valid buffer indices: 0-255
- When `pos = 256`, buffer is completely full (all 256 bytes written)

### SysEx Message Flow
1. **Start:** Receive CIN 0x4 with F0 → `pos = 0`, `active = 1`
2. **Continue:** Receive CIN 0x4 packets → `pos` grows: 3, 6, 9, ..., up to 255
3. **End:** Receive CIN 0x5/0x6/0x7 with F7 → Validate and route, then reset

## Testing
Comprehensive test suite in `Tests/test_sysex_overflow.c`:

```bash
cd Tests
gcc -o test_sysex_overflow test_sysex_overflow.c -Wall -Wextra
./test_sysex_overflow
```

### Test Cases
1. **Normal SysEx (10 bytes):** F0 + 8 data bytes + F7
2. **Near-full buffer (255 bytes):** F0 + 253 data bytes + F7
3. **Full buffer (256 bytes):** F0 + 254 data bytes + F7
4. **Overflow (>256 bytes):** Buffer overflow detection and rejection

All tests pass ✓

## Impact

### Benefits
- **More Maintainable:** Consistent patterns across all handlers
- **Defensive:** Explicit handling of overflow cases
- **Robust:** Guaranteed cleanup on all code paths
- **Compatible:** Works seamlessly with MIOS Studio and other DAWs

### Compatibility
- No breaking changes to the API
- Maintains full compatibility with existing code
- Improves robustness for edge cases

## Files Modified
- `Services/usb_midi/usb_midi.c` - Core SysEx handling improvements
- `Tests/test_sysex_overflow.c` - New comprehensive test suite

## References
- USB MIDI 1.0 Specification
- MIOS32 USB MIDI implementation patterns
- MidiCore USB MIDI architecture

## Author
MidiCore Development Team
Date: 2026-01-27
