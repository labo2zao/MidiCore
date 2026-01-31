# USB CDC Debug Fragmentation - Quick Fix Guide

## The Problem

Messages look like this:
```
[RX SysEx] Cable:0[RX SysEx] Cable:0 40[COMP-RX] 01[COMP-RX]
 CIN:0x [TX] Cable:0 90 3C 64 (Note On)
```

**Cause:** Multiple `dbg_print()` calls in debug code

## The Rule

**ALWAYS buffer complete messages:**

### ✓ CORRECT
```c
char buf[100];
snprintf(buf, sizeof(buf), "[Debug] Value: %d Status: %s\r\n", val, str);
dbg_print(buf);
```

### ✗ WRONG
```c
dbg_print("[Debug] Value: ");
dbg_print_uint(val);
dbg_print(" Status: ");
dbg_print(str);
dbg_print("\r\n");
```

## Why It Matters

Each `dbg_print()` call:
- Goes through USB CDC stack
- Can be interrupted by other messages
- Causes fragmentation when multiple calls used

Single buffered call:
- Builds message atomically on stack
- One USB CDC transmission
- Cannot be fragmented

## Fixed Issues

| File | Function | Issue | Commit |
|------|----------|-------|--------|
| `App/tests/module_tests.c` | `usb_midi_rx_debug_hook()` | SysEx logging (9 calls) | 06648e8 |
| Previous | Various | Multiple fragmentation issues | 9d91da9 |

## How to Apply

When writing debug code:

1. **Count your `dbg_print()` calls**
   - If > 1, you need to buffer

2. **Calculate buffer size**
   - Message length + format specifiers + safety margin
   - Typically 60-100 bytes is sufficient

3. **Use snprintf()**
   ```c
   char buf[SIZE];
   snprintf(buf, sizeof(buf), "format", args...);
   dbg_print(buf);
   ```

4. **One call only**
   - Single `dbg_print(buf)` at the end

## Testing

After fixing:
1. Rebuild firmware
2. Send MIDI/SysEx data
3. Verify output is clean and readable
4. No interleaved characters or broken messages

## Documentation

- `USB_CDC_DEBUG_FRAGMENTATION_FIX.md` - Original fix documentation
- `USB_CDC_SYSEX_DEBUG_FRAGMENTATION_FIX.md` - SysEx fix details
