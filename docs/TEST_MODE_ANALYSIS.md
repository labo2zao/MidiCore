# Test Mode Analysis - MODULE_TEST_USB_DEVICE_MIDI

## Discovery

The system has been running in **TEST MODE** all along! The `.cproject` file (line 67) defines `MODULE_TEST_USB_DEVICE_MIDI`, which activates a special USB MIDI testing harness instead of normal application operation.

## What is MODULE_TEST_USB_DEVICE_MIDI?

This is a **development/testing configuration** that:
- Bypasses normal application initialization
- Runs a USB MIDI test harness
- Logs MIDI activity to UART for debugging
- Sends test MIDI messages every 2 seconds
- Is designed for testing USB MIDI functionality in isolation

## How Test Mode Works

### Normal Mode (without MODULE_TEST_USB_DEVICE_MIDI):
```
StartDefaultTask()
└─ app_entry_start()
   └─ app_init_and_start()
      ├─ Initialize all modules (looper, router, SRIO, etc.)
      ├─ Create CLI task
      ├─ Create AINSER task
      ├─ Create MIDI I/O task
      └─ Normal application runs
```

### Test Mode (with MODULE_TEST_USB_DEVICE_MIDI):
```
StartDefaultTask()
└─ app_entry_start()
   └─ app_init_and_start()
      └─ module_test_run()
         └─ module_test_usb_device_midi_run()
            ├─ Print test header
            ├─ Setup USB MIDI RX hooks
            ├─ Run forever loop:
            │  ├─ Send test MIDI messages
            │  ├─ Log received MIDI
            │  └─ Delay 2 seconds
            └─ NEVER RETURNS (infinite loop)
```

## Code Changes When Test Mode Active

### 1. Services/mios32_query/mios32_query.c
Extra debug output at lines: 74, 88, 103, 121, 166, 206

### 2. Services/usb_midi/usb_midi.c  
Debug hooks enabled at lines: 163, 173, 196, 205, 251, 258

### 3. USB_DEVICE/App/usbd_composite.c
Extra debugging at lines: 124, 134, 139, 145, 152

### 4. Services/looper/looper.c & looper.h
Reduced memory allocation (lines 23, 1809, 47, 72, 98)
- Smaller buffers for testing
- Reduced feature set

## Why This Explains Everything

### "code never reach app_init_and_start"
**FALSE ALARM!** The code DOES reach `app_init_and_start()`, but:
- Test mode is detected
- `module_test_run()` is called
- Test code runs instead of normal app init
- Normal initialization functions are never called
- System appears "stuck" in test harness

### Previous Issues
All previous problems were likely test mode specific:
- Test mode has different init sequence
- Test mode expects specific behavior
- My fixes assumed normal mode
- Fixes may have broken test mode assumptions

## How to Disable Test Mode

### Option 1: Edit .cproject (Recommended)
Comment out or remove line 67:
```xml
<!-- <listOptionValue builtIn="false" value="MODULE_TEST_USB_DEVICE_MIDI"/> -->
```

### Option 2: Build Configuration
In STM32CubeIDE:
1. Right-click project → Properties
2. C/C++ Build → Settings
3. MCU GCC Compiler → Preprocessor
4. Find and remove `MODULE_TEST_USB_DEVICE_MIDI` from defined symbols

### Option 3: Code Override
In `Config/module_config.h`, force undefine:
```c
#ifdef MODULE_TEST_USB_DEVICE_MIDI
#undef MODULE_TEST_USB_DEVICE_MIDI
#endif
```

## Test Mode Features

When active, the test provides:

### 1. USB MIDI RX Monitoring
- Logs all received MIDI messages to UART
- Decodes message types (Note On/Off, CC, etc.)
- Shows channel and data values
- Filters out SysEx to reduce clutter

### 2. USB MIDI TX Testing
- Sends test Note On/Off messages every 2 seconds
- Tests MIOS32 terminal functionality
- Reports TX queue status
- Detects USB enumeration issues

### 3. Diagnostic Output
- Reports USB MIDI ready status
- Shows TX queue size/usage/drops
- Verifies UART debug output
- Tests MIOS Studio integration

## When to Use Test Mode

**Use MODULE_TEST_USB_DEVICE_MIDI when:**
- Debugging USB MIDI enumeration issues
- Testing USB MIDI TX/RX functionality
- Verifying MIOS Studio integration
- Isolating USB problems from app complexity

**DO NOT use for:**
- Normal application operation
- Production builds
- Full system integration testing
- Performance testing

## Impact on System Resources

### Memory Impact (Test vs Normal)

**Test Mode:**
- Looper buffers: REDUCED (smaller for testing)
- Extra debug hooks: ~2KB code
- Test harness: ~3KB code
- Total: ~5KB extra, some savings in looper

**Normal Mode:**
- Full looper buffers
- No debug hooks
- No test harness
- Optimized for production

### Stack Impact

Test mode runs in DefaultTask:
- No additional tasks created
- CLI task NOT created
- AINSER task NOT created
- Simpler stack requirements

## File Locations

### Test Definition:
- `.cproject` line 67

### Test Entry Point:
- `App/tests/module_tests.c:7129` - `module_test_usb_device_midi_run()`

### Test Detection:
- `App/tests/module_tests.c:289-291` - `module_test_get_active_id()`

### Conditional Compilation:
- Search codebase for `#ifdef MODULE_TEST_USB_DEVICE_MIDI`

## Related Test Modes

The framework supports multiple test modes:

- `MODULE_TEST_USB_DEVICE_MIDI` - USB Device MIDI test (current)
- `MODULE_TEST_USB_HOST_MIDI` - USB Host MIDI test
- `MODULE_TEST_OLED_SSD1322` - OLED display test
- `MODULE_TEST_AINSER` - Analog input test
- `MODULE_TEST_ROUTER` - MIDI router test
- `MODULE_TEST_BREATH` - Breath controller test
- `MODULE_TEST_ALL` - Run all tests sequentially

Only ONE test mode should be active at a time.

## Recommendation

**For normal operation:** DISABLE `MODULE_TEST_USB_DEVICE_MIDI`

The test mode should only be used for isolated USB MIDI debugging. For full application functionality including CLI, looper, SRIO, etc., the system must run in normal mode without any MODULE_TEST_* defines.

## Summary

The "problem" is not a bug - it's that the system is intentionally running in test mode! The test mode works as designed by bypassing normal app initialization and running a focused USB MIDI test harness.

To fix the "code never reach app_init_and_start" issue: **Disable test mode** by removing `MODULE_TEST_USB_DEVICE_MIDI` from the .cproject file.
