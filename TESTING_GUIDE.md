# MidiCore Testing Guide

## Testing the Heap Fix (40KB Heap)

### What Was Fixed

**Issue:** OpenOCD errors at 0xa5a5a5xx addresses (FreeRTOS stack fill pattern)
**Cause:** Heap exhaustion with only 36KB heap
**Fix:** Increased `configTOTAL_HEAP_SIZE` to 40KB (commit a7f4fb0)

### Before Flashing (Old Firmware)

The OpenOCD errors showed corrupted task control blocks:
```
Error: Failed to read memory at 0xa5a5a5d9
Error: Failed to read memory at 0xa5a5a5f1
Error: Failed to read memory at 0xa5a5a5a5
```

Boot log showed:
```
[HEAP] After CLI task: 4104 bytes free
```
→ Only 329 bytes margin for remaining tasks (too tight!)

### After Flashing (New Firmware)

Expected boot log:
```
========================================
   EARLY HEAP DIAGNOSTICS
   Function: app_init_and_start()
========================================
Heap total:       40960 bytes (40KB)    ← Changed from 36KB
Heap free now:    20288 bytes
Heap min ever:    20288 bytes
Heap used:        20672 bytes
========================================

[HEAP] After CLI task: ~8200 bytes free  ← Changed from 4104
[INIT] About to start MIDI IO task...
[MIDI-TASK] MidiIOTask started
[MIDI-TASK] Init complete, entering main loop
[MIDI-TASK] Waiting for USB enumeration (500ms)...
[MIDI-TASK] USB enumeration complete, ready for queries
[STACK] Monitor task started
```

### Testing Checklist

**1. Flash New Firmware**
```bash
# Use OpenOCD or ST-Link to flash firmware with 40KB heap fix
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program build/MidiCore.elf verify reset exit"
```

**2. Connect UART**
- Port: UART5 (PC12/PD2)
- Baud: 115200
- Capture full boot log (15+ seconds)

**3. Verify Heap**
Look for:
```
Heap total:       40960 bytes (40KB)     ✓ Should show 40KB
[HEAP] After CLI task: 8200 bytes free   ✓ Should have ~8KB free
```

**4. Verify Tasks**
All tasks should start without errors:
```
[HEAP] After AinTask: ...
[HEAP] After pressure task: ...
[HEAP] After calibration task: ...
[HEAP] After OledDemo task: ...
[HEAP] After CLI task: ~8200 bytes free
[MIDI-TASK] MidiIOTask started
[STACK] Monitor task started
```

**5. Verify No Corruption**
- No OpenOCD memory errors
- No hard faults
- No system crashes
- All tasks running

**6. Test MIOS Studio Recognition**
With stable system, test MIOS Studio:
```
1. Open MIOS Studio
2. Connect USB
3. Watch UART for:
   [USB-RX] Cable:0 CIN:0x4 ...  (queries arriving)
   [MIDICORE-Q] Received query... (queries detected)
   [MIDICORE-R] Sending response... (responses sent)
4. Device should appear in MIOS Studio device list
```

### Success Criteria

✅ Heap shows 40KB total (not 36KB)
✅ ~8200 bytes free after CLI task (not 4104)
✅ All tasks initialize successfully
✅ No OpenOCD memory errors
✅ System stable and responsive
✅ MIOS Studio recognizes device (with all 10 fixes)

### If Issues Persist

**If heap still too low:**
- Check if any large global buffers added
- Review task stack sizes in app_init.c
- Consider reducing less critical task stacks

**If OpenOCD errors return:**
- Verify firmware actually flashed (check version)
- Check FreeRTOSConfig.h has 40KB heap
- Capture full UART log for analysis

**If MIOS Studio doesn't recognize:**
- Verify all 10 MIOS Studio fixes applied (see PR)
- Ensure MODULE_DEBUG_MIDICORE_QUERIES=1
- Check for [USB-RX] messages in log
- Review MIOS_STUDIO_RECOGNITION.md

### Memory Summary

**STM32F407VGT6 Resources:**
- 128KB main RAM
- 64KB CCM RAM
- Total: 192KB

**Heap Usage (40KB):**
- 40KB heap = 31% of main RAM
- Remaining: 88KB for stacks/globals
- Safe and sustainable

**Task Allocation:**
- DefaultTask: 16KB
- CliTask: 8KB
- MidiIOTask: 2KB
- StackMon: 1KB
- AinTask: 1KB
- PressureTask: 1KB
- CalibrationTask: 1KB
- OledDemoTask: 1KB
- TCBs + overhead: ~1KB
- **Total: ~33KB**
- **Margin: 7KB** ← Healthy!

### References

- `HEAP_EXHAUSTION_FIX.md` - Complete technical analysis
- `MIOS_STUDIO_RECOGNITION.md` - MIOS Studio fixes
- Commit a7f4fb0 - Heap fix
- Commits d0ee7da through 925ccde - MIOS Studio fixes
