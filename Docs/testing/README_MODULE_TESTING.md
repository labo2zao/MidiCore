# Module Testing Guide for MidiCore

## Overview

MidiCore now provides a unified module testing framework that allows testing individual modules in isolation. This facilitates development, debugging, and validation of each component independently.

## Architecture

### Production vs Test Mode

- **Production Mode**: The default behavior. `StartDefaultTask` runs the full application via `app_entry_start()`.
- **Test Mode**: When a `MODULE_TEST_xxx` define is set at compile time, `StartDefaultTask` runs the specific module test instead.

### Key Files

- `App/tests/module_tests.h` - Test framework API
- `App/tests/module_tests.c` - Test implementations
- `App/tests/app_test_task.h` - Dedicated test task (optional)
- `App/tests/app_test_task.c` - Test task implementation
- `Core/Src/main.c` - Modified `StartDefaultTask` with test support

## Available Module Tests

| Test Name | Define | Description |
|-----------|--------|-------------|
| AINSER64 | `MODULE_TEST_AINSER64` | Tests 64-channel analog input (MCP3208 + mux) |
| SRIO | `MODULE_TEST_SRIO` | Tests shift register DIN/DOUT (74HC165/595) |
| MIDI DIN | `MODULE_TEST_MIDI_DIN` | Tests MIDI DIN I/O with LiveFX transform and MIDI learn |
| Router | `MODULE_TEST_ROUTER` | Tests MIDI router and message forwarding |
| Looper | `MODULE_TEST_LOOPER` | Tests MIDI looper recording/playback |
| UI | `MODULE_TEST_UI` | Comprehensive automated UI page navigation & OLED test (all 29 modes) |
| Patch/SD | `MODULE_TEST_PATCH_SD` | Tests SD card mounting and patch loading |
| Pressure | `MODULE_TEST_PRESSURE` | Tests I2C pressure sensor (XGZP6847) |
| Breath | `MODULE_TEST_BREATH` | Tests breath controller (pressure sensor + MIDI CC) - [Guide](BREATH_CONTROLLER_TEST_GUIDE.md) |
| USB Host MIDI | `MODULE_TEST_USB_HOST_MIDI` | Tests USB Host MIDI device communication |

**Note:** The table above shows **preprocessor defines** you use in your build configuration. The internal enum values (in code) have an `_ID` suffix (e.g., `MODULE_TEST_AINSER64_ID`) to avoid naming conflicts.

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

# Optional: select DIN1 UART (0-3) for MIDI IN/OUT
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

### Example 5: Test Looper

```bash
# Compile with Looper test
make CFLAGS+="-DMODULE_TEST_LOOPER=1"

# The test will automatically:
# - Start recording after 1 second
# - Stop recording after 7 seconds
# - Play back for 8 seconds
# - Stop for 2 seconds
# - Repeat cycle
```

**Expected Output:**
- Looper cycles through REC → PLAY → STOP states
- MIDI events recorded and played back

### Example 6: Test UI (OLED) with Automated Navigation

```bash
# Compile with UI test
make CFLAGS+="-DMODULE_TEST_UI=1"

# Connect UART terminal at 115200 baud to view test progress
# Observe the OLED for automated page navigation and test mode rendering
```

**Expected Output:**
- Comprehensive UART log showing 6 test phases:
  - Phase 1: OLED and UI initialization
  - Phase 2: Direct page navigation through all UI pages (3s each)
  - Phase 3: Button-based navigation test (Button 5 cycles)
  - Phase 4: All 29 OLED test modes rendered and validated
  - Phase 5: Encoder stress test (rapid changes, large jumps)
  - Phase 6: Status line validation
- OLED display shows:
  - Each UI page rendered in sequence
  - All 29 OLED test modes including:
    - Pattern/Grayscale tests (0-6)
    - Animations (7-10, 15-19)
    - Advanced graphics (11-14, 17-19)
    - Hardware driver tests (20-27)
    - Vortex tunnel demo (28)
- Test completes in ~2-3 minutes, then enters manual mode
- Buttons/encoders can be used for manual testing after automated tests

**OLED Test Modes Validated:**
The test automatically validates all SSD1322 enhancements:
- Mode 0-6: Basic display tests (patterns, grayscale, text)
- Mode 7-10: Animation tests (scrolling, ball, performance, circles)
- Mode 11-14: Advanced features (bitmap, patterns, stress, auto-cycle)
- Mode 15-16: Utility tests (burn-in prevention, performance stats)
- Mode 17-19: 3D & UI elements (wireframe cube, advanced graphics, UI widgets)
- Mode 20-27: Hardware driver tests (8 hardware test patterns)
- Mode 28: Vortex tunnel demo

### Example 7: Test Patch/SD

```bash
# Compile with Patch/SD test
make CFLAGS+="-DMODULE_TEST_PATCH_SD=1"

# Insert SD card with patches and monitor UART output
```

**Expected Output:**
- SD card mount status reported
- Patch list/load feedback printed to UART

### Example 8: Test Pressure Sensor

```bash
# Compile with pressure sensor test
make CFLAGS+="-DMODULE_TEST_PRESSURE=1"

# Ensure I2C pressure sensor is connected
```

**Expected Output:**
- Pressure readings printed to UART
- Values change with applied pressure

### Example 9: Test USB Host MIDI

```bash
# Compile with USB Host MIDI test
make CFLAGS+="-DMODULE_TEST_USB_HOST_MIDI=1"

# Connect a class-compliant USB MIDI device
```

**Expected Output:**
- USB device enumeration messages
- Incoming MIDI events printed to UART

## Production Mode (Default)

If no test define is set, the system runs in production mode:

```bash
# Normal build - runs full application
make

# Or explicitly disable all tests
make CFLAGS+="-DMODULE_TEST_NONE=1"
```

This will:
1. Initialize USB Host MIDI
2. Call `app_entry_start()`
3. Start all configured modules
4. Run the full MidiCore application

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
| `MODULE_TEST_BREATH` | `MODULE_ENABLE_PRESSURE`, `MODULE_ENABLE_EXPRESSION` (optional) |

If required modules are not enabled, the test will idle in an infinite loop.

## Adding New Tests

To add a new module test:

1. **Add enum value** in `module_tests.h`:
   ```c
   typedef enum {
     // ... existing tests
     MODULE_TEST_MY_NEW_MODULE,
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
   case MODULE_TEST_MY_NEW_MODULE:
     module_test_my_new_module_run();
     break;
   ```

5. **Add to compile-time selection** in `module_tests_get_compile_time_selection()`:
   ```c
   #elif defined(MODULE_TEST_MY_NEW_MODULE)
     return MODULE_TEST_MY_NEW_MODULE;
   ```

6. **Add to name table**:
   ```c
   static const char* test_names[] = {
     // ... existing names
     "MY_NEW_MODULE",
   };
   ```

## Debugging Tips

### UART Output

Most tests output debug information via UART2 (USART2):
- Baud rate: 31250 (MIDI baud) or 115200 (check your configuration)
- Connect a USB-UART adapter to see output

### OLED Display

If `MODULE_ENABLE_OLED` is enabled, some tests can display information on the SSD1322 display.

### LED Indicators

Use DOUT LEDs to indicate test status:
- Blinking = test running
- Solid = test passed
- Off = test failed or module disabled

### Safe Mode

If a test crashes, the system may enter safe mode. Check:
- Boot reason register
- Safe mode status
- Panic codes

## Best Practices

1. **Test One Module at a Time**: Disable other modules to isolate issues
2. **Use UART Debug**: Add debug prints to trace execution
3. **Check Module Dependencies**: Ensure required modules are enabled
4. **Start Simple**: Begin with basic tests (SRIO, AINSER64) before complex ones (Looper, UI)
5. **Document Your Tests**: Add comments explaining what each test validates

## Integration with CI/CD

Tests can be automated in CI/CD pipelines:

```yaml
# Example GitHub Actions workflow
- name: Test AINSER64
  run: make CFLAGS+="-DMODULE_TEST_AINSER64"

- name: Test MIDI DIN
  run: make CFLAGS+="-DMODULE_TEST_MIDI_DIN"
```

## Migration from Legacy Tests

If you're using the old test system:

**Old:**
```c
#define APP_TEST_DIN_MIDI
```

**New:**
```c
#define MODULE_TEST_MIDI_DIN 1
```

Both will work due to backward compatibility, but the new defines are recommended for consistency.

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

## Support

For questions or issues with the testing framework:
1. Check this guide
2. Review `module_tests.c` implementation
3. Consult `Modules_MidiCore_Detail_par_Module.txt` for module details
4. Check `README_MODULE_CONFIG.md` for module configuration

## Future Enhancements

Planned improvements to the test framework:
- Runtime test selection via SD config file
- Test result reporting and logging
- Automated test sequences
- Performance benchmarking
- Memory usage profiling
