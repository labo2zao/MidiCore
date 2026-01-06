#include "Services/usb_midi/usb_midi.h"
#include "Services/router/router.h"

void usb_midi_init(void) {}

void usb_midi_send_packet(uint8_t cin, uint8_t b0, uint8_t b1, uint8_t b2) {
  (void)cin; (void)b0; (void)b1; (void)b2;
  // Stub: enabled when ENABLE_USBD_MIDI + CubeMX MIDI class
}

void usb_midi_rx_packet(const uint8_t packet4[4]) {
  // Minimal 3-byte message from USB packet
  router_msg_t msg;
  msg.type = ROUTER_MSG_3B;
  msg.b0 = packet4[1];
  msg.b1 = packet4[2];
  msg.b2 = packet4[3];
  router_process(ROUTER_NODE_USB_IN, &msg);
}
