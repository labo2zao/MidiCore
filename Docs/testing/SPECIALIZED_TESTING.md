# Specialized Testing Guide

## Table of Contents
- [Overview](#overview)
- [Breath Controller Testing](#breath-controller-testing)
- [Quick Start Guides](#quick-start-guides)
- [USB MIDI Device Testing](#usb-midi-device-testing)
- [Test Execution and Validation](#test-execution-and-validation)
- [Final Debug Checklist](#final-debug-checklist)

---

## Overview

This guide covers specialized testing scenarios for MidiCore, including breath controller testing, quick start procedures, USB MIDI validation, and comprehensive debugging checklists.

---

## Breath Controller Testing

### Overview

The `MODULE_TEST_BREATH` test provides comprehensive validation of the complete breath controller signal chain in MidiCore, from I2C pressure sensor reading through to MIDI CC output.

### Purpose

Essential for:
- **Wind Controller Setup**: Validating breath-controlled synthesizers
- **Accordion Bellows**: Testing bidirectional pressure sensing (push/pull)
- **Hardware Validation**: Verifying I2C communication and sensor operation
- **Calibration**: Tuning pressure-to-MIDI mapping curves

### Hardware Requirements

**Pressure Sensor Options:**

1. **XGZP6847D** (Recommended)
   - 24-bit high-resolution I2C ADC
   - Differential pressure sensor
   - Range: ±40 kPa (configurable)
   - I2C address: 0x6D (7-bit)
   - Supply: 3.3V

2. **Generic 16-bit I2C Sensors**
   - MCP3425/MCP3426/MCP3427/MCP3428
   - Generic U16/S16 big-endian sensors

**Electrical Connections:**
```
STM32F407          Pressure Sensor
-----------        ---------------
SCL (I2C1/2)  -->  SCL
SDA (I2C1/2)  <->  SDA
3.3V          -->  VCC
GND           -->  GND

Pull-up resistors: 4.7kΩ on SCL and SDA (required!)
```

**Debug Output:**
- **UART2** (PA2/PA3): 115200 baud, 8N1
- Connect USB-UART adapter to view real-time sensor data

### Running the Test

**Step 1: Enable the Test**

In STM32CubeIDE:
1. Right-click project → Properties
2. C/C++ Build → Settings → MCU GCC Compiler → Preprocessor
3. Add define: `MODULE_TEST_BREATH`
4. Click Apply and Close

**Step 2: Build and Flash**
```bash
make CFLAGS+="-DMODULE_TEST_BREATH"
```

**Step 3: Connect UART Terminal**
- Port: COM port of your USB-UART adapter
- Baud: 115200, 8N1

**Step 4: Observe Output**

Expected output:
```
==============================================
UART Debug Verification: OK
==============================================

=== Breath Controller Module Test ===

=== Pressure Sensor Configuration ===
  Enabled:     YES
  I2C Bus:     1
  I2C Address: 0x6D
  Sensor Type: XGZP6847D 24-bit
  Map Mode:    Center at 0 Pa
  Range:       -40000 to 40000 Pa
  Interval:    20 ms

=== Expression/MIDI CC Configuration ===
  Enabled:     YES
  MIDI Ch:     1
  CC Number:   2 (Breath Controller)
  Curve:       Exponential (gamma=1.80)
  Output:      0 to 127 (7-bit MIDI)
  Rate:        20 ms
  Smoothing:   200

Time(s) | Raw Value | Pressure(Pa) | 12-bit | CC# | CC Val | Status
--------|-----------|--------------|--------|-----|--------|--------
    0.2 |         0 |           +0 |   2048 |   2 |     63 | OK
    0.4 |      5000 |       +5000 |   2560 |   2 |     81 | OK
    0.6 |     10000 |      +10000 |   3072 |   2 |     98 | OK
```

**Step 5: Test Breath Input**

**For Wind Controllers:**
- Blow gently into the sensor tube
- Observe increasing positive pressure values
- CC value should increase proportionally
- Release breath, watch values return to baseline

**For Accordion Bellows (Bidirectional):**
- Push bellows: positive pressure → CC#11 (push)
- Pull bellows: negative pressure → CC#2 (pull)
- Neutral position: near 0 Pa, deadband prevents flipping

### Configuration Files

**pressure.ngc** (Pressure sensor hardware):
```
PRESSURE_ENABLE=1
PRESSURE_I2C_BUS=1
PRESSURE_ADDR7=0x6D
PRESSURE_TYPE=2            # 2=XGZP6847D
PRESSURE_MAP_MODE=1        # 1=Center at 0 Pa
PRESSURE_PMIN_PA=-40000
PRESSURE_PMAX_PA=40000
PRESSURE_INTERVAL_MS=20
```

**expression.ngc** (MIDI CC mapping):
```
EXPRESSION_ENABLE=1
EXPRESSION_MIDI_CH=0       # Channel 1
EXPRESSION_CC_NUM=2        # Breath Controller
EXPRESSION_BIDIR=0         # 0=Unidirectional
EXPRESSION_RAW_MIN=0
EXPRESSION_RAW_MAX=4095
EXPRESSION_OUT_MIN=0
EXPRESSION_OUT_MAX=127
EXPRESSION_RATE_MS=20
EXPRESSION_SMOOTHING=200
EXPRESSION_DEADBAND_CC=2
EXPRESSION_CURVE=1         # Exponential
EXPRESSION_CURVE_PARAM=180 # Gamma*100
```

### Troubleshooting

**Problem: "I2C_ERR" Status**

Causes:
- Incorrect I2C address
- Missing pull-up resistors
- Sensor not powered
- Wrong I2C bus selected

Solutions:
1. Verify I2C address (0x6D for XGZP6847D)
2. Add 4.7kΩ pull-ups on SCL and SDA
3. Check 3.3V power at sensor
4. Verify I2C bus in pressure.ngc

**Problem: "DISABLED" Status**

Cause: Pressure module not enabled

Solutions:
1. Set `PRESSURE_ENABLE=1` in pressure.ngc
2. Ensure `MODULE_ENABLE_PRESSURE 1` in module_config.h
3. Rebuild and reflash

**Problem: Values Don't Change**

Causes:
- Sensor not connected
- Sensor damaged
- No breath input
- Range misconfigured

Solutions:
1. Verify physical connection
2. Try blowing harder
3. Check PMIN_PA and PMAX_PA settings
4. Test with known working sensor

**Problem: Erratic/Noisy Values**

Causes:
- Electrical noise on I2C
- Insufficient smoothing
- Mechanical vibration

Solutions:
1. Increase `EXPRESSION_SMOOTHING` to 240
2. Add shielded cable for I2C
3. Mount sensor away from vibration
4. Increase `EXPRESSION_DEADBAND_CC` to 3-4

---

## Quick Start Guides

### PATCH_SD Quick Start

**What This Test Does:**
Validates SD card operations, MIDI file export, and scene chaining persistence.

**Quick Setup (5 minutes):**

1. **Prepare SD Card**
   ```bash
   # Format as FAT32 (not exFAT!)
   # Copy config.ngc to root directory
   cp sdcard/config.ngc /path/to/sdcard/
   ```

2. **Build & Flash**
   ```bash
   make CFLAGS+="-DMODULE_TEST_PATCH_SD"
   ```

3. **Connect & Run**
   - Insert SD card
   - Connect UART2 (115200 baud, 8N1)
   - Power on
   - Watch test results

**Expected Output (30 seconds):**
```
TEST 1: SD Card Mount
[PASS] SD card mounted successfully

TEST 2: Config File Loading
[PASS] Loaded 0:/config.ngc

TEST 3: Config Parameter Reading
[PASS] Read 4 config parameters

TEST 4: Config File Saving
[PASS] Config saved to 0:/test_config.ngc

TEST 5-10: MIDI Export and Scene Chaining
[PASS] All tests completed

RESULT: ALL TESTS PASSED!
```

**Files Created:**
- `0:/test_config.ngc` - Test configuration
- `0:/test_track0.mid` - MIDI export
- `0:/looper/quicksave_0_track_*.bin` - Session data

### Testing Quick Start

**Prerequisites:**
- STM32CubeIDE installed
- MidiCore project opened
- Target hardware connected
- USB-UART adapter for debug (optional but recommended)

**Quick Test Examples:**

**1. Test AINSER64 (Analog Inputs):**
```bash
# Add preprocessor define: MODULE_TEST_AINSER64
# Build and flash
# Connect UART2 to see output
# Expected: Console shows values from all 64 analog channels
```

**2. Test SRIO (Digital I/O):**
```bash
# Add define: MODULE_TEST_SRIO
# Build and flash
# Connect UART2
# Expected: Hexadecimal dump of button states
```

**3. Test MIDI DIN:**
```bash
# Add define: MODULE_TEST_MIDI_DIN
# Build and flash
# Send MIDI to DIN input
# Expected: MIDI messages echoed/processed
```

**4. Test Looper:**
```bash
# Add define: MODULE_TEST_LOOPER
# Build and flash
# Send MIDI notes during recording
# Expected: Records for 7s, plays for 8s, stops for 2s, repeats
```

**5. Test Breath Controller:**
```bash
# Add define: MODULE_TEST_BREATH
# Build and flash
# Connect UART2 at 115200 baud
# Blow/suck on sensor
# Expected: Real-time pressure and CC values displayed
```

**6. Return to Production Mode:**
```bash
# Remove all MODULE_TEST_xxx defines
# Build and flash
# Expected: Full MidiCore application runs
```

---

## USB MIDI Device Testing

### Overview

This test validates USB Device MIDI functionality by receiving MIDI data from a DAW or MIDI software via USB and printing it to UART for debugging.

### Quick Start

1. **Enable the Test**
   Add compiler define: `APP_TEST_USB_MIDI`

2. **Configure Modules**
   ```c
   #define MODULE_ENABLE_USB_MIDI 1
   ```

3. **Hardware Setup**
   - Connect STM32 USB port to computer
   - Connect UART2 TX (PA2) to USB-Serial adapter
   - Settings: 115200 baud, 8N1

4. **Build and Flash**

5. **Test with DAW**
   - Open your DAW (Ableton, Logic, Reaper, etc.)
   - Select the board as MIDI input/output device
   - Send MIDI notes/CCs from DAW
   - Observe UART output

### Example Output

```
=====================================
USB MIDI Device Test
=====================================
USB Device MIDI: Enabled
Debug UART: UART2 (115200 baud)
Test send interval: 2000 ms
-------------------------------------
[TX] Sending test Note On: Cable:0 90 3C 64
[RX] Cable:0 90 3C 64 (Note On Ch:1 Note:60 Vel:100)
[RX] Cable:0 80 3C 00 (Note Off Ch:1 Note:60 Vel:0)
[TX] Sending test Note Off: Cable:0 80 3C 00
[RX] Cable:0 B0 07 7F (CC Ch:1 CC:7 Val:127)
```

### Configuration Options

```c
#define APP_TEST_USB_MIDI_SEND_INTERVAL 2000  // ms between test messages
#define APP_TEST_USB_MIDI_BASE_NOTE 60        // Middle C
#define APP_TEST_USB_MIDI_CHANNEL 0           // Channel 1
#define APP_TEST_USB_MIDI_VELOCITY 100        // Note velocity
#define APP_TEST_USB_MIDI_CABLE 0             // USB MIDI cable
```

### Troubleshooting

**No UART Output:**
- Check UART connections (TX on PA2 for UART2)
- Verify baud rate is 115200

**USB Device Not Detected:**
- Check USB cable (must support data)
- Verify USB_OTG_FS is configured in CubeMX
- Try different USB port or cable

---

## Test Execution and Validation

### Test Environment Setup

**Hardware Requirements:**
- STM32F4 MCU with firmware flashed
- SSD1322 OLED display (optional)
- SD card reader with FAT32 card
- MIDI IN/OUT ports connected
- UART debug connection

**Software Requirements:**
- Firmware built and flashed successfully
- SD card contains config files
- MIDI controller or keyboard
- MIDI monitor software (optional)

### Initial System Check

- [ ] Power on - system boots without errors
- [ ] Display shows main looper page
- [ ] MIDI IN/OUT functional
- [ ] SD card detected and mounted
- [ ] Button/encoder inputs responsive

### Phase 1: UI Pages Testing

**7 UI Pages to Test:**
1. Main Looper: Timeline, playhead, loop markers
2. Song Mode: 4×8 track/scene matrix
3. MIDI Monitor: Real-time message display
4. SysEx Viewer: Hex dump with manufacturer ID
5. Config Editor: 43-parameter editor
6. LiveFX: Per-track effects control
7. Humanizer/LFO: Unified modulation interface

### Phase 2: Advanced Features Testing

**16 Enhancement Features:**
1. Tempo Tap
2. Undo/Redo System (configurable stack depth)
3. Loop Quantization (5 resolution options)
4. MIDI Clock Sync (±0.1 BPM accuracy)
5. Track Mute/Solo (zero latency)
6. Copy/Paste (clipboard system)
7. Global Transpose (±24 semitones)
8. Randomizer (velocity/timing/note skip)
9. Humanizer (groove-aware variations)
10. Arpeggiator (5 patterns, 1-4 octaves)
11. Footswitch Mapping (8 inputs, 13 actions)
12. MIDI Learn (32 mappings)
13. Quick-Save Slots (8 session slots)
14. Extended Rhythm Grid (14 subdivisions)
15. LFO Module (6 waveforms)
16. Automation System (3-tier architecture)

### Test Execution Workflow

**Recommended Test Sequence:**

**Phase 1: Basic Operation** (2-3 hours)
1. Power on, verify display init
2. Test main looper timeline page
3. Record simple MIDI loop (1 track)
4. Verify playback
5. Test transport controls

**Phase 2: UI Pages** (3-4 hours)
1. Navigate through all 7 UI pages
2. Test Song Mode (scene matrix)
3. Verify MIDI Monitor captures messages
4. Check Config Editor parameter access
5. Test LiveFX real-time effects

**Phase 3: Advanced Features** (4-6 hours)
1. Test undo/redo functionality
2. Verify MIDI clock sync
3. Test mute/solo controls
4. Validate copy/paste operations
5. Try randomizer and humanizer effects

**Phase 4: Automation System** (3-4 hours)
1. Record envelope automation
2. Set up modulation matrix routing
3. Create scene-based snapshots
4. Test scene chaining with automation

**Phase 5: Stress Testing** (2-3 hours)
1. Fill all 4 tracks with MIDI events
2. Test all 8 scenes
3. Verify performance with all effects active
4. Long-duration playback test (>30 minutes)

**Total Estimated Time:** 16-23 hours

### Performance Benchmarks

| Metric | Target | Measured | Status |
|--------|--------|----------|--------|
| Looper latency (recording) | <5ms | ______ ms | ☐ Pass ☐ Fail |
| Looper latency (playback) | <5ms | ______ ms | ☐ Pass ☐ Fail |
| LiveFX processing latency | <5ms | ______ ms | ☐ Pass ☐ Fail |
| MIDI Learn processing | <1ms | ______ ms | ☐ Pass ☐ Fail |
| Footswitch response | <1ms | ______ ms | ☐ Pass ☐ Fail |
| Quick-Save (save) | <200ms | ______ ms | ☐ Pass ☐ Fail |
| Quick-Save (load) | <300ms | ______ ms | ☐ Pass ☐ Fail |

---

## Final Debug Checklist

### Critical Verification Steps

#### Step 1: Build Verification

```bash
# Clean build
Project → Clean → Clean all projects
Project → Build Project

# Expected: 0 errors, 0 warnings (warnings OK)
# Check console for "Build Finished"
```

#### Step 2: Configuration File Verification

**Check Config/module_config.h:**
```c
#define MODULE_ENABLE_USB_MIDI   1  // Must be 1
#define MODULE_ENABLE_USBH_MIDI  0  // Must be 0 (Device only)
```

**Check Core/Src/main.c:**
```c
#include "Config/module_config.h"  // Must be FIRST include

/* USER CODE BEGIN 2 */
#if MODULE_ENABLE_USB_MIDI
  MX_USB_DEVICE_Init();
  usb_midi_init();
#endif
/* USER CODE END 2 */
```

#### Step 3: Clock Configuration Verification

**Check MidiCore.ioc → RCC Configuration:**
- HSE: 8 MHz
- PLL_M: 8
- PLL_N: 336
- PLL_P: 2
- **PLL_Q: 7** ← CRITICAL for USB
- **USB Clock: 48 MHz** ← MUST be exactly 48 MHz

**Formula:** `(8 MHz * 336) / (8 * 7) = 48 MHz` ✓

#### Step 4: USB Hardware Verification

**Physical Connections:**
- [ ] PA11 (D-) connected to USB connector pin 2
- [ ] PA12 (D+) connected to USB connector pin 3
- [ ] USB GND connected to board GND
- [ ] 3.3V stable on STM32
- [ ] 5V VBUS present on USB cable

**Cable Test:**
- [ ] USB cable is data cable (not charge-only)
- [ ] Test with different USB cable
- [ ] Test with different USB port on PC

#### Step 5: GPIO Configuration Verification

**Check .ioc file → Pinout view:**
- PA11: USB_OTG_FS_DM (AF10)
- PA12: USB_OTG_FS_DP (AF10)
- Both pins: Speed = VERY HIGH

#### Step 6: Debugger Register Verification

With ST-Link debugger:
1. Set breakpoint after `MX_USB_DEVICE_Init()`
2. Run and wait for breakpoint
3. Check registers:

```c
RCC->AHB2ENR & RCC_AHB2ENR_OTGFSEN  // Should be non-zero
USB_OTG_FS->GCCFG    // Bit 16=1, Bit 21=1
USB_OTG_FS->GOTGCTL  // Bit 6=1, Bit 7=1
USB_OTG_FS->DCTL     // Bit 1=0 (device NOT soft-disconnected)
```

#### Step 7: Interrupt Verification

**Check Core/Src/stm32f4xx_it.c:**
```c
void OTG_FS_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);  // NOT HAL_HCD_IRQHandler!
}
```

#### Step 8: Windows Device Manager Check

**When plugging USB:**
- Does Device Manager refresh?
- Does Windows play a sound?
- Does anything appear (even "Unknown device")?

### Common Issues and Solutions

**Issue 1: Clock Not 48 MHz**
- Verify PLL_Q = 7 in .ioc file
- Regenerate code

**Issue 2: GPIO Not Configured**
- Check AF10 assignment
- Verify VERY_HIGH speed

**Issue 3: Interrupt Not Working**
- Verify OTG_FS_IRQHandler calls HAL_PCD_IRQHandler
- Check NVIC enabled

**Issue 4: VBUS Sensing Issue (STM32F407 specific)**
- Verify PWRDWN=1, NOVBUSSENS=1
- Check BVALOEN=1, BVALOVAL=1

### Success Criteria

**Device mode working when:**
1. Windows plays USB connect sound
2. "MidiCore 4x4" appears in Device Manager
3. 4 MIDI In/Out ports visible in DAW
4. MIDI data can be sent/received

---

## Summary

This specialized testing guide provides comprehensive coverage for:

✅ **Breath Controller** - Complete hardware validation and calibration  
✅ **Quick Starts** - Fast paths to testing key features  
✅ **USB MIDI** - Device and Host testing procedures  
✅ **Test Execution** - Systematic validation workflow  
✅ **Debug Checklist** - Critical verification steps  

Use these guides to ensure robust, production-ready firmware for MidiCore.

---

*Document Version*: 1.0  
*Last Updated*: January 2026  
*Status*: Complete
