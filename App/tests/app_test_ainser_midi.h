#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// AINSER64 -> MIDI CC selftest runner intended to be called from StartDefaultTask().
//
// Enable by adding compiler symbol:
//   APP_TEST_AINSER_MIDI
//
// This will:
//   - initialise the shared SPI bus + AINSER64 backend
//   - continuously scan the 64 AINSER channels (0..63)
//   - on significant change, send a MIDI CC for the corresponding channel
//
// Requires:
//   AINSER64 hardware wired to SPI3 + CS as defined in Config/ainser64_pins.h
//
// Optional compile-time overrides (shared with DIN test):
//   APP_TEST_MIDI_OUT_PORT    (default 1 -> UART2)
//   APP_TEST_MIDI_CH          (default 0 -> MIDI channel 1)
//   APP_TEST_MIDI_VELOCITY    (ignored here, we send CC values)
// Specific to AINSER test:
//   APP_TEST_AINSER_CC_BASE   (default 16 -> first CC# used for channel 0)
//   APP_TEST_AINSER_THRESHOLD (default 8  -> minimal 12-bit delta to trigger a CC)
//
// Notes:
//   - We quantise 12-bit ADC values (0..4095) to 7-bit CC (0..127) by v >> 5.
//   - Initial readings only initialise the cache, no CC is sent.

void app_test_ainser_midi_run_forever(void);

#ifdef __cplusplus
}
#endif
