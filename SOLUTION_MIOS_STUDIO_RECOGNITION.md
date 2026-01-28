# MIOS Studio Connection - Complete Solution Package

## 📋 Overview

This package provides a complete solution for diagnosing and fixing MIOS Studio recognition issues with MidiCore devices. It includes diagnostic tools, comprehensive documentation, and troubleshooting guides.

## 🚀 Quick Start (5 Minutes)

### 1. Check USB Connection
```bash
# Windows: Open Device Manager (Win+X)
# Linux:
lsusb | grep 16C0:0489
ls /dev/ttyACM*
```

### 2. Run Diagnostic Tools
```bash
cd Tools
pip install python-rtmidi pyserial  # First time only
python3 test_mios32_recognition.py  # Test MIDI protocol
python3 test_cdc_terminal.py         # Test COM port
```

### 3. Interpret Results
- ✅ **All pass:** Firmware OK, check MIOS Studio settings
- ❌ **MIOS32 fails:** Rebuild and reflash firmware  
- ❌ **CDC fails:** Driver/permissions issue

## 📚 Documentation Files

| File | Purpose | When to Use |
|------|---------|-------------|
| **QUICK_START_MIOS_STUDIO.md** | Fast solutions, common issues | Start here - quickest path to solution |
| **MIOS_STUDIO_RECOGNITION_INVESTIGATION.md** | Technical deep dive, protocol specs | Need detailed understanding or advanced troubleshooting |
| **Tools/test_mios32_recognition.py** | Test MIOS32 query/response | Device not recognized by MIOS Studio |
| **Tools/test_cdc_terminal.py** | Test CDC terminal communication | Terminal not working in MIOS Studio |
| **COM_PORT_DISCOVERY_EXPLANATION.md** | How OS discovers COM ports | Understanding device enumeration |
| **MIOS32_PROTOCOL_FIX_FINAL.md** | Query protocol implementation | Understanding query handler |

## 🔧 Diagnostic Tools

### test_mios32_recognition.py

**What it does:**
- Sends MIOS32 queries via USB MIDI (same as MIOS Studio)
- Tests all 9 query types
- Validates response format
- Reports pass/fail for each query

**Usage:**
```bash
cd Tools
python3 test_mios32_recognition.py
```

**Success output:**
```
Testing Query Type 0x01: Operating System
✓ Valid response: "MIOS32"

Testing Query Type 0x08: Application Name
✓ Valid response: "MidiCore"

Test Summary:
  Total queries: 9
  Passed: 9
  Failed: 0

✓ All tests passed! Device should be recognized by MIOS Studio.
```

### test_cdc_terminal.py

**What it does:**
- Tests USB CDC (Virtual COM port)
- Checks line coding, control lines
- Tests echo and data transfer
- Offers interactive terminal mode

**Usage:**
```bash
cd Tools
python3 test_cdc_terminal.py
```

**Success output:**
```
Test Summary:
  Line Coding          PASS
  Control Lines        PASS
  Echo                 PASS
  Data Transfer        PASS

Overall: 4/4 tests passed

✓ All tests passed! CDC terminal should work with MIOS Studio.
```

## 🔍 How MIOS Studio Finds Devices

### Two-Stage Process

**Stage 1: USB Enumeration**
```
Device plugs in → OS detects → Loads drivers → Creates:
  - MIDI device ("MidiCore 4x4")
  - COM port (e.g., COM3 or /dev/ttyACM0)
```

**Stage 2: MIOS32 Handshake**
```
MIOS Studio sends query via MIDI:
  F0 00 00 7E 32 00 00 01 F7

Device responds:
  F0 00 00 7E 32 00 0F "MIOS32" F7

→ Device RECOGNIZED!
```

## ✅ Implementation Status

All code is **CORRECT** and **COMPLETE**:

| Component | Status | File |
|-----------|--------|------|
| USB CDC (Terminal) | ✅ CORRECT | `Services/usb_cdc/usb_cdc.c` |
| USB MIDI (SysEx) | ✅ CORRECT | `Services/usb_midi/usb_midi.c` |
| MIOS32 Query Handler | ✅ CORRECT | `Services/mios32_query/mios32_query.c` |
| Initialization | ✅ CORRECT | `Core/Src/main.c`, `App/app_init.c` |
| Descriptors | ✅ CORRECT | `USB_DEVICE/App/usbd_desc.c` |

## 🐛 Common Issues & Solutions

### Issue: "No MIDI devices found"
**Cause:** USB cable is charge-only or USB port issue  
**Solution:** Try different USB cable/port

### Issue: "Device appears but not recognized"
**Cause:** Wrong firmware (missing query handler)  
**Solution:** 
```bash
# Run diagnostic
python3 Tools/test_mios32_recognition.py

# If fails: rebuild and reflash firmware
# In STM32CubeIDE: Clean → Build → Flash
```

### Issue: "Terminal doesn't work"
**Cause:** CDC driver not loaded  
**Solution:**
```bash
# Test CDC
python3 Tools/test_cdc_terminal.py

# Windows: Reinstall device driver
# Linux: sudo usermod -a -G dialout $USER
```

## 🎯 Troubleshooting Flowchart

```
Start
  ↓
Does device appear in Device Manager/lsusb?
  ├─ No → Try different USB cable/port
  └─ Yes ↓
       ↓
Run test_mios32_recognition.py
  ├─ All pass → MIOS Studio configuration issue
  │             - Restart MIOS Studio
  │             - Manually select MIDI ports
  │             - Update MIOS Studio to v2.4+
  └─ Fail → Firmware issue
            - Check MODULE_ENABLE_USB_MIDI = 1
            - Rebuild firmware (Clean + Build)
            - Reflash device
            - Re-test
```

## 📦 What's Included

### Documentation
- `QUICK_START_MIOS_STUDIO.md` - Fast solutions (start here!)
- `MIOS_STUDIO_RECOGNITION_INVESTIGATION.md` - Technical deep dive
- `COM_PORT_DISCOVERY_EXPLANATION.md` - Device enumeration details
- `MIOS32_PROTOCOL_FIX_FINAL.md` - Protocol implementation details

### Diagnostic Tools
- `Tools/test_mios32_recognition.py` - MIDI protocol tester
- `Tools/test_cdc_terminal.py` - CDC terminal tester

### Source Code
- `Services/mios32_query/` - Query handler implementation
- `Services/usb_midi/` - USB MIDI with SysEx support
- `Services/usb_cdc/` - USB CDC terminal support
- `USB_DEVICE/` - USB device class drivers

## 🔬 Technical Details

### MIOS32 Query Protocol

**Query Format:**
```
F0 00 00 7E 32 <dev_id> <cmd> <query_type> F7

Where:
- F0        : SysEx start
- 00 00 7E  : MIOS manufacturer ID
- 32        : MIOS32 device ID
- <dev_id>  : Device instance (0x00)
- <cmd>     : Command (0x00 = query)
- <query_type>: What to query (0x01-0x09)
- F7        : SysEx end
```

**Response Format:**
```
F0 00 00 7E 32 <dev_id> 0x0F <string> F7

Where:
- 0x0F    : ACK response code
- <string>: ASCII response (NO null terminator!)
```

### Query Types

| Type | Response | Example |
|------|----------|---------|
| 0x01 | Operating System | "MIOS32" |
| 0x02 | Board Name | "STM32F407VGT6" |
| 0x03 | Core Family | "STM32F4" |
| 0x04 | Chip ID | "12345678" |
| 0x05 | Serial Number | "000001" |
| 0x06 | Flash Size | "1048576" |
| 0x07 | RAM Size | "131072" |
| 0x08 | Application Name | "MidiCore" |
| 0x09 | Version | "1.0.0" |

## 🛠️ Requirements

### Hardware
- STM32F407VGT6 (or compatible)
- USB cable with data support (not charge-only!)
- ST-Link debugger (for flashing)

### Software
- STM32CubeIDE (for building/flashing)
- Python 3.6+ (for diagnostic tools)
- MIOS Studio v2.4+ (for testing)

### Python Dependencies
```bash
pip install python-rtmidi pyserial
```

## 🏗️ Building Firmware

```bash
# In STM32CubeIDE:
1. Right-click project → Clean Project
2. Project → Build Project
3. Verify: 0 errors, 0 warnings
4. Run → Debug (or Run → Run)
5. Wait for "Download verified successfully"
6. Disconnect and reconnect USB
7. Test with diagnostic tools
```

## ✨ Key Features Verified

- ✅ USB Composite Device (MIDI + CDC)
- ✅ 4-port USB MIDI (cables 0-3)
- ✅ USB CDC Virtual COM port
- ✅ MIOS32 query protocol (all 9 types)
- ✅ SysEx RX buffering per-cable
- ✅ TX queue with flow control
- ✅ RX queue for deferred processing
- ✅ Terminal echo functionality
- ✅ MIOS Studio compatible

## 🤝 Support

If diagnostics pass but issue persists, collect:

```bash
# 1. Device info
lsusb -v -d 16C0:0489 > device_info.txt  # Linux
# or Device Manager screenshot (Windows)

# 2. Diagnostic results
python3 Tools/test_mios32_recognition.py > mios32_test.txt
python3 Tools/test_cdc_terminal.py > cdc_test.txt

# 3. Build log
# Copy from STM32CubeIDE Console

# 4. MIOS Studio version
# Help → About in MIOS Studio
```

## 📖 Further Reading

- **MIOS32 Documentation:** http://midibox.org/mios32/manual/
- **MIOS Studio:** http://www.ucapps.de/mios_studio.html
- **USB MIDI Spec:** USB Device Class Definition for MIDI Devices v1.0
- **MidiCore Project:** https://github.com/labodezao/MidiCore

---

## Summary

**Problem:** MIOS Studio not recognizing MidiCore device

**Root Cause Options:**
1. Firmware not flashed (most common)
2. Module disabled in config
3. USB cable/driver issue
4. MIOS Studio configuration

**Solution Path:**
1. Run diagnostic tools (2 min)
2. Interpret results
3. Apply appropriate fix
4. Verify with MIOS Studio

**Expected Outcome:**
- Device appears in MIOS Studio device list
- Terminal receives debug messages
- Full MIOS32 protocol compatibility

---

**Version:** 1.0  
**Date:** 2026-01-28  
**Status:** ✅ COMPLETE - All code verified, tools provided, documentation complete
