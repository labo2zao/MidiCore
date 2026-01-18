#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
  DIN -> MIDI self-test runner

  Purpose
  - Minimal, manual test harness that maps DIN (digital inputs, e.g. MBHP SRIO)
    to MIDI messages (Note On/Off and CC). Designed for quick hardware sanity
    checks and interactive testing of DIN mapping and MIDI output.

  How it runs
  - The function app_test_din_midi_run_forever() is intended to be invoked from
    the RTOS StartDefaultTask() path (see Core/Src/main.c). It contains an
    infinite loop and therefore does not return. Use it only for dedicated
    test builds or when you intentionally want the firmware to run this test
    harness instead of the normal app entry point.

  Enabling the test
  - Enable by defining the compiler symbol APP_TEST_DIN_MIDI (project-wide).
    Example (Makefile):
      CFLAGS += -DAPP_TEST_DIN_MIDI
    Example (CubeIDE / Project Properties → C/C++ Build → Settings → MCU GCC
    Compiler → Preprocessor → Defined symbols):
      APP_TEST_DIN_MIDI

  Requirements
  - SRIO hardware and the SRIO service must be enabled:
      - Define/ensure SRIO_ENABLE and SRIO pin configuration in
        Services/srio/srio_user_config.h
  - UART MIDI (MBHP) and/or USB Host MIDI available depending on compile flags.

  Optional compile-time configuration macros
  - APP_TEST_MIDI_BASE_NOTE  (default 36)
      Base MIDI note number used by the default DIN map (C2).
  - APP_TEST_MIDI_CH        (default 0)
      MIDI channel (0 == channel 1).
  - APP_TEST_MIDI_VELOCITY  (default 96)
      Default velocity for Note On.
  - APP_TEST_MIDI_USE_USBH  (default 1)
      If 1, duplicate messages to USB Host MIDI (if present). Set to 0 to
      disable USBH output.

  Behaviour
  - Initializes MIDI router and DIN mapping (loads overrides from SD if present)
  - Initializes UART MIDI (hal_uart_midi_init) and SRIO
  - Continuously polls SRIO DIN state and translates changes into MIDI Note
    On/Off or CC messages via midi_router_xxx functions
  - If the SD card contains cfg/din_map.ngc it will be loaded to override
    the default mapping.

  Example: minimal Makefile flags for running this test
    CFLAGS += -DMODULE_ENABLE_OLED=0
    CFLAGS += -DMODULE_ENABLE_LOOPER=0
    CFLAGS += -DMODULE_ENABLE_UI=0
    CFLAGS += -DAPP_TEST_DIN_MIDI

  Safety / Notes
  - Because this test function does not return, normal application startup
    (app_entry_start) will not run while the test is active.
  - Use this for development and hardware bring-up only. Avoid shipping builds
    with APP_TEST_DIN_MIDI enabled.
*/

void app_test_din_midi_run_forever(void);

#ifdef __cplusplus
}
#endif
