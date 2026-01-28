# USB CDC Regular MIDI Debug Fragmentation Fix

## Problem

Debug output was showing severe fragmentation with regular MIDI messages:

```
[RX] Cable:0 90 45 5C[COMP-RX] EP:01 MIDI.DataOut=0x803add1 midi_data=0x20011170
[RX-TASK] Processing 2 packet(s)
[RX] Cable:0 90 47 5C[COMP-RX] EP:01 MIDI.DataOut=0x803add1 midi_data=0x20011170
 (Note On Ch:1 Note:66 Vel:123)[COMP-RX] EP:01 MIDI.DataOut=0x803add1 midi_data=0x20011170
```

Messages were being split across multiple lines and interrupted by `[COMP-RX]` messages from interrupt context.

## Root Cause

The `module_test_usb_midi_print_packet()` function in `App/tests/module_tests.c` was making **multiple separate calls** to `dbg_printf()` and `dbg_print()`:

```c
// BEFORE (BROKEN):
dbg_printf("[RX] Cable:%d %02X %02X %02X", cable, status, data1, data2);

// Decode message type
if (msg_type == 0x90 && data2 > 0) {
  dbg_printf(" (Note On Ch:%d Note:%d Vel:%d)", channel, data1, data2);
} else if (msg_type == 0x80 || (msg_type == 0x90 && data2 == 0)) {
  dbg_printf(" (Note Off Ch:%d Note:%d)", channel, data1);
}
// ... more cases ...

dbg_print("\r\n");
```

This resulted in **3 separate USB CDC transmissions**:
1. `"[RX] Cable:0 90 45 5C"`
2. `" (Note On Ch:1 Note:69 Vel:92)"` (or other decoded message)
3. `"\r\n"`

Each transmission could be interrupted by:
- `[COMP-RX]` messages from interrupt context
- `[TX]` messages from task context
- Other debug output

## Solution

Buffer the **complete message** in a single buffer, building it in stages, before calling `dbg_print()` **once**:

```c
// AFTER (FIXED):
char buf[100];
int len;

// Build base message
len = snprintf(buf, sizeof(buf), "[RX] Cable:%d %02X %02X %02X", 
               cable, status, data1, data2);

// Append decoded message to same buffer
uint8_t msg_type = status & 0xF0;
uint8_t channel = (status & 0x0F) + 1;

if (msg_type == 0x90 && data2 > 0) {
  snprintf(buf + len, sizeof(buf) - len, " (Note On Ch:%d Note:%d Vel:%d)\r\n", 
           channel, data1, data2);
} else if (msg_type == 0x80 || (msg_type == 0x90 && data2 == 0)) {
  snprintf(buf + len, sizeof(buf) - len, " (Note Off Ch:%d Note:%d)\r\n", 
           channel, data1);
}
// ... more cases ...

// Single atomic call
dbg_print(buf);
```

This results in **1 atomic USB CDC transmission** containing the complete formatted message.

## Technical Details

### Buffer Building Pattern

The key technique is:
1. Use `snprintf()` to build the first part, capturing the return value
2. Use `buf + len` and `sizeof(buf) - len` to append to the same buffer
3. The second `snprintf()` continues where the first left off
4. Single `dbg_print(buf)` sends the complete message atomically

### Why This Works

- All message construction happens in local stack buffer (atomic, no interrupts)
- `snprintf()` is reentrant and safe
- Buffer pointer arithmetic ensures we append, not overwrite
- Single `dbg_print()` call means one USB CDC transmission
- USB CDC driver handles the complete buffer as one unit

### Buffer Size Calculation

Chosen 100 bytes for buffer because:
- Base message: `"[RX] Cable:0 90 45 5C"` = ~22 bytes
- Decoded message: `" (Note On Ch:1 Note:127 Vel:127)\r\n"` = ~35 bytes
- Total: ~57 bytes + safety margin = 100 bytes

## Expected Output

### Before (Broken)
```
[RX] Cable:0 90 45 5C[COMP-RX] EP:01 MIDI.DataOut=0x803add1
 (Note On Ch:1 Note:69 Vel:92)[COMP-RX] EP:01 MIDI.DataOut=0x803add1
[RX] Cable:0 90 47 5C[COMP-RX] EP:01 MIDI.DataOut=0x803add1
```

### After (Fixed)
```
[RX] Cable:0 90 45 5C (Note On Ch:1 Note:69 Vel:92)
[COMP-RX] EP:01 MIDI.DataOut=0x803add1 midi_data=0x20011170
[RX] Cable:0 90 47 5C (Note On Ch:1 Note:71 Vel:92)
[COMP-RX] EP:01 MIDI.DataOut=0x803add1 midi_data=0x20011170
```

Clean, complete messages on single lines, no fragmentation!

## Files Modified

- `App/tests/module_tests.c` - Fixed `module_test_usb_midi_print_packet()` to use buffered output

## Related Fixes

This completes the USB CDC debug fragmentation fixes:

| Fix | Function | Issue | Commit |
|-----|----------|-------|--------|
| **This fix** | `module_test_usb_midi_print_packet()` | Regular MIDI (3 calls) | Current |
| Previous | `usb_midi_rx_debug_hook()` SysEx | SysEx logging (9 calls) | 06648e8 |
| Original | Various | CDC fragmentation | 9d91da9 |

## Testing

1. **Rebuild** firmware
2. **Send various MIDI messages** from host:
   - Note On/Off
   - CC
   - Program Change
   - Pitch Bend
3. **Verify** output is clean, complete lines with no fragmentation
4. **Send multiple messages** rapidly to stress test

Example test sequence:
```
Send: Note On C4 Vel 100
Send: Note On E4 Vel 100
Send: Note Off C4
Send: CC 1 Val 64
```

Expected clean output:
```
[RX] Cable:0 90 3C 64 (Note On Ch:1 Note:60 Vel:100)
[RX] Cable:0 90 40 64 (Note On Ch:1 Note:64 Vel:100)
[RX] Cable:0 80 3C 00 (Note Off Ch:1 Note:60)
[RX] Cable:0 B0 01 40 (CC Ch:1 CC:1 Val:64)
```

## Pattern Summary

**The universal pattern for ALL debug output:**

```c
// ✓ CORRECT: Single buffered message
char buf[SIZE];
snprintf(buf, sizeof(buf), "Message: %d\r\n", value);
dbg_print(buf);

// ✓ CORRECT: Build message in stages
char buf[SIZE];
int len = snprintf(buf, sizeof(buf), "Part1: %d", val1);
snprintf(buf + len, sizeof(buf) - len, " Part2: %d\r\n", val2);
dbg_print(buf);

// ✗ WRONG: Multiple calls
dbg_printf("Message: %d", value);
dbg_print("\r\n");
```

## Memory Reinforcement

This reinforces the critical rule:
**ALL debug output must use snprintf() to buffer complete messages before calling dbg_print() once**

This applies to:
- ISR context (highest priority for atomicity)
- Task context (can be interrupted by ISR)
- Any debug code, regardless of complexity
- Messages built in stages (use buffer pointer arithmetic)
