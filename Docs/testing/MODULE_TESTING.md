# Module Testing Guide for MidiCore

## Table of Contents
- [Overview](#overview)
- [Architecture](#architecture)
- [Available Module Tests](#available-module-tests)
- [How to Use](#how-to-use)
- [Test Examples](#test-examples)
- [MODULE_TEST_ALL - Comprehensive Test Runner](#module_test_all---comprehensive-test-runner)
- [Module-Specific Tests](#module-specific-tests)
- [Adding New Tests](#adding-new-tests)
- [Troubleshooting](#troubleshooting)
- [Best Practices](#best-practices)

---

## Overview

MidiCore provides a unified module testing framework that allows testing individual modules in isolation. This facilitates development, debugging, and validation of each component independently.

### Production vs Test Mode

- **Production Mode**: The default behavior. `StartDefaultTask` runs the full application via `app_entry_start()`.
- **Test Mode**: When a `MODULE_TEST_xxx` define is set at compile time, `StartDefaultTask` runs the specific module test instead.

### Key Files

- `App/tests/module_tests.h` - Test framework API
- `App/tests/module_tests.c` - Test implementations
- `App/tests/app_test_task.h` - Dedicated test task (optional)
- `App/tests/app_test_task.c` - Test task implementation
- `Core/Src/main.c` - Modified `StartDefaultTask` with test support

---

## Architecture

### Test Framework Features

1. **Compile-Time Selection**: Choose test via preprocessor defines
2. **Runtime Selection**: Select test via SD card configuration (future)
3. **Isolated Testing**: Each test runs independently
4. **Production Safety**: Easy to disable all tests for release builds
5. **MIOS32 Compatibility**: Full hardware compatibility with MIOS32 pinout

---

## Available Module Tests

| Test Name | Define | Description | Duration |
|-----------|--------|-------------|----------|
| **AINSER64** | `MODULE_TEST_AINSER64` | 64-channel analog input (MCP3208 + mux) | Continuous |
| **SRIO** | `MODULE_TEST_SRIO` | Shift register DIN/DOUT (74HC165/595) | Continuous |
| **MIDI DIN** | `MODULE_TEST_MIDI_DIN` | MIDI DIN I/O with LiveFX transform and MIDI learn | Continuous |
| **Router** | `MODULE_TEST_ROUTER` | MIDI router matrix (comprehensive 8-phase test) | ~5 seconds |
| **Looper** | `MODULE_TEST_LOOPER` | MIDI looper recording/playback | Continuous |
| **UI** | `MODULE_TEST_UI` | OLED display and user interface | ~2-3 minutes |
| **Patch/SD** | `MODULE_TEST_PATCH_SD` | SD card config, MIDI export, scene chaining | ~10 seconds |
| **Pressure** | `MODULE_TEST_PRESSURE` | I2C pressure sensor (XGZP6847) | Continuous |
| **Breath** | `MODULE_TEST_BREATH` | Breath controller (pressure + MIDI CC) | Continuous |
| **USB Host MIDI** | `MODULE_TEST_USB_HOST_MIDI` | USB Host MIDI communication | Continuous |
| **USB Device MIDI** | `MODULE_TEST_USB_DEVICE_MIDI` | USB Device MIDI (DAW connection) | Continuous |
| **All Tests** | `MODULE_TEST_ALL` | Runs all finite tests sequentially (OLED + PATCH_SD) | ~20 seconds |

**Note:** The table above shows **preprocessor defines** you use in your build configuration. The internal enum values (in code) have an `_ID` suffix (e.g., `MODULE_TEST_AINSER64_ID`) to avoid naming conflicts.

---

## How to Use

### Method 1: Compiler Defines (Recommended)

Add the test define to your build configuration:

**STM32CubeIDE:**
1. Right-click project → Properties
2. C/C++ Build → Settings
3. MCU GCC Compiler → Preprocessor
4. Add define: `MODULE_TEST_AINSER64` (or any other test)
5. Build and flash

**Note:** You can optionally add `=1` (e.g., `MODULE_TEST_AINSER64=1`), but it's not required. Just `MODULE_TEST_AINSER64` works fine.

**Command Line:**
```bash
# Example: Test AINSER64 module
make CFLAGS+="-DMODULE_TEST_AINSER64"

# Example: Test MIDI DIN module
make CFLAGS+="-DMODULE_TEST_MIDI_DIN"

# Optional: select DIN UART port (0-3) for MIDI IN/OUT
# (avoid UART2 if it is used for debug at 115200 during tests)
make CFLAGS+="-DMODULE_TEST_MIDI_DIN -DTEST_MIDI_DIN_UART_PORT=2"

# You can also use =1 syntax if preferred
make CFLAGS+="-DMODULE_TEST_SRIO=1"
```

### Method 2: Modify module_tests.c

For quick testing, you can temporarily modify the compile-time selection in `module_tests.c`:

```c
module_test_t module_tests_get_compile_time_selection(void)
{
  // Force a specific test (use enum ID values)
  return MODULE_TEST_AINSER64_ID;
  
  // ... rest of function
}
```

**Note:** The enum values use `_ID` suffix (e.g., `MODULE_TEST_AINSER64_ID`) to avoid conflicts with preprocessor defines (e.g., `MODULE_TEST_AINSER64`).

### Method 3: Use Existing Legacy Defines

The new framework is backward compatible with existing test defines:

- `APP_TEST_DIN_MIDI` → `MODULE_TEST_MIDI_DIN`
- `APP_TEST_AINSER_MIDI` → `MODULE_TEST_AINSER64`
- `DIN_SELFTEST` → `MODULE_TEST_SRIO`
- `LOOPER_SELFTEST` → `MODULE_TEST_LOOPER`

---

## Test Examples

### Example 1: Test AINSER64 Analog Inputs

```bash
# Compile with AINSER64 test
make CFLAGS+="-DMODULE_TEST_AINSER64"

# Flash to device
make flash

# Connect UART2 to see debug output
# The test will continuously scan all 64 analog channels
```

**Expected Output:**
- UART debug messages showing channel values
- Values changing as analog inputs change

### Example 2: Test SRIO Digital Inputs

```bash
# Compile with SRIO test
make CFLAGS+="-DMODULE_TEST_SRIO"

# Connect UART to see DIN values
# Press buttons connected to shift registers
```

**Expected Output:**
- Hexadecimal dump of DIN register values
- Values change when buttons are pressed/released

**Troubleshooting:**
- If nothing changes, verify `/PL` and `RCLK` pin mapping in `main.h`.
- By default, MIOS32-compatible pins are used (`MIOS_SPI1_RC2` and `OLED_CS`).
- Define `SRIO_USE_EXPLICIT_PINS` to force `SRIO_RC2` (`/PL`) and `SRIO_RC1` (`RCLK`) instead.
- `MODULE_TEST_SRIO` automatically enables `SRIO_ENABLE` for the build.
- The SRIO test prints SPI CPOL/CPHA and prescaler alongside the pinout; SRIO defaults to MIOS32 mode (CPOL=LOW, CPHA=2EDGE) and prescaler 128 unless overridden.
- If DIN reads stay fixed (e.g., always `0xFF`), verify `/PL` polarity; set `SRIO_DIN_PL_ACTIVE_LOW=0` if RC2 is inverted on your board.
- If the chain is unstable, reduce SRIO SPI speed with `SRIO_SPI_PRESCALER` (default: 128).

### Example 3: Test MIDI Router (Comprehensive)

```bash
# Compile with Router test
make CFLAGS+="-DMODULE_TEST_ROUTER=1"

# Monitor UART output to see test phases
```

**Expected Output:**
- Phase 1: Router initialization with node mapping
- Phase 2: Basic routing configuration (3 routes)
- Phase 3: Channel filtering tests
- Phase 4: All message types tested (Note, CC, PC, Pressure, Pitch Bend)
- Phase 5: Multi-destination routing (1→3 outputs)
- Phase 6: Dynamic route enable/disable
- Phase 7: Channel mask validation (16 channels)
- Phase 8: Complete routing table display
- Test summary with ✓ indicators
- Continuous monitoring mode with periodic status

**Test Duration:** ~5 seconds automated tests + continuous monitoring

**Features Validated:**
- 16x16 routing matrix functionality
- Per-route channel filtering (16-bit chanmask)
- All MIDI message types routing
- Multi-destination routing
- Route labels and modification
- Real-time route changes

### Example 4: Test MIDI DIN with LiveFX and MIDI Learn

```bash
# Compile with MIDI DIN test (includes LiveFX and MIDI learn)
make CFLAGS+="-DMODULE_TEST_MIDI_DIN"

# Optional: choose DIN UART port (0-3) if needed
make CFLAGS+="-DMODULE_TEST_MIDI_DIN -DTEST_MIDI_DIN_UART_PORT=2"

# The test includes:
# - MIDI I/O: Receives from DIN IN1, sends to DIN OUT1
# - LiveFX Transform: Real-time transpose, velocity scaling, force-to-scale
# - MIDI Learn: Map CC messages to LiveFX parameters
```

**Expected Output:**
- Incoming MIDI DIN events logged over the debug UART
- MIDI messages transformed by LiveFX (when enabled)
- Real-time parameter control via CC messages (MIDI learn)

**MIDI Learn Commands (Channel 1):**
- CC 20: Enable/Disable LiveFX (value > 64 = enabled)
- CC 21: Transpose down (-1 semitone)
- CC 22: Transpose up (+1 semitone)
- CC 23: Transpose reset (0)
- CC 24: Velocity scale down (-10%)
- CC 25: Velocity scale up (+10%)
- CC 26: Velocity scale reset (100%)
- CC 27: Force-to-scale toggle (value > 64 = on)
- CC 28: Scale type (0-11)
- CC 29: Scale root (0=C, 1=C#, ..., 11=B)

**Test Workflow:**
1. Connect MIDI controller to DIN IN1
2. Connect DIN OUT1 to synth or DAW
3. Send CC 20 (value 127) to enable LiveFX
4. Send CC 22 multiple times to transpose notes up
5. Play notes on controller - they will be transposed
6. Send CC 27 (value 127) to enable force-to-scale
7. Play notes - they will snap to the selected scale
8. Monitor UART debug output for detailed status

---

## MODULE_TEST_ALL - Comprehensive Test Runner

### Overview

The `MODULE_TEST_ALL` test provides automated sequential execution of all finite tests in the MidiCore system. This enables comprehensive system validation in a single test run, ideal for CI/CD pipelines and production testing.

### Purpose

Run all tests that complete and return (as opposed to tests that loop forever) in a single automated session, providing aggregated pass/fail statistics and detailed per-test results.

### Tests Included

MODULE_TEST_ALL executes these finite tests:

#### 1. OLED_SSD1322 Driver Test
**Duration:** ~10-15 seconds  
**What it tests:**
- GPIO pin control and verification
- Software SPI bit-bang implementation
- Display initialization sequence
- Pattern rendering (white, black, checkerboard, stripes, gradient)
- Timing validation

**Returns:** 0 on success, negative on error

#### 2. PATCH_SD Test
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

### Tests Excluded

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

### How to Run

**STM32CubeIDE:**
1. Right-click project → Properties
2. C/C++ Build → Settings → MCU GCC Compiler → Preprocessor
3. Add define: `MODULE_TEST_ALL`
4. Build and flash
5. Connect UART2 (115200 baud, 8N1)
6. Observe test output

**Command Line:**
```bash
make CFLAGS+="-DMODULE_TEST_ALL"
```

### Example Output

#### Successful Run
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

### Return Values

- **0** - All tests passed (at least one test ran)
- **-1** - One or more tests failed, OR no tests ran (all skipped)

### Performance

**Timing:**
- Total runtime: 15-25 seconds (varies by enabled tests)
- OLED_SSD1322: ~10-15 seconds
- PATCH_SD: ~5-10 seconds
- Delays between tests: 500ms
- Startup overhead: 200ms

**Memory:**
- Stack usage: ~3 KB (combines both tests)
- Heap usage: None (all stack-based)
- Result tracking: ~100 bytes (test_result_t array)

### Use Cases

#### 1. Automated CI/CD Testing
```bash
#!/bin/bash
# Build and flash with MODULE_TEST_ALL
make clean
make CFLAGS+="-DMODULE_TEST_ALL"
make flash

# Capture UART output and check result
# Exit code 0 = all tests passed
```

#### 2. Production Testing
- Flash device with MODULE_TEST_ALL enabled
- Connect UART and SD card
- Power on and wait ~20 seconds
- Check UART for "ALL TESTS PASSED"
- If pass, device validated for shipment

#### 3. Regression Testing
- Run after firmware updates
- Verify no functionality broken
- Quick validation of critical systems

---

## Module-Specific Tests

### PATCH_SD Test

For detailed documentation on SD card testing, MIDI export, and scene chaining, see the comprehensive test guide covering:
- 10 automated tests
- Config file management
- MIDI export functionality
- Scene chaining persistence
- Quick-save system validation

### Router Test

For detailed router testing documentation, including:
- 8-phase comprehensive testing
- Node mapping and routing configuration
- Channel filtering validation
- Message type testing
- Multi-destination routing

### MIDI DIN Test

For comprehensive MIDI DIN testing with LiveFX, including:
- LiveFX transformation features
- MIDI learn functionality
- 6 new advanced features (channel filtering, presets, curves, etc.)
- Quick start examples

See the specialized testing guide for complete workflows and troubleshooting.

---

## Module Dependencies

Some tests require specific modules to be enabled in `Config/module_config.h`:

| Test | Required Modules |
|------|------------------|
| `MODULE_TEST_AINSER64` | `MODULE_ENABLE_AINSER64`, `MODULE_ENABLE_SPI_BUS` |
| `MODULE_TEST_SRIO` | `MODULE_ENABLE_SRIO`, `MODULE_ENABLE_SPI_BUS` |
| `MODULE_TEST_MIDI_DIN` | `MODULE_ENABLE_MIDI_DIN`, `MODULE_ENABLE_ROUTER`, `MODULE_ENABLE_LIVEFX` |
| `MODULE_TEST_ROUTER` | `MODULE_ENABLE_ROUTER` |
| `MODULE_TEST_LOOPER` | `MODULE_ENABLE_LOOPER` |
| `MODULE_TEST_UI` | `MODULE_ENABLE_UI`, `MODULE_ENABLE_OLED` |
| `MODULE_TEST_PATCH_SD` | `MODULE_ENABLE_PATCH` |
| `MODULE_TEST_PRESSURE` | `MODULE_ENABLE_PRESSURE` |
| `MODULE_TEST_FOOTSWITCH` | `MODULE_ENABLE_LOOPER` (GPIO-based, no SRIO required) |

If required modules are not enabled, the test will idle in an infinite loop.

---

## Adding New Tests

To add a new module test:

1. **Add enum value** in `module_tests.h`:
   ```c
   typedef enum {
     // ... existing tests
     MODULE_TEST_MY_NEW_MODULE_ID,
   } module_test_t;
   ```

2. **Declare test function** in `module_tests.h`:
   ```c
   void module_test_my_new_module_run(void);
   ```

3. **Implement test** in `module_tests.c`:
   ```c
   void module_test_my_new_module_run(void)
   {
     // Initialize module
     my_module_init();
     
     // Test loop
     for (;;) {
       // Test code here
       osDelay(10);
     }
   }
   ```

4. **Add to test runner** in `module_tests_run()`:
   ```c
   case MODULE_TEST_MY_NEW_MODULE_ID:
     module_test_my_new_module_run();
     break;
   ```

5. **Add to compile-time selection** in `module_tests_get_compile_time_selection()`:
   ```c
   #elif defined(MODULE_TEST_MY_NEW_MODULE)
     return MODULE_TEST_MY_NEW_MODULE_ID;
   ```

6. **Add to name table**:
   ```c
   static const char* test_names[] = {
     // ... existing names
     "MY_NEW_MODULE",
   };
   ```

---

## Troubleshooting

### Test doesn't run

- Check that the module is enabled in `module_config.h`
- Verify the define is set correctly
- Check UART output for error messages

### Test crashes immediately

- Check module initialization order
- Verify hardware connections
- Check for stack overflow (increase task stack size)

### Test runs but no output

- Verify UART baud rate and connections
- Check that debug output is enabled
- Ensure module is actually processing data

### "NO TESTS RUN"

**Cause:** Both `MODULE_ENABLE_OLED` and `MODULE_ENABLE_PATCH` are disabled

**Solution:**
- Enable at least one module in `Config/module_config.h`
- Or run individual tests directly

---

## Best Practices

1. **Test One Module at a Time**: Disable other modules to isolate issues
2. **Use UART Debug**: Add debug prints to trace execution
3. **Check Module Dependencies**: Ensure required modules are enabled
4. **Start Simple**: Begin with basic tests (SRIO, AINSER64) before complex ones (Looper, UI)
5. **Document Your Tests**: Add comments explaining what each test validates

---

## Integration with CI/CD

Tests can be automated in CI/CD pipelines:

```yaml
# Example GitHub Actions workflow
- name: Test AINSER64
  run: make CFLAGS+="-DMODULE_TEST_AINSER64"

- name: Test MIDI DIN
  run: make CFLAGS+="-DMODULE_TEST_MIDI_DIN"
```

---

## MIOS32 Hardware Compatibility

MidiCore is **100% compatible with MIOS32 hardware**. All UART ports match MIOS32 pinout:
- Port 0 = UART1 (PA9/PA10) - MIDI OUT1/IN1
- Port 1 = UART2 (PA2/PA3) - MIDI OUT2/IN2 (default debug)
- Port 2 = UART3 (PB10/PB11) - MIDI OUT3/IN3
- Port 3 = UART5 (PC12/PD2) - MIDI OUT4/IN4

---

## Summary

The MidiCore module testing framework provides:
- ✅ Comprehensive test coverage
- ✅ Easy integration with development workflow
- ✅ Production-ready validation
- ✅ CI/CD pipeline support
- ✅ MIOS32 hardware compatibility

For detailed examples and troubleshooting, refer to the specialized testing guides for each module.
