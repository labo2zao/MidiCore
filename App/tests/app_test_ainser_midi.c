#include "App/tests/app_test_ainser_midi.h"

#include "cmsis_os2.h"

#include "Hal/spi_bus.h"
#include "Hal/ainser64_hw/hal_ainser64_hw_step.h"
#include "Hal/uart_midi/hal_uart_midi.h"
#include "Services/usb_host_midi/usb_host_midi.h"
#include "Services/ainser/ainser_map.h"
#include "Services/midi/midi_router.h"
#include "Services/patch/patch_sd_mount.h"

#ifndef APP_TEST_MIDI_OUT_PORT
#define APP_TEST_MIDI_OUT_PORT 1u // UART2 by default (ports: 0=UART1,1=UART2,2=UART3,3=UART5)
#endif

#ifndef APP_TEST_MIDI_CH
#define APP_TEST_MIDI_CH 0u // MIDI channel 1
#endif



// Helper: send a 3-byte MIDI message.
// By default we send on UART (MBHP MIDI Out).
// If APP_TEST_MIDI_USE_USBH is defined, we duplicate to USB Host MIDI too.
static inline void midi_send3(uint8_t status, uint8_t d1, uint8_t d2)
{
  // UART (MBHP MIDI Out)
  hal_uart_midi_send_byte((uint8_t)APP_TEST_MIDI_OUT_PORT, status);
  hal_uart_midi_send_byte((uint8_t)APP_TEST_MIDI_OUT_PORT, d1);
  hal_uart_midi_send_byte((uint8_t)APP_TEST_MIDI_OUT_PORT, d2);

#ifdef APP_TEST_MIDI_USE_USBH
  (void)usb_host_midi_send3(status, d1, d2);
#endif
}

static void ainser_test_output_cb(uint8_t channel, uint8_t cc, uint8_t value)
{
  midi_router_cc(MIDI_ROUTER_SRC_AINSER, channel, cc, value);
}


void app_test_ainser_midi_run_forever(void)
{
  // Init shared SPI bus & AINSER backend.
  spibus_init();
  (void)hal_ainser64_init();

  // Optional: leave link LED modulation enabled (default), looks nice when scanning.

  // Init MIDI router then mapping layer (defaults + output callback).
  midi_router_init();
  // Init mapping layer (defaults + output callback).
  ainser_map_init_defaults();
  ainser_map_set_output_cb(ainser_test_output_cb);
  // Try to mount SD and load AINSER map overrides if present.
  if (patch_sd_mount_retry(3) == 0) {
    (void)ainser_map_load_sd("0:/cfg/ainser_map.ngc");
  }

  // Main loop: scan all 8 mux steps, 8 channels each.
  for (;;)
  {
    for (uint8_t step = 0; step < 8; ++step)
    {
      uint16_t vals[8];
      if (hal_ainser64_read_bank_step(0u, step, vals) != 0)
      {
        // In case of SPI error, just skip this step and try again later.
        continue;
      }

      for (uint8_t ch = 0; ch < 8; ++ch)
      {
        uint8_t idx = (uint8_t)(step * 8u + ch); // 0..63
        uint16_t v = vals[ch];
        ainser_map_process_channel(idx, v);
      }
    }

    // Small delay to yield CPU, overall scan rate still high enough.
    osDelay(1);
  }
}
