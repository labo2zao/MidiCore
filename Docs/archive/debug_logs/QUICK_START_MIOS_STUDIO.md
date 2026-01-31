# Quick Start: MIOS Studio Recognition Fix

## TL;DR - Fast Solution

If MIOS Studio doesn't recognize your MidiCore device, follow these steps:

### Step 1: Verify USB Connection (30 seconds)

**Windows:**
```powershell
# Open Device Manager (Win+X → Device Manager)
# Look for:
- Sound, video and game controllers → "MidiCore 4x4" ✓
- Ports (COM & LPT) → "USB Serial Device (COMx)" ✓
```

**Linux:**
```bash
lsusb | grep 16C0:0489  # Should show MidiCore device
ls /dev/ttyACM*         # Should show COM port
```

**Result:**
- ✅ Both appear → Go to Step 2
- ❌ Missing → Check USB cable (must support data, not just charging)

---

### Step 2: Run Diagnostic Scripts (2 minutes)

```bash
cd Tools

# Install requirements (first time only)
pip install python-rtmidi pyserial

# Test MIOS32 protocol (CRITICAL for recognition)
python3 test_mios32_recognition.py

# Test CDC terminal (for debug messages)
python3 test_cdc_terminal.py
```

**Expected Results:**
```
MIOS32 Test: 9/9 queries passed ✓
CDC Test: 4/4 tests passed ✓
```

---

### Step 3: Interpret Results

#### ✅ ALL TESTS PASS
**Problem:** Firmware is working correctly
**Solution:** MIOS Studio configuration issue
```bash
# Try:
1. Restart MIOS Studio
2. Manually select MIDI IN/OUT ports
3. Check MIOS Studio version (need v2.4+)
4. Try on different computer to isolate issue
```

#### ❌ MIOS32 TESTS FAIL
**Problem:** Firmware doesn't have query handler or wrong firmware version
**Solution:** Rebuild and reflash firmware

```bash
# In STM32CubeIDE:
1. Right-click project → Clean Project
2. Project → Build Project
3. Verify no compilation errors
4. Run → Debug (or Run → Run)
5. Wait for "Download verified successfully"
6. Disconnect/reconnect USB
7. Re-run test scripts
```

**Check build output for these critical files:**
```
Services/mios32_query/mios32_query.o         ← MUST exist
Services/usb_midi/usb_midi.o                 ← MUST exist  
Services/usb_cdc/usb_cdc.o                   ← MUST exist
```

#### ❌ CDC TESTS FAIL (but MIOS32 pass)
**Problem:** COM port driver issue
**Solution:** Driver installation

**Windows:**
```powershell
# Uninstall device:
1. Device Manager → Ports (COM & LPT)
2. Right-click device → Uninstall
3. Check "Delete driver software"
4. Unplug USB, wait 5 seconds, replug
5. Windows will reinstall driver
```

**Linux:**
```bash
# Check permissions:
groups  # Should see 'dialout'

# If not:
sudo usermod -a -G dialout $USER
# Logout and login again

# Check driver:
lsmod | grep cdc_acm  # Should show module loaded
```

---

## Understanding MIOS Studio Detection

### What MIOS Studio Does

```
1. Enumerate USB MIDI devices
   ↓
2. Send MIOS32 query via MIDI:
   F0 00 00 7E 32 00 00 01 F7
   ↓
3. Wait for response:
   F0 00 00 7E 32 00 0F "MIOS32" F7
   ↓
4. If response matches:
   → Device RECOGNIZED ✓
   → Query device name
   → Enable terminal
```

**Key Point:** Recognition happens via **USB MIDI**, not CDC!
CDC terminal is secondary (for debug messages only).

---

## Common Issues & Solutions

### Issue: "No MIDI devices found"

**Causes:**
- USB cable is charge-only (no data lines)
- USB port has power issues
- MIDI class driver not loaded

**Solution:**
```bash
# Try different USB cable
# Try different USB port
# Try different computer (to isolate issue)
```

---

### Issue: "Device appears but MIOS Studio doesn't detect it"

**Causes:**
- Firmware doesn't have MIOS32 query handler
- Query response is malformed
- Wrong firmware flashed

**Solution:**
```bash
# Run diagnostic:
python3 Tools/test_mios32_recognition.py

# If fails → rebuild and reflash firmware
# If passes → MIOS Studio configuration issue
```

---

### Issue: "MIOS Studio detects but terminal doesn't work"

**Causes:**
- CDC driver not loaded
- Wrong COM port selected
- Terminal not enabled in MIOS Studio

**Solution:**
```bash
# Test CDC directly:
python3 Tools/test_cdc_terminal.py

# In MIOS Studio:
1. Check "Terminal" tab is visible
2. Verify correct COM port selected
3. Try send test message
```

---

### Issue: "Works on one computer but not another"

**Causes:**
- Driver differences
- USB controller differences  
- MIOS Studio version differences

**Solution:**
```bash
# Check MIOS Studio version (need v2.4+)
# Update MIOS Studio to latest
# Install STM32 VCP driver on problem computer
```

---

## Module Configuration

Ensure these are enabled in `Config/module_config.h`:

```c
// REQUIRED for MIOS Studio recognition
#define MODULE_ENABLE_USB_MIDI 1  // ← CRITICAL
#define MODULE_ENABLE_USB_CDC  1  // ← For terminal

// These should be compiled in:
- Services/mios32_query/mios32_query.c
- Services/usb_midi/usb_midi.c
- Services/usb_cdc/usb_cdc.c
```

---

## Build Verification Checklist

Before testing with MIOS Studio:

- [ ] Clean build (no errors)
- [ ] `mios32_query.o` compiled
- [ ] `usb_midi.o` compiled  
- [ ] `usb_cdc.o` compiled
- [ ] Firmware flashed successfully
- [ ] USB device appears in system
- [ ] MIDI ports visible
- [ ] COM port visible
- [ ] Diagnostic scripts pass

---

## Emergency Debugging

If nothing works, collect this info:

```bash
# 1. USB device info
# Windows: Device Manager screenshot
# Linux:
lsusb -v -d 16C0:0489 > usb_device_info.txt

# 2. Diagnostic results
python3 Tools/test_mios32_recognition.py > mios32_test.txt
python3 Tools/test_cdc_terminal.py > cdc_test.txt

# 3. Build log
# Copy build console output from IDE

# 4. MIOS Studio version
# Help → About in MIOS Studio
```

Send these files when requesting support.

---

## Success Indicators

You've succeeded when:

✅ Device appears in Device Manager / lsusb  
✅ MIDI ports are visible  
✅ COM port is visible  
✅ `test_mios32_recognition.py` shows 9/9 passed  
✅ `test_cdc_terminal.py` shows 4/4 passed  
✅ MIOS Studio shows device in device list  
✅ MIOS Studio terminal receives messages  

---

## Still Not Working?

Read the comprehensive investigation document:
[MIOS_STUDIO_RECOGNITION_INVESTIGATION.md](MIOS_STUDIO_RECOGNITION_INVESTIGATION.md)

It contains:
- Technical deep dive into USB CDC/MIDI
- MIOS32 protocol specification
- Detailed troubleshooting
- Source code analysis
- Reference documentation

---

## Quick Command Reference

```bash
# Test MIOS32 recognition
cd Tools && python3 test_mios32_recognition.py

# Test CDC terminal
cd Tools && python3 test_cdc_terminal.py

# Check USB device (Linux)
lsusb | grep 16C0
ls /dev/ttyACM*

# Check permissions (Linux)
groups | grep dialout

# Add to dialout group (Linux)
sudo usermod -a -G dialout $USER
```

---

**Last Updated:** 2026-01-28  
**Version:** 1.0  
**Support:** See MIOS_STUDIO_RECOGNITION_INVESTIGATION.md for details
