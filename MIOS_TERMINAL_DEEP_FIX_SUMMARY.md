# MIOS Studio Terminal Deep Fix - Complete Analysis & Solution

## Problem Statement
MIOS Studio recognizes the device (USB MIDI works, queries work), but the terminal window shows nothing.
TeraTerm works fine (USB CDC), but MIOS Studio terminal is blank.

## Root Causes Identified

### Level 1: Surface Issues (FIXED)
1. **3-second boot delay** - No MIOS debug for first 3 seconds
   - **Fixed:** Reduced to 1 second
2. **Aggressive rate limiting** - Only 10 messages/second
   - **Fixed:** Increased to 50 messages/second  
3. **No heartbeat** - User doesn't know if terminal is alive
   - **Fixed:** Added 10-second heartbeat messages

### Level 2: TX Queue Issues (FIXED)
4. **TX queue too small** - Only 32 packets (~3-4 complete SysEx messages)
   - **Fixed:** Increased to 64 packets (~6-8 messages)
5. **Silent packet drops** - No error reporting when queue full
   - **Fixed:** Added return values and error counting
6. **No diagnostics** - Can't see what's failing
   - **Fixed:** Added extensive diagnostic logging

### Level 3: Deep Architecture Issues (DIAGNOSED)
7. **Possible: hmidi NULL or not ready** - TX stops if interface not ready
   - **Diagnostic Added:** Reports hmidi pointer and is_ready flag
8. **Possible: Composite class data issue** - Class data retrieval might fail
   - **Diagnostic Added:** Can track if class data is NULL
9. **Possible: Endpoint always busy** - TX never completes
   - **Diagnostic Added:** Reports endpoint busy conditions

## Changes Made

### File: Services/usb_midi/usb_midi.c
- Increased `USB_MIDI_TX_QUEUE_SIZE` from 32 to 64
- Changed `usb_midi_send_packet()` to return `bool`
- Added `tx_queue_drops` counter
- Added `tx_queue_count()` helper
- Added extensive diagnostics in `tx_queue_send_next()`
- Added `usb_midi_get_tx_status()` API

### File: Services/usb_midi/usb_midi.h
- Updated `usb_midi_send_packet()` signature to return `bool`
- Added `usb_midi_get_tx_status()` declaration

### File: Services/usb_midi/usb_midi_sysex.c
- Changed `usb_midi_send_sysex()` to return `bool`
- Tracks if any packets fail to send
- Returns overall success status

### File: Services/usb_midi/usb_midi_sysex.h
- Updated `usb_midi_send_sysex()` signature to return `bool`

### File: Services/mios32_query/mios32_query.c
- `mios32_debug_send_message()` now checks return value
- Returns false if TX failed (queue full)

### File: App/tests/test_debug.c
- Added `tx_queue_full_count` tracking
- Reports TX queue full errors every 10 occurrences
- Reports stats: sent, dropped, TxQFull
- Reduced boot delay to 1 second
- Increased rate limit to 50 msg/s
- Added 10-second heartbeat

## Diagnostic Output

### Via USB CDC (TeraTerm will show)
```
*** MIOS Terminal Ready ***                           (at 1 second after boot)
[MIOS] Terminal active (sent:123)                     (every 10 seconds)
[MIOS Stats] Sent:200 Dropped:15 TxQFull:3 Rate:50msg/s  (every 200 messages)
[MIOS ERROR] USB MIDI TX queue full 10 times! Packets dropped.  (every 10 TX fails)
[TX-FAIL] hmidi=0x20001234 ready=1 attempts=1000      (if hmidi NULL/not ready)
[TX-BUSY] Endpoint busy, queue=45 items               (if endpoint always busy)
```

### What Each Message Means

**`[TX-FAIL]`** - CRITICAL ERROR
- `hmidi=NULL` → Composite class data retrieval is broken
- `ready=0` → MIDI interface not initialized properly
- This means NO packets are being sent!

**`[TX-BUSY]`** - Performance Warning  
- Endpoint is always busy
- TX completion callback might not be working
- Queue fills up because nothing drains

**`[MIOS ERROR]`** - Queue Overflow
- Too many messages too fast
- Need to increase rate limit delay or queue size further

## Testing Steps

### Step 1: Flash Firmware & Connect
```bash
# Build and flash
# Connect device
# Open TeraTerm on CDC port
# Open MIOS Studio
```

### Step 2: Monitor CDC Output (TeraTerm)
Look for:
- `*** MIOS Terminal Ready ***` at boot
- `[MIOS] Terminal active` every 10 seconds
- Any `[TX-FAIL]` or `[TX-BUSY]` messages
- `[MIOS ERROR]` messages

### Step 3: Check MIOS Studio Terminal
- Does it show the heartbeat message?
- Does it show ANY output at all?
- Send MIDI notes - do debug messages appear?

### Step 4: Interpret Results

#### Scenario A: MIOS Terminal Shows Heartbeat ✅
**Problem SOLVED!** Changes fixed the issue.

#### Scenario B: CDC Shows `[TX-FAIL] hmidi=NULL` ❌
**Root Cause:** Composite class data retrieval is broken.
**Fix Needed:** Check `USBD_COMPOSITE_GetClassData()` in composite mode.

#### Scenario C: CDC Shows `[TX-FAIL] ready=0` ❌  
**Root Cause:** MIDI interface not initialized.
**Fix Needed:** Check `USBD_MIDI_Init()` was called and sets `is_ready=1`.

#### Scenario D: CDC Shows `[TX-BUSY]` Repeatedly ❌
**Root Cause:** TX completion callback not working.
**Fix Needed:** Check `USBD_MIDI_DataIn()` calls `usb_midi_tx_complete()`.

#### Scenario E: CDC Shows `[MIOS ERROR]` Repeatedly ❌
**Root Cause:** TX queue still too small or rate limit too high.
**Fix Needed:** Increase queue size to 128 or reduce rate limit.

#### Scenario F: CDC Shows Nothing After Boot Message ❌
**Root Cause:** Debug output disabled or not reaching MIOS function.
**Fix Needed:** Check `MODULE_ENABLE_USB_MIDI` is defined and `dbg_print()` is being called.

## Next Actions Based on Test Results

### If hmidi is NULL:
1. Add debug to `USBD_COMPOSITE_Init()` to verify it runs
2. Check `composite_class_data.midi_class_data` is set
3. Verify `USBD_COMPOSITE_GetClassData()` returns correct pointer

### If is_ready is 0:
1. Add debug to `USBD_MIDI_Init()` to verify it runs
2. Check `midi_class_data.is_ready = 1` line executes
3. Verify no race condition clearing the flag

### If endpoint always busy:
1. Check `USBD_MIDI_DataIn()` is being called
2. Verify `usb_midi_tx_complete()` is called
3. Check for USB interrupt configuration issues

### If queue keeps filling:
1. Increase `USB_MIDI_TX_QUEUE_SIZE` to 128
2. Reduce rate limit to 20-30 msg/s
3. Consider throttling based on queue fullness

## Code Architecture

```
dbg_print() [App/tests/test_debug.c]
    ↓
    ├→ USB CDC → usb_cdc_send() → TeraTerm ✅ WORKS
    │
    └→ USB MIDI → mios32_debug_send_message() [mios32_query.c]
                      ↓
                  usb_midi_send_sysex() [usb_midi_sysex.c]
                      ↓
                  usb_midi_send_packet() [usb_midi.c]
                      ↓
                  TX Queue (64 packets)
                      ↓
                  tx_queue_send_next() [usb_midi.c]
                      ↓
                  USBD_LL_Transmit() ← Checks hmidi & is_ready
                      ↓
                  USB Hardware
                      ↓
                  USBD_MIDI_DataIn() callback
                      ↓
                  usb_midi_tx_complete()
                      ↓
                  tx_queue_send_next() ← Sends next packet
```

## Expected Behavior After Fix

1. **Boot** → See `*** MIOS Terminal Ready ***` in BOTH TeraTerm and MIOS Studio
2. **Idle** → See heartbeat every 10 seconds in both terminals
3. **MIDI Activity** → See `[RX]` messages in both terminals  
4. **No Errors** → No `[TX-FAIL]` or `[TX-BUSY]` messages
5. **Statistics** → Occasional `[MIOS Stats]` showing sent count, minimal drops

## Files Modified

- `App/tests/test_debug.c` - Timing, heartbeat, error reporting
- `Services/usb_midi/usb_midi.c` - Queue size, diagnostics, return values
- `Services/usb_midi/usb_midi.h` - Function signatures
- `Services/usb_midi/usb_midi_sysex.c` - Return value propagation
- `Services/usb_midi/usb_midi_sysex.h` - Function signature
- `Services/mios32_query/mios32_query.c` - Check send result

## Compilation Notes

All callers of `usb_midi_send_packet()` and `usb_midi_send_sysex()` that ignore the return value will get compiler warnings. This is intentional - it highlights places that should check for TX failures.

Files that may need updates:
- `Services/router/router_send.c` - MIDI routing
- `Services/usb_midi/usb_midi_mtc.c` - MTC messages
- `Services/bootloader/bootloader_protocol.c` - Bootloader
- `Services/dream/dream_sysex.c` - Dream sampler
- `App/tests/*.c` - Test modules

## Success Criteria

✅ MIOS Studio terminal shows debug output
✅ No `[TX-FAIL]` messages in CDC
✅ No `[TX-BUSY]` messages in CDC
✅ Heartbeat appears every 10 seconds
✅ Debug messages appear when sending MIDI
✅ `[MIOS Stats]` shows TxQFull=0

## Failure Escalation

If after this fix the terminal still doesn't work:
1. Collect diagnostic output from TeraTerm
2. Determine which `[TX-FAIL]` condition is occurring
3. Follow the "Next Actions" section above
4. May need to instrument composite class initialization
5. May need USB protocol analyzer to verify packets on wire

---

**Author:** GitHub Copilot  
**Date:** 2026-01-28  
**Version:** 1.0 - Deep Diagnostic Fix
