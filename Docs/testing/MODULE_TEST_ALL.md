# MODULE_TEST_ALL - Comprehensive Test Runner

## Overview

The `MODULE_TEST_ALL` test provides automated sequential execution of all finite tests in the MidiCore system. This enables comprehensive system validation in a single test run, ideal for CI/CD pipelines and production testing.

## Purpose

Run all tests that complete and return (as opposed to tests that loop forever) in a single automated session, providing aggregated pass/fail statistics and detailed per-test results.

## Tests Included

MODULE_TEST_ALL executes these finite tests:

### 1. OLED_SSD1322 Driver Test
**Duration:** ~10-15 seconds  
**What it tests:**
- GPIO pin control and verification
- Software SPI bit-bang implementation
- Display initialization sequence
- Pattern rendering (white, black, checkerboard, stripes, gradient)
- Timing validation

**Returns:** 0 on success, negative on error

### 2. PATCH_SD Test  
**Duration:** ~5-10 seconds  
**What it tests:**
- SD card mount with retry logic
- Config file loading (multiple fallback paths)
- Config parameter reading and validation
- Config file saving with verification
- MIDI export (track/scene/all formats)
- Scene chaining configuration
- Quick-save system (8 slots)
- Scene chain persistence

**Returns:** 0 on success, negative on error

## Tests Excluded

The following tests run in infinite loops and cannot be included:

- **MODULE_TEST_GDB_DEBUG** - UART verification (continuous output)
- **MODULE_TEST_AINSER64** - Analog input scanning (continuous read)
- **MODULE_TEST_SRIO** - Digital input monitoring (continuous scan)
- **MODULE_TEST_SRIO_DOUT** - LED pattern cycling (continuous display)
- **MODULE_TEST_MIDI_DIN** - MIDI I/O echo (continuous processing)
- **MODULE_TEST_ROUTER** - MIDI routing (continuous operation)
- **MODULE_TEST_LOOPER** - Loop recording/playback (continuous)
- **MODULE_TEST_LFO** - Waveform generation (continuous output)
- **MODULE_TEST_HUMANIZER** - MIDI humanization (continuous processing)
- **MODULE_TEST_UI** - UI interaction (continuous display)
- **MODULE_TEST_UI_PAGE_*** - All UI page tests (continuous)
- **MODULE_TEST_PRESSURE** - Pressure sensor (continuous read)
- **MODULE_TEST_USB_HOST_MIDI** - USB Host (continuous monitoring)
- **MODULE_TEST_USB_DEVICE_MIDI** - USB Device (continuous I/O)

**Note:** Run these tests individually when needed for specific module validation.

## How to Run

### STM32CubeIDE
1. Right-click project → Properties
2. C/C++ Build → Settings → MCU GCC Compiler → Preprocessor
3. Add define: `MODULE_TEST_ALL`
4. Build and flash
5. Connect UART2 (115200 baud, 8N1)
6. Observe test output

### Command Line
```bash
make CFLAGS+="-DMODULE_TEST_ALL"
```

## Example Output

### Successful Run
```
==============================================
   MODULE_TEST_ALL - Comprehensive Suite
==============================================

==============================================
Running: MODULE_TEST_OLED_SSD1322
==============================================
[Test output for OLED...]
[PASS] MODULE_TEST_OLED_SSD1322 completed successfully

==============================================
Running: MODULE_TEST_PATCH_SD
==============================================
[Test output for PATCH_SD...]
[PASS] MODULE_TEST_PATCH_SD completed successfully

==============================================
       MODULE_TEST_ALL - FINAL SUMMARY
==============================================

Individual Test Results:
----------------------------------------------
  OLED_SSD1322    : [PASS]
  PATCH_SD        : [PASS]

Test Statistics:
----------------------------------------------
Tests Passed:  2
Tests Failed:  0
Tests Skipped: 0
Total Run:     2
----------------------------------------------

RESULT: ALL TESTS PASSED!

All finite tests completed successfully.
System validated and ready for operation.

Note: Tests that run forever are not included:
  - AINSER64, SRIO, MIDI_DIN, Router, Looper,
  - LFO, Humanizer, UI pages, Pressure,
  - USB Host/Device MIDI
  Run these tests individually for validation.
==============================================
```

### With Failures
```
==============================================
       MODULE_TEST_ALL - FINAL SUMMARY
==============================================

Individual Test Results:
----------------------------------------------
  OLED_SSD1322    : [PASS]
  PATCH_SD        : [FAIL]

Test Statistics:
----------------------------------------------
Tests Passed:  1
Tests Failed:  1
Tests Skipped: 0
Total Run:     2
----------------------------------------------

RESULT: SOME TESTS FAILED

Please review failed tests above and check:
  - Hardware connections
  - Module configurations
  - Required peripherals present
```

### With Skipped Tests
```
==============================================
       MODULE_TEST_ALL - FINAL SUMMARY
==============================================

Individual Test Results:
----------------------------------------------
  OLED_SSD1322    : [SKIP]
  PATCH_SD        : [PASS]

Test Statistics:
----------------------------------------------
Tests Passed:  1
Tests Failed:  0
Tests Skipped: 1
Total Run:     1
----------------------------------------------

RESULT: ALL TESTS PASSED!
```

## Return Values

- **0** - All tests passed (at least one test ran)
- **-1** - One or more tests failed, OR no tests ran (all skipped)

## Requirements

### Hardware
- STM32F407VG microcontroller
- OLED SSD1322 display (256x64) - optional, test skipped if `MODULE_ENABLE_OLED` disabled
- SD card with FAT32 filesystem - optional, test skipped if `MODULE_ENABLE_PATCH` disabled
- UART connection for debug output (115200 baud)

### Software
- At least one of:
  - `MODULE_ENABLE_OLED` (for OLED test)
  - `MODULE_ENABLE_PATCH` (for PATCH_SD test)
- If both disabled, returns -1 (no tests run)

## Performance

### Timing
- **Total runtime:** 15-25 seconds (varies by enabled tests)
  - OLED_SSD1322: ~10-15 seconds
  - PATCH_SD: ~5-10 seconds
- **Delays between tests:** 500ms
- **Startup overhead:** 200ms

### Memory
- **Stack usage:** ~3 KB (combines both tests)
- **Heap usage:** None (all stack-based)
- **Result tracking:** ~100 bytes (test_result_t array)

## Use Cases

### 1. Automated CI/CD Testing
```bash
#!/bin/bash
# Build and flash with MODULE_TEST_ALL
make clean
make CFLAGS+="-DMODULE_TEST_ALL"
make flash

# Capture UART output and check result
# Exit code 0 = all tests passed
```

### 2. Production Testing
- Flash device with MODULE_TEST_ALL enabled
- Connect UART and SD card
- Power on and wait ~20 seconds
- Check UART for "ALL TESTS PASSED"
- If pass, device validated for shipment

### 3. Regression Testing
- Run after firmware updates
- Verify no functionality broken
- Quick validation of critical systems

### 4. System Validation
- Verify hardware assembly correct
- Check all required peripherals present
- Validate connections and configurations

## Integration with CI/CD

### GitHub Actions Example
```yaml
name: Hardware Test

on: [push, pull_request]

jobs:
  test:
    runs-on: self-hosted  # Runner with STM32 hardware
    steps:
      - uses: actions/checkout@v2
      
      - name: Build test firmware
        run: make CFLAGS+="-DMODULE_TEST_ALL"
      
      - name: Flash to device
        run: make flash
      
      - name: Run tests and capture output
        run: |
          timeout 30 cat /dev/ttyUSB0 > test_output.txt &
          sleep 25
          
      - name: Check results
        run: |
          if grep -q "ALL TESTS PASSED" test_output.txt; then
            echo "✅ All tests passed"
            exit 0
          else
            echo "❌ Tests failed"
            cat test_output.txt
            exit 1
          fi
```

## Troubleshooting

### "NO TESTS RUN"
**Cause:** Both `MODULE_ENABLE_OLED` and `MODULE_ENABLE_PATCH` are disabled

**Solution:**
- Enable at least one module in `Config/module_config.h`
- Or run individual tests directly

### "SOME TESTS FAILED"
**Cause:** One or more tests encountered errors

**Solution:**
1. Review UART output for specific test that failed
2. Check that test's documentation for troubleshooting
3. Verify hardware connections for failed test
4. Run failed test individually for more detail

### Test Hangs
**Cause:** One test stuck or timeout issue

**Solution:**
- Check UART output to see which test is running
- Verify that test's hardware requirements met
- Power cycle device and retry

## Limitations

1. **Only Finite Tests** - Cannot include tests that run forever
2. **Sequential Execution** - Tests run one at a time (not parallel)
3. **No Partial Retry** - Must re-run all tests if one fails
4. **Fixed Order** - Tests always run in same sequence

## Future Enhancements

Potential improvements for future versions:
- Test selection via runtime configuration
- Parallel test execution where possible
- Test timeout enforcement
- Performance benchmarking mode
- Log to SD card option
- JSON output format for parsing

## Related Documentation

- [MODULE_TEST_PATCH_SD.md](MODULE_TEST_PATCH_SD.md) - PATCH_SD test details
- [README_MODULE_TESTING.md](README_MODULE_TESTING.md) - General test framework
- [TESTING_QUICKSTART.md](TESTING_QUICKSTART.md) - Quick start guide

## Version History

- **v1.0** (2026-01-23) - Initial implementation
  - Sequential execution of OLED_SSD1322 and PATCH_SD
  - Aggregated statistics and reporting
  - Module availability checking
  - Comprehensive documentation

## Authors

- Implementation: Copilot Coding Agent
- Integration: MidiCore Project Team
