# MidiCore Emulator - MIOS Studio Terminal Tester

## Purpose

This Python script **EMULATES MidiCore** and **creates a virtual MIDI port** that MIOS Studio can connect to. This tests if MIOS Studio terminal is working correctly without needing actual hardware.

## Key Features

✅ **Creates its own virtual MIDI port** - No manual setup needed!  
✅ **5-second delay** - Time to connect MIOS Studio before messages start  
✅ **Automatic countdown** - Shows when messages will start  
✅ **Works like real hardware** - MIOS Studio sees it as a device  

## Requirements

```bash
pip install python-rtmidi
```

**Platform Support:**
- **macOS:** Works natively ✅
- **Linux:** Works natively ✅  
- **Windows:** May need virtual MIDI driver (loopMIDI recommended)

## Quick Start

### Step 1: Start Emulator

```bash
python tools/midicore_emulator.py
```

Output:
```
✓ MidiCore Emulator started
✓ Created virtual MIDI port: 'MidiCore Emulator'

In MIOS Studio:
  1. Device 'MidiCore Emulator' should appear in device list
  2. Select it and click 'Query'
  3. Open Terminal window (View → Terminal)
  4. You should see test messages!

============================================================
STARTING TEST SEQUENCE
============================================================

⏳ Waiting 5 seconds for you to connect MIOS Studio...
   1. Open MIOS Studio
   2. Select 'MidiCore Emulator' device
   3. Click 'Query' button
   4. Open Terminal window (View → Terminal)

   Starting in 5 seconds...
   Starting in 4 seconds...
   Starting in 3 seconds...
   ...
```

### Step 2: Connect MIOS Studio

During the 5-second countdown:

1. **Open MIOS Studio**
2. **Look for "MidiCore Emulator"** in device list
3. **Select it** and click **"Query"** button
4. **Open Terminal window:** View → Terminal

### Step 3: Watch Messages Appear

After countdown, MIOS Studio Terminal should show:
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

### Step 4: Interpret Results

**✅ If Messages Appear in Terminal:**
- MIOS Studio terminal is WORKING perfectly!
- The problem is in your MidiCore firmware
- Use the firmware debug tool to diagnose

**❌ If Terminal is Blank:**
- MIOS Studio terminal has an issue
- Not a firmware problem
- Try updating MIOS Studio or contact support

## Command Line Options

```bash
# Default - creates port named "MidiCore Emulator"
python tools/midicore_emulator.py

# Custom device name
python tools/midicore_emulator.py --name "My Test Device"

# Verbose mode (shows raw MIDI messages)
python tools/midicore_emulator.py --verbose

# Help
python tools/midicore_emulator.py --help
```

## How It Works

1. **Creates Virtual MIDI Ports**
   - Input port: Receives queries from MIOS Studio
   - Output port: Sends responses and debug messages
   - Both named "MidiCore Emulator" (or custom name)

2. **Responds to Queries**
   - Like real MidiCore firmware
   - Returns device info, version, etc.

3. **Sends Debug Messages**
   - MIOS32 SysEx format (0x0D command)
   - Test messages to verify terminal

4. **5-Second Countdown**
   - Gives you time to connect
   - Clear instructions shown

## Verbose Mode

```bash
python tools/midicore_emulator.py --verbose
```

Shows raw MIDI data:
```
[RX] F0 00 00 7E 32 00 00 01 F7
[QUERY] Type=0x01 (OS)
[TX RESPONSE] MIOS32
[TX] F0 00 00 7E 32 00 0F 4D 49 4F 53 33 32 F7
[TX DEBUG] '*** MidiCore Emulator Started ***\r\n'
[TX] F0 00 00 7E 32 00 0D 2A 2A 2A ... F7
```

## Platform-Specific Notes

### macOS
Works out of the box! Virtual MIDI ports are natively supported.

### Linux
Works out of the box! ALSA supports virtual MIDI ports.

### Windows
May need a virtual MIDI driver like **loopMIDI**:
1. Download from: https://www.tobias-erichsen.de/software/loopmidi.html
2. Install loopMIDI
3. Run the emulator
4. Should work!

## Troubleshooting

### "ERROR starting emulator"

**Problem:** Virtual MIDI port creation failed

**Solutions:**
- **Windows:** Install loopMIDI driver
- **Linux:** Check ALSA is installed (`sudo apt-get install libasound2-dev`)
- **macOS:** Should work, check System Preferences → Security

### "Device doesn't appear in MIOS Studio"

**Problem:** MIOS Studio can't see virtual port

**Solutions:**
- Restart MIOS Studio after starting emulator
- Check MIOS Studio MIDI settings
- Try verbose mode to see if queries are received

### "Terminal shows nothing"

**Problem:** This is what we're testing!

**If Python shows messages sent but MIOS Studio terminal is blank:**
→ MIOS Studio terminal has a problem

**Solutions:**
- Update MIOS Studio (need v2.4+)
- Check Terminal window is open
- Check device is selected
- Try on different computer

## Comparison with Real Firmware

| Feature | Real MidiCore | Emulator |
|---------|---------------|----------|
| MIOS32 Queries | ✅ | ✅ |
| Debug Messages | ✅ | ✅ |
| Query Responses | ✅ | ✅ |
| Message Format | ✅ | ✅ |
| Heartbeat | ✅ | ✅ |
| 5s Delay | ❌ | ✅ NEW! |
| Virtual Port | ❌ | ✅ NEW! |
| Hardware I/O | ✅ | ❌ |

## What Changed (v2.0)

**OLD Version (v1.0):**
- Required manual virtual MIDI port setup (loopMIDI/IAC)
- Connected to existing port
- Started sending immediately
- Complex setup

**NEW Version (v2.0):**
- ✅ Creates virtual port automatically
- ✅ 5-second countdown before messages
- ✅ Simpler usage
- ✅ Better user experience

## Examples

### Basic Usage
```bash
$ python tools/midicore_emulator.py

✓ MidiCore Emulator started
✓ Created virtual MIDI port: 'MidiCore Emulator'
...
⏳ Waiting 5 seconds for you to connect MIOS Studio...
   Starting in 5 seconds...
```

### Custom Name
```bash
$ python tools/midicore_emulator.py --name "Test Device 123"

✓ MidiCore Emulator started
✓ Created virtual MIDI port: 'Test Device 123'
```

### Debug Mode
```bash
$ python tools/midicore_emulator.py --verbose

[TX DEBUG] '*** MidiCore Emulator Started ***\r\n'
[TX] F0 00 00 7E 32 00 0D 2A 2A 2A 20 4D 69 64 69 43 6F 72 65 ...
```

## Files

- `midicore_emulator.py` - Main emulator script
- `README_MIDICORE_EMULATOR.md` - This documentation

## Support

If terminal works with emulator but not real firmware:
→ Firmware issue - use firmware debug tool

If terminal doesn't work with emulator:
→ MIOS Studio issue - update or contact support

---

**Author:** GitHub Copilot  
**Date:** 2026-01-28  
**Version:** 2.0 (with auto virtual port + 5s delay)


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
