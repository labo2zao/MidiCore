#include "App/tests/app_test_din_midi.h"

#include "cmsis_os2.h"
#include "main.h"

#include "Services/srio/srio.h"
#include "Services/srio/srio_user_config.h"
#include "Services/din/din_map.h"
#include "Services/patch/patch_sd_mount.h"
#include "Services/midi/midi_router.h"
#include "Services/usb_host_midi/usb_host_midi.h"

#include "Hal/uart_midi/hal_uart_midi.h"

#include <string.h>

#ifndef APP_TEST_MIDI_BASE_NOTE
#define APP_TEST_MIDI_BASE_NOTE 36
#endif

#ifndef APP_TEST_MIDI_CH
#define APP_TEST_MIDI_CH 0
#endif

#ifndef APP_TEST_MIDI_VELOCITY
#define APP_TEST_MIDI_VELOCITY 96
#endif

#ifndef APP_TEST_MIDI_USE_USBH
#define APP_TEST_MIDI_USE_USBH 1
#endif

static uint8_t din_prev[SRIO_DIN_BYTES];

static void app_test_din_output_cb(DIN_MapType type, uint8_t channel,
                                   uint8_t number, uint8_t value)
{
  if (type == DIN_MAP_TYPE_NOTE) {
    if (value) {
      midi_router_note_on(MIDI_ROUTER_SRC_DIN, channel, number, value);
    } else {
      midi_router_note_off(MIDI_ROUTER_SRC_DIN, channel, number, 0u);
    }
  } else if (type == DIN_MAP_TYPE_CC) {
    midi_router_cc(MIDI_ROUTER_SRC_DIN, channel, number, value);
  }
}

void app_test_din_midi_run_forever(void)
{
#ifdef SRIO_ENABLE
  // 0) SD + DIN mapping
  if (patch_sd_mount_retry(3) == 0) {
    din_map_init_defaults((uint8_t)APP_TEST_MIDI_BASE_NOTE);
    din_map_set_output_cb(app_test_din_output_cb);
    (void)din_map_load_sd("0:/cfg/din_map.ngc");
  } else {
    din_map_init_defaults((uint8_t)APP_TEST_MIDI_BASE_NOTE);
    din_map_set_output_cb(app_test_din_output_cb);
  }

  // 1) UART MIDI
  (void)hal_uart_midi_init();

  // 2) SRIO
  srio_config_t scfg = {
    .hspi              = SRIO_SPI_HANDLE,
    .din_pl_port       = SRIO_DIN_PL_PORT,
    .din_pl_pin        = SRIO_DIN_PL_PIN,
    .dout_rclk_port    = SRIO_DOUT_RCLK_PORT,
    .dout_rclk_pin     = SRIO_DOUT_RCLK_PIN,
    .dout_oe_port      = NULL,
    .dout_oe_pin       = 0,
    .dout_oe_active_low= 1,
    .din_bytes         = SRIO_DIN_BYTES,
    .dout_bytes        = SRIO_DOUT_BYTES
  };
  srio_init(&scfg);

  memset(din_prev, 0xFF, sizeof(din_prev));
  (void)srio_read_din(din_prev);

  // 3) Boucle principale
  for (;;) {
#if APP_TEST_MIDI_USE_USBH
    usb_host_midi_task();
#endif
    uint8_t din[SRIO_DIN_BYTES];
    if (srio_read_din(din) == 0) {
      for (uint16_t bit = 0; bit < (uint16_t)(SRIO_DIN_BYTES * 8u); ++bit) {
        uint16_t byte = (uint16_t)(bit >> 3);
        uint8_t  mask = (uint8_t)(1u << (bit & 7u));
        uint8_t prev = (din_prev[byte] & mask) ? 1u : 0u;
        uint8_t cur  = (din[byte]      & mask) ? 1u : 0u;
        if (prev != cur) {
          // active-low : pressed quand la ligne passe à 0
          uint8_t pressed = (cur == 0u) ? 1u : 0u;
          din_map_process_event((uint8_t)bit, pressed);
        }
      }
      memcpy(din_prev, din, SRIO_DIN_BYTES);
    }
    osDelay(1);
  }
#else
  for (;;)
    osDelay(1000);
#endif
}
