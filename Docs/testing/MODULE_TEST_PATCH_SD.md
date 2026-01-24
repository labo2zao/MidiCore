# MODULE_TEST_PATCH_SD - Comprehensive SD Card Testing

## Overview

The `MODULE_TEST_PATCH_SD` test provides comprehensive validation of SD card functionality including configuration management, MIDI file export, and scene chaining persistence. This test is essential for verifying the complete data storage and retrieval pipeline.

## Purpose

This test validates three critical subsystems:
1. **SD Card Configuration** - Loading and saving configuration files
2. **MIDI Export** - Exporting looper data to standard MIDI files
3. **Scene Chaining Persistence** - Saving and restoring scene chains across power cycles

## Requirements

### Hardware
- STM32F407VG microcontroller
- SD card socket with SPI interface
- FAT32-formatted SD card (8GB or smaller recommended)
- UART connection for debug output (115200 baud)

### Software
- `MODULE_ENABLE_PATCH` must be enabled in `Config/module_config.h`
- `MODULE_ENABLE_LOOPER` should be enabled for MIDI export tests (optional)
- FATFS middleware configured and operational

## How to Run

### STM32CubeIDE
1. Right-click project → Properties
2. C/C++ Build → Settings → MCU GCC Compiler → Preprocessor
3. Add define: `MODULE_TEST_PATCH_SD`
4. Build and flash the project
5. Connect UART2 to serial terminal (115200 baud, 8N1)
6. Insert SD card with `config.ngc` file
7. Power on device and observe test output

### Command Line
```bash
make CFLAGS+="-DMODULE_TEST_PATCH_SD"
```

## Test Sequence

The test runs 10 comprehensive tests in sequence:

### Test 1: SD Card Mount
**Purpose:** Verify SD card detection and FAT32 mounting

**Expected Output:**
```
TEST 1: SD Card Mount
--------------------------------------
[PASS] SD card mounted successfully
```

**Troubleshooting:**
- Check SD card is inserted
- Verify FAT32 format
- Check SPI connections

### Test 2: Config File Loading
**Purpose:** Load configuration from SD card

**Expected Output:**
```
TEST 2: Config File Loading
--------------------------------------
Trying: 0:/config.ngc...
[PASS] Loaded 0:/config.ngc
```

**Tested Files:**
- `0:/config.ngc` (primary)
- `0:/config_minimal.ngc` (fallback)
- `0:/config_full.ngc` (fallback)

### Test 3: Config Parameter Reading
**Purpose:** Verify ability to read configuration parameters

**Expected Output:**
```
TEST 3: Config Parameter Reading
--------------------------------------
[PASS] SRIO_DIN_ENABLE = 1
[PASS] AINSER_ENABLE = 1
[PASS] MIDI_DEFAULT_CHANNEL = 0
[PASS] AIN_ENABLE = 1
[PASS] Read 4 config parameters
```

### Test 4: Config File Saving
**Purpose:** Test config write capability and verification

**Expected Output:**
```
TEST 4: Config File Saving
--------------------------------------
[PASS] Config saved to 0:/test_config.ngc
[PASS] Verified saved value: 123
```

**Created Files:**
- `0:/test_config.ngc` - Test configuration file

### Test 5: MIDI Export - Single Track
**Purpose:** Export single looper track to MIDI file

**Expected Output:**
```
TEST 5: MIDI Export - Single Track
--------------------------------------
Looper initialized
[SKIP] Track is empty (expected)
```

**Note:** With empty tracks, returns `-2` which is expected behavior

**Created Files:**
- `0:/test_track0.mid` (if track has data)

### Test 6: MIDI Export - All Tracks
**Purpose:** Export all 4 tracks to multi-track MIDI file

**Expected Output:**
```
TEST 6: MIDI Export - All Tracks
--------------------------------------
[SKIP] No tracks have data (expected)
```

**Created Files:**
- `0:/test_all_tracks.mid` (if any track has data)

### Test 7: MIDI Export - Scene Export
**Purpose:** Export specific scene to MIDI file

**Expected Output:**
```
TEST 7: MIDI Export - Scene Export
--------------------------------------
[PASS/SKIP] Scene export completed
         (Scene empty, which is expected)
```

**Created Files:**
- `0:/test_scene_A.mid` (if scene has data)

### Test 8: Scene Chaining Configuration
**Purpose:** Configure and verify scene chains (A→B→C→A loop)

**Expected Output:**
```
TEST 8: Scene Chaining Configuration
--------------------------------------
[PASS] Scene chains configured: A->B->C->A
```

**Tested Features:**
- `looper_set_scene_chain()` - Configure chain
- `looper_get_scene_chain()` - Read next scene
- `looper_is_scene_chain_enabled()` - Check enabled status

### Test 9: Quick-Save System
**Purpose:** Test session save/load to persistent slots

**Expected Output:**
```
TEST 9: Quick-Save System
--------------------------------------
Saving to quick-save slot 0...
[PASS] Quick-save successful
[PASS] Slot 0 marked as used
[PASS] Slot name: Test Session
Loading from quick-save slot 0...
[PASS] Quick-load successful
```

**Created Files:**
- `0:/looper/quicksave_0_track_0.bin`
- `0:/looper/quicksave_0_track_1.bin`
- `0:/looper/quicksave_0_track_2.bin`
- `0:/looper/quicksave_0_track_3.bin`

### Test 10: Scene Chaining Persistence
**Purpose:** Verify scene chains survive save/load cycle

**Expected Output:**
```
TEST 10: Scene Chaining Persistence
--------------------------------------
[PASS] Scene chain A->B persisted
```

## Test Summary

At the end of execution, a comprehensive summary is displayed:

### Successful Run Example
```
==============================================
            TEST SUMMARY
==============================================
Tests Passed: 10
Tests Failed: 0
Total Tests:  10
----------------------------------------------
RESULT: ALL TESTS PASSED!

Features Verified:
  - SD card mount/unmount
  - Config file load/save
  - Config parameter read/write
  - MIDI export (track/scene/all)
  - Scene chaining configuration
  - Quick-save system
  - Scene chain persistence
==============================================
```

### Failed Run Example
```
==============================================
            TEST SUMMARY
==============================================
Tests Passed: 3
Tests Failed: 2
Total Tests:  5
----------------------------------------------
RESULT: SOME TESTS FAILED

Troubleshooting:
  1. Check SD card is inserted
  2. Verify card is FAT32 formatted
  3. Check SPI connections
  4. Verify write protection is off
  5. Ensure config.ngc exists on card
==============================================
```

## Files Created During Test

The test creates several files on the SD card:

| File | Purpose | Size |
|------|---------|------|
| `0:/test_config.ngc` | Test configuration file | ~1 KB |
| `0:/test_track0.mid` | Single track MIDI export | Variable |
| `0:/test_all_tracks.mid` | Multi-track MIDI export | Variable |
| `0:/test_scene_A.mid` | Scene MIDI export | Variable |
| `0:/looper/quicksave_0_track_*.bin` | Quick-save session data | ~10-50 KB per track |

## Integration with Existing Systems

### Config System (`Services/patch/`)
- Uses `patch_adv` for advanced config management
- Supports `.NGC` format (MIDIbox NG compatible)
- Section-based key-value pairs
- Conditional evaluation support

### Looper System (`Services/looper/`)
- 4-track looper with scene management
- 8 scenes (A-H) with chaining support
- Quick-save slots (0-7) for session persistence
- Standard MIDI File (SMF) Format 1 export

### File System (`Services/fs/`)
- FATFS integration via STM32 middleware
- SD card guard for thread-safe access
- Atomic file operations support

## Return Values

The test function returns:
- `0` - All tests passed
- `-1` - One or more tests failed or module disabled

## Troubleshooting

### SD Card Mount Failed
**Symptoms:** Test 1 fails immediately

**Solutions:**
1. Check SD card is properly inserted
2. Verify card is FAT32 formatted (not exFAT)
3. Try smaller card (≤8GB more reliable)
4. Check SPI pins in `Config/sd_pins.h`
5. Verify SD_CS pin configuration
6. Check 3.3V power supply to SD card

### Config File Not Found
**Symptoms:** Test 2 fails

**Solutions:**
1. Copy `sdcard/config.ngc` to SD card root
2. Rename file to exactly `config.ngc` (case-sensitive)
3. Verify file is not corrupted
4. Check file permissions

### MIDI Export Fails
**Symptoms:** Tests 5-7 fail or show errors

**Solutions:**
1. Verify `MODULE_ENABLE_LOOPER` is enabled
2. Check SD card has write permissions
3. Ensure sufficient free space on card
4. Verify looper initialization succeeded

### Quick-Save Fails
**Symptoms:** Test 9 fails

**Solutions:**
1. Check write permissions on SD card
2. Verify `/looper/` directory exists (created automatically)
3. Ensure sufficient free space (~200 KB needed)
4. Check for SD card errors in UART output

### Scene Chains Not Persisted
**Symptoms:** Test 10 fails

**Solutions:**
1. Verify Test 9 passed (quick-save prerequisite)
2. Check that scene chains were set in Test 8
3. Ensure quick-save includes scene metadata

## Performance Notes

### Timing
- Total test duration: ~5-10 seconds
- SD card mount: ~500-2000 ms (with retries)
- Config load: ~100-500 ms
- Config save: ~100-500 ms
- MIDI export: ~50-500 ms per file
- Quick-save: ~100-1000 ms depending on data size

### Memory Usage
- Stack: ~2 KB for test execution
- Heap: Minimal (uses static buffers where possible)
- FATFS buffers: ~512 bytes per file handle

## Related Documentation

- [Module Testing Guide](README_MODULE_TESTING.md) - General test framework
- [Testing Quickstart](TESTING_QUICKSTART.md) - How to run tests
- [SD Card README](../../sdcard/README.md) - Config file format
- [Looper API](../../Services/looper/looper.h) - Looper functions

## Module Dependencies

This test requires:
- `MODULE_ENABLE_PATCH` (mandatory)
- `MODULE_ENABLE_LOOPER` (optional, for MIDI export)
- FATFS middleware (mandatory)
- SPI interface for SD card (mandatory)

## Version History

- **v1.0** (2026-01-23) - Initial comprehensive implementation
  - All 10 tests implemented
  - Full UART diagnostics
  - Scene chaining persistence
  - Quick-save system validation

## Authors

- Implementation: Copilot Coding Agent
- Integration: MidiCore Project Team
- Based on MIOS32 framework concepts
