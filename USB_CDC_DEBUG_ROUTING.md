# USB CDC Debug Routing

## Overview

All debug functions (`dbg_*`) in MidiCore now route their output to **USB CDC (Virtual COM port)** by default when `MODULE_ENABLE_USB_CDC=1`.

This provides a cleaner, more convenient debugging experience without requiring separate UART cables or baud rate reconfiguration.

## Benefits

### 1. No UART Reconfiguration Needed
- All MIDI DIN ports stay at 31250 baud (MIDI standard)
- No need to dedicate a UART for debug at 115200 baud
- Cleaner hardware setup

### 2. Standard Virtual COM Port
- Works with any serial terminal software
- Compatible with MIOS Studio terminal
- No special drivers needed (standard CDC ACM)

### 3. All MIDI Ports Available
- Port 0 (DIN1): USART2 @ 31250 baud - Available for MIDI
- Port 1 (DIN2): USART3 @ 31250 baud - Available for MIDI  
- Port 2 (DIN3): USB OTG - Reserved
- Port 3 (DIN4): UART5 @ 31250 baud - Available for MIDI

### 4. Multi-Output Support
- Primary: USB CDC (Virtual COM port)
- Secondary: OLED display (if `MODULE_ENABLE_OLED=1`)
- All output automatically mirrored to both

## Implementation

### Files Modified

1. **`App/tests/test_debug.c`**
   - `dbg_putc()` - Routes to `usb_cdc_send()`
   - `dbg_print()` - Routes to `usb_cdc_send()`
   - `test_debug_init()` - Skips UART reconfig when CDC enabled

2. **`App/tests/test_debug.h`**
   - Updated documentation to reflect USB CDC routing
   - Clarified `test_debug_init()` is optional with USB CDC

### Output Routing Logic

```c
#if MODULE_ENABLE_USB_CDC
  // Primary: USB CDC (Virtual COM port)
  usb_cdc_send((const uint8_t*)str, len);
#else
  // Fallback: UART (original behavior)
  UART_HandleTypeDef* huart = get_debug_uart_handle();
  HAL_UART_Transmit(huart, (const uint8_t*)str, len, 1000);
#endif

// Optional: OLED mirroring (works in both modes)
if (oled_mirror_is_enabled()) {
  oled_mirror_print(str);
}
```

## Usage

### Basic Usage

```c
#include "App/tests/test_debug.h"

void my_test(void) {
  // Optional - just prints startup message
  test_debug_init();
  
  // All output goes to USB CDC
  dbg_print("Test started\n");
  dbg_printf("Value: %d\n", 42);
  dbg_print_hex8(0xAB);
  dbg_println();
}
```

### Viewing Output

**Option 1: MIOS Studio**
```
1. Open MIOS Studio
2. Device should auto-detect
3. Terminal tab shows debug output
```

**Option 2: Any Serial Terminal**
```bash
# Linux/macOS
screen /dev/ttyACM0

# Windows
PuTTY → Serial → COM11 (check Device Manager)

# Python
python -m serial.tools.miniterm COM11
```

**Baud Rate:** Doesn't matter! CDC handles it automatically.

### Test Modules

All test modules automatically use USB CDC:

```c
// MODULE_TEST_USB_DEVICE_MIDI
[TX] Sending test Note On: Cable:0 90 3C 64
[RX] Cable:0 90 3C 64 (Note On Ch:1 Note:60 Vel:100)

// MODULE_TEST_AINSER64  
Channel 0: raw=2048 filt=2048 pos14=8192

// MODULE_TEST_DIN_MIDI
DIN MIDI Test - Port 0 (USART2)
Sending Note On...
```

## Configuration

### Enable USB CDC Debug (Default)

```c
// Config/module_config.h
#define MODULE_ENABLE_USB_CDC 1
```

All debug functions route to USB CDC automatically.

### Disable USB CDC Debug (Fallback to UART)

```c
// Config/module_config.h
#define MODULE_ENABLE_USB_CDC 0
```

Debug functions use UART at 115200 baud (requires `test_debug_init()`).

### Enable OLED Mirroring (Optional)

```c
// Config/module_config.h
#define MODULE_ENABLE_OLED 1
```

Debug output mirrored to OLED display in addition to USB CDC/UART.

## Compatibility

### Backward Compatibility

✅ **Fully backward compatible**

- Old code using `dbg_*` functions works unchanged
- Fallback to UART when `MODULE_ENABLE_USB_CDC=0`
- No changes needed to existing test modules

### MIOS32 Compatibility

✅ **MIOS32-compatible API**

```c
// All MIOS32-style functions work identically
DEBUG_MSG("Hello %d\n", 42);  // Macro → dbg_printf()
dbg_print("Hello\n");          // Direct function call
```

## Technical Details

### Initialization Order

USB CDC must be initialized before debug functions are used:

```c
// In main.c (already correct):
usb_cdc_init();           // Register CDC callbacks
MX_USB_DEVICE_Init();     // Start USB device
// ... later in test code:
test_debug_init();        // Optional with USB CDC
dbg_print("Ready\n");     // Works via USB CDC
```

### Flow Control

USB CDC has built-in flow control:
- TX queue: 32 packets deep
- Automatic retry if busy
- No data loss under normal conditions

### Performance

- **USB CDC**: ~1 packet (64 bytes) per millisecond
- **Buffering**: Handled by `usb_cdc` service layer
- **Latency**: Typically <10ms from `dbg_print()` to terminal

## Troubleshooting

### Debug Output Not Appearing

**Check USB enumeration:**
```bash
# Linux
lsusb | grep 16C0:0489  # Should show MidiCore

# Windows
Device Manager → Ports → Should see "USB Serial Device (COMx)"
```

**Check terminal connection:**
```bash
# Try different terminal software
# Verify correct COM port selected
# USB cable must support data (not charge-only)
```

### CDC Echo Test Failing

See `Tools/test_cdc_terminal.py` for diagnostics.

### Module Not Compiled

Check build output for:
```
Building file: ../Services/usb_cdc/usb_cdc.c
Building file: ../App/tests/test_debug.c
```

If missing, verify `MODULE_ENABLE_USB_CDC=1` in config.

## Related Files

- `Services/usb_cdc/usb_cdc.c` - USB CDC service implementation
- `USB_DEVICE/Class/CDC/Src/usbd_cdc.c` - USB CDC class driver
- `App/tests/test_debug.c` - Debug output implementation
- `App/tests/test_debug.h` - Debug API declarations
- `Tools/test_cdc_terminal.py` - CDC test script

## Examples

### Simple Test

```c
#include "App/tests/test_debug.h"

void simple_test(void) {
  test_debug_init();
  dbg_print("=== Simple Test ===\n");
  dbg_printf("Counter: %d\n", 0);
  dbg_print_hex8(0xFF);
  dbg_println();
  dbg_print("Done!\n");
}
```

**Output (via USB CDC):**
```
=== Simple Test ===
Counter: 0
FF
Done!
```

### Advanced Test with Hex Dump

```c
void hex_dump_test(void) {
  uint8_t data[] = {0x90, 0x3C, 0x7F, 0x00};
  
  dbg_print("MIDI Packet: ");
  dbg_print_bytes(data, 4, ' ');
  dbg_println();
  
  dbg_printf("Status: 0x%02X\n", data[0]);
  dbg_printf("Note: %d\n", data[1]);
  dbg_printf("Velocity: %d\n", data[2]);
}
```

**Output (via USB CDC):**
```
MIDI Packet: 90 3C 7F 00
Status: 0x90
Note: 60
Velocity: 127
```

## Summary

✅ All `dbg_*` functions now route to USB CDC by default  
✅ No UART reconfiguration needed  
✅ All MIDI ports available at 31250 baud  
✅ Works with standard serial terminals  
✅ OLED mirroring still supported  
✅ Fully backward compatible  
✅ MIOS32 API compatible  

**Result:** Cleaner, more convenient debugging experience with USB CDC as the primary debug output channel.
