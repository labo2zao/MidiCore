# Quick Start: Testing Modules in MidiCore

This guide provides quick examples for testing each module.

## Prerequisites

- STM32CubeIDE installed
- MidiCore project opened in CubeIDE
- Target hardware connected (STM32F407)

## Quick Test Examples

### 1. Test AINSER64 (Analog Inputs)

**Steps:**
1. Right-click project → Properties
2. C/C++ Build → Settings → MCU GCC Compiler → Preprocessor
3. Add: `MODULE_TEST_AINSER64`
4. Click Apply and Close
5. Build project
6. Flash to device
7. Connect UART2 to see output

**Expected Result:** Console shows values from all 64 analog channels.

---

### 2. Test SRIO (Digital I/O)

**Steps:**
1. Add preprocessor define: `MODULE_TEST_SRIO`
2. Build and flash
3. Connect UART2 to see DIN values

**Expected Result:** Hexadecimal dump of button states from shift registers.

---

### 3. Test MIDI DIN

**Steps:**
1. Add preprocessor define: `MODULE_TEST_MIDI_DIN`
2. Build and flash
3. Send MIDI to DIN input
4. Monitor MIDI output

**Expected Result:** MIDI messages are echoed/processed.

---

### 4. Test Looper

**Steps:**
1. Add preprocessor define: `MODULE_TEST_LOOPER`
2. Build and flash
3. Send MIDI notes during recording phase

**Expected Result:** 
- Records for 7 seconds
- Plays back for 8 seconds
- Stops for 2 seconds
- Repeats cycle

---

### 5. Return to Production Mode

**Steps:**
1. Remove all `MODULE_TEST_xxx` defines
2. Build and flash

**Expected Result:** Full MidiCore application runs normally.

---

## Adding Defines in STM32CubeIDE

### Method 1: Project Properties (Recommended)

1. Right-click project in Project Explorer
2. Select **Properties**
3. Navigate to: **C/C++ Build → Settings**
4. Select: **MCU GCC Compiler → Preprocessor**
5. In "Defined symbols (-D)" section, click **Add (+ icon)**
6. Enter define name (e.g., `MODULE_TEST_AINSER64`)
7. Click **OK**
8. Click **Apply and Close**
9. Build project (Ctrl+B)

### Method 2: Direct Code Modification (Quick Testing)

Edit `App/tests/module_tests.c`, function `module_tests_get_compile_time_selection()`:

```c
module_test_t module_tests_get_compile_time_selection(void)
{
  // Uncomment the test you want to run:
  // return MODULE_TEST_AINSER64;
  // return MODULE_TEST_SRIO;
  // return MODULE_TEST_MIDI_DIN;
  // return MODULE_TEST_ROUTER;
  // return MODULE_TEST_LOOPER;
  // return MODULE_TEST_UI;
  // return MODULE_TEST_PATCH_SD;
  // return MODULE_TEST_PRESSURE;
  // return MODULE_TEST_USB_HOST_MIDI;
  
  // ... rest of function
}
```

---

## Viewing Debug Output

Most tests output debug information via UART2:

**Hardware Setup:**
- Connect USB-UART adapter to USART2
- TX: PA2
- RX: PA3 (if needed)
- GND: Common ground

**Serial Settings:**
- Baud rate: 31250 (MIDI baud) or check your configuration
- Data bits: 8
- Stop bits: 1
- Parity: None

**Tools:**
- PuTTY, Tera Term, Arduino Serial Monitor, etc.

---

## Example Build Configuration

### Configuration for AINSER64 Test Only

**Module Config (`Config/module_config.h`):**
```c
#define MODULE_ENABLE_AINSER64  1
#define MODULE_ENABLE_SPI_BUS   1
#define MODULE_ENABLE_AIN       1

// Disable everything else for minimal build
#define MODULE_ENABLE_SRIO      0
#define MODULE_ENABLE_OLED      0
#define MODULE_ENABLE_LOOPER    0
#define MODULE_ENABLE_UI        0
// ... etc
```

**Preprocessor Defines:**
```
MODULE_TEST_AINSER64
```

This creates a minimal build that only tests AINSER64.

---

## Troubleshooting

### "Undefined reference" errors

**Problem:** Build fails with undefined references.

**Solution:** Enable required modules in `Config/module_config.h`:
- For AINSER64: Enable `MODULE_ENABLE_AINSER64` and `MODULE_ENABLE_SPI_BUS`
- For SRIO: Enable `MODULE_ENABLE_SRIO`
- For MIDI: Enable `MODULE_ENABLE_MIDI_DIN`
- etc.

### No output on UART

**Problem:** Test runs but no UART output.

**Solutions:**
1. Check UART connections (PA2 = TX)
2. Verify baud rate matches (31250 or 115200)
3. Check that UART is initialized in test
4. Verify USB-UART adapter is working

### Test doesn't start

**Problem:** Device runs but test doesn't execute.

**Solutions:**
1. Verify preprocessor define is set correctly
2. Check that required modules are enabled
3. Add debug LED toggle in test to verify execution
4. Check for crashes (connect debugger)

### Build takes too long

**Problem:** Build is slow with all modules.

**Solution:** Disable unused modules in `Config/module_config.h` for faster iteration during testing.

---

## Quick Reference: Test Defines

| Define | Tests |
|--------|-------|
| `MODULE_TEST_AINSER64` | 64-ch analog input |
| `MODULE_TEST_SRIO` | Shift register DIN/DOUT |
| `MODULE_TEST_MIDI_DIN` | MIDI IN/OUT via UART |
| `MODULE_TEST_ROUTER` | MIDI routing |
| `MODULE_TEST_LOOPER` | MIDI recording/playback |
| `MODULE_TEST_UI` | OLED display & UI |
| `MODULE_TEST_PATCH_SD` | SD card & patches |
| `MODULE_TEST_PRESSURE` | I2C pressure sensor |
| `MODULE_TEST_USB_HOST_MIDI` | USB Host MIDI |

---

## Need More Help?

See `README_MODULE_TESTING.md` for comprehensive documentation.
