# MIOS Studio Terminal Debugging - Complete Guide

## Problem
MIOS Studio recognizes device but terminal shows nothing.

## Solution: 3-Part Diagnostic System

We've created three tools to diagnose and fix this issue:

### 1. Firmware Debug Tool (Built-in)
### 2. MidiCore Emulator (Python)
### 3. Firmware Test Code

---

## Tool 1: Firmware Debug Tool (IN FIRMWARE)

**Purpose:** Diagnose why real firmware isn't sending to MIOS Studio

**Location:** Built into firmware test code

**When to use:** When testing real MidiCore hardware

**What it does:**
- Reports USB MIDI status (Ready, Queue size, Drops)
- Sends 5 numbered test messages
- Reports success/failure for each message
- Shows queue usage
- All diagnostics go to TeraTerm (CDC)

**How to use:**
1. Flash firmware to MidiCore
2. Open TeraTerm on USB CDC port
3. Watch for debug output
4. Open MIOS Studio terminal
5. Compare what TeraTerm reports vs. what MIOS Studio shows

**Expected Output in TeraTerm:**
```
========================================
MIOS STUDIO TERMINAL DEBUG TOOL
========================================
USB MIDI Status:
  Ready: YES
  TX Queue Size: 64 packets
  TX Queue Used: 3 packets
  TX Queue Drops: 0 packets

[CDC] Test #1: SENT SUCCESSFULLY to MIOS Terminal
      Queue: 8/64 used, 0 drops
[CDC] Test #2: SENT SUCCESSFULLY to MIOS Terminal
      Queue: 7/64 used, 0 drops
...
```

**If all tests succeed but MIOS Studio terminal blank:**
→ Messages ARE being sent correctly
→ Problem is with MIOS Studio (use Tool 2 to confirm)

---

## Tool 2: MidiCore Emulator (PYTHON)

**Purpose:** Test if MIOS Studio terminal works WITHOUT real hardware

**Location:** `tools/midicore_emulator.py`

**When to use:** Before debugging firmware, to isolate MIOS Studio issues

**What it does:**
- Emulates MidiCore device
- Sends test messages to MIOS Studio
- Proves if MIOS Studio terminal is working
- No hardware needed!

**Setup:**
```bash
# 1. Install Python library
pip install python-rtmidi

# 2. Create virtual MIDI port:
#    Windows: Install loopMIDI
#    macOS: Enable IAC Driver
#    Linux: sudo modprobe snd-virmidi

# 3. Run emulator
python tools/midicore_emulator.py

# 4. Connect MIOS Studio to virtual port
# 5. Open MIOS Studio Terminal window
```

**Expected Output in Python:**
```
✓ MidiCore Emulator started on: loopMIDI Port

[SENT] *** MidiCore Emulator Started ***
[SENT] Test Message #1
[SENT] Test Message #2
[SENT] Test Message #3
[SENT] Test Message #4
[SENT] Test Message #5
```

**Expected Output in MIOS Studio Terminal:**
```
*** MidiCore Emulator Started ***
Test Message #1
Test Message #2
Test Message #3
Test Message #4
Test Message #5
```

**If terminal shows messages:**
→ MIOS Studio terminal WORKS
→ Problem is in your firmware (use Tool 1)

**If terminal is blank:**
→ MIOS Studio terminal BROKEN
→ Not a firmware issue!
→ Update MIOS Studio or contact support

---

## Tool 3: Firmware Test Code

**Purpose:** Continuous monitoring and test messages

**Location:** `App/tests/module_tests.c`

**What it does:**
- Sends test messages on boot
- Periodic heartbeat every 10 seconds
- Test messages every 2 seconds during MIDI test
- Statistics every 30 seconds

**Already built into firmware - just flash and run!**

---

## Diagnostic Flow Chart

```
START: Is MIOS Studio terminal blank?
  ↓
  YES
  ↓
Step 1: Run MidiCore Emulator (Python)
  ↓
  Does MIOS Studio terminal show messages?
  ↓
  ├─ YES → MIOS Studio WORKS
  │        Problem is in FIRMWARE
  │        ↓
  │        Step 2: Flash firmware with debug tool
  │        Open TeraTerm
  │        ↓
  │        Do test messages report "SENT SUCCESSFULLY"?
  │        ↓
  │        ├─ YES → Messages sent but not received
  │        │        Check MIOS Studio configuration:
  │        │        - Terminal window open?
  │        │        - Correct device selected?
  │        │        - SysEx not filtered?
  │        │
  │        └─ NO → Messages failing to send
  │                 Check TeraTerm diagnostics:
  │                 - USB MIDI Ready?
  │                 - TX Queue full?
  │                 - Drops counter increasing?
  │
  └─ NO → MIOS Studio BROKEN
           Not a firmware issue!
           - Update MIOS Studio
           - Try different computer
           - Contact MIOS Studio support
```

---

## Quick Reference

### Test MIOS Studio (No Hardware)
```bash
pip install python-rtmidi
python tools/midicore_emulator.py
# Connect MIOS Studio to virtual port
# Open Terminal window
# Watch for messages
```

### Test Firmware (With Hardware)
```bash
# Flash firmware
# Open TeraTerm on USB CDC
# Open MIOS Studio terminal
# Compare outputs
```

### Check Diagnostics
```
TeraTerm will show:
- USB MIDI status
- Test message results
- Queue usage
- Success/failure

MIOS Studio will show:
- The actual messages (if working)
- Nothing (if broken)
```

---

## Files Reference

| File | Purpose | When to Use |
|------|---------|-------------|
| `App/tests/module_tests.c` | Firmware debug tool | Testing real hardware |
| `App/tests/test_debug.c` | Debug message routing | Already built-in |
| `tools/midicore_emulator.py` | Virtual MidiCore | Testing MIOS Studio |
| `tools/README_MIDICORE_EMULATOR.md` | Emulator guide | Setup instructions |
| `MIOS_STUDIO_TERMINAL_DEBUG_TOOL_GUIDE.md` | Firmware tool guide | Interpret diagnostics |

---

## Common Scenarios

### Scenario A: Emulator Works, Firmware Doesn't

**Diagnosis:** MIOS Studio is fine, firmware has issues

**Solution:**
1. Check TeraTerm output from firmware
2. Look for "FAILED to send" messages
3. Check USB MIDI status
4. Check TX queue drops

### Scenario B: Neither Works

**Diagnosis:** MIOS Studio terminal broken

**Solution:**
1. Update MIOS Studio (need v2.4+)
2. Try different computer
3. Check MIOS Studio settings
4. Contact MIOS Studio support

### Scenario C: Both Work

**Diagnosis:** Everything is working!

**Conclusion:** The issue was temporary or configuration-related

---

## Support

If you need help:

**For emulator issues:**
- Check virtual MIDI port is created
- Run with `--verbose` flag
- Check Python and library versions

**For firmware issues:**
- Share TeraTerm debug output
- Share MIOS Studio terminal status
- Include USB MIDI status from diagnostics

**For MIOS Studio issues:**
- Verify with emulator first
- Check MIOS Studio version
- Try different computer

---

**Created:** 2026-01-28  
**Author:** GitHub Copilot  
**Version:** 1.0
