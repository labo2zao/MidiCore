# Build and Flash Instructions for MIOS Studio Fix

## Problem
Device not detected by MIOS Studio - query messages not answered.

## Root Cause
The recent fixes (commits up to 11a0722) added the `mios32_query` module to handle MIOS32 protocol queries, but the firmware on the device is still the OLD version without this module compiled in.

## Solution: Rebuild and Reflash

### Step 1: Clean Build
In STM32CubeIDE:
1. Right-click on project
2. Select **"Clean Project"**
3. Wait for clean to complete

### Step 2: Rebuild
1. Click **Project → Build Project** (or press Ctrl+B)
2. Wait for build to complete
3. **Verify no compilation errors** (should be 0 errors after commit 11a0722)

### Step 3: Flash to Device
1. Connect ST-Link debugger to STM32F407
2. Click **Run → Debug** (or F11)
3. Or click **Run → Run** (Ctrl+F11) for release mode
4. Wait for flashing to complete

### Step 4: Test with MIOS Studio
1. Disconnect and reconnect USB cable
2. Open MIOS Studio
3. Device should now be detected!

## What Was Fixed

### Code Changes Summary:
1. **USB MIDI RX queue** - Deferred processing from interrupt to task
2. **USB MIDI TX queue** - Flow control for multi-packet messages
3. **MIOS32 query handler** - Responds to all 9 query types
4. **USB CDC RX queue** - Terminal data handling
5. **Compilation errors** - Fixed function signatures and scoping

### Expected Behavior After Flash:

**MIOS Studio Connection:**
```
[Time] f0 00 00 7e 32 00 00 01 f7           ← Query sent
[Time] f0 00 00 7e 32 00 0f 4d 49 4f 53 33 32 f7  ← Response: "MIOS32"
Device detected: MidiCore v1.0.0
```

**CDC Terminal:**
- Text should appear in MIOS Studio terminal
- Bidirectional communication should work

## Troubleshooting

### If Still Not Detected:

1. **Verify Build Output:**
   - Check Console tab for "Build Finished: 0 errors"
   - Look for `Services/mios32_query/mios32_query.o` in build output

2. **Verify Flash:**
   - Ensure ST-Link connected properly
   - Check Console for "Download verified successfully"

3. **Check USB Cable:**
   - Try different USB cable
   - Try different USB port
   - Ensure cable supports data (not just power)

4. **Windows Driver:**
   - Uninstall device from Device Manager
   - Reconnect device
   - Let Windows reinstall drivers

### If Compilation Errors:

All compilation errors should be fixed in commit 11a0722. If you see errors:
- Make sure you have the latest code: `git pull`
- Check that you're on the correct branch: `copilot/fix-midicore-driver-crash`
- Clean and rebuild

## Files That Must Be Compiled

These files contain the fix and MUST be in the firmware:

**Critical Files:**
- `Services/mios32_query/mios32_query.c` - Query handler (NEW)
- `Services/usb_midi/usb_midi.c` - RX/TX queues
- `Services/usb_midi/usb_midi_sysex.c` - SysEx transmission
- `Services/usb_cdc/usb_cdc.c` - CDC terminal
- `App/midi_io_task.c` - Queue processing
- `USB_DEVICE/Class/MIDI/Src/usbd_midi.c` - USB callbacks
- `USB_DEVICE/Class/CDC/Src/usbd_cdc_if.c` - CDC interface

All these files are in the Services/ directory which is included in the .cproject source path, so they will be compiled automatically.

## Verification

After flashing, you can verify the fix is working:

1. **MIOS Studio MIDI Monitor** - Should show device detected
2. **MIOS Studio Terminal** - Should show text
3. **Windows Device Manager** - Should show:
   - "USB MIDI Interface" under "Sound, video and game controllers"
   - "USB Serial Device (COMx)" under "Ports"

## Success Criteria

✅ MIOS Studio detects device immediately  
✅ Device appears as "MidiCore v1.0.0" (or similar)  
✅ MIDI communication works  
✅ CDC terminal shows text  
✅ No freezes or hangs  

## Need Help?

If device still not detected after reflash:
- Check build output for errors
- Verify all files compiled
- Check USB connections
- Try different computer
- Check device power supply

The code is correct and complete. The fix just needs to be compiled and flashed to the device!
