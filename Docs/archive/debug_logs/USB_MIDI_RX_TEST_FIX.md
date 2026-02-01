# USB MIDI RX Test Fix

## Problem

When running `MODULE_TEST_USB_DEVICE_MIDI`, USB MIDI receive (RX) was only showing:
```
[COMP-RX] EP:01 MIDI_OK
```

No actual MIDI data was being printed, even though:
- USB was correctly receiving packets from the host
- The composite layer was routing to the MIDI class
- The debug hook was properly defined

## Root Cause

**The test loops never called `usb_midi_process_rx_queue()`.**

### USB MIDI RX Architecture

MidiCore uses a **deferred RX queue** architecture for USB MIDI:

1. **ISR Context (Interrupt)** - `usb_midi_rx_packet()`:
   - USB hardware receives data on endpoint 0x01
   - USB core calls USBD_COMPOSITE_DataOut()
   - Composite layer prints `[COMP-RX] EP:01 MIDI_OK`
   - MIDI class forwards to `usb_midi_rx_packet()`
   - **Packets are QUEUED, not processed**
   - Returns immediately to avoid blocking ISR

2. **Task Context (Main Loop)** - `usb_midi_process_rx_queue()`:
   - **Must be called explicitly from application**
   - Dequeues packets from RX queue
   - Validates cable number
   - Calls `usb_midi_rx_debug_hook()` for logging
   - Routes to MIDI router
   - Handles SysEx assembly

### The Bug

Both test functions had infinite loops but never called `usb_midi_process_rx_queue()`:

**Before (Bug):**
```c
for (;;) {
    uint32_t now = osKernelGetTickCount();
    // Send test messages...
    osDelay(10);
}
```

**Result:** RX packets accumulated in the queue but were never processed → No MIDI data output.

## Solution

Added `usb_midi_process_rx_queue()` call to both test loops:

**After (Fixed):**
```c
for (;;) {
    // CRITICAL: Process queued RX packets from USB interrupt
    usb_midi_process_rx_queue();
    
    uint32_t now = osKernelGetTickCount();
    // Send test messages...
    osDelay(10);
}
```

## Files Changed

1. **`App/tests/app_test_usb_midi.c`** (line 339)
   - Fixed `app_test_usb_midi_run_forever()` test loop
   - Used by `APP_TEST_USB_MIDI` mode

2. **`App/tests/module_tests.c`** (line 7164)
   - Fixed `module_test_usb_device_midi_run()` test loop
   - Used by `MODULE_TEST_USB_DEVICE_MIDI` mode

## Expected Output After Fix

### Before (Bug):
```
[COMP-RX] EP:01 MIDI_OK
```

### After (Fixed):
```
[COMP-RX] EP:01 MIDI_OK
[RX-ISR] Cable:0 CIN:09
[RX-TASK] Processing 1 packet(s)
[RX] Cable:0 90 3C 64 (Note On Ch:1 Note:60 Vel:100)
```

When sending a CC:
```
[COMP-RX] EP:01 MIDI_OK
[RX-ISR] Cable:0 CIN:0B
[RX-TASK] Processing 1 packet(s)
[RX] Cable:0 B0 07 7F (CC Ch:1 CC:7 Val:127)
```

When sending SysEx:
```
[COMP-RX] EP:01 MIDI_OK
[RX-ISR] Cable:0 CIN:04
[RX-TASK] Processing 1 packet(s)
[RX SysEx] Cable:0 CIN:0x04 Data: F0 00 00
[COMP-RX] EP:01 MIDI_OK
[RX-ISR] Cable:0 CIN:07
[RX-TASK] Processing 1 packet(s)
[RX SysEx] Cable:0 CIN:0x07 Data: 7E 7F F7
```

## How to Test

1. **Rebuild firmware:**
   ```
   Project → Clean → Clean all projects
   Project → Build All (Ctrl+B)
   ```

2. **Flash to device:**
   ```
   Run → Debug (F11)
   ```

3. **Connect USB to computer**

4. **Send MIDI from DAW/host:**
   - Use Ableton, Logic, Reaper, or MIDI-OX
   - Send Note On/Off, CC, SysEx messages
   - Select "MidiCore" MIDI device

5. **Watch terminal output:**
   - Open terminal connected to debug UART (115200 baud)
   - Or use USB CDC virtual COM port if enabled
   - You should now see detailed MIDI data, not just `MIDI_OK`

## Reference

This fix follows the same pattern used in production code:

- **`App/midi_io_task.c`** (line 19): Main MIDI I/O task calls `usb_midi_process_rx_queue()`
- **`Services/usb_midi/usb_midi.h`** (lines 60-74): Documents the requirement

### Documentation Quote:
```c
/**
 * @brief Process queued RX packets - MUST be called from task context!
 * 
 * Call this regularly from main loop or dedicated USB MIDI task. It processes
 * all queued RX packets, handles SysEx assembly, MIOS32 queries, and routing.
 * 
 * CRITICAL: Do NOT call from interrupt context!
 * 
 * Example usage in main loop:
 *   while(1) {
 *     usb_midi_process_rx_queue();  // Process received MIDI
 *     // ... other tasks ...
 *   }
 */
void usb_midi_process_rx_queue(void);
```

## Lessons Learned

1. **Deferred processing requires explicit task-side call** - ISR queuing is only half the story
2. **Test code should mirror production patterns** - Both should call the same processing functions
3. **Debug messages at ISR level don't guarantee task-level processing** - `[COMP-RX] MIDI_OK` proved USB reception, but not application processing

## Related Issues

This fix resolves the symptom described in:
- `REBUILD_FIRMWARE_NOW.md` - Explains the expected vs actual output
- `USB_MIDI_RX_TX_TEST_GUIDE.md` - Testing procedure (needs update)
- `DEFINITIVE_DEBUG_TEST.md` - Debug procedures

## Memory Stored

This critical fact has been stored in repository memory for future reference:
```
usb_midi_process_rx_queue() must be called from task context in any 
loop/task that needs to process USB MIDI RX data
```

## Commit

```
commit 68c1c53
Author: GitHub Copilot
Date:   Tue Jan 28 03:XX:XX 2026

    Fix USB MIDI RX: Add missing usb_midi_process_rx_queue() call to test loops
    
    - Fixed app_test_usb_midi_run_forever() in app_test_usb_midi.c
    - Fixed module_test_usb_device_midi_run() in module_tests.c
    - Both test loops now process RX queue correctly
    - Resolves issue where only [COMP-RX] EP:01 MIDI_OK was shown
```
