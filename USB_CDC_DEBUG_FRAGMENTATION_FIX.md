# USB CDC Debug Output Fragmentation Fix

## Problem

When using `MODULE_TEST_USB_DEVICE_MIDI`, debug output via USB CDC (virtual COM port) was severely corrupted:

```
[TX-QUEUE] CIN:[TX-QUEUE] CIN:[TX-QUEUE] CIN:[TX-QUEUE] CIN:
[TX-DBG] Code:[TX-DBG] Code:[TX-DBG] Code:
```

Messages were truncated mid-output and interleaved, making debugging impossible.

## Root Cause

### Multiple Small USB CDC Sends from Interrupt Context

Debug functions made multiple `dbg_print()` calls, each resulting in a USB CDC send:

```c
// OLD CODE - Multiple CDC sends
void test_debug_tx_packet_queued(uint8_t cin, uint8_t b0)
{
  dbg_print("[TX-QUEUE] CIN:");  // CDC send #1
  dbg_print_hex8(cin);            // CDC send #2
  dbg_print(" B0:");              // CDC send #3
  dbg_print_hex8(b0);             // CDC send #4
  dbg_print("\r\n");              // CDC send #5
}
```

### Why This Caused Fragmentation

1. **Interrupt context**: Debug functions called from USB TX complete interrupt
2. **Rapid calls**: Multiple packets queued/sent quickly triggers many debug calls
3. **CDC buffering**: Each `usb_cdc_send()` queues a packet (up to 64 bytes)
4. **Race conditions**: Interrupt preempts previous debug call mid-output
5. **Result**: Incomplete messages like `[TX-QUEUE] CIN:` without the hex values

**Example timeline:**
```
Time 0: Start debug call #1: "[TX-QUEUE] CIN:"
Time 1: Interrupt! Start debug call #2: "[TX-QUEUE] CIN:"
Time 2: Call #2 continues: "09"
Time 3: Call #1 resumes: "08"
Result: "[TX-QUEUE] CIN:[TX-QUEUE] CIN:0908"
```

## Solution

### Buffer Complete Message Before Sending

Build the entire message in a local buffer using `snprintf()`, then send once:

```c
// NEW CODE - Single atomic CDC send
void test_debug_tx_packet_queued(uint8_t cin, uint8_t b0)
{
  char buffer[40];
  
  // Build complete message in buffer
  int len = snprintf(buffer, sizeof(buffer), 
                     "[TX-QUEUE] CIN:%02X B0:%02X\r\n", 
                     cin, b0);
  
  if (len > 0 && len < (int)sizeof(buffer)) {
    dbg_print(buffer);  // Single atomic CDC send
  }
}
```

### Why This Works

1. **Single buffer**: Complete message built in stack buffer
2. **Atomic send**: Only one `dbg_print()` → `usb_cdc_send()` call
3. **Complete packets**: USB CDC gets full message in one write
4. **No interleaving**: Even if interrupted, each message is complete
5. **Result**: Clean output like `[TX-QUEUE] CIN:09 B0:90`

## Implementation Details

### Files Modified

**`App/tests/app_test_usb_midi.c`:**

1. **`test_debug_tx_trace()`** - TX error code debug
   - Before: 5+ `dbg_print()` calls (fragmented)
   - After: 1 `snprintf()` + 1 `dbg_print()` (atomic)

2. **`test_debug_tx_packet_queued()`** - Packet queue debug
   - Before: 5 `dbg_print()` calls (fragmented)
   - After: 1 `snprintf()` + 1 `dbg_print()` (atomic)

3. **Added include**: `#include <stdio.h>` for `snprintf()`

### Code Changes

**Before (5 CDC sends per message):**
```c
void test_debug_tx_trace(uint8_t code)
{
  dbg_print("[TX-DBG] Code:");
  dbg_print_hex8(code);
  
  switch(code) {
    case 0x01:
      dbg_print(" Class data NULL or not ready");
      break;
    // ... more cases
  }
  dbg_print("\r\n");
}
```

**After (1 CDC send per message):**
```c
void test_debug_tx_trace(uint8_t code)
{
  char buffer[80];
  const char* msg;
  
  switch(code) {
    case 0x01: msg = "Class data NULL or not ready"; break;
    // ... more cases
  }
  
  // Build complete message in buffer for atomic send
  int len = snprintf(buffer, sizeof(buffer), 
                     "[TX-DBG] Code:%02X %s\r\n", 
                     code, msg);
  
  if (len > 0 && len < (int)sizeof(buffer)) {
    dbg_print(buffer);  // Single atomic send
  }
}
```

## Results

### Before Fix
```
[TX-QUEUE] CIN:[TX-QUEUE] CIN:[TX-QUEUE] CIN:[TX-QUEUE] CIN:
[TX-DBG] Code:[TX-DBG] Code:[TX-DBG] Code:
[TX] Cable:0 80 3C 00 (Note Off)
[TX-QUEUE] CIN:[TX-QUEUE] CIN:[TX-DBG] Code:[TX-QUEUE] CIN:
```

### After Fix
```
[TX-QUEUE] CIN:09 B0:90
[TX] Cable:0 90 3C 64 (Note On)
[TX-DBG] Code:03 Endpoint BUSY
[TX-QUEUE] CIN:08 B0:80
[TX] Cable:0 80 3C 00 (Note Off)
[TX-DBG] Code:03 Endpoint BUSY
```

Clean, complete messages with proper formatting!

## Performance Impact

### Memory
- **Stack usage**: +80 bytes per debug call (local buffer)
- **Impact**: Negligible (debug functions not in critical path)

### CPU
- **`snprintf()`**: ~100 CPU cycles vs. multiple `dbg_print()` calls
- **Impact**: Positive - fewer function calls, less overhead

### USB CDC
- **Packets**: 1 per message vs. 5 per message
- **Bandwidth**: Same total bytes, but better utilization
- **Impact**: Positive - less USB overhead, no fragmentation

## Best Practices

### Atomic Debug Output from Interrupt Context

When calling debug functions from interrupt context or fast paths:

✅ **DO:**
```c
// Build complete message, send once
char msg[80];
snprintf(msg, sizeof(msg), "Event: %d Value: 0x%04X\r\n", evt, val);
dbg_print(msg);
```

❌ **DON'T:**
```c
// Multiple calls = fragmentation risk
dbg_print("Event: ");
dbg_print_uint(evt);
dbg_print(" Value: 0x");
dbg_print_hex16(val);
dbg_println();
```

### When Buffering Is Not Needed

Buffering is only critical when:
- Called from interrupt context
- Multiple rapid successive calls
- Using USB CDC output (not UART)

For task context with UART output, multiple calls are fine.

## Related Issues

### USB CDC Queue Depth

USB CDC has a TX queue (32 packets). With fragmented output:
- 5 calls × 20 debug messages = 100 packets queued
- Queue overflows, messages lost

With atomic output:
- 1 call × 20 debug messages = 20 packets queued
- Queue never fills, all messages delivered

### UART vs. USB CDC

**UART** (115200 baud):
- Hardware FIFO buffering
- Blocking transmit waits for space
- Multiple calls OK (serialized by UART)

**USB CDC** (full speed):
- Software queue buffering
- Non-blocking queues packet
- Multiple calls → separate packets → fragmentation

## Testing

### Verification

1. Build with `MODULE_TEST_USB_DEVICE_MIDI` enabled
2. Flash firmware
3. Connect USB CDC terminal (COM port)
4. Observe debug output during MIDI transmission

**Expected:** Clean formatted messages like:
```
[TX-QUEUE] CIN:09 B0:90
[TX] Cable:0 90 3C 64 (Note On)
```

**Not:** Fragmented garbage like:
```
[TX-QUEUE] CIN:[TX-QUEUE] CIN:
```

### Test Cases

- ✅ Single MIDI message sent
- ✅ Rapid burst of 10 messages
- ✅ Continuous transmission at 10 Hz
- ✅ TX queue full condition (code 0xFF)
- ✅ Endpoint busy condition (code 0x03)

All cases now produce clean debug output.

## Lessons Learned

1. **USB CDC is not UART**: Different buffering and timing characteristics
2. **Interrupt context requires atomic writes**: Build complete message before sending
3. **Debug functions should be interrupt-safe**: Even "harmless" debug can cause issues
4. **`snprintf()` is your friend**: Fast, safe, predictable for message formatting

## Summary

✅ **Problem**: Garbled USB CDC debug output from interrupt context  
✅ **Cause**: Multiple small writes interleaved/fragmented  
✅ **Solution**: Buffer complete message, send atomically with `snprintf()`  
✅ **Result**: Clean, readable debug output  
✅ **Impact**: Better debugging experience, no performance loss  

**Recommendation:** Apply this pattern to all debug functions called from interrupt context or fast paths.
