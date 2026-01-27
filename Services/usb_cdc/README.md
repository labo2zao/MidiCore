# USB CDC Service - Clean Implementation

## Overview

This is a **clean-room implementation** of USB CDC (Communications Device Class) for MidiCore. It provides Virtual COM Port functionality compatible with MIOS Studio and other terminal applications.

## Commercial License Compliance

This implementation is:
- ✅ **Original code** - No MIOS32 source code copied
- ✅ **Clean-room design** - Inspired by CDC concepts, not MIOS32 code
- ✅ **USB standard compliance** - Based on USB CDC spec v1.2
- ✅ **Commercially licensable** - No licensing conflicts

## Architecture

```
Application Layer
    ↓
Terminal API (MIOS Studio compatible)
    ↓
USB CDC Service (usb_cdc.h/.c)
    ↓
STM32 USB Device Middleware
    ↓
STM32 HAL USB Driver
```

## API Functions

### Core Functions
- `usb_cdc_init()` - Initialize CDC interface
- `usb_cdc_send()` - Send data to terminal
- `usb_cdc_is_connected()` - Check connection status
- `usb_cdc_register_receive_callback()` - Register RX callback

### Terminal API (MIOS Studio Compatible)
- `USB_CDC_TerminalInit()` - Initialize for terminal use
- `USB_CDC_TerminalAvailable()` - Check if terminal connected
- `USB_CDC_TerminalPutChar()` - Send single character
- `USB_CDC_TerminalPutString()` - Send string
- `USB_CDC_TerminalWrite()` - Send buffer
- `USB_CDC_TerminalRegisterRxCallback()` - Register RX handler

## Usage Example

```c
#include "Services/usb_cdc/usb_cdc.h"

// Initialize
USB_CDC_TerminalInit();

// Check connection
if (USB_CDC_TerminalAvailable()) {
    // Send string
    USB_CDC_TerminalPutString("Hello from MidiCore!\r\n");
    
    // Send buffer
    uint8_t data[] = {0x01, 0x02, 0x03};
    USB_CDC_TerminalWrite(data, sizeof(data));
}

// Register callback for received data
void on_data_received(const uint8_t *buf, uint32_t len) {
    // Handle received data
}
USB_CDC_TerminalRegisterRxCallback(on_data_received);
```

## MIOS Studio Compatibility

The terminal API is designed to work seamlessly with MIOS Studio:
- Standard CDC ACM enumeration
- 115200 baud (or any baud rate - CDC is baud-independent)
- Compatible with MIOS Studio terminal window
- Bidirectional communication

## Features

- ✅ Non-blocking transmit
- ✅ Interrupt-driven receive
- ✅ Connection state detection
- ✅ Composite device support (MIDI + CDC)
- ✅ Multiple terminal applications support
- ✅ Windows/macOS/Linux compatible

## File Structure

- `usb_cdc.h` - Public API and terminal functions
- `usb_cdc.c` - Implementation wrapping STM32 USB middleware
- `README.md` - This file

## Dependencies

- STM32 USB Device Middleware (STM32_USB_Device_Library)
- USB CDC Device Class (USB_DEVICE/Class/CDC/)
- STM32 HAL (for USB peripheral)

## Integration

1. Enable in `Config/module_config.h`:
   ```c
   #define MODULE_ENABLE_USB_CDC 1
   ```

2. Configure USB in CubeMX:
   - Enable USB_OTG_FS
   - Set as Device mode
   - Add CDC class

3. Initialize in application:
   ```c
   USB_CDC_TerminalInit();
   ```

4. Use terminal functions for debug output

## Technical Notes

### Composite Device
When both MIDI and CDC are enabled, the device enumerates as a composite device with:
- 2 MIDI interfaces (Audio Control + MIDIStreaming)
- 2 CDC interfaces (Communication + Data)

### Endpoints
CDC uses separate endpoints from MIDI to avoid conflicts:
- EP2 (IN/OUT): CDC Data
- EP3 (IN): CDC Control

### Baud Rate
USB CDC is baud-rate independent. The baud rate setting from the terminal is acknowledged but ignored - USB full-speed operates at 12 Mbps regardless.

## License

MidiCore Project - Original Implementation
Licensed for commercial use.

Based on:
- USB CDC Specification v1.2 (USB-IF)
- STM32 USB Device Library (STMicroelectronics)

No MIOS32 code used.
