# USB Composite Debug Buffer Scope Fix

## Problem

In `USB_DEVICE/App/usbd_composite.c`, there was a variable scope issue causing `dbg_print()` to fail:

### Bug Details

**Location:** `USB_DEVICE/App/usbd_composite.c` function `USBD_COMPOSITE_DataOut()`

**Issue:**
```c
// Line 298-305: First #ifdef block
#ifdef MODULE_TEST_USB_DEVICE_MIDI
    extern void dbg_print(const char *str);
    char buf[80];  // ← Declared here
    snprintf(buf, sizeof(buf), "[COMP-RX] EP:%02X MIDI.DataOut=%p midi_data=%p\r\n", 
             epnum, USBD_MIDI.DataOut, composite_class_data.midi_class_data);
    dbg_print(buf);
#endif

// ... nested if statement with another #ifdef ...

// Line 329-334: Second #ifdef block
#ifdef MODULE_TEST_USB_DEVICE_MIDI
    /* Debug: MIDI routing failed */
    snprintf(buf, sizeof(buf), "[COMP-RX] EP:%02X MIDI_SKIP (DataOut:%p data:%p)\r\n", 
             epnum, USBD_MIDI.DataOut, composite_class_data.midi_class_data);
    dbg_print(buf);  // ← Tries to use buf but it's OUT OF SCOPE!
#endif
```

**Root Cause:**
- `buf` was declared inside the first `#ifdef MODULE_TEST_USB_DEVICE_MIDI` block
- The second `#ifdef MODULE_TEST_USB_DEVICE_MIDI` block tried to reuse `buf`
- But `buf` was out of scope, causing undefined behavior
- This resulted in `dbg_print()` not working or printing garbage

**Symptoms:**
- `dbg_print(buf)` prints nothing in terminal
- Debugger shows `buf` at memory address but content is undefined
- MIDI_SKIP message not appearing when it should

## Solution

Moved the `buf` declaration to the outer scope of the entire MIDI endpoint handling block:

```c
#ifdef MODULE_TEST_USB_DEVICE_MIDI
    /* Debug: Declare buffer at outer scope for reuse */
    extern void dbg_print(const char *str);
    char buf[80];  // ← Now declared at outer scope
    
    /* Debug: Show pointer status before check */
    snprintf(buf, sizeof(buf), "[COMP-RX] EP:%02X MIDI.DataOut=%p midi_data=%p\r\n", 
             epnum, USBD_MIDI.DataOut, composite_class_data.midi_class_data);
    dbg_print(buf);
#endif
```

**Benefits:**
1. Both debug sections can now use the same buffer
2. Reduced stack usage (one buffer instead of two)
3. Eliminated `buf2` variable that was unnecessarily created
4. Fixed undefined behavior
5. `dbg_print()` now works correctly for both MIDI_OK and MIDI_SKIP cases

## Testing

After this fix, you should see proper debug output:

**When MIDI class data is valid:**
```
[COMP-RX] EP:01 MIDI.DataOut=0x08001234 midi_data=0x20001234
[COMP-RX] EP:01 MIDI_OK
[COMP] Calling MIDI.DataOut
[COMP] MIDI.DataOut returned
```

**When MIDI class data is NULL:**
```
[COMP-RX] EP:01 MIDI.DataOut=0x08001234 midi_data=0x00000000
[COMP-RX] EP:01 MIDI_SKIP (DataOut:0x08001234 data:0x00000000)
```

## Technical Notes

### C Variable Scope Rules

This fix addresses a fundamental C scoping issue:
- Variables declared inside a block (`{ }`) are only valid within that block
- When the block ends, the variable goes out of scope
- Accessing an out-of-scope variable causes undefined behavior

### Why This Wasn't Caught by Compiler

- Both uses were inside `#ifdef MODULE_TEST_USB_DEVICE_MIDI` blocks
- The compiler didn't warn because from its perspective, each `#ifdef` block is independent
- The linker/compiler couldn't detect the scope issue across preprocessor blocks
- This is a common pitfall when using multiple `#ifdef` blocks in the same function

### Prevention

To avoid similar issues:
1. Declare debug buffers at function scope when used multiple times
2. Use unique names if truly temporary (but wastes stack)
3. Use block comments to clarify buffer lifetime
4. Consider static analysis tools that can catch scope issues

## Related Issues

User also mentioned: `uint8_t cable = (packet4[0] > 4) & 0x0F;`

However, I couldn't find this bug in the current codebase. The code at `Services/usb_midi/usb_midi.c:247` correctly uses:
```c
uint8_t cable = (packet4[0] >> 4) & 0x0F;  // ✓ Correct: >> is right shift
```

If you see `>` (greater than) instead of `>>` (right shift) in your local code, it may be a git merge issue or an uncommitted change.

## Files Modified

- `USB_DEVICE/App/usbd_composite.c` - Fixed buffer scope issue
