#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
  USB MIDI Device -> UART debug test runner

  Purpose
  - Test USB MIDI Device functionality by receiving MIDI data from a DAW via USB
    and printing it to UART for debugging. Also provides functionality to send
    test MIDI messages over USB to verify bidirectional communication.

  How it runs
  - Call app_test_usb_midi_run_forever() from StartDefaultTask() for a
    dedicated test run. The function contains an infinite loop and does not
    return.

  Enabling the test
  - Enable by defining the compiler symbol APP_TEST_USB_MIDI.
    Example (Makefile):
      CFLAGS += -DAPP_TEST_USB_MIDI
    Example (CubeIDE / Project Properties → C/C++ Build → Settings → MCU GCC
    Compiler → Preprocessor → Defined symbols):
      APP_TEST_USB_MIDI

  Requirements
  - USB Device MIDI must be enabled (MODULE_ENABLE_USB_MIDI)
  - USB_OTG_FS configured in CubeMX as Device or OTG mode
  - UART configured for debug output (default: UART2 at 115200 baud)

  Optional compile-time configuration macros
  - APP_TEST_USB_MIDI_SEND_INTERVAL (default 2000)
      Interval in milliseconds between automatic test MIDI messages sent via USB.
  - APP_TEST_USB_MIDI_BASE_NOTE (default 60)
      Base MIDI note (middle C) for test note messages.
  - APP_TEST_USB_MIDI_CHANNEL (default 0)
      MIDI channel for test messages (0 = channel 1).
  - APP_TEST_USB_MIDI_VELOCITY (default 100)
      Velocity for test note messages.
  - APP_TEST_USB_MIDI_CABLE (default 0)
      USB MIDI cable number to use for sending (0-3).
  - TEST_DEBUG_UART_PORT (default 1)
      Which UART port to use for debug output (0=UART1, 1=UART2, etc.)

  Behaviour
  - Initializes USB Device MIDI and debug UART
  - Registers a callback to intercept USB MIDI packets received from DAW
  - Prints received MIDI data to UART in human-readable format with hex values
  - Periodically sends test MIDI messages (Note On/Off) via USB to verify
    bidirectional communication
  - Runs indefinitely, logging all USB MIDI activity to UART

  Example UART output:
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

  Example: minimal Makefile flags for running this test
    CFLAGS += -DMODULE_ENABLE_OLED=0
    CFLAGS += -DMODULE_ENABLE_LOOPER=0
    CFLAGS += -DMODULE_ENABLE_UI=0
    CFLAGS += -DMODULE_ENABLE_USB_MIDI=1
    CFLAGS += -DAPP_TEST_USB_MIDI

  Notes
  - This test is designed to work with USB Device MIDI only (not USB Host)
  - Connect the board to a computer via USB and use a DAW (e.g., Ableton,
    Logic, Reaper) or MIDI monitoring tool (e.g., MIDI-OX) to send/receive
    MIDI messages
  - The test intercepts USB MIDI packets before they reach the router, allowing
    raw packet inspection
  - Use this test for verification of USB MIDI Device functionality, descriptor
    configuration, and bidirectional MIDI communication
  - Do not enable in production firmware

  Safety / Notes
  - Because this test function does not return, normal application startup
    (app_entry_start) will not run while the test is active.
  - Use this for development and hardware bring-up only. Avoid shipping builds
    with APP_TEST_USB_MIDI enabled.
*/

void app_test_usb_midi_run_forever(void);

/**
 * @brief Send a MIDI Control Change message via USB
 * @param cc_number CC number (0-127)
 * @param cc_value CC value (0-127)
 * 
 * Sends a CC message on the configured channel and cable.
 * Useful for interactive testing and automation.
 * 
 * Example:
 *   app_test_usb_midi_send_cc(7, 127);  // Volume to max
 */
void app_test_usb_midi_send_cc(uint8_t cc_number, uint8_t cc_value);

/**
 * @brief Send a generic 3-byte MIDI message via USB
 * @param status MIDI status byte (includes channel)
 * @param data1 First data byte
 * @param data2 Second data byte
 * 
 * Sends any 3-byte MIDI message on the configured cable.
 * 
 * Example:
 *   app_test_usb_midi_send3(0x90, 60, 100);  // Note On Ch1, C4, vel 100
 */
void app_test_usb_midi_send3(uint8_t status, uint8_t data1, uint8_t data2);

#ifdef __cplusplus
}
#endif
