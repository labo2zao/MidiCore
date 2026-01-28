# MidiCore Emulator - Quick Start Guide for Windows/loopMIDI

## You Have loopMIDI Installed - Perfect! ✅

The emulator will create a virtual MIDI port that MIOS Studio can discover.

## Quick Start (5 Steps)

### Step 1: Run the Emulator

```bash
python tools/midicore_emulator.py
```

You'll see:
```
✓ MidiCore Emulator started
✓ Created virtual MIDI port: 'MidiCore Emulator'

⏳ Waiting 5 seconds for you to connect MIOS Studio...
   Starting in 5 seconds...
   Starting in 4 seconds...
   ...
```

### Step 2: Open MIOS Studio (During Countdown)

1. Open MIOS Studio
2. Look for **"MidiCore Emulator"** in device list
3. Select it
4. Click **"Query"** button

### Step 3: Open Terminal Window

View → Terminal (or Ctrl+T)

### Step 4: Watch Messages Flow!

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
