# USB MIDI Device Test Module - Usage Guide

## Overview

This test module validates USB Device MIDI functionality by receiving MIDI data from a DAW or MIDI software via USB and printing it to UART for debugging. It also periodically sends test MIDI messages back to verify bidirectional communication.

## Quick Start

### 1. Enable the Test

Add this compiler define to your build configuration:

**CubeIDE:**
- Project → Properties → C/C++ Build → Settings
- MCU GCC Compiler → Preprocessor → Defined symbols
- Add: `APP_TEST_USB_MIDI`

**Makefile:**
```makefile
CFLAGS += -DAPP_TEST_USB_MIDI
```

### 2. Configure Modules

Ensure these are enabled in `Config/module_config.h`:
```c
#define MODULE_ENABLE_USB_MIDI 1
```

Optional: Disable unnecessary modules to save flash/RAM:
```c
#define MODULE_ENABLE_OLED 0
#define MODULE_ENABLE_LOOPER 0
#define MODULE_ENABLE_UI 0
```

### 3. Hardware Setup

**USB Connection:**
- Connect STM32 USB port to your computer
- Ensure USB_OTG_FS is configured in CubeMX (Device or OTG mode)

**UART Debug Connection:**
- Connect UART2 TX (PA2) to USB-to-Serial adapter
- Settings: 115200 baud, 8 data bits, no parity, 1 stop bit

### 4. Build and Flash

1. Build the project in CubeIDE or with make
2. Flash to your STM32 board
3. Open serial terminal (e.g., PuTTY, screen, minicom)

### 5. Test with DAW

**Using Ableton Live:**
1. Preferences → Link/MIDI → MIDI Ports
2. Enable "Track" and "Remote" for the STM32 device
3. Create a MIDI track
4. Send MIDI notes → Observe UART output

**Using MIDI-OX (Windows):**
1. Options → MIDI Devices
2. Select STM32 as Input and Output
3. View → Input Monitor to see received messages
4. Send test messages and observe UART output

**Using MIDI Monitor (macOS):**
1. Launch MIDI Monitor
2. Select STM32 device
3. Send MIDI and observe both MIDI Monitor and UART

## Example Output

When the test is running, you'll see output like this in your serial terminal:

```
=====================================
USB MIDI Device Test
=====================================
USB Device MIDI: Enabled
Debug UART: UART2 (115200 baud)
Test send interval: 2000 ms
Test channel: 1
Test note: 60
USB Cable: 0
-------------------------------------
Test started. Waiting for USB MIDI data from DAW...
Sending test MIDI messages every 2000 ms
-------------------------------------
[TX] Sending test Note On: Cable:0 90 3C 64
[RX] Cable:0 90 3C 64 (Note On Ch:1 Note:60 Vel:100)
[RX] Cable:0 80 3C 00 (Note Off Ch:1 Note:60 Vel:0)
[TX] Sending test Note Off: Cable:0 80 3C 00
[RX] Cable:0 B0 07 7F (CC Ch:1 CC:7 Val:127)
[RX] Cable:0 B0 0A 40 (CC Ch:1 CC:10 Val:64)
[TX] Sending test Note On: Cable:0 90 3C 64
```

## Configuration Options

You can customize the test behavior by defining these macros before compilation:

```c
// Time between automatic test messages (default: 2000ms)
#define APP_TEST_USB_MIDI_SEND_INTERVAL 3000

// Base MIDI note for test messages (default: 60 = Middle C)
#define APP_TEST_USB_MIDI_BASE_NOTE 48

// MIDI channel (0-15, where 0 = channel 1)
#define APP_TEST_USB_MIDI_CHANNEL 0

// Note velocity for test messages (default: 100)
#define APP_TEST_USB_MIDI_VELOCITY 120

// USB MIDI cable number (0-3 for 4 virtual ports)
#define APP_TEST_USB_MIDI_CABLE 0

// Debug UART port (0=UART1, 1=UART2, 2=UART3, 3=UART5)
#define TEST_DEBUG_UART_PORT 1

// Debug UART baud rate
#define TEST_DEBUG_UART_BAUD 115200
```

## Advanced Usage

### Sending Custom MIDI Messages

While the test is running, you can call these functions for interactive testing:

```c
// Send a Control Change message
app_test_usb_midi_send_cc(7, 127);  // Volume CC to maximum

// Send any 3-byte MIDI message
app_test_usb_midi_send3(0x90, 60, 100);  // Note On C4, velocity 100
app_test_usb_midi_send3(0xB0, 1, 64);    // Modulation wheel to center
```

### Testing Multiple Cables

USB MIDI supports 4 virtual cables (ports). Change the cable:

```c
#define APP_TEST_USB_MIDI_CABLE 1  // Use cable 1 instead of 0
```

Each cable maps to a different router node:
- Cable 0 → `ROUTER_NODE_USB_PORT0`
- Cable 1 → `ROUTER_NODE_USB_PORT1`
- Cable 2 → `ROUTER_NODE_USB_PORT2`
- Cable 3 → `ROUTER_NODE_USB_PORT3`

## Troubleshooting

### No UART Output

- Check UART connections (TX on PA2 for UART2)
- Verify baud rate is 115200
- Ensure `TEST_DEBUG_UART_PORT` matches your hardware

### USB Device Not Detected

- Check USB cable (must support data, not just power)
- Verify USB_OTG_FS is configured in CubeMX
- Check that `MODULE_ENABLE_USB_MIDI=1`
- Try different USB port or cable

### No MIDI Reception

- Verify DAW MIDI routing is correct
- Check that your DAW is sending to the correct USB MIDI port
- Some DAWs require explicit MIDI output enabling

### Test Messages Not Sent

- Check the send interval isn't too long
- Verify USB connection is stable
- Check `MODULE_ENABLE_USB_MIDI=1` is set

### Unexpected MIDI Data

- Use MIDI Monitor/MIDI-OX to verify what DAW is sending
- Check USB cable for noise/interference
- Verify MIDI channel settings match

## Integration with Module Tests

This test is integrated into the module test framework. You can also run it via:

**Compile-time selection:**
```c
#define MODULE_TEST_USB_DEVICE_MIDI
```

**Runtime selection** (if test framework is used):
```c
module_tests_run(MODULE_TEST_USB_DEVICE_MIDI_ID);
```

## Technical Details

### USB MIDI Packet Format

USB MIDI uses 4-byte packets:
```
[Cable/CIN] [Status] [Data1] [Data2]
     ^         ^        ^       ^
     |         |        |       +-- Second data byte
     |         |        +---------- First data byte
     |         +------------------- MIDI status byte
     +----------------------------- Cable (4 bits) + Code Index (4 bits)
```

### Debug Hook Implementation

The test uses a weak symbol override to intercept USB MIDI packets:

```c
// In usb_midi.c (weak default)
__attribute__((weak)) void usb_midi_rx_debug_hook(const uint8_t packet4[4]);

// In app_test_usb_midi.c (override)
void usb_midi_rx_debug_hook(const uint8_t packet4[4]) {
  print_usb_midi_packet(packet4);
}
```

This allows the test to observe all received MIDI without modifying the router.

## Safety Notes

- **This is a test module** - Do not enable in production firmware
- The test runs forever and prevents normal application startup
- Disable `APP_TEST_USB_MIDI` after testing is complete
- The weak symbol override is only active when this test is compiled in

## See Also

- `App/tests/README_TESTS.md` - General test framework documentation
- `App/tests/app_test_usb_midi.h` - API reference
- `Services/usb_midi/usb_midi.h` - USB MIDI service API
- `README_USB_DEVICE_INTEGRATION.md` - USB Device integration guide
