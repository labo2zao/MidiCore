# MIOS Studio Terminal Debug Tool - User Guide

## Quick Start

Flash the firmware and open TeraTerm on the USB CDC port. You'll immediately see:

```
========================================
MIOS STUDIO TERMINAL DEBUG TOOL
========================================
```

This debug tool will tell you EXACTLY why MIOS Studio terminal is blank.

## Reading the Diagnostics

### 1. USB MIDI Status Check

```
USB MIDI Status:
  Ready: YES
  TX Queue Size: 64 packets
  TX Queue Used: 5 packets
  TX Queue Drops: 0 packets
```

**Interpretation:**

| Ready | Queue Used | Drops | Meaning |
|-------|------------|-------|---------|
| YES | Low (<30) | 0 | ✅ Everything normal |
| YES | High (>50) | 0 | ⚠️ Queue pressure, but working |
| YES | Any | >0 | ❌ Queue overflow, messages being dropped |
| NO | Any | Any | ❌ MIDI not initialized, nothing will work |

**If Ready = NO:**
- USB MIDI interface not initialized
- Check USB enumeration
- Check `usb_midi_init()` was called
- Check composite device initialization

### 2. Test Message Results

```
Sending 5 test messages to MIOS Studio terminal...

[CDC] Test #1: SENT SUCCESSFULLY to MIOS Terminal
      Queue: 8/64 used, 0 drops
[CDC] Test #2: SENT SUCCESSFULLY to MIOS Terminal
      Queue: 7/64 used, 0 drops
[CDC] Test #3: SENT SUCCESSFULLY to MIOS Terminal
      Queue: 6/64 used, 0 drops
[CDC] Test #4: SENT SUCCESSFULLY to MIOS Terminal
      Queue: 5/64 used, 0 drops
[CDC] Test #5: SENT SUCCESSFULLY to MIOS Terminal
      Queue: 4/64 used, 0 drops
```

**What This Means:**
- Messages ARE being sent via USB MIDI
- They're in the correct MIOS32 SysEx format
- TX queue is working
- If you DON'T see them in MIOS Studio → MIOS Studio issue

**If You See FAILED:**
```
[CDC] Test #1: FAILED - TX queue full or MIDI not ready
      Queue: 64/64 used, 15 drops
```

**What This Means:**
- TX queue is completely full
- Messages can't be sent
- Either too much MIDI traffic or queue not draining
- Check for TX completion callback issues

### 3. Continuous Monitoring

Every 30 seconds you'll see:

```
[MIOS STATS] MIDI Ready:YES Queue:5/64 Drops:0
```

**Watch for:**
- Drops counter increasing → Queue overflow
- Queue constantly high (>50) → Bandwidth saturation
- Ready changing to NO → USB disconnection/reset

## Diagnostic Scenarios

### Scenario A: All Tests Send Successfully ✅

**TeraTerm shows:**
```
[CDC] Test #1: SENT SUCCESSFULLY to MIOS Terminal
[CDC] Test #2: SENT SUCCESSFULLY to MIOS Terminal
[CDC] Test #3: SENT SUCCESSFULLY to MIOS Terminal
[CDC] Test #4: SENT SUCCESSFULLY to MIOS Terminal
[CDC] Test #5: SENT SUCCESSFULLY to MIOS Terminal
```

**MIOS Studio Terminal shows:**
```
Nothing (blank)
```

**Diagnosis:** Messages are being sent correctly but MIOS Studio isn't receiving them.

**Possible Causes:**
1. **MIOS Studio Terminal Not Open**
   - Open: View → Terminal
   - Make sure terminal window is visible

2. **Terminal Not Connected to Device**
   - In terminal window, check device dropdown
   - Select "MidiCore" or your device name
   - Try disconnect/reconnect

3. **Wrong MIDI Port Selected**
   - MIOS Studio might be connected to wrong port
   - Try different USB MIDI port in settings

4. **MIOS Studio Version Issue**
   - Need MIOS Studio 2.4 or later
   - Older versions may not handle terminal correctly
   - Download latest from midibox.org

5. **SysEx Filter Active**
   - Check MIOS Studio settings
   - Make sure SysEx messages aren't filtered
   - Debug messages are SysEx command 0x0D

6. **USB Cable Issue**
   - Try different USB cable
   - Try different USB port on computer
   - Some cables don't handle high-speed MIDI well

### Scenario B: Tests Fail to Send ❌

**TeraTerm shows:**
```
USB MIDI Status:
  Ready: YES
  TX Queue Size: 64 packets
  TX Queue Used: 64 packets
  TX Queue Drops: 127 packets

[CDC] Test #1: FAILED - TX queue full or MIDI not ready
      Queue: 64/64 used, 128 drops
```

**Diagnosis:** TX queue is completely full, can't send messages.

**Solution:**
1. **Reduce Debug Output Rate**
   - Too many dbg_print() calls
   - Each call tries to send to MIOS terminal
   - Reduce frequency or disable MIOS debug temporarily

2. **Increase TX Queue Size**
   - Edit `USB_MIDI_TX_QUEUE_SIZE` in usb_midi.c
   - Current: 64 packets
   - Try: 128 packets
   - Rebuild firmware

3. **Check TX Completion Callback**
   - TX queue might not be draining
   - Verify `USBD_MIDI_DataIn()` is being called
   - Check `usb_midi_tx_complete()` is triggering

### Scenario C: MIDI Not Ready ❌

**TeraTerm shows:**
```
USB MIDI Status:
  Ready: NO
  TX Queue Size: 0 packets
  TX Queue Used: 0 packets
  TX Queue Drops: 0 packets

*** ERROR: USB MIDI interface not ready! ***
Check USB initialization and enumeration.
```

**Diagnosis:** USB MIDI interface not initialized.

**Solution:**
1. **Check USB Enumeration**
   - Device Manager (Windows) / lsusb (Linux)
   - Should show "MidiCore 4x4" USB MIDI device
   - If not visible, USB not enumerating

2. **Check Initialization Order**
   - `usb_midi_init()` must be called before `MX_USB_DEVICE_Init()`
   - Check main.c initialization sequence

3. **Check Composite Device**
   - With MIDI + CDC, need composite descriptor
   - Verify `usbd_composite.c` is being used
   - Check class data pointers are set correctly

### Scenario D: Intermittent Success ⚠️

**TeraTerm shows:**
```
[CDC] Test #1: SENT SUCCESSFULLY to MIOS Terminal
[CDC] Test #2: FAILED - TX queue full or MIDI not ready
[CDC] Test #3: SENT SUCCESSFULLY to MIOS Terminal
[CDC] Test #4: FAILED - TX queue full or MIDI not ready
[CDC] Test #5: SENT SUCCESSFULLY to MIOS Terminal
```

**Diagnosis:** Queue pressure, some messages drop.

**Solution:**
1. **Increase Delay Between Messages**
   - Currently 100ms between test messages
   - Try 200ms or 500ms
   - Gives queue time to drain

2. **Reduce Background Traffic**
   - Disable other MIDI output temporarily
   - Stop test Note On/Off messages
   - Reduces queue pressure

3. **Check for Queue Leaks**
   - Monitor drops counter
   - Should not increase rapidly
   - If constantly increasing → queue drain issue

## What To Report Back

When testing, please report:

1. **USB MIDI Status:**
   - Ready: YES/NO
   - Queue size/used/drops

2. **Test Results:**
   - How many sent successfully?
   - How many failed?
   - Queue status after each test

3. **MIOS Studio Terminal:**
   - Do you see ANY of the 5 test messages?
   - Do you see them all?
   - Do they appear correctly formatted?

4. **MIOS Stats (after 30+ seconds):**
   - Is drops counter increasing?
   - What's typical queue usage?

## Understanding MIOS32 Debug Format

Messages sent to MIOS Studio terminal use this SysEx format:

```
F0 00 00 7E 32 00 0D <ascii_text> F7

Where:
  F0         = SysEx start
  00 00 7E   = MIOS32 manufacturer ID
  32         = MIOS32 device ID
  00         = Device instance (first device)
  0D         = Debug message command
  <text>     = ASCII text (variable length)
  F7         = SysEx end
```

If MIOS Studio isn't showing messages, either:
1. Messages aren't being sent (diagnostics will show FAILED)
2. Messages are sent but MIOS Studio doesn't recognize format
3. Messages are sent but MIOS Studio terminal not configured

## Quick Troubleshooting Checklist

- [ ] USB MIDI Status shows "Ready: YES"
- [ ] TX Queue Drops counter is 0 or low
- [ ] All 5 test messages send successfully
- [ ] MIOS Studio terminal window is open
- [ ] Terminal connected to correct device
- [ ] MIOS Studio version 2.4 or later
- [ ] SysEx not filtered in MIOS Studio settings
- [ ] USB cable is good quality
- [ ] Tried different USB port

## Expected Good Output

```
========================================
MIOS STUDIO TERMINAL DEBUG TOOL
========================================
USB MIDI Status:
  Ready: YES
  TX Queue Size: 64 packets
  TX Queue Used: 3 packets
  TX Queue Drops: 0 packets

Sending 5 test messages to MIOS Studio terminal...
(Watch for these in MIOS Studio Terminal window)

[CDC] Test #1: SENT SUCCESSFULLY to MIOS Terminal
      Queue: 8/64 used, 0 drops
[CDC] Test #2: SENT SUCCESSFULLY to MIOS Terminal
      Queue: 7/64 used, 0 drops
[CDC] Test #3: SENT SUCCESSFULLY to MIOS Terminal
      Queue: 6/64 used, 0 drops
[CDC] Test #4: SENT SUCCESSFULLY to MIOS Terminal
      Queue: 5/64 used, 0 drops
[CDC] Test #5: SENT SUCCESSFULLY to MIOS Terminal
      Queue: 4/64 used, 0 drops

Test complete. Check MIOS Studio Terminal window.
If you see the test messages there, terminal is WORKING.
If not, messages are being sent but not received.
========================================
```

And in MIOS Studio Terminal window:
```
*** MIOS Terminal Test #1 ***
*** MIOS Terminal Test #2 ***
*** MIOS Terminal Test #3 ***
*** MIOS Terminal Test #4 ***
*** MIOS Terminal Test #5 ***
```

---

**Author:** GitHub Copilot  
**Date:** 2026-01-28  
**Version:** 1.0
