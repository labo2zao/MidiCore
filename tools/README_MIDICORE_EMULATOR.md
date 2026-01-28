# MidiCore Emulator - MIOS Studio Terminal Tester

## Purpose

This Python script **EMULATES MidiCore** to test if MIOS Studio terminal is working correctly. Instead of testing the real firmware, this script acts as a virtual MidiCore device that sends test messages to MIOS Studio.

## Why Use This?

✅ Test if MIOS Studio terminal works without hardware  
✅ Isolate MIOS Studio issues from firmware issues  
✅ Quick verification before debugging firmware  
✅ Works on any computer with Python  

## Requirements

```bash
pip install python-rtmidi
```

**Plus you need a virtual MIDI port:**

- **Windows:** Install [loopMIDI](https://www.tobias-erichsen.de/software/loopmidi.html)
- **macOS:** Enable IAC Driver (Audio MIDI Setup → MIDI Studio → IAC Driver)
- **Linux:** `sudo modprobe snd-virmidi`

## Quick Start

### Step 1: Create Virtual MIDI Port

**Windows (loopMIDI):**
1. Download and install loopMIDI
2. Open loopMIDI
3. Click "+" to create a port (e.g., "loopMIDI Port")

**macOS (IAC Driver):**
1. Open "Audio MIDI Setup"
2. Window → Show MIDI Studio
3. Double-click "IAC Driver"
4. Check "Device is online"

**Linux:**
```bash
sudo modprobe snd-virmidi
# Creates virtual MIDI ports
```

### Step 2: Start Emulator

```bash
python tools/midicore_emulator.py
```

Or specify port:
```bash
python tools/midicore_emulator.py --port "loopMIDI"
```

Output:
```
✓ MidiCore Emulator started on: loopMIDI Port

Connect MIOS Studio to this port and open Terminal window.
You should see test messages appearing in MIOS Studio terminal.

============================================================
STARTING TEST SEQUENCE
============================================================
Sending test messages to MIOS Studio terminal...
Watch MIOS Studio Terminal window for these messages:
============================================================

[SENT] *** MidiCore Emulator Started ***
[SENT] MIOS Studio Terminal Test
[SENT] If you see this, terminal is WORKING!
[SENT] Test Message #1
[SENT] Test Message #2
[SENT] Test Message #3
[SENT] Test Message #4
[SENT] Test Message #5
[SENT] All test messages sent!

Initial test sequence complete!
Sent 11 messages.

Now sending periodic heartbeat messages every 5 seconds...
Press Ctrl+C to stop.

[SENT] [Heartbeat #1] Terminal alive, sent 12 messages
```

### Step 3: Connect MIOS Studio

1. Open MIOS Studio
2. Device should appear in device list (as "loopMIDI Port" or "IAC Driver")
3. Select the device
4. Open Terminal window: **View → Terminal**

### Step 4: Check Result

**✅ If Terminal WORKS:**
```
*** MidiCore Emulator Started ***
MIOS Studio Terminal Test
If you see this, terminal is WORKING!

Test Message #1
Test Message #2
Test Message #3
Test Message #4
Test Message #5

All test messages sent!
If you see all 5 messages above, terminal is working perfectly.

[Heartbeat #1] Terminal alive, sent 12 messages
[Heartbeat #2] Terminal alive, sent 13 messages
```

**❌ If Terminal BLANK:**
- Messages show in Python script output
- But nothing appears in MIOS Studio terminal
- This means MIOS Studio has a problem, NOT your firmware!

## Command Line Options

```bash
# List available MIDI ports
python tools/midicore_emulator.py --list

# Use specific port
python tools/midicore_emulator.py --port "loopMIDI"

# Verbose mode (shows raw MIDI)
python tools/midicore_emulator.py --verbose

# Help
python tools/midicore_emulator.py --help
```

## Features

### Automatic Query Responses

When MIOS Studio queries the device, the emulator responds automatically:

```
[QUERY] Type=0x01 (OS)          → Responds: "MIOS32"
[QUERY] Type=0x02 (Board)       → Responds: "STM32F407VGT6"
[QUERY] Type=0x08 (App Name 1)  → Responds: "MidiCore"
[QUERY] Type=0x09 (App Name 2)  → Responds: "v1.0"
```

This makes the emulator appear as a real MidiCore device to MIOS Studio.

### Test Message Sequence

1. **Initial Banner** - Shows emulator started
2. **5 Numbered Messages** - Easy to count and verify
3. **Periodic Heartbeats** - Shows terminal is continuously working

### Verbose Debug Mode

```bash
python tools/midicore_emulator.py --verbose
```

Shows raw MIDI data:
```
[TX DEBUG] '*** MidiCore Emulator Started ***\r\n'
[TX] F0 00 00 7E 32 00 0D 2A 2A 2A 20 4D 69 64 69 43 6F 72 65 ... F7
```

## Troubleshooting

### "No MIDI ports found"

**Problem:** No virtual MIDI ports available

**Solutions:**
- Windows: Install loopMIDI
- macOS: Enable IAC Driver
- Linux: Run `sudo modprobe snd-virmidi`

### "MIOS Studio doesn't show device"

**Problem:** MIOS Studio can't see virtual port

**Solutions:**
- Restart MIOS Studio after creating virtual port
- Check MIOS Studio MIDI settings
- Try using a different virtual MIDI tool

### "Terminal shows nothing"

**Problem:** This is what we're testing!

**If Python shows messages sent but MIOS Studio terminal is blank:**
→ MIOS Studio terminal has a problem (not your firmware)

**Solutions:**
- Update MIOS Studio (need v2.4+)
- Check Terminal window is actually open
- Check device is selected in terminal
- Try View → Terminal to reopen window

### "Permission denied" (Linux)

```bash
sudo usermod -a -G dialout $USER
# Log out and back in
```

## Interpreting Results

### Scenario 1: Terminal Shows Messages ✅

**Result:** MIOS Studio terminal is WORKING perfectly!

**Conclusion:** The problem is in your MidiCore firmware, not MIOS Studio.

**Next Steps:**
- Check firmware USB MIDI initialization
- Check mios32_debug_send_message() is being called
- Check TX queue isn't full
- Use the firmware debug tool we created

### Scenario 2: Terminal is Blank ❌

**Result:** MIOS Studio terminal is NOT working!

**Conclusion:** MIOS Studio has a problem, not your firmware.

**Next Steps:**
- Update MIOS Studio to latest version
- Try different computer
- Try different virtual MIDI tool
- Contact MIOS Studio developers

### Scenario 3: Some Messages Appear ⚠️

**Result:** Terminal is partially working

**Conclusion:** Possible timing or buffering issue in MIOS Studio.

**Next Steps:**
- Try slower message rate
- Check for SysEx filtering in MIOS Studio
- Check message format is correct

## Comparison with Real Firmware

| Feature | Real MidiCore | This Emulator |
|---------|---------------|---------------|
| MIOS32 Queries | ✅ | ✅ |
| Debug Messages | ✅ | ✅ |
| Query Responses | ✅ | ✅ |
| Message Format | ✅ | ✅ |
| Continuous Output | ✅ | ✅ |
| Hardware I/O | ✅ | ❌ |
| MIDI Routing | ✅ | ❌ |

The emulator provides everything needed to test MIOS Studio terminal!

## Advanced Usage

### Custom Messages

Edit the script to send custom messages:

```python
# In run_test_sequence(), add:
self.send_debug_message("Your custom test message\r\n")
```

### Change Heartbeat Rate

```python
# In run(), change:
time.sleep(5)  # Change to 10 for 10-second interval
```

### Add More Device Info

```python
# In __init__, modify device_info dict:
self.device_info = {
    0x08: "YourAppName",
    0x09: "v2.0",
}
```

## Files

- `midicore_emulator.py` - Main emulator script (600+ lines)
- `README_MIDICORE_EMULATOR.md` - This documentation

## Support

If terminal works with emulator but not real firmware:
→ Check firmware USB MIDI implementation
→ Use the debug tool we created earlier

If terminal doesn't work with emulator:
→ MIOS Studio issue, not firmware
→ Try different MIOS Studio version
→ Contact MIOS Studio support

---

**Author:** GitHub Copilot  
**Date:** 2026-01-28  
**Version:** 1.0
