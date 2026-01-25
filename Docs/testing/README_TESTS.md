# MidiCore: App/tests — Quick test harnesses

This directory contains small manual test harnesses intended for hardware bring-up
and interactive testing. They are not unit tests; they are runtime test helpers
that map hardware inputs to MIDI output to verify wiring and service behaviour.

Available test modules
- app_test_din_midi: map DIN (SRIO digital inputs) to MIDI Note/CC messages
- app_test_ainser_midi: scan AINSER64 analog inputs and send MIDI CCs
- app_test_usb_midi: test USB MIDI Device (receive from DAW, print to UART, send test messages)

How to use
1. Enable only the modules you need for testing (disable UI/OLED/Looper to
   reduce flash/ram and to simplify logs).
2. Define the corresponding APP_TEST_* symbol in your build (project-wide):
   - APP_TEST_DIN_MIDI
   - APP_TEST_AINSER_MIDI
   - APP_TEST_USB_MIDI

3. The test runner function (app_test_*_run_forever) is intended to be called
   from StartDefaultTask (see Core/Src/main.c). It contains an infinite loop
   and will prevent the normal application entrypoint (app_entry_start) from
   running.

Example Makefile flags (for testing)
```makefile
# Minimal configuration for test run
CFLAGS += -DMODULE_ENABLE_OLED=0
CFLAGS += -DMODULE_ENABLE_LOOPER=0
CFLAGS += -DMODULE_ENABLE_UI=0
CFLAGS += -DAPP_TEST_DIN_MIDI
```

CubeIDE / GUI example (Project properties → C/C++ Build → Settings → MCU GCC
Compiler → Preprocessor → Defined symbols):
- Add `APP_TEST_AINSER_MIDI` to run the AINSER test.

Common pitfalls & troubleshooting
- Nothing happens at startup:
  - Ensure APP_TEST_* is defined and that the branch of code in Core/Src/main.c
    is compiled with that define.
  - Check the module configuration flags (Config/module_config.h or compiler
    defines) to ensure required services (SRIO, SPI, etc.) are enabled.
- SD card map overrides not loaded:
  - The tests attempt to mount the SD and load cfg/<module>_map.ngc. This is
    optional; missing files fall back to defaults.
- USB Host MIDI not sending:
  - APP_TEST_MIDI_USE_USBH controls duplication to USB Host for the DIN test.
  - Ensure USB Host MIDI service is present in your module configuration.

## USB MIDI Device Test (app_test_usb_midi)

This test validates USB Device MIDI functionality by receiving MIDI data from a 
DAW or MIDI software and printing it to UART, while also periodically sending 
test MIDI messages back to the host.

### Features
- Receives USB MIDI packets from DAW/computer via USB
- Decodes and prints MIDI messages to UART in human-readable format
- Sends periodic test MIDI messages (Note On/Off) via USB
- Provides functions to send CC and custom MIDI messages
- Supports all 4 USB MIDI cables (ports)

### Requirements
- `MODULE_ENABLE_USB_MIDI=1` in Config/module_config.h
- USB Device configured in CubeMX (USB_OTG_FS in Device or OTG mode)
- UART configured for debug output (default: UART2 at 115200 baud)

### Example UART output
```
=====================================
USB MIDI Device Test
=====================================
USB Device MIDI: Enabled
Debug UART: UART2 (115200 baud)
Test send interval: 2000 ms
-------------------------------------
[RX] Cable:0 90 3C 64 (Note On Ch:1 Note:60 Vel:100)
[RX] Cable:0 80 3C 00 (Note Off Ch:1 Note:60 Vel:0)
[TX] Sending test Note On: Cable:0 90 3C 64
[TX] Sending test Note Off: Cable:0 80 3C 00
[RX] Cable:0 B0 07 7F (CC Ch:1 CC:7 Val:127)
```

### Configuration options
Define these before including the test to customize behavior:
```c
#define APP_TEST_USB_MIDI_SEND_INTERVAL 2000  // ms between test messages
#define APP_TEST_USB_MIDI_BASE_NOTE 60        // Middle C
#define APP_TEST_USB_MIDI_CHANNEL 0           // Channel 1 (0-based)
#define APP_TEST_USB_MIDI_VELOCITY 100        // Note velocity
#define APP_TEST_USB_MIDI_CABLE 0             // USB MIDI cable 0-3
```

### Usage with DAW
1. Build firmware with `APP_TEST_USB_MIDI` defined
2. Connect board to computer via USB
3. Connect UART to serial terminal (115200 baud, 8N1)
4. Open your DAW (Ableton, Logic, Reaper, etc.) or MIDI monitoring tool
5. Select the board as MIDI input/output device
6. Send MIDI notes/CCs from DAW and observe UART output
7. DAW should receive test MIDI messages every 2 seconds

### Public API functions
The test module provides these functions for advanced usage:
```c
void app_test_usb_midi_send_cc(uint8_t cc_number, uint8_t cc_value);
void app_test_usb_midi_send3(uint8_t status, uint8_t data1, uint8_t data2);
```

Safety
- These tests are for development and hardware validation only. Do not enable
  APP_TEST_* defines in production firmware images.
