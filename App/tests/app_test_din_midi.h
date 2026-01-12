#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// DIN->MIDI selftest runner intended to be called from StartDefaultTask().
//
// Enable by adding compiler symbol:
//   APP_TEST_DIN_MIDI
//
// Requires:
//   SRIO_ENABLE (for pin mapping in Services/srio/srio_user_config.h)
//
// Optional compile-time overrides:
//   APP_TEST_MIDI_OUT_PORT   (default 1 -> UART2)
//   APP_TEST_MIDI_CH         (default 0 -> MIDI channel 1)
//   APP_TEST_MIDI_BASE_NOTE  (default 36 -> C2)
//   APP_TEST_MIDI_VELOCITY   (default 100)

void app_test_din_midi_run_forever(void);

#ifdef __cplusplus
}
#endif
