# MidiCore Emulator - Quick Start Guide for Windows/loopMIDI

## GREAT NEWS: Auto-Detection on Windows! ✅

The emulator now **auto-detects Windows** and automatically finds loopMIDI. Just run it!

```bash
python midicore_emulator.py
```

No need to type `--use-existing` anymore!

## Quick Start (Windows with loopMIDI)

### Step 1: Create loopMIDI Port

1. **Open loopMIDI** application
2. **Click '+' button** to create a new port
3. **Name it** (default "loopMIDI Port" is fine)
4. **Keep loopMIDI running**

### Step 2: Run Emulator (Auto-Detection!)

```bash
python tools/midicore_emulator.py
```

You'll see:
```
✓ Detected Windows - searching for loopMIDI port...
✓ Found: loopMIDI Port
✓ Connected successfully!

⏳ Waiting 5 seconds for you to connect MIOS Studio...
   Starting in 5 seconds...
   Starting in 4 seconds...
   ...
```

### Step 3: Open MIOS Studio (During Countdown)

1. Open MIOS Studio
2. Look for **"loopMIDI Port"** in device list
3. Select it
4. Click **"Query"** button

### Step 4: Open Terminal Window

View → Terminal (or Ctrl+T)

### Step 5: Watch Messages Flow!

**Initial test (5 numbered messages):**
```
*** MidiCore Emulator Started ***
Test Message #1
Test Message #2
Test Message #3
Test Message #4
Test Message #5
```

**Then continuous messages every 2 seconds:**
```
[Test #1] MIOS Studio terminal receiving messages OK
[Test #2] Heartbeat - 15 messages sent
[Test #3] Continuous terminal test - all working!
[Test #4] MIOS Studio terminal receiving messages OK
[Test #5] Heartbeat - 19 messages sent
[Test #6] Continuous terminal test - all working!
...
```

### Step 6: Stop with Ctrl+C

Press **Ctrl+C** in Python console when done.

## Command Line Options

```bash
# Simple (auto-detects loopMIDI on Windows)
python tools/midicore_emulator.py

# Use specific port (override auto-detection)
python tools/midicore_emulator.py --use-existing "My Port Name"

# List available MIDI ports
python tools/midicore_emulator.py --list

# Verbose mode (shows raw MIDI)
python tools/midicore_emulator.py --verbose
```

## What You'll See

### Python Console:
```
✓ Detected Windows - searching for loopMIDI port...
✓ Found: loopMIDI Port
✓ Connected successfully!
✓ Using existing MIDI port: 'loopMIDI Port'

[SENT] [Test #1] MIOS Studio terminal receiving messages OK
[SENT] [Test #2] Heartbeat - 15 messages sent
[SENT] [Test #3] Continuous terminal test - all working!
```

### MIOS Studio Terminal:
```
[Test #1] MIOS Studio terminal receiving messages OK
[Test #2] Heartbeat - 15 messages sent
[Test #3] Continuous terminal test - all working!
```

## Interpreting Results

### ✅ If You See Messages in MIOS Studio Terminal

**Conclusion:** MIOS Studio terminal is WORKING perfectly!

**Next Step:** The problem is in your MidiCore firmware
- Check TeraTerm output from firmware debug tool
- Look for "SENT SUCCESSFULLY" or "FAILED" messages
- Check USB MIDI status (Ready, Queue full, etc.)

### ❌ If MIOS Studio Terminal is Blank

**Conclusion:** MIOS Studio configuration issue

**Try:**
1. Make sure Terminal window is open (View → Terminal)
2. Make sure "loopMIDI Port" device is selected
3. Click "Query" button again
4. Close and reopen terminal window
5. Restart MIOS Studio

### ⚠️ If Some Messages Missing

**Conclusion:** Timing or buffering issue

**Try:**
- Messages are sent every 2 seconds - give it time
- Check if any appear after waiting 30+ seconds
- Try verbose mode to see raw MIDI

## Troubleshooting

### "No module named 'rtmidi'"

```bash
pip install python-rtmidi
```

### "ERROR: No MIDI ports found"

**Problem:** No loopMIDI port created

**Solution:**
1. Open loopMIDI application
2. Click '+' to create a port
3. Make sure it's enabled (green checkmark)
4. Run emulator again

### "ERROR: Port matching 'loopMIDI' not found"

**Problem:** Port name doesn't match

**Solutions:**
1. List available ports:
   ```bash
   python tools/midicore_emulator.py --list
   ```
2. Use exact port name from list
3. Or use partial match (case-insensitive)

### "Device doesn't appear in MIOS Studio"

1. Make sure emulator is running (countdown should be visible)
2. Make sure loopMIDI application is running
3. Refresh MIOS Studio device list
4. Restart MIOS Studio
5. Select "loopMIDI Port" (not "MidiCore Emulator")

## Expected Timeline

```
00:00 - Start emulator with --use-existing
00:01 - "Waiting 5 seconds..." appears
00:01 - Open MIOS Studio (you have 5 seconds)
00:02 - Select "loopMIDI Port" device
00:03 - Click "Query"
00:04 - Open Terminal window
00:05 - Countdown ends
00:06 - Initial test messages appear
00:08 - Continuous messages start (every 2 seconds)
00:10 - [Test #1] appears
00:12 - [Test #2] appears
00:14 - [Test #3] appears
...   - Continue until you press Ctrl+C
```

## Success Criteria

✅ loopMIDI port created and enabled  
✅ Emulator connects to port successfully  
✅ Countdown appears in Python console  
✅ "loopMIDI Port" appears in MIOS Studio device list  
✅ Query succeeds (device info shown)  
✅ Terminal window opens  
✅ 5 initial test messages appear  
✅ Continuous messages flow every 2 seconds  

If ALL of these work → MIOS Studio terminal is fine, check firmware!

## Why --use-existing is Required on Windows

**Technical Reason:**
- macOS/Linux support creating virtual MIDI ports with `open_virtual_port()`
- Windows python-rtmidi doesn't support this API
- Windows requires connecting to ports created by MIDI drivers (like loopMIDI)
- That's why we need `--use-existing` option

**What This Means:**
- You CANNOT just run `python midicore_emulator.py` on Windows
- You MUST create a loopMIDI port first
- Then use `--use-existing "loopMIDI"` to connect to it

## Notes

- **Always use --use-existing on Windows**
- **loopMIDI must be running** while emulator runs
- **Port name is case-insensitive** ("loopmidi" works too)
- **Partial match works** ("loop" will find "loopMIDI Port")
- **Use --list** to see exact port names

---

**Version:** 2.1 (Windows fix)
**Platform:** Windows with loopMIDI  
**Date:** 2026-01-28


**Initial test (5 numbered messages):**
```
*** MidiCore Emulator Started ***
Test Message #1
Test Message #2
Test Message #3
Test Message #4
Test Message #5
```

**Then continuous messages every 2 seconds:**
```
[Test #1] MIOS Studio terminal receiving messages OK
[Test #2] Heartbeat - 15 messages sent
[Test #3] Continuous terminal test - all working!
[Test #4] MIOS Studio terminal receiving messages OK
[Test #5] Heartbeat - 19 messages sent
[Test #6] Continuous terminal test - all working!
...
```

### Step 5: Stop with Ctrl+C

Press **Ctrl+C** in Python console when done.

## What You'll See

### Python Console:
```
[SENT] [Test #1] MIOS Studio terminal receiving messages OK
[SENT] [Test #2] Heartbeat - 15 messages sent
[SENT] [Test #3] Continuous terminal test - all working!
```

### MIOS Studio Terminal:
```
[Test #1] MIOS Studio terminal receiving messages OK
[Test #2] Heartbeat - 15 messages sent
[Test #3] Continuous terminal test - all working!
```

## Interpreting Results

### ✅ If You See Messages in MIOS Studio Terminal

**Conclusion:** MIOS Studio terminal is WORKING perfectly!

**Next Step:** The problem is in your MidiCore firmware
- Check TeraTerm output from firmware debug tool
- Look for "SENT SUCCESSFULLY" or "FAILED" messages
- Check USB MIDI status (Ready, Queue full, etc.)

### ❌ If MIOS Studio Terminal is Blank

**Conclusion:** MIOS Studio configuration issue

**Try:**
1. Make sure Terminal window is open (View → Terminal)
2. Make sure "MidiCore Emulator" device is selected
3. Click "Query" button again
4. Close and reopen terminal window
5. Restart MIOS Studio

### ⚠️ If Some Messages Missing

**Conclusion:** Timing or buffering issue

**Try:**
- Messages are sent every 2 seconds - give it time
- Check if any appear after waiting 30+ seconds
- Try verbose mode to see raw MIDI

## Troubleshooting

### "No module named 'rtmidi'"

```bash
pip install python-rtmidi
```

### "ERROR starting emulator"

With loopMIDI installed, this should work. Try:
1. Restart loopMIDI service
2. Create a port manually in loopMIDI first
3. Run as administrator

### "Device doesn't appear in MIOS Studio"

1. Make sure emulator is running (countdown should be visible)
2. Refresh MIOS Studio device list
3. Restart MIOS Studio
4. Check Windows MIDI settings

## Command Options

```bash
# Default
python tools/midicore_emulator.py

# Custom name
python tools/midicore_emulator.py --name "Test Device"

# Verbose (see raw MIDI)
python tools/midicore_emulator.py --verbose
```

## Expected Timeline

```
00:00 - Start emulator
00:01 - "Waiting 5 seconds..." appears
00:01 - Open MIOS Studio (you have 5 seconds)
00:02 - Select "MidiCore Emulator" device
00:03 - Click "Query"
00:04 - Open Terminal window
00:05 - Countdown ends
00:06 - Initial test messages appear
00:08 - Continuous messages start (every 2 seconds)
00:10 - [Test #1] appears
00:12 - [Test #2] appears
00:14 - [Test #3] appears
...   - Continue until you press Ctrl+C
```

## Success Criteria

✅ Countdown appears in Python console  
✅ "MidiCore Emulator" appears in MIOS Studio device list  
✅ Query succeeds (device info shown)  
✅ Terminal window opens  
✅ 5 initial test messages appear  
✅ Continuous messages flow every 2 seconds  

If ALL of these work → MIOS Studio terminal is fine, check firmware!

## Notes

- **Continuous messages** make it easy to see terminal is working
- **Every 2 seconds** is frequent enough to verify but not spam
- **Numbered messages** help track if any are missing
- **Three message types** show variety of content
- **Press Ctrl+C anytime** to stop

---

**Version:** 2.0  
**Platform:** Windows with loopMIDI  
**Date:** 2026-01-28
