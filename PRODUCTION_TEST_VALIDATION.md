# MidiCore Production Test Validation

## Purpose

This document tracks the validation status of all MidiCore module tests as part of the production finalization process. Each test validates a specific hardware module or software service to ensure the complete system works correctly.

## Test Status Overview

| Test ID | Module | Status | Notes |
|---------|--------|--------|-------|
| MODULE_TEST_OLED_SSD1322_ID | OLED Display | ✅ VALIDATED | Display patterns working |
| MODULE_TEST_AINSER64_ID | Analog Inputs | ✅ VALIDATED | 64 channels reading correctly |
| MODULE_TEST_GDB_DEBUG_ID | UART Debug | ✅ VALIDATED | Debug output working |
| MODULE_TEST_SRIO_ID | Digital Inputs | ✅ VALIDATED | Button inputs working |
| MODULE_TEST_MIDI_DIN_ID | MIDI DIN I/O | ✅ VALIDATED | MIDI communication working |
| MODULE_TEST_USB_DEVICE_MIDI_ID | USB MIDI | ✅ VALIDATED | USB MIDI working |
| MODULE_TEST_SRIO_DOUT_ID | LED Outputs | ⏳ PENDING | LED patterns to be tested |
| MODULE_TEST_ROUTER_ID | MIDI Router | ⏳ PENDING | 16-node routing to be tested |
| MODULE_TEST_LOOPER_ID | Looper | ⏳ PENDING | Record/playback to be tested |
| MODULE_TEST_LFO_ID | LFO Module | ⏳ PENDING | Waveforms/modulation to be tested |
| MODULE_TEST_HUMANIZER_ID | Humanizer | ⏳ PENDING | Timing/velocity variation to be tested |
| MODULE_TEST_UI_ID | UI System | ⏳ PENDING | Full UI navigation to be tested |
| MODULE_TEST_UI_PAGE_SONG_ID | Song Mode UI | ⏳ PENDING | Song page to be tested |
| MODULE_TEST_UI_PAGE_MIDI_MONITOR_ID | MIDI Monitor | ⏳ PENDING | MIDI monitor page to be tested |
| MODULE_TEST_UI_PAGE_SYSEX_ID | SysEx UI | ⏳ PENDING | SysEx page to be tested |
| MODULE_TEST_UI_PAGE_CONFIG_ID | Config Editor | ⏳ PENDING | Config page to be tested |
| MODULE_TEST_UI_PAGE_LIVEFX_ID | LiveFX UI | ⏳ PENDING | LiveFX page to be tested |
| MODULE_TEST_UI_PAGE_RHYTHM_ID | Rhythm Trainer | ⏳ PENDING | Rhythm page to be tested |
| MODULE_TEST_UI_PAGE_HUMANIZER_ID | Humanizer UI | ⏳ PENDING | Humanizer page to be tested |
| MODULE_TEST_PATCH_SD_ID | SD Card | ❌ FAILED | Hardware issue: card mount failed |
| MODULE_TEST_PRESSURE_ID | Pressure Sensor | ⏳ PENDING | I2C pressure sensor to be tested |
| MODULE_TEST_BREATH_ID | Breath Controller | ⏳ PENDING | Breath + MIDI CC to be tested |
| MODULE_TEST_USB_HOST_MIDI_ID | USB Host MIDI | ⏳ PENDING | USB host to be tested |
| MODULE_TEST_FOOTSWITCH_ID | Footswitch | ⏳ PENDING | 8 footswitches to be tested |

## Legend

- ✅ **VALIDATED**: Test passed successfully, module confirmed working
- ⏳ **PENDING**: Test not yet run, awaiting validation
- ❌ **FAILED**: Test failed, issue identified (hardware or software)
- 🔧 **IN PROGRESS**: Test currently being debugged/fixed

## Validated Tests (6/24)

### 1. MODULE_TEST_OLED_SSD1322_ID ✅
**Hardware**: OLED SSD1322 256x64 display (Newhaven NHD-3.12)  
**Test**: Display patterns, graphics primitives, UI rendering  
**Result**: PASS - All patterns display correctly  
**Validated**: 2026-01-25

### 2. MODULE_TEST_AINSER64_ID ✅
**Hardware**: AINSER64 analog input module (MCP3208 + 74HC4051)  
**Test**: 64 analog channels, Hall effect sensors, ADC accuracy  
**Result**: PASS - All channels reading correctly  
**Validated**: 2026-01-25

### 3. MODULE_TEST_GDB_DEBUG_ID ✅
**Hardware**: UART debug output (USART2)  
**Test**: Serial communication, baud rate, debug output  
**Result**: PASS - Debug output working at 115200 baud  
**Validated**: 2026-01-25

### 4. MODULE_TEST_SRIO_ID ✅
**Hardware**: SRIO DIN (74HC165 shift registers, button inputs)  
**Test**: Digital input scanning, button detection, MIDI generation  
**Result**: PASS - Button inputs working, MIDI notes generated  
**Validated**: 2026-01-25

### 5. MODULE_TEST_MIDI_DIN_ID ✅
**Hardware**: MIDI DIN I/O (UART-based, 5-pin DIN connectors)  
**Test**: MIDI input/output, message parsing, LiveFX transform  
**Result**: PASS - MIDI communication working bidirectionally  
**Validated**: 2026-01-25

### 6. MODULE_TEST_USB_DEVICE_MIDI_ID ✅
**Hardware**: USB Device MIDI (USB 2.0 Full Speed)  
**Test**: USB MIDI class device, DAW communication, bidirectional MIDI  
**Result**: PASS - USB MIDI working with computer  
**Validated**: 2026-01-25

## Pending Tests (17/24)

### Priority 1: Core Hardware (7 tests)

#### MODULE_TEST_SRIO_DOUT_ID
**Hardware**: SRIO DOUT (74HC595 shift registers, LED outputs)  
**Test**: LED patterns, visual verification of all 64 outputs  
**Why Important**: Visual feedback system for user interface  
**How to Test**: Flash with flag, observe LED patterns cycling  

#### MODULE_TEST_ROUTER_ID
**Hardware**: MIDI Router (16 nodes, software-based)  
**Test**: Message routing, node mapping, latency  
**Why Important**: Central MIDI message distribution  
**How to Test**: Send MIDI to various inputs, verify routing to outputs  

#### MODULE_TEST_PRESSURE_ID
**Hardware**: I2C pressure sensor (bellows pressure)  
**Test**: I2C communication, pressure reading accuracy  
**Why Important**: Accordion bellows dynamics  
**How to Test**: Read sensor values, verify I2C bus operation  

#### MODULE_TEST_BREATH_ID
**Hardware**: Breath controller (pressure + MIDI CC)  
**Test**: Pressure to MIDI CC mapping, expression control  
**Why Important**: Accordion expression dynamics  
**How to Test**: Apply pressure, verify CC output  

#### MODULE_TEST_USB_HOST_MIDI_ID
**Hardware**: USB Host MIDI (STM32 OTG HS in host mode)  
**Test**: USB host operation, external MIDI device connection  
**Why Important**: Connect USB MIDI keyboards/controllers  
**How to Test**: Connect USB MIDI device, verify communication  

#### MODULE_TEST_FOOTSWITCH_ID
**Hardware**: 8 footswitches (via SRIO DIN)  
**Test**: Footswitch input mapping, function assignment  
**Why Important**: Hands-free control during performance  
**How to Test**: Press footswitches, verify MIDI/function output  

#### MODULE_TEST_PATCH_SD_ID ⚠️
**Hardware**: SD card via SPI (FAT32 filesystem)  
**Test**: Mount, read config.ngc, patch loading  
**Current Status**: FAILED - Mount error  
**Why Important**: Patch storage and loading system  
**Known Issue**: SD card not inserted, not FAT32, or SPI hardware issue  
**How to Fix**: 
1. Verify SD card is inserted
2. Format as FAT32
3. Check SPI connections (MOSI, MISO, SCK, CS)
4. Verify write protection is OFF
5. Create test config.ngc file on card

### Priority 2: Services (3 tests)

#### MODULE_TEST_LOOPER_ID
**Software**: Looper service (4 tracks, record/playback)  
**Test**: MIDI recording, playback, undo, automation  
**Why Important**: Core looper functionality for accordion  
**How to Test**: Record MIDI, playback, test undo, verify automation  

#### MODULE_TEST_LFO_ID
**Software**: LFO service (waveforms, modulation)  
**Test**: Waveform generation, MIDI CC modulation  
**Why Important**: Automatic modulation effects  
**How to Test**: Enable LFO, verify CC output, test waveforms  

#### MODULE_TEST_HUMANIZER_ID
**Software**: Humanizer service (timing/velocity variation)  
**Test**: MIDI humanization, groove templates  
**Why Important**: Natural-sounding MIDI playback  
**How to Test**: Play MIDI, verify timing/velocity variation  

### Priority 3: UI Pages (8 tests)

All UI page tests validate specific user interface pages:

#### MODULE_TEST_UI_ID
**Test**: General UI navigation, page switching, encoder/button input  
**Why Important**: Overall UI system functionality  

#### MODULE_TEST_UI_PAGE_SONG_ID
**Test**: Song mode page, song selection, playback control  
**Why Important**: Song/setlist management  

#### MODULE_TEST_UI_PAGE_MIDI_MONITOR_ID
**Test**: MIDI monitor display, realtime message viewing  
**Why Important**: MIDI debugging and troubleshooting  

#### MODULE_TEST_UI_PAGE_SYSEX_ID
**Test**: SysEx page, system exclusive message handling  
**Why Important**: Advanced MIDI configuration  

#### MODULE_TEST_UI_PAGE_CONFIG_ID
**Test**: Config editor page, parameter editing  
**Why Important**: System configuration management  

#### MODULE_TEST_UI_PAGE_LIVEFX_ID
**Test**: LiveFX page, real-time effect control  
**Why Important**: Performance effects control  

#### MODULE_TEST_UI_PAGE_RHYTHM_ID
**Test**: Rhythm trainer page, click track, metronome  
**Why Important**: Practice and timing assistance  

#### MODULE_TEST_UI_PAGE_HUMANIZER_ID
**Test**: Humanizer/LFO page, modulation control  
**Why Important**: Humanization parameter adjustment  

## Test Execution Guide

### How to Run a Specific Test

1. **Configure test in `Config/module_config.h`**:
```c
#define PRODUCTION_MODE 0  // Enable dev/test mode
#define MODULE_TEST_OLED 0 // Disable OLED test (if enabling other test)
```

2. **Select test in compile-time or runtime config**  
   (See test_config_runtime.c for runtime selection)

3. **Build and flash**:
```bash
# Clean build
rm -rf Debug/
# Build in STM32CubeIDE
# Flash to hardware
```

4. **Observe test behavior**:
   - UART output for debug messages
   - OLED display for visual feedback
   - MIDI output for MIDI tests
   - LED patterns for DOUT tests

5. **Document results**:
   - Update this file with PASS/FAIL status
   - Note any issues discovered
   - Record test date

### Test Environment Requirements

**Hardware**:
- STM32F407VGT6 board (MidiCore hardware)
- OLED display connected
- UART debug cable (115200 baud)
- Test hardware for specific module (buttons, sensors, MIDI devices, etc.)

**Software**:
- STM32CubeIDE 2.0.0 or later
- ST-Link programmer/debugger
- Serial terminal (PuTTY, screen, etc.)

**Tools**:
- Multimeter (for hardware verification)
- Logic analyzer (for SPI/I2C debugging)
- MIDI monitor software (for MIDI testing)
- SD card reader (for patch testing)

## Known Issues

### 1. SD Card Mount Failure ❌
**Test**: MODULE_TEST_PATCH_SD_ID  
**Error**: `[FAIL] SD card mount failed!`  
**Root Cause**: Hardware issue (not code-related)  
**Possible Causes**:
1. SD card not inserted
2. Card not formatted as FAT32
3. SPI connection problem (MOSI, MISO, SCK, CS)
4. Card write-protected
5. Card not compatible (use Class 4-10, 2GB-32GB)

**Fix**: Hardware verification and card preparation required

### 2. FreeRTOS Heap Too Small (FIXED) ✅
**Issue**: Task creation failed with 1KB heap  
**Fix**: Restored to 10KB (commit adde9c1)  
**Status**: RESOLVED

### 3. OLED Test Not Running (FIXED) ✅
**Issue**: Preprocessor logic error in test selection  
**Fix**: Changed from `defined()` to value check (commit 2274496)  
**Status**: RESOLVED

## Test Statistics

- **Total Tests**: 24
- **Validated**: 6 (25%)
- **Pending**: 17 (71%)
- **Failed**: 1 (4%) - Hardware issue
- **Coverage**: 25% complete

## Next Steps

1. **Immediate** (Priority 1): Test core hardware modules
   - SRIO DOUT (LEDs)
   - MIDI Router
   - Pressure sensor
   - Footswitches

2. **Short-term** (Priority 2): Test services
   - Looper
   - LFO
   - Humanizer

3. **Medium-term** (Priority 3): Test all UI pages
   - General UI
   - 7 specific UI pages

4. **Hardware Fix**: Resolve SD card mount issue
   - Verify card inserted and formatted
   - Check SPI connections
   - Test with different SD card

## Production Readiness Criteria

For production deployment, the following must be validated:

**Critical (Must Pass)**:
- ✅ OLED Display
- ✅ Analog Inputs (AINSER64)
- ✅ Digital Inputs (SRIO)
- ✅ MIDI DIN I/O
- ⏳ LED Outputs (SRIO DOUT)
- ⏳ MIDI Router
- ⏳ Looper Core Functions

**Important (Should Pass)**:
- ⏳ Pressure Sensor
- ⏳ USB Device MIDI (already validated)
- ⏳ General UI Navigation
- ⏳ Main UI Pages (song, config)

**Optional (Nice to Have)**:
- ⏳ USB Host MIDI
- ⏳ Advanced UI Pages
- ⏳ LFO/Humanizer
- ❌ SD Card (can use defaults without SD)

## Conclusion

Production mode is **PARTIALLY READY** (25% validated):
- ✅ Memory optimization complete
- ✅ Boot process working
- ✅ Core hardware validated (OLED, inputs, MIDI)
- ⏳ Remaining hardware modules to be tested
- ⏳ Service modules to be validated
- ⏳ UI system to be fully tested

**Next milestone**: Complete Priority 1 hardware tests to reach 50% validation.
