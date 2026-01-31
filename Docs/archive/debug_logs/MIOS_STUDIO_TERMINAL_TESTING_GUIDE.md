# MIOS Studio Terminal Testing Guide

## Problem
MIOS Studio recognizes device but terminal shows nothing.

## Quick Test Procedure

### Step 1: Flash Firmware
Build and flash firmware with these flags:
```
DEBUG
MODULE_TEST_USB_DEVICE_MIDI
SRIO_ENABLE
USE_HAL_DRIVER
STM32F407xx
```

### Step 2: Connect Both Terminals

**TeraTerm (USB CDC):**
1. Find the USB Serial Device (COMx) port
2. Connect at 115200 baud, 8-N-1, no flow control
3. You should see extensive debug output

**MIOS Studio:**
1. Device should appear in device list (confirmed working)
2. Open Terminal window (View → Terminal)
3. Connect to MidiCore device
4. Terminal should show messages

### Step 3: What You Should See

**Immediately After Boot (in TeraTerm):**
```
Testing MIOS Studio terminal...
Sending 5 test messages directly...
  Message 1 sent successfully
  Message 2 sent successfully
  Message 3 sent successfully
  Message 4 sent successfully
  Message 5 sent successfully
If you see messages in MIOS Studio terminal, it's working!
```

**Immediately After Boot (in MIOS Studio Terminal):**
```
*** MIOS Terminal Test #1 ***
*** MIOS Terminal Test #2 ***
*** MIOS Terminal Test #3 ***
*** MIOS Terminal Test #4 ***
*** MIOS Terminal Test #5 ***
```

**Every 2 Seconds (both terminals):**
```
[TX] Cable:0 90 3C 64 (Note On)
[MIOS TEST] Note On sent, waiting for Note Off...
[TX] Cable:0 80 3C 00 (Note Off)
[MIOS TEST] Note Off sent, cycle complete.
```

**After 1+ Seconds (MIOS Studio Terminal):**
```
*** MIOS Terminal Ready ***
```

**Every 10 Seconds (MIOS Studio Terminal):**
```
[MIOS] Terminal active (sent:123)
```

## Diagnostic Scenarios

### Scenario A: Nothing in MIOS Studio Terminal ❌
**What it means:** Messages are not reaching MIOS Studio

**Check TeraTerm for these patterns:**

**Pattern 1: "Message X FAILED to send!"**
- Root cause: TX queue full, messages can't be sent
- Fix: Increase USB_MIDI_TX_QUEUE_SIZE or reduce message rate

**Pattern 2: All "Message X sent successfully"**
- Root cause: Messages sent but MIOS Studio not receiving
- Possible causes:
  1. Wrong SysEx format (but format was verified correct)
  2. MIOS Studio terminal not connected to correct device
  3. MIOS Studio terminal window not open
  4. MIOS Studio version issue (need v2.4+)

**Pattern 3: Mix of success/failure**
- Root cause: Intermittent TX queue issues
- Some messages get through, others don't
- Look for "[MIOS ERROR]" messages in TeraTerm about queue full

### Scenario B: Test Messages Appear, But Not Continuous ⚠️
**What it means:** Initial messages work, but dbg_print() doesn't

**Check for:**
- Boot delay issue (1-second delay)
- Rate limiting too aggressive (20ms = 50 msg/s)
- Messages sent from ISR context (blocked by IPSR check)

**In TeraTerm, look for:**
```
[MIOS Stats] Sent:200 Dropped:150 TxQFull:50 Rate:50msg/s
```
High dropped/TxQFull counts indicate queue pressure.

### Scenario C: Everything Works ✅
**What you'll see:**
- Test messages appear immediately in MIOS Studio
- Continuous "[MIOS TEST]" messages every 2 seconds
- Heartbeat every 10 seconds
- Terminal responsive and stable

## MIOS32 Debug Message Format

**Protocol:** MIOS32 SysEx Debug Message (Command 0x0D)

**Format:**
```
F0 00 00 7E 32 00 0D <ascii_text> F7

Where:
  F0         = SysEx start
  00 00 7E   = MIOS32 manufacturer ID
  32         = MIOS32 device ID (MIOS32_QUERY_DEVICE_ID)
  00         = Device instance (first device)
  0D         = Debug message command (MIOS32_CMD_DEBUG_MESSAGE)
  <text>     = ASCII text (variable length, max ~240 chars)
  F7         = SysEx end
```

**Example:** "Hello\r\n"
```
F0 00 00 7E 32 00 0D 48 65 6C 6C 6F 0D 0A F7
```

## Code Flow

### Direct Test Messages (Immediate)
```
module_test_usb_device_midi_run()
  └─> mios32_debug_send_message() x5
      └─> usb_midi_send_sysex()
          └─> usb_midi_send_packet() x ~8-10 packets per message
              └─> TX queue (64 packets)
```

### Regular Debug Messages (Via dbg_print)
```
dbg_printf("[MIOS TEST] ...")
  └─> dbg_print()
      ├─> usb_cdc_send() → TeraTerm
      └─> mios32_debug_send_message() → MIOS Studio
          ├─ Check: Not in ISR (IPSR == 0)
          ├─ Check: 1 second after boot
          ├─ Check: 20ms since last message
          └─> usb_midi_send_sysex()
```

## TX Queue Monitoring

**In TeraTerm, watch for:**

```
[TX-FAIL] hmidi=0x20001234 ready=1 attempts=1000
```
- Means: MIDI interface issues
- Action: Check USB MIDI initialization

```
[TX-BUSY] Endpoint busy, queue=45 items
```
- Means: Endpoint stuck, queue backing up
- Action: Check USB interrupt priority

```
[MIOS ERROR] USB MIDI TX queue full 50 times!
```
- Means: Too many messages, queue overflow
- Action: Reduce message rate or increase queue size

```
[MIOS Stats] Sent:1000 Dropped:10 TxQFull:5 Rate:50msg/s
```
- Good: Low drop rate (<1%)
- Bad: High drop rate (>10%)

## Troubleshooting Steps

### If Terminal Completely Blank:

1. **Verify USB MIDI Connection**
   - Check MIOS Studio device list shows MidiCore
   - Try disconnect/reconnect
   - Try different USB port

2. **Check TeraTerm Output**
   - Confirms firmware is running
   - Shows "sent successfully" or "FAILED"
   - Shows TX queue diagnostics

3. **Verify MIOS Studio Version**
   - Need v2.4 or later for proper CDC support
   - Older versions might not handle terminal correctly

4. **Check Terminal Window**
   - View → Terminal must be open
   - Make sure it's connected to MidiCore device
   - Try close/reopen terminal window

5. **Check Module Configuration**
   - MODULE_ENABLE_USB_MIDI must be 1
   - MODULE_ENABLE_USB_CDC can be 0 or 1
   - Verify in TeraTerm that #if blocks are active

### If Intermittent:

1. **TX Queue Pressure**
   - Check "[MIOS Stats]" in TeraTerm
   - High TxQFull means queue overflow
   - Reduce message rate or increase queue size

2. **USB Bandwidth**
   - MIDI + CDC + Debug all share USB bandwidth
   - Try reducing debug message rate
   - Try stopping MIDI traffic

3. **Timing Issues**
   - 1-second boot delay means early messages lost
   - But heartbeat should appear after 10 seconds
   - If nothing after 30 seconds, something is broken

## Expected Performance

**Normal Operation:**
- Test messages: 5/5 sent successfully
- Continuous messages: Every 2 seconds in both terminals
- Heartbeat: Every 10 seconds
- Drop rate: <1%
- TX queue usage: <50% (< 32 packets)

**Under Load:**
- Some drops acceptable (rate limiting working)
- TX queue may reach 50-80% usage
- Messages delayed but not lost
- Heartbeat may be late but still appears

## Files Modified

- `App/tests/module_tests.c` - Added aggressive test messages
- `App/tests/test_debug.c` - MIOS debug routing (already present)
- `Services/mios32_query/mios32_query.c` - Debug message builder (already present)
- `Services/usb_midi/usb_midi.c` - TX queue and diagnostics (already present)

## Success Criteria

✅ Test messages 1-5 appear in MIOS Studio terminal  
✅ "[MIOS TEST]" messages appear every 2 seconds  
✅ Heartbeat appears every 10 seconds  
✅ TeraTerm shows all "sent successfully"  
✅ No "[MIOS ERROR]" about queue full  
✅ Drop rate < 5%  

---

**Author:** GitHub Copilot  
**Date:** 2026-01-28  
**Version:** 1.0
