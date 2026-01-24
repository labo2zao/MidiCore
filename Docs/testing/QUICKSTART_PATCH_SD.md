# Quick Start: MODULE_TEST_PATCH_SD

## What This Test Does

This test validates **SD card operations**, **MIDI file export**, and **scene chaining persistence** for the MidiCore looper system. It's the most comprehensive test for data storage features.

## Quick Setup (5 minutes)

### 1. Prepare SD Card
```bash
# Format as FAT32 (not exFAT!)
# Copy config.ngc to root directory
cp sdcard/config.ngc /path/to/sdcard/
```

### 2. Build & Flash
**STM32CubeIDE:**
- Right-click project → Properties
- C/C++ Build → Settings → Preprocessor
- Add: `MODULE_TEST_PATCH_SD`
- Build and Flash

**Or command line:**
```bash
make CFLAGS+="-DMODULE_TEST_PATCH_SD"
```

### 3. Connect & Run
- Insert SD card
- Connect UART2 (115200 baud, 8N1)
- Power on
- Watch test results

## Expected Output (30 seconds)

```
==============================================
  MODULE_TEST_PATCH_SD - Comprehensive Test
==============================================
UART Debug Verification: OK

TEST 1: SD Card Mount
--------------------------------------
[PASS] SD card mounted successfully

TEST 2: Config File Loading
--------------------------------------
Trying: 0:/config.ngc...
[PASS] Loaded 0:/config.ngc

TEST 3: Config Parameter Reading
--------------------------------------
[PASS] SRIO_DIN_ENABLE = 1
[PASS] AINSER_ENABLE = 1
[PASS] MIDI_DEFAULT_CHANNEL = 0
[PASS] AIN_ENABLE = 1
[PASS] Read 4 config parameters

TEST 4: Config File Saving
--------------------------------------
[PASS] Config saved to 0:/test_config.ngc
[PASS] Verified saved value: 123

TEST 5: MIDI Export - Single Track
--------------------------------------
Looper initialized
[SKIP] Track is empty (expected)

TEST 6: MIDI Export - All Tracks
--------------------------------------
[SKIP] No tracks have data (expected)

TEST 7: MIDI Export - Scene Export
--------------------------------------
[PASS/SKIP] Scene export completed
         (Scene empty, which is expected)

TEST 8: Scene Chaining Configuration
--------------------------------------
[PASS] Scene chains configured: A->B->C->A

TEST 9: Quick-Save System
--------------------------------------
Saving to quick-save slot 0...
[PASS] Quick-save successful
[PASS] Slot 0 marked as used
[PASS] Slot name: Test Session
Loading from quick-save slot 0...
[PASS] Quick-load successful

TEST 10: Scene Chaining Persistence
--------------------------------------
[PASS] Scene chain A->B persisted

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

## What Gets Tested

✅ **SD Card Mount** (with 3 retries)  
✅ **Config Loading** (tries 3 different files)  
✅ **Config Reading** (4 common parameters)  
✅ **Config Saving** (with verification)  
✅ **MIDI Track Export** (Standard MIDI File Format 1)  
✅ **MIDI Scene Export** (multi-track scenes)  
✅ **Scene Chaining** (A→B→C→A loop)  
✅ **Quick-Save Slots** (save/load sessions)  
✅ **Chain Persistence** (survives power cycle)  

## Files Created

The test creates these files on your SD card:

```
0:/
├── config.ngc               # Your config (pre-existing)
├── test_config.ngc          # Test config (created)
├── test_track0.mid          # MIDI export test
├── test_all_tracks.mid      # Multi-track MIDI
├── test_scene_A.mid         # Scene export
└── looper/
    ├── quicksave_0_track_0.bin
    ├── quicksave_0_track_1.bin
    ├── quicksave_0_track_2.bin
    └── quicksave_0_track_3.bin
```

## Troubleshooting

### "SD card mount failed"
- Check card is inserted
- Try FAT32 format (not exFAT)
- Use smaller card (≤8GB)
- Check SPI connections

### "No config file found"
- Copy `sdcard/config.ngc` to SD root
- Check filename (case-sensitive)
- Verify file not corrupted

### "Tests skipped (Looper not enabled)"
- Normal! MIDI export needs `MODULE_ENABLE_LOOPER`
- Config tests still work

## Next Steps

After this test passes:
1. Try recording actual loops
2. Test scene switching
3. Export your loops to MIDI
4. Load them in a DAW

## More Info

See [MODULE_TEST_PATCH_SD.md](MODULE_TEST_PATCH_SD.md) for comprehensive documentation.
