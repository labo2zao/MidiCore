# USB MIDI RX Decoding Issue - Summary

## User's Problem

Output shows packets being received but not decoded:

```
[RX-TASK] Processing 2 packet(s)
[COMP-RX] EP:01 MIDI.DataOut=0x803ada5 midi_data=0x20011170
[RX-TASK] Processing 2 packet(s)
[COMP-RX] EP:01 MIDI.DataOut=0x803ada5 midi_data=0x20011170
...
[TX] Cable:0 90 3C 64 (Note On)
```

### What's Working
- ✓ USB receiving data (composite layer prints `[COMP-RX]`)
- ✓ Packets being queued (`[RX-TASK]` shows processing)
- ✓ Pointers appear to have values
- ✓ TX direction works (shows `[TX]` message)

### What's Missing
- ✗ `[COMP-RX] EP:01 MIDI_OK` (should appear after pointers checked)
- ✗ `[MIDI-DataOut] ENTRY` (should appear when MIDI class called)
- ✗ `[RX-ISR]` (should appear when packets queued)
- ✗ `[RX]` decoded MIDI (should show actual MIDI data)

## Diagnosis

The condition at `USB_DEVICE/App/usbd_composite.c:309` is failing:

```c
if (USBD_MIDI.DataOut != NULL && composite_class_data.midi_class_data != NULL)
```

Even though both pointers display as having values, the condition evaluates to FALSE.

## Solution Applied

### Phase 1: Fixed Buffer Scope Issue (Commit 63a7df5)
Fixed debug buffer being out of scope when used in multiple `#ifdef` blocks.

### Phase 2: Added Explicit Condition Check (Commit 7fc3f8b)
Added explicit boolean evaluation to pinpoint which pointer is NULL:

```c
uint8_t dataout_ok = (USBD_MIDI.DataOut != NULL) ? 1 : 0;
uint8_t mididata_ok = (composite_class_data.midi_class_data != NULL) ? 1 : 0;
snprintf(buf, sizeof(buf), "[COMP-RX] Check: DataOut=%d midi_data=%d\r\n", 
         dataout_ok, mididata_ok);
dbg_print(buf);
```

## Expected Output After Fix

### Success Case (Both Pointers Valid)
```
[COMP-RX] EP:01 MIDI.DataOut=0x0803ada5 midi_data=0x20011170
[COMP-RX] Check: DataOut=1 midi_data=1
[COMP-RX] EP:01 MIDI_OK
[COMP] Calling MIDI.DataOut
[MIDI-DataOut] ENTRY
[MIDI-RX] Len:4
[MIDI-RX] Calling callback
[MIDI-RX] Cable:0 90 3C 64
[RX-ISR] Cable:0 CIN:09
[RX-TASK] Processing 1 packet(s)
[RX] Cable:0 90 3C 64 (Note On Ch:1 Note:60 Vel:100)
```

### Failure Case (One Pointer NULL)
```
[COMP-RX] EP:01 MIDI.DataOut=0x0803ada5 midi_data=0x20011170
[COMP-RX] Check: DataOut=0 midi_data=1    ← Shows DataOut is actually NULL!
[COMP-RX] EP:01 MIDI_SKIP (DataOut:0x0803ada5 data:0x20011170)
```

Or:
```
[COMP-RX] EP:01 MIDI.DataOut=0x0803ada5 midi_data=0x20011170
[COMP-RX] Check: DataOut=1 midi_data=0    ← Shows midi_data is actually NULL!
[COMP-RX] EP:01 MIDI_SKIP (DataOut:0x0803ada5 data:0x20011170)
```

## Why This Helps

The new `[COMP-RX] Check:` line will immediately show:
1. If DataOut=0, then `USBD_MIDI.DataOut` is NULL despite printf showing a value
2. If midi_data=0, then `composite_class_data.midi_class_data` is NULL
3. If both are 1, then something else is wrong with the condition

## Possible Root Causes

### Cause 1: Pointer Formatting Issue
The `%p` format might display garbage even for NULL pointers. The explicit boolean check avoids this.

### Cause 2: Compiler Optimization
The condition might be optimized in a way that doesn't match expectation. Separate boolean evaluation should help.

### Cause 3: Initialization Order
If one pointer is being set to NULL after init due to:
- Race condition
- Memory corruption
- Incorrect deinitialization

### Cause 4: Wrong Pointer Comparison
If there's an issue with how NULL is compared on this platform/compiler.

## How to Test

1. **Rebuild** firmware with latest changes
2. **Flash** to device
3. **Send MIDI** from host to device
4. **Look for** the `[COMP-RX] Check:` line
5. **Report** which value is 0 or if both are 1

## Follow-Up Actions

Depending on the `[COMP-RX] Check:` output:

### If DataOut=0
- Check `USBD_MIDI` struct initialization in `usbd_midi.c`
- Verify linker is including the MIDI class callbacks
- Check if USBD_MIDI is being modified after init

### If midi_data=0
- Check `USBD_COMPOSITE_Init()` in `usbd_composite.c`
- Verify `USBD_MIDI.Init()` is being called and succeeds
- Check if `composite_class_data` is being cleared

### If both are 1 but still fails
- Compiler/optimizer issue
- Need to restructure the condition
- May need to check in debugger with breakpoint

## Files Modified

| File | Commit | Changes |
|------|--------|---------|
| `USB_DEVICE/App/usbd_composite.c` | 63a7df5 | Fixed buffer scope |
| `USB_DEVICE/App/usbd_composite.c` | 7fc3f8b | Added explicit condition check |
| `USB_COMPOSITE_BUFFER_SCOPE_FIX.md` | 63a7df5 | Documentation |
| `USB_MIDI_RX_NOT_DECODING_DEBUG.md` | 7fc3f8b | Diagnostic guide |
| `USB_MIDI_RX_DECODING_ISSUE_SUMMARY.md` | Current | This summary |

## Next Steps

User needs to:
1. Pull latest changes
2. Rebuild and flash firmware
3. Test and report the `[COMP-RX] Check:` line output
4. Depending on result, we'll apply appropriate fix

## Related Documentation

- `USB_MIDI_INIT_DEBUG_GUIDE.md` - Init debugging
- `USB_MIDI_RX_DEBUG_BREAKPOINTS.md` - Breakpoint locations
- `EXACT_LINE_NUMBERS.md` - Line number reference
- `USB_COMPOSITE_BUFFER_SCOPE_FIX.md` - Previous buffer fix
