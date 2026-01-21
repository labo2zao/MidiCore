# Quick Start: Testing Modules in MidiCore

This guide provides quick examples for testing each module.

## MIOS32 Hardware Compatibility

MidiCore is **100% compatible with MIOS32 hardware**. All UART ports match MIOS32 pinout:
- Port 0 = UART1 (PA9/PA10) - MIDI OUT1/IN1
- Port 1 = UART2 (PA2/PA3) - MIDI OUT2/IN2 (default debug)
- Port 2 = UART3 (PB10/PB11) - MIDI OUT3/IN3
- Port 3 = UART5 (PC12/PD2) - MIDI OUT4/IN4

See [README_MIOS32_UART_CONFIG.md](README_MIOS32_UART_CONFIG.md) for full UART configuration guide.

## Prerequisites

- STM32CubeIDE installed
- MidiCore project opened in CubeIDE
- Target hardware connected (STM32F407)
- USB-UART adapter for debug output (optional but recommended)

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
  // Uncomment the test you want to run (use _ID enum values):
  // return MODULE_TEST_AINSER64_ID;
  // return MODULE_TEST_SRIO_ID;
  // return MODULE_TEST_MIDI_DIN_ID;
  // return MODULE_TEST_ROUTER_ID;
  // return MODULE_TEST_LOOPER_ID;
  // return MODULE_TEST_UI_ID;
  // return MODULE_TEST_PATCH_SD_ID;
  // return MODULE_TEST_PRESSURE_ID;
  // return MODULE_TEST_USB_HOST_MIDI_ID;
  
  // ... rest of function
}
```

**Note:** In code, enum values use `_ID` suffix (e.g., `MODULE_TEST_AINSER64_ID`) to avoid conflicts with preprocessor defines.

---

## Viewing Debug Output

Most tests output debug information via UART using the MIOS32-compatible `dbg_print()` API.

**Default UART Configuration:**
- Debug Output: UART2 (PA2/PA3) at 115200 baud
- MIDI DIN: UART1 (PA9/PA10) at 31250 baud

**Hardware Setup:**
- Connect USB-UART adapter to debug UART:
  - Debug TX: PA2 (UART2) - Default
  - Debug RX: PA3 (UART2) - Optional
  - GND: Common ground

**Serial Settings:**
- Baud rate: 115200 (debug) or 31250 (MIDI)
- Data bits: 8
- Stop bits: 1
- Parity: None

**Tools:**
- PuTTY, Tera Term, Arduino Serial Monitor, etc.

**Configure Different UART:**
Add to build defines to use a different UART for debug:
```
TEST_DEBUG_UART_PORT=2  // Use UART3 (PB10/PB11) for debug
```

See [README_MIOS32_UART_CONFIG.md](README_MIOS32_UART_CONFIG.md) for detailed UART configuration.

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
