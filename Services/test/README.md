# Test Module Service

Runtime module testing service for MidiCore firmware via UART terminal.

## Overview

The Test Module Service provides a CLI-based interface for running module tests at runtime through the UART terminal. It integrates with the existing test infrastructure in `App/tests/` and allows selective execution of hardware and software module tests.

**MIOS Studio Compatible**: Uses standard MIOS32-style UART debug output via `dbg_print()` at 115200 baud, fully compatible with MIOS Studio terminal.

## MIOS Studio Integration

This module is designed to work seamlessly with [MIOS Studio](http://www.ucapps.de/mios_studio.html), the standard terminal tool for MIOS32/MIDIbox projects.

### Why MIOS Studio?

- **Professional MIDI/Debug Tool**: Industry standard for MIOS32 projects
- **UART Terminal**: Built-in serial terminal for debug output
- **MIDI Monitor**: Simultaneous MIDI and debug monitoring
- **Easy Setup**: Simple connection to STM32 UART

### Connection Setup

1. **Hardware**: Connect USB-to-UART adapter to debug UART
   - **Default (MIOS32 compatible)**: USART3 (PD8=TX, PD9=RX)
   - Alternative: USART2 (PA2/PA3)
   - Alternative: UART5 (PC12/PD2)
   
2. **MIOS Studio Settings**:
   - Baud Rate: **115200**
   - Data Bits: 8
   - Parity: None  
   - Stop Bits: 1
   - Port: Select your USB-to-UART COM port

3. **Open Terminal** tab in MIOS Studio

4. **Test connection**: Power on MidiCore, you should see initialization messages

### UART Configuration

The debug output uses MIOS32-compatible UART mapping (see `Docs/configuration/README_MIOS32_UART_CONFIG.md`):

```c
// Default debug UART configuration (MIOS32 compatible)
TEST_DEBUG_UART_PORT = 1        // USART3 (PD8/PD9) - Recommended for terminal
TEST_DEBUG_UART_BAUD = 115200   // Standard terminal baud rate

// MIDI DIN ports run at 31250 baud (separate from debug)
```

Can be changed via compile-time defines or in `App/tests/test_debug.h`.

## Architecture

```
┌─────────────────────────────────┐
│   UART Terminal (MIOS Studio)   │
│   115200 baud, 8N1              │
└─────────────────────────────────┘
              │
              ▼
┌─────────────────────────────────┐
│        CLI System                │
│   (Services/cli/)               │
└─────────────────────────────────┘
              │
              ▼
┌─────────────────────────────────┐
│    Test Module Service           │
│   (Services/test/)              │
│   - test.c / test.h             │
│   - test_cli.c / test_cli.h     │
└─────────────────────────────────┘
              │
              ▼
┌─────────────────────────────────┐
│    Test Framework                │
│   (App/tests/)                  │
│   - module_tests.c/.h           │
│   - test_debug.c/.h             │
│   - Individual test files       │
└─────────────────────────────────┘
```

## Features

- **Runtime Test Selection** - Choose which module to test via CLI
- **UART Output** - All test results via UART (MIOS Studio compatible)
- **Test Discovery** - List all available tests
- **Status Reporting** - Query test execution status
- **Module Registry Integration** - Configurable via module parameters

## Available Tests

The following tests are available from `App/tests/module_tests.c`:

### Hardware Tests
- `ainser64` - Test AINSER64 analog inputs (64 channels)
- `srio` - Test SRIO digital inputs (buttons)
- `srio_dout` - Test SRIO digital outputs (LEDs)
- `midi_din` - Test MIDI DIN I/O
- `pressure` - Test I2C pressure sensor
- `breath` - Test breath controller
- `oled_ssd1322` - Test OLED display driver
- `footswitch` - Test footswitch inputs

### Service Module Tests
- `router` - Test MIDI router
- `looper` - Test looper recording/playback
- `lfo` - Test LFO module
- `humanizer` - Test Humanizer module
- `patch_sd` - Test SD card patch loading
- `usb_host_midi` - Test USB Host MIDI
- `usb_device_midi` - Test USB Device MIDI

### UI Tests
- `ui` - Test UI/OLED general
- `ui_song` - Test Song Mode UI page
- `ui_midi_monitor` - Test MIDI Monitor UI page
- `ui_sysex` - Test SysEx UI page
- `ui_config` - Test Config Editor UI page
- `ui_livefx` - Test LiveFX UI page
- `ui_rhythm` - Test Rhythm Trainer UI page
- `ui_humanizer` - Test Humanizer/LFO UI page

### Debug Tests
- `gdb_debug` - Test GDB debug / UART verification

## CLI Commands

### List Available Tests

```
test list
```

Shows all available tests with descriptions.

### Run a Test

```
test run <test_name>
```

Examples:
```
test run ainser64
test run srio
test run midi_din
test run router
```

**Note:** Most tests run in infinite loops. Reset the device to stop.

### Show Test Status

```
test status
```

Shows current or last test execution status.

### Get Test Information

```
test info <test_name>
```

Example:
```
test info ainser64
```

### Clear Test Results

```
test clear
```

Clears test result history.

## Usage with MIOS Studio

1. **Connect UART** - Connect to UART2 (115200 baud, 8N1)
2. **Open MIOS Studio** - Terminal tab
3. **List tests:**
   ```
   test list
   ```
4. **Run a test:**
   ```
   test run ainser64
   ```
5. **Watch output** - Test results appear in terminal
6. **Reset device** - To stop the test (Ctrl+R in MIOS Studio or hardware reset)

## Module Registry Integration

The test module registers itself with the module registry and supports these parameters:

- `enabled` (bool) - Enable/disable test module
- `verbose` (bool) - Verbose output mode
- `timeout_ms` (int) - Test timeout in milliseconds

### Via CLI

```
module get test enabled
module set test enabled true
module set test verbose true
module set test timeout_ms 30000
```

## Configuration

Test module configuration in `module_config.h`:

```c
#define MODULE_ENABLE_TEST 1  // Enable test module
```

## Integration

### 1. Module Initialization

Add to your initialization code (e.g., `App/app_init.c`):

```c
#if MODULE_ENABLE_TEST
#include "Services/test/test.h"
#include "Services/test/test_cli.h"
#endif

// In init function:
#if MODULE_ENABLE_TEST
test_init();
test_cli_init();
#endif
```

### 2. Build Configuration

Ensure these are defined in `module_config.h`:
```c
#define MODULE_ENABLE_CLI 1
#define MODULE_ENABLE_TEST 1
```

## Test Development

To add a new test to the framework:

1. Add test ID to `module_test_t` enum in `App/tests/module_tests.h`
2. Implement test function in `App/tests/module_tests.c`
3. Add test descriptor to `g_test_descriptors` in `Services/test/test.c`
4. Test via CLI: `test run <new_test_name>`

## Limitations

- **Infinite Loops** - Most tests run forever, requiring device reset to stop
- **Single Test** - Only one test can run at a time
- **No Multitasking** - Test execution blocks CLI until complete (or device reset)
- **UART Only** - No network or USB CDC terminal support (UART terminal required)

## Examples

### Example Session 1: Test AINSER64

```
> test list
=== Available Tests ===
...
  1. ainser64
     Test AINSER64 analog inputs
...

> test run ainser64

=== Starting Test ===
Test: ainser64
Duration: Infinite (until reset)
======================

[Test output appears...]
Ch 0: 512
Ch 1: 1024
Ch 2: 0
...
[Continues until reset]
```

### Example Session 2: Test MIDI Router

```
> test info router

=== Test Information ===
Name: router
Description: Test MIDI router

Usage: test run router

> test run router

[Router test executes, showing MIDI routing activity...]
```

### Example Session 3: Module Control

```
> module list
...
test - Module testing service
...

> module params test

Parameters for module 'test':
  enabled (bool): Enable test module
  verbose (bool): Verbose output
  timeout_ms (int): Test timeout (ms)

> module set test verbose false
OK

> test run srio
[Less verbose output...]
```

## Troubleshooting

### "Test not found" Error
- Use `test list` to see available tests
- Check spelling of test name

### "Test module is disabled" Error
- Enable via: `module set test enabled true`

### No Output During Test
- Check UART connection (115200 baud)
- Verify test_debug.h UART is configured
- Try `test set verbose true`

### Test Won't Stop
- Most tests run infinitely by design
- Reset device (hardware reset or Ctrl+R in MIOS Studio)
- This is normal behavior for hardware tests

## Files

- `Services/test/test.h` - Test module API
- `Services/test/test.c` - Test module implementation
- `Services/test/test_cli.h` - CLI commands API
- `Services/test/test_cli.c` - CLI command implementation
- `Services/test/README.md` - This file
- `App/tests/module_tests.h` - Test framework definitions
- `App/tests/module_tests.c` - Test framework implementation

## See Also

- [README_CLI_MODULE_SYSTEM.md](../../README_CLI_MODULE_SYSTEM.md) - CLI system overview
- [Services/cli/README.md](../cli/README.md) - CLI documentation
- [App/tests/test_debug.h](../../App/tests/test_debug.h) - UART debug functions
- [MIOS Studio](http://www.ucapps.de/mios_studio.html) - Terminal software

## License

Part of MidiCore firmware - same license as main project.
