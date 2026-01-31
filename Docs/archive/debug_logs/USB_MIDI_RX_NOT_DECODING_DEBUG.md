# USB MIDI RX Not Decoding - Debug Enhancement

## Problem

User reports output showing:
```
[RX-TASK] Processing 2 packet(s)
[COMP-RX] EP:01 MIDI.DataOut=0x803ada5 midi_data=0x20011170
```

But missing:
- `[COMP-RX] EP:01 MIDI_OK` or `MIDI_SKIP`
- `[MIDI-DataOut] ENTRY`
- `[RX-ISR]` messages  
- `[RX]` decoded MIDI messages

## Analysis

The output shows:
1. ✓ Composite layer receives data (`[COMP-RX]` with pointer values)
2. ✓ Task processes queue (`[RX-TASK] Processing 2 packet(s)`)
3. ✗ But neither MIDI_OK nor MIDI_SKIP appears
4. ✗ USBD_MIDI.DataOut() never called (`[MIDI-DataOut] ENTRY` missing)
5. ✗ No decoded MIDI output

This means the condition check at line 309 in `usbd_composite.c` is FAILING:
```c
if (USBD_MIDI.DataOut != NULL && composite_class_data.midi_class_data != NULL)
```

Even though the pointers appear to have values, the condition is evaluating to false.

## Debug Enhancement

Added explicit boolean check output to diagnose the issue:

```c
/* Debug: Show actual condition result */
uint8_t dataout_ok = (USBD_MIDI.DataOut != NULL) ? 1 : 0;
uint8_t mididata_ok = (composite_class_data.midi_class_data != NULL) ? 1 : 0;
snprintf(buf, sizeof(buf), "[COMP-RX] Check: DataOut=%d midi_data=%d\r\n", 
         dataout_ok, mididata_ok);
dbg_print(buf);
```

Also added `(void*)` casts to pointer printf to ensure consistent formatting.

## Expected Output

With this fix, user should now see:
```
[COMP-RX] EP:01 MIDI.DataOut=0x0803ada5 midi_data=0x20011170
[COMP-RX] Check: DataOut=1 midi_data=1
[COMP-RX] EP:01 MIDI_OK
[COMP] Calling MIDI.DataOut
[MIDI-DataOut] ENTRY
...
```

Or if condition fails:
```
[COMP-RX] EP:01 MIDI.DataOut=0x0803ada5 midi_data=0x20011170
[COMP-RX] Check: DataOut=0 midi_data=1   ← Shows which is NULL!
[COMP-RX] EP:01 MIDI_SKIP (DataOut:0x0803ada5 data:0x20011170)
```

## Possible Issues

### Issue 1: Pointer Formatting
The pointer `0x803ada5` shown by user is missing leading zero and looks misaligned. Could be:
- Printf format issue
- Terminal rendering issue
- Actual pointer corruption

### Issue 2: Condition Evaluation
The condition might be failing due to:
- Optimizer issues
- Undefined behavior
- Pointer comparison with NULL not working as expected
- Compiler flags

### Issue 3: Missing Debug Output
Some debug messages not appearing could be:
- USB CDC buffer overflow
- Interrupt priority issues
- Debug output being dropped

## Next Steps

1. User rebuilds with this debug enhancement
2. Check if `[COMP-RX] Check:` line appears
3. Verify if both values are 1 or if one is 0
4. This will pinpoint which pointer is actually NULL despite display

## Files Modified

- `USB_DEVICE/App/usbd_composite.c` - Added explicit condition check debug output
