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
| MIDI DIN | `MODULE_TEST_MIDI_DIN` | Tests MIDI DIN input/output via UART |
| Router | `MODULE_TEST_ROUTER` | Tests MIDI router and message forwarding |
| Looper | `MODULE_TEST_LOOPER` | Tests MIDI looper recording/playback |
| UI | `MODULE_TEST_UI` | Tests OLED display and user interface |
| Patch/SD | `MODULE_TEST_PATCH_SD` | Tests SD card mounting and patch loading |
| Pressure | `MODULE_TEST_PRESSURE` | Tests I2C pressure sensor (XGZP6847) |
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

### Example 3: Test MIDI Router

```bash
# Compile with Router test
make CFLAGS+="-DMODULE_TEST_ROUTER=1"

# Send MIDI to device and monitor output
```

**Expected Output:**
- MIDI messages routed according to configuration
- Can test different routing rules

### Example 4: Test Looper

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
| `MODULE_TEST_MIDI_DIN` | `MODULE_ENABLE_MIDI_DIN` |
| `MODULE_TEST_ROUTER` | `MODULE_ENABLE_ROUTER` |
| `MODULE_TEST_LOOPER` | `MODULE_ENABLE_LOOPER` |
| `MODULE_TEST_UI` | `MODULE_ENABLE_UI`, `MODULE_ENABLE_OLED` |
| `MODULE_TEST_PATCH_SD` | `MODULE_ENABLE_PATCH` |
| `MODULE_TEST_PRESSURE` | `MODULE_ENABLE_PRESSURE` |

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
