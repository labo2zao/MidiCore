# MidiCore: App/tests — Quick test harnesses

This directory contains small manual test harnesses intended for hardware bring-up
and interactive testing. They are not unit tests; they are runtime test helpers
that map hardware inputs to MIDI output to verify wiring and service behaviour.

Available test modules
- app_test_din_midi: map DIN (SRIO digital inputs) to MIDI Note/CC messages
- app_test_ainser_midi: scan AINSER64 analog inputs and send MIDI CCs

How to use
1. Enable only the modules you need for testing (disable UI/OLED/Looper to
   reduce flash/ram and to simplify logs).
2. Define the corresponding APP_TEST_* symbol in your build (project-wide):
   - APP_TEST_DIN_MIDI
   - APP_TEST_AINSER_MIDI

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

Safety
- These tests are for development and hardware validation only. Do not enable
  APP_TEST_* defines in production firmware images.
