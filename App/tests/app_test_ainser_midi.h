#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
  AINSER64 -> MIDI CC self-test runner

  Purpose
  - Scans an AINSER64 input module (64 analog channels) and converts channel
    values into MIDI CC messages. Intended for hardware bring-up of the AINSER64
    board, SPI bus validation, and verifying mapping/CC output.

  How it runs
  - Call app_test_ainser_midi_run_forever() from StartDefaultTask() for a
    dedicated test run. The function contains an infinite loop and does not
    return.

  Enabling the test
  - Enable by defining the compiler symbol APP_TEST_AINSER_MIDI.
    Example (Makefile):
      CFLAGS += -DAPP_TEST_AINSER_MIDI

  Requirements
  - AINSER64 hardware wired to the SPI bus configured in Config/ainser64_pins.h
  - SPI bus enabled (MODULE_ENABLE_SPI_BUS) and hal_ainser64 backend available

  Optional compile-time configuration macros
  - APP_TEST_MIDI_OUT_PORT (default 1)
      Which UART port to send MIDI bytes on (MBHP style mapping: 0=UART1,
      1=UART2, 2=UART3, 3=UART5).
  - APP_TEST_MIDI_CH (default 0)
      MIDI channel to use (0 == channel 1).
  - APP_TEST_AINSER_CC_BASE (default 16)
      First CC number used for channel 0; channel N uses CC = CC_BASE + N.
  - APP_TEST_AINSER_THRESHOLD (default 8)
      Minimal 12-bit delta (raw ADC) required to send a CC update.
  - APP_TEST_MIDI_USE_USBH (ignored for AINSER: AINSER test routes via midi_router)

  Behaviour
  - Initializes SPI bus and the AINSER64 HAL backend
  - Initializes midi_router and the AINSER mapping layer (loads overrides
    from SD if present)
  - Continuously scans the 64 channels (8 mux steps of 8 channels each),
    quantises 12-bit ADC values (0..4095) to 7-bit CC by v >> 5, and sends CC
    messages when a change above threshold occurs.

  Example: minimal Makefile flags for running this test
    CFLAGS += -DMODULE_ENABLE_OLED=0
    CFLAGS += -DMODULE_ENABLE_LOOPER=0
    CFLAGS += -DMODULE_ENABLE_UI=0
    CFLAGS += -DAPP_TEST_AINSER_MIDI

  Notes
  - Initial readings initialize internal cache; the test does not emit CCs for
    the initial sample unless a significant difference is observed later.
  - Use this test for verification of AINSER wiring, SPI reliability and
    mapping behavior. Do not enable in production firmware.
*/

void app_test_ainser_midi_run_forever(void);

#ifdef __cplusplus
}
#endif
