# USB CDC (Virtual COM Port) Integration Guide

## Overview

This document describes how to enable and use USB CDC ACM (Abstract Control Model) Virtual COM Port functionality in MidiCore firmware. When enabled, the device exposes both USB MIDI (4 ports) and a Virtual COM Port, creating a composite USB device.

## Features

- **Virtual COM Port (VCP)**: Serial communication over USB for debugging, terminal commands, or data transfer
- **Composite Device**: Concurrent MIDI + CDC operation (both interfaces work simultaneously)
- **MIOS32 Compatible**: API shims for MIOS32_USB_CDC_* functions for MIOS Studio compatibility
- **Configurable**: Disabled by default, opt-in via compile-time flag
- **Cross-platform**: Works on Windows, macOS, and Linux

## Quick Start

### 1. Enable USB CDC

Edit `Config/module_config.h`:

```c
#define MODULE_ENABLE_USB_CDC 1  // Enable CDC (default: 0)
```

### 2. CubeMX Configuration

Ensure USB_OTG_FS is configured in CubeMX:
- **Mode**: Device_Only (or OTG with Device role)
- **PHY**: Integrated FS PHY selected
- **Clock**: 48 MHz USB clock from PLL

**Important**: The descriptors and endpoints are already configured in the code. You only need to ensure the USB peripheral is enabled in CubeMX.

### 3. Initialize CDC Service

In your application initialization (after `MX_USB_DEVICE_Init()`):

```c
#if MODULE_ENABLE_USB_CDC
#include "Services/usb_cdc/usb_cdc.h"

usb_cdc_init();

// Register receive callback (optional)
void my_cdc_rx_callback(const uint8_t *buf, uint32_t len) {
    // Process received data
    // Example: echo data back
    usb_cdc_send(buf, len);
}

usb_cdc_register_receive_callback(my_cdc_rx_callback);
#endif
```

### 4. Send Data via CDC

```c
const char *msg = "Hello from MidiCore!\r\n";
usb_cdc_send((const uint8_t*)msg, strlen(msg));
```

### 5. Check Connection Status

```c
if (usb_cdc_is_connected()) {
    // Host has opened the COM port (DTR active)
    usb_cdc_send((const uint8_t*)"Connected!\r\n", 13);
}
```

## API Reference

### Service API (`Services/usb_cdc/usb_cdc.h`)

#### `void usb_cdc_init(void)`
Initialize CDC service. Call once after USB Device initialization.

#### `int32_t usb_cdc_send(const uint8_t *buf, uint32_t len)`
Send data via CDC.
- **Parameters**: `buf` - data buffer, `len` - number of bytes
- **Returns**: Bytes sent (>= 0) or error code (< 0)
  - `USB_CDC_OK` (0): Success
  - `USB_CDC_ERROR` (-1): General error
  - `USB_CDC_BUSY` (-2): Previous transmission in progress
  - `USB_CDC_NOT_READY` (-3): USB not connected

#### `void usb_cdc_register_receive_callback(usb_cdc_rx_callback_t callback)`
Register callback for received data.
- **Parameters**: `callback` - function to call when data is received
- **Note**: Callback is called from interrupt context - keep it fast

#### `uint8_t usb_cdc_is_connected(void)`
Check if CDC interface is connected.
- **Returns**: 1 if connected (DTR active), 0 otherwise

### MIOS32 Compatibility API

For MIOS32 and MIOS Studio compatibility, these shims are available:

```c
int32_t MIOS32_USB_CDC_Init(void);
int32_t MIOS32_USB_CDC_SendBlock(const uint8_t *buf, uint32_t len);
int32_t MIOS32_USB_CDC_RegisterRxCallback(usb_cdc_rx_callback_t callback);
uint8_t MIOS32_USB_CDC_IsConnected(void);
```

## USB Device Configuration

### Descriptors

The device uses **composite device descriptors** when both MIDI and CDC are enabled:
- **bDeviceClass**: 0x00 (Composite)
- **MIDI Interfaces**: 0, 1 (Audio Control + MIDIStreaming)
- **CDC Interfaces**: 2, 3 (Communication + Data)

### Endpoints

| Endpoint | Type | Direction | Purpose | Size |
|----------|------|-----------|---------|------|
| 0x01 OUT | Bulk | OUT | MIDI data from host | 64 bytes |
| 0x81 IN  | Bulk | IN  | MIDI data to host | 64 bytes |
| 0x02 OUT | Bulk | OUT | CDC data from host | 64 bytes |
| 0x82 IN  | Bulk | IN  | CDC data to host | 64 bytes |
| 0x83 IN  | Interrupt | IN | CDC control/notification | 8 bytes |

### FIFO Allocation

STM32F4 USB_OTG_FS has 1.25 KB total FIFO space. Suggested allocation:
- **RX FIFO**: 256 bytes
- **TX0 (Control)**: 64 bytes
- **TX1 (MIDI)**: 128 bytes
- **TX2 (CDC Data)**: 128 bytes
- **TX3 (CDC Notification)**: 64 bytes

**Note**: If you encounter endpoint/FIFO conflicts, adjust `usbd_conf.c` FIFO sizes accordingly.

## Host-Side Setup

### Windows

1. Connect the device
2. Open **Device Manager** → **Ports (COM & LPT)**
3. You should see "MidiCore 4x4" as a COM port (e.g., COM3)
4. Open a terminal (PuTTY, Tera Term, etc.) to the COM port
   - **Baud rate**: Any (ignored by CDC ACM virtual port)
   - **Data bits**: 8
   - **Parity**: None
   - **Stop bits**: 1

**Driver**: Windows 10/11 uses built-in `usbser.sys` driver (no custom driver needed).

### macOS

1. Connect the device
2. List devices: `ls /dev/tty.usbmodem*`
3. You should see something like `/dev/tty.usbmodem1234`
4. Open terminal:
   ```bash
   screen /dev/tty.usbmodem1234
   ```
   Or use any serial terminal app.

### Linux

1. Connect the device
2. Device appears as `/dev/ttyACM0` (or ACM1, ACM2, etc.)
3. Add user to dialout group (one-time):
   ```bash
   sudo usermod -a -G dialout $USER
   # Log out and back in
   ```
4. Open terminal:
   ```bash
   screen /dev/ttyACM0
   ```
   Or:
   ```bash
   minicom -D /dev/ttyACM0
   ```

## MIOS Studio Integration

MIOS Studio expects certain descriptor strings for proper device detection:

1. **Product String**: Should contain "MidiCore" or match MIOS32 naming convention
2. **VID/PID**: Configurable in `USB_DEVICE/App/usbd_desc.c`
   - Default: VID=0x16C0 (Generic), PID=0x0489 (MIDI Device)
   - For CDC+MIDI composite, consider unique PID if needed

**Note**: MIOS Studio will detect the device via both MIDI and VCP interfaces. The VCP can be used for debugging, firmware updates, or configuration.

## Testing Procedure

### Test 1: Basic Enumeration

1. Enable CDC: `MODULE_ENABLE_USB_CDC = 1`
2. Build and flash firmware
3. Connect device to PC via USB
4. Verify:
   - Windows: Device Manager shows both MIDI device AND COM port
   - macOS: `ls /dev/tty.usbmodem*` and `ls /dev/cu.usbmodem*` show devices
   - Linux: `/dev/ttyACM0` appears

### Test 2: CDC Echo Test

```c
// In application code
void cdc_echo_callback(const uint8_t *buf, uint32_t len) {
    usb_cdc_send(buf, len);  // Echo back
}

usb_cdc_init();
usb_cdc_register_receive_callback(cdc_echo_callback);
```

1. Open serial terminal to COM port
2. Type characters
3. Verify they echo back

### Test 3: Concurrent MIDI + CDC

1. Open a MIDI monitor/DAW (e.g., MIOS Studio, Ableton Live)
2. Open a serial terminal to the COM port
3. Send MIDI data (e.g., press a key on the accordion)
4. Send serial data via terminal
5. Verify both work simultaneously without interference

### Test 4: MIOS Studio Detection

1. Open MIOS Studio
2. Device should appear in MIDI device list
3. VCP should be selectable for terminal/debugging
4. Test MIDI communication and VCP communication separately

## Troubleshooting

### Device Not Enumerating

- Check CubeMX USB configuration (USB_OTG_FS enabled)
- Verify 48 MHz USB clock is configured
- Check USB cable (must support data, not just charging)
- Check `Error_Handler()` breakpoints in `usb_device.c`

### COM Port Not Appearing

- Ensure `MODULE_ENABLE_USB_CDC = 1`
- Check composite descriptors are correct (see `usbd_desc.c`)
- Windows: Check Device Manager for "Unknown Device" (driver issue)
- Linux: Check `dmesg | tail` for USB enumeration errors

### Data Not Sending/Receiving

- Check `usb_cdc_is_connected()` returns 1
- Ensure host has opened the COM port (DTR must be active)
- Check send buffer is not full (`usb_cdc_send()` returns `USB_CDC_BUSY`)
- Verify callback is registered for receiving

### MIDI Stops Working After Enabling CDC

- Check endpoint assignments don't conflict
- Verify FIFO allocation in `usbd_conf.c`
- Ensure both classes are registered in `usb_device.c`
- Check descriptor `wTotalLength` is correct

### Build Errors

- If CDC files are missing, verify directory structure:
  ```
  USB_DEVICE/Class/CDC/Inc/usbd_cdc.h
  USB_DEVICE/Class/CDC/Inc/usbd_cdc_if.h
  USB_DEVICE/Class/CDC/Src/usbd_cdc.c
  USB_DEVICE/Class/CDC/Src/usbd_cdc_if.c
  Services/usb_cdc/usb_cdc.h
  Services/usb_cdc/usb_cdc.c
  ```
- Check `.cproject` includes `USB_DEVICE` source path
- Clean and rebuild project

## Performance Considerations

- **Latency**: CDC typically has ~1ms latency (USB frame period)
- **Throughput**: Up to ~750 KB/s for Full Speed USB (64-byte packets)
- **CPU Load**: Minimal - interrupt-driven with DMA support
- **RAM Usage**: ~200 bytes per CDC interface (buffers + state)

## Advanced Configuration

### Custom VID/PID

Edit `USB_DEVICE/App/usbd_desc.c`:

```c
#define USBD_VID       0x16C0  // Your Vendor ID
#define USBD_PID_FS    0x0489  // Your Product ID (composite device)
```

**Note**: For commercial products, obtain your own VID from USB-IF.

### Custom Product String

Edit `USB_DEVICE/App/usbd_desc.c`:

```c
#define USBD_PRODUCT_STRING_FS  "My Custom Device"
```

### Disable CDC at Runtime

CDC is compile-time enabled. To disable at runtime, don't call `usb_cdc_init()`. The descriptors will still expose the interface, but it won't function.

## See Also

- [USB_CONFIGURATION_GUIDE.md](USB_CONFIGURATION_GUIDE.md) - General USB configuration
- [MIOS32_USB.md](../mios32/MIOS32_USB.md) - MIOS32 USB compatibility notes
- Services/usb_cdc/usb_cdc.h - Service API header
- USB_DEVICE/Class/CDC/Inc/usbd_cdc.h - CDC class driver header

## References

- USB CDC ACM Specification: [USB Class Definitions for Communications Devices v1.2](https://www.usb.org/document-library/class-definitions-communication-devices-12)
- USB 2.0 Specification: [Universal Serial Bus Specification Revision 2.0](https://www.usb.org/document-library/usb-20-specification)
- STM32 USB Device Library: STM32CubeF4 USB Device Middleware Documentation
