# CRITICAL: You Must Rebuild and Flash Firmware!

## Current Situation

The test results you're seeing are **EXPECTED** because:
- ✅ The fix is in the Git repository (commit 9ce8fec)
- ❌ The fix is NOT on your physical device yet
- ❌ You're testing against OLD firmware

**Both MIDI and CDC are failing because your device is running firmware WITHOUT the initialization order fix.**

## The Fix That Was Applied

Changed `Core/Src/main.c` to register USB callbacks BEFORE starting USB device:

```c
// OLD ORDER (what's currently on your device - BROKEN):
MX_USB_DEVICE_Init();  // Starts USB → callbacks NULL
usb_midi_init();       // Too late
usb_cdc_init();        // Too late

// NEW ORDER (what's in Git - FIXED):
usb_midi_init();       // Register callbacks FIRST ✅
usb_cdc_init();        // Register callbacks FIRST ✅
MX_USB_DEVICE_Init();  // NOW start USB with callbacks ready
```

## Step-by-Step: Apply the Fix

### Step 1: Pull Latest Code

Open Git Bash or terminal in your MidiCore directory:
```bash
git fetch origin
git checkout copilot/investigate-midicore-recognition
git pull origin copilot/investigate-midicore-recognition
```

### Step 2: Open STM32CubeIDE

1. Launch STM32CubeIDE
2. File → Open Projects from File System
3. Navigate to your MidiCore directory
4. Select the project and open it

### Step 3: Clean Build

```
1. Right-click "MidiCore" project in Project Explorer
2. Click "Clean Project"
3. Wait for "Build Finished" message in Console
```

### Step 4: Build Project

```
1. Click Project → Build Project (or press Ctrl+B)
2. Watch Console tab - should see:
   - "Building file: ../Core/Src/main.c"
   - "Building file: ../Services/usb_midi/usb_midi.c"
   - "Building file: ../Services/usb_cdc/usb_cdc.c"
3. Wait for "Build Finished: 0 errors"
```

**CRITICAL VERIFICATION:**
Check Console output contains these lines:
```
Building file: ../Services/mios32_query/mios32_query.c
Finished building: ../Services/mios32_query/mios32_query.c
```

If you don't see mios32_query.c being compiled, the module is excluded! See troubleshooting below.

### Step 5: Flash to Device

```
1. Connect ST-Link debugger to your STM32F407
2. Click Run → Debug (F11) or Run → Run (Ctrl+F11)
3. Watch Console - must see:
   "Download verified successfully"
4. If you see this, flash succeeded!
```

### Step 6: Power Cycle Device

**CRITICAL - Don't skip this!**
```
1. Disconnect USB cable from computer
2. Wait 10 seconds (not 5!)
3. Reconnect USB cable
```

This ensures device boots with new firmware and re-enumerates.

### Step 7: Test

Now run the Python tests:
```bash
cd Tools

# Test 1: MIDI Loopback (comprehensive)
python3 test_midi_loopback.py

# Test 2: MIOS32 Queries (should now PASS)
python3 test_mios32_recognition.py

# Test 3: CDC Terminal (should now PASS)
python3 test_cdc_terminal.py
```

**Expected Results:**
```
test_midi_loopback.py:
  Simple SysEx        PASS ✅
  Note On/Off         PASS ✅
  MIOS32 Query        PASS ✅

test_mios32_recognition.py:
  Total queries: 9
  Passed: 9           ← Was 0/9 before
  Failed: 0

test_cdc_terminal.py:
  Line Coding          PASS
  Control Lines        PASS
  Echo                 PASS ← Was FAIL before
  Data Transfer        PASS ← Was FAIL before
  Overall: 4/4 tests passed
```

## Troubleshooting

### Issue: "mios32_query.c not being compiled"

**Problem:** Module excluded from build

**Solution:**
```
1. In Project Explorer, expand Services/mios32_query/
2. Right-click on the folder
3. Properties → C/C++ Build
4. UNCHECK "Exclude resource from build"
5. Click Apply and Close
6. Rebuild (Clean → Build)
```

### Issue: "Flash says success but tests still fail"

**Problem:** Flash didn't actually work OR device not power cycled

**Solution:**
```
1. Check Console for "Download verified successfully"
2. If not seen, flash failed:
   - Check ST-Link connection
   - Try different USB cable for ST-Link
   - Check target device power
3. If flash succeeded, power cycle properly:
   - FULLY disconnect USB (not just reset)
   - Wait 10 seconds
   - Reconnect
```

### Issue: "Build has errors"

**Check:**
```
1. MODULE_ENABLE_USB_MIDI = 1 in Config/module_config.h
2. MODULE_ENABLE_USB_CDC = 1 in Config/module_config.h
3. All files accessible (not read-only)
4. STM32CubeIDE has correct toolchain
```

### Issue: "Device not appearing after flash"

**Possible causes:**
```
1. Wrong firmware flashed (bootloader address)
2. Device in bootloader mode (press reset)
3. USB cable bad (try different cable)
4. Windows driver issue (Device Manager → Uninstall → Replug)
```

## Why Your Tests Are Failing Now

You're running tests against **old firmware** that doesn't have:
- ✅ Initialization order fix
- ✅ MIOS32 query handler with callbacks registered before Init
- ✅ CDC echo with callbacks registered before Init

The firmware on your device was compiled BEFORE these fixes were added to the repository.

## After Following These Steps

Once you rebuild and flash:
1. ✅ MIOS32 queries will work (9/9 pass)
2. ✅ CDC terminal will echo (4/4 pass)
3. ✅ MIOS Studio will recognize device
4. ✅ Terminal in MIOS Studio will work

## Quick Checklist

- [ ] Pulled latest code from Git
- [ ] Opened project in STM32CubeIDE
- [ ] Clean build (0 errors)
- [ ] Verified mios32_query.c compiled
- [ ] Flashed firmware successfully
- [ ] Saw "Download verified successfully"
- [ ] Power cycled device (10 seconds)
- [ ] Device re-appeared in device list
- [ ] Ran test_midi_loopback.py → Should show 3/3 PASS
- [ ] Ran test_mios32_recognition.py → Should show 9/9 PASS
- [ ] Ran test_cdc_terminal.py → Should show 4/4 PASS

If ALL checkboxes checked and tests STILL fail, then there's another issue. But 99% chance it will work after proper rebuild/flash.

---

**The fix is done. You just need to compile and flash it to your device!**

**Status:** Waiting for user to rebuild and flash firmware  
**Next:** Run tests after flash to verify fix works
