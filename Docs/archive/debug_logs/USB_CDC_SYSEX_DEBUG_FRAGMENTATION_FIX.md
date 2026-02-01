# USB CDC SysEx Debug Fragmentation Fix

## Problem

USB CDC debug output was severely fragmented with messages interleaving:

```
[RX SysEx] Cable:0[RX SysEx] Cable:0 [COMP-RX] EP:01 MIDI.DataOut=0x803adf1 midi_data=0x20011170
40[COMP-RX] EP:01 MIDI.DataOut=0x803adf1 midi_data=0x20011170
01[COMP-RX] EP:01 MIDI.DataOut=0x803adf1 midi_data=0x20011170
[RX SysEx] Cable:0[COMP-RX] EP:01 MIDI.DataOut=0x803adf1 midi_data=0x20011170
 CIN:0x [TX] Cable:0 90 3C 64 (Note On)
 3[TX] Cable:0 80 3C 00 (Note Off)
```

The messages were completely unreadable due to severe interleaving.

## Root Cause

In `App/tests/module_tests.c`, the function `usb_midi_rx_debug_hook()` was building SysEx debug messages using **multiple separate calls** to debug functions:

```c
// BEFORE (BROKEN):
dbg_print("[RX SysEx] Cable:");
dbg_print_uint(cable);
dbg_print(" CIN:0x");
dbg_print_hex8(cin);
dbg_print(" Data:");
for (uint8_t i = 1; i < 4; i++) {
  dbg_print(" ");
  dbg_print_hex8(packet4[i]);
}
dbg_print("\r\n");
```

This resulted in **9 separate USB CDC transmissions**:
1. `"[RX SysEx] Cable:"`
2. Cable number
3. `" CIN:0x"`
4. CIN value
5. `" Data:"`
6. `" "`
7. First byte
8. `" "` + second byte + `" "` + third byte
9. `"\r\n"`

Each transmission could be interrupted by messages from:
- Interrupt context (`[COMP-RX]` messages)
- Other task context (`[TX]` messages)
- Other debug output

This caused the severe message fragmentation seen.

## Solution

Buffer the **complete message** using `snprintf()` before calling `dbg_print()` **once**:

```c
// AFTER (FIXED):
char buf[60];
snprintf(buf, sizeof(buf), "[RX SysEx] Cable:%u CIN:0x%X Data: %02X %02X %02X\r\n",
         (unsigned int)cable, (unsigned int)cin,
         (unsigned int)packet4[1], (unsigned int)packet4[2], (unsigned int)packet4[3]);
dbg_print(buf);
```

This results in **1 atomic USB CDC transmission** containing the complete message.

## Technical Details

### Why Multiple Calls Cause Fragmentation

The USB CDC debug output path:
1. `dbg_print()` → `usb_cdc_send()` → USB CDC driver
2. Each call goes through the entire USB stack
3. USB CDC has buffers and timing that can be interrupted
4. FreeRTOS task switching can occur between calls
5. Higher priority interrupts can inject their messages

### Why Single Buffered Call Works

1. `snprintf()` builds message in local stack buffer (atomic, no interrupts)
2. Single `dbg_print(buf)` call sends complete message
3. USB CDC driver handles the buffer as one unit
4. Even if interrupted, the complete message is already in the USB CDC queue

### Pattern to Follow

**ALWAYS use this pattern for debug output:**

```c
// ✓ CORRECT: Buffer complete message
char buf[SIZE];
snprintf(buf, sizeof(buf), "Message: %d %s\r\n", value, string);
dbg_print(buf);

// ✗ WRONG: Multiple calls
dbg_print("Message: ");
dbg_print_uint(value);
dbg_print(" ");
dbg_print(string);
dbg_print("\r\n");
```

## Expected Output

### Before (Broken)
```
[RX SysEx] Cable:0[RX SysEx] Cable:0 [COMP-RX] EP:01 MIDI.DataOut=0x803adf1
40[COMP-RX] EP:01 MIDI.DataOut=0x803adf1
```

### After (Fixed)
```
[RX SysEx] Cable:0 CIN:0x4 Data: F0 7E 7F
[COMP-RX] EP:01 MIDI.DataOut=0x803adf1 midi_data=0x20011170
[RX SysEx] Cable:0 CIN:0x7 Data: 06 01 F7
```

Clean, readable, no interleaving!

## Files Modified

- `App/tests/module_tests.c` - Fixed `usb_midi_rx_debug_hook()` SysEx logging

## Related Issues

This is the same class of bug as documented in:
- `USB_CDC_DEBUG_FRAGMENTATION_FIX.md` - Original CDC fragmentation fix
- Commit 9d91da9 - Previous fragmentation fixes

The pattern must be followed **everywhere** debug output is generated, especially:
- ISR context (highest priority)
- Task context (can be interrupted by ISR)
- Any code that generates multi-part messages

## Testing

1. **Rebuild** firmware
2. **Send SysEx** messages from MIDI host
3. **Verify** output is clean and not fragmented
4. **Verify** SysEx messages are formatted correctly

Example test:
- Send MIDI Device Inquiry: `F0 7E 7F 06 01 F7`
- Expected output:
  ```
  [RX SysEx] Cable:0 CIN:0x4 Data: F0 7E 7F
  [RX SysEx] Cable:0 CIN:0x7 Data: 06 01 F7
  ```

## Memory Stored

This fix reinforces the critical rule:
**Debug output from any context must buffer complete messages using snprintf() before calling dbg_print() to avoid USB CDC packet fragmentation**

This applies to:
- ISR context
- Task context
- Any debug code that builds messages piece by piece
