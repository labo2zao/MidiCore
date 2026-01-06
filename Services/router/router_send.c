#include "Services/router/router_send.h"
#include "Hal/uart_midi/hal_uart_midi.h"

#ifdef ENABLE_USBD_MIDI
#include "Services/usb_midi/usb_midi.h"
#endif

#if defined(ENABLE_USBH_MIDI) && (ENABLE_USBH_MIDI)
#include "Services/usb_host_midi/usb_host_midi.h"
#endif

static void send_bytes_uart(uint8_t port, const router_msg_t* msg) {
  if (msg->type == ROUTER_MSG_1B) {
    hal_uart_midi_send_byte(port, msg->b0);
  } else if (msg->type == ROUTER_MSG_2B) {
    hal_uart_midi_send_byte(port, msg->b0);
    hal_uart_midi_send_byte(port, msg->b1);
  } else if (msg->type == ROUTER_MSG_3B) {
    hal_uart_midi_send_byte(port, msg->b0);
    hal_uart_midi_send_byte(port, msg->b1);
    hal_uart_midi_send_byte(port, msg->b2);
  } else if (msg->type == ROUTER_MSG_SYSEX) {
    // Optional later: stream sysex bytes out
    for (uint16_t i=0;i<msg->len;i++) hal_uart_midi_send_byte(port, msg->data[i]);
  }
}

int router_send_default(uint8_t out_node, const router_msg_t* msg) {
  if (!msg) return -1;

  // DIN OUT 1..4 map to ports 0..3
  if (out_node >= ROUTER_NODE_DIN_OUT1 && out_node <= ROUTER_NODE_DIN_OUT4) {
    uint8_t port = (uint8_t)(out_node - ROUTER_NODE_DIN_OUT1);
    send_bytes_uart(port, msg);
    return 0;
  }

#ifdef ENABLE_USBD_MIDI
  if (out_node == ROUTER_NODE_USB_OUT) {
    // Needs real CIN mapping; minimal: assume 3-byte channel voice
    uint8_t cin = 0x09; // NOTE ON default; TODO map per status
    usb_midi_send_packet(cin, msg->b0, msg->b1, msg->b2);
    return 0;
  }
#endif


#if defined(ENABLE_USBH_MIDI) && (ENABLE_USBH_MIDI)
  if (out_node == ROUTER_NODE_USBH_OUT) {
    if (msg->type == ROUTER_MSG_1B) return usb_host_midi_send3(msg->b0, 0, 0);
    if (msg->type == ROUTER_MSG_2B) return usb_host_midi_send3(msg->b0, msg->b1, 0);
    if (msg->type == ROUTER_MSG_3B) return usb_host_midi_send3(msg->b0, msg->b1, msg->b2);
    return -1;
  }
#endif

  return 0;
}
