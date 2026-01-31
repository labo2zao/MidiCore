# CRITICAL: MIOS32 Queries Not Responding - Troubleshooting Guide

## Problem

The Python test script `test_mios32_recognition.py` finds the MidiCore device and opens MIDI ports successfully, but all 9 MIOS32 queries timeout with no response.

```
Testing Query Type 0x01: Operating System
  Sending: f0 00 00 7e 32 00 00 01 f7
✗ No response (timeout after 1.0s)
```

## Root Cause Analysis

The code in the repository is **CORRECT**:
- ✅ MIOS32 query handler exists (`Services/mios32_query/mios32_query.c`)
- ✅ USB MIDI integration correct (calls `mios32_query_process()` in 3 places)
- ✅ Query format parsing correct
- ✅ Response generation correct

**The issue is that the firmware ON THE DEVICE doesn't have this code!**

## Solution: Rebuild and Flash Firmware

The device is running old firmware without the MIOS32 query handler. You MUST:

1. **Clean build** the project
2. **Flash** the new firmware to the device
3. **Test** with the Python script

### Step-by-Step Fix

#### 1. Verify Module is Enabled

Check `Config/module_config.h`:
```c
#define MODULE_ENABLE_USB_MIDI 1  // Must be 1
```

#### 2. Clean Build in STM32CubeIDE

```
Right-click project → Clean Project
Wait for clean to complete

Project → Build Project (or Ctrl+B)
Wait for build (should see "0 errors")
```

**CRITICAL: Verify these files are compiled:**
```
Debug/Services/mios32_query/mios32_query.o  ← MUST exist
Debug/Services/usb_midi/usb_midi.o          ← MUST exist
Debug/Services/usb_midi/usb_midi_sysex.o    ← MUST exist
```

Check build output in Console tab for these lines:
```
Building file: ../Services/mios32_query/mios32_query.c
Finished building: ../Services/mios32_query/mios32_query.c
```

If you don't see `mios32_query.o` compiled, the module is not included in the build!

#### 3. Flash Firmware

```
Connect ST-Link debugger
Run → Debug (F11) or Run → Run (Ctrl+F11)
Wait for "Download verified successfully"
```

#### 4. Power Cycle Device

```
1. Disconnect USB cable
2. Wait 5 seconds
3. Reconnect USB cable
```

This ensures the device starts with the new firmware.

#### 5. Test with Python Script

```bash
cd Tools
python3 test_mios32_recognition.py
```

**Expected output:**
```
Testing Query Type 0x01: Operating System
  Sending: f0 00 00 7e 32 00 00 01 f7
  Received: f0 00 00 7e 32 00 0f 4d 49 4f 53 33 32 f7
✓ Valid response: "MIOS32"
```

## Diagnostic Tests

### Test 1: Verify Firmware Version

Run the new loopback test first:
```bash
cd Tools
python3 test_midi_loopback.py
```

This tests:
1. Simple SysEx (checks if USB MIDI RX/TX works at all)
2. Note On/Off (checks if MIDI routing works)
3. MIOS32 Query (checks if query handler is present)

**If Simple SysEx fails:** USB MIDI is not working (driver/hardware issue)
**If MIOS32 Query fails:** Firmware doesn't have query handler (need to flash)

### Test 2: Check Build Configuration

In STM32CubeIDE:
```
Right-click project → Properties
→ C/C++ Build → Settings
→ Tool Settings → MCU GCC Compiler → Include paths

Verify these are listed:
- ../Services/mios32_query
- ../Services/usb_midi
```

If not listed, the directories are not in the build path!

### Test 3: Check Source Files

Verify files exist:
```bash
ls Services/mios32_query/mios32_query.c
ls Services/mios32_query/mios32_query.h
```

Both should exist with recent modification dates.

### Test 4: Check Integration

Verify USB MIDI calls the query handler:
```bash
grep -n "mios32_query_process" Services/usb_midi/usb_midi.c
```

Should show 3 occurrences at lines ~315, ~358, ~400.

## Common Issues

### Issue 1: "mios32_query.o not found"

**Cause:** Module not added to build system

**Solution:**
1. In STM32CubeIDE Project Explorer
2. Find `Services/mios32_query/`
3. Right-click → Properties → C/C++ Build
4. Uncheck "Exclude resource from build"

### Issue 2: "Build succeeds but queries still fail"

**Cause:** Old firmware still on device (flash didn't work)

**Solution:**
1. In STM32CubeIDE, check Console output during flash
2. Look for "Download verified successfully"
3. If not seen, flash failed silently
4. Try: Run → Debug Configurations → Reload
5. Or manually erase flash first

### Issue 3: "undefined reference to usb_midi_send_sysex"

**Cause:** Linker can't find USB MIDI functions

**Solution:**
1. Verify `Services/usb_midi/usb_midi_sysex.c` is in build
2. Check `#if MODULE_ENABLE_USB_MIDI` in both files
3. Rebuild completely (Clean + Build)

### Issue 4: "Device appears but Python script finds wrong port"

**Cause:** Multiple MIDI devices, script picks wrong one

**Solution:**
1. When script shows port list, note the correct port numbers
2. Edit script if needed to manually select ports
3. Or unplug other MIDI devices during test

## Verification Checklist

Before testing:
- [ ] `MODULE_ENABLE_USB_MIDI = 1` in `Config/module_config.h`
- [ ] Clean build completed (0 errors)
- [ ] `mios32_query.o` exists in `Debug/Services/mios32_query/`
- [ ] `usb_midi.o` exists in `Debug/Services/usb_midi/`
- [ ] Flash succeeded ("Download verified successfully")
- [ ] Device power cycled (USB unplugged/replugged)
- [ ] Device appears in MIDI device list
- [ ] Correct MIDI ports selected in test script

If all checkboxes are checked and queries still fail:
1. Run `test_midi_loopback.py` to isolate issue
2. Check if USB MIDI is working at all (Simple SysEx test)
3. If Simple SysEx works but MIOS32 fails → firmware issue
4. If Simple SysEx fails → USB driver/hardware issue

## Expected Behavior (Working System)

When working correctly:

**Python Test Output:**
```
Testing Query Type 0x01: Operating System
  Sending: f0 00 00 7e 32 00 00 01 f7
  Received: f0 00 00 7e 32 00 0f 4d 49 4f 53 33 32 f7
✓ Valid response: "MIOS32"

...

Test Summary:
  Total queries: 9
  Passed: 9
  Failed: 0

✓ All tests passed! Device should be recognized by MIOS Studio.
```

**MIOS Studio:**
- Device appears automatically in device list
- Shows as "MidiCore 1.0.0"
- Terminal is functional
- Can send/receive MIDI

## Technical Details

### MIOS32 Query Flow

```
Python Script (Host)
  ↓ Send: F0 00 00 7E 32 00 00 01 F7
USB Driver
  ↓
STM32 USB MIDI Endpoint
  ↓
usb_midi_rx_packet() [interrupt context]
  ↓
Queue packet in rx_queue[]
  ↓
usb_midi_process_rx_queue() [task context - 1ms MIDI IO task]
  ↓
Parse CIN, accumulate in sysex_buffer[cable]
  ↓
On SysEx end (F7):
  mios32_query_is_query_message() → true
  ↓
  mios32_query_process()
  ↓
  mios32_query_send_response(0x01, ...)
  ↓
  Build: F0 00 00 7E 32 00 0F "MIOS32" F7
  ↓
  usb_midi_send_sysex()
  ↓
  Queue TX packets in tx_queue[]
  ↓
  Send to USB MIDI EP_IN with flow control
  ↓
USB Driver
  ↓
Python Script receives response
```

### Why Queries Might Not Work

1. **Not in firmware** (most common - 90%)
   - Old firmware flashed
   - Module not compiled in build
   - Flash didn't complete successfully

2. **Runtime issue** (rare - 5%)
   - Task not running
   - Queue overflow
   - USB endpoint issue

3. **Hardware issue** (rare - 5%)
   - USB cable bad
   - Driver issue
   - Port conflict

## Next Steps

1. **Follow the Step-by-Step Fix** above
2. **Run test_midi_loopback.py** to diagnose
3. **Check build output** for mios32_query.o
4. **Verify flash succeeded** in Console
5. **Power cycle** device
6. **Re-test** with Python script

If still not working after following all steps:
- Collect build log
- Collect flash log  
- Run diagnostic tests
- Report with full output

---

**Last Updated:** 2026-01-28  
**Status:** Firmware must be rebuilt and flashed to device
