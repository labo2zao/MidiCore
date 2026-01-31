#include "Services/router/router_send.h"
#include "Hal/uart_midi/hal_uart_midi.h"

#ifdef ENABLE_USBD_MIDI
#include "Services/usb_midi/usb_midi.h"
#include "Services/usb_midi/usb_midi_sysex.h"
/* NOTE: usb_midi_send_packet() and usb_midi_send_sysex() now return bool
 * Return value indicates success (true) or failure due to TX queue full (false)
 * 
 * For normal MIDI routing (this file), we don't check return values because:
 * 1. MIDI traffic is continuous and lossy by nature
 * 2. Missing one note in thousands is acceptable
 * 3. TX queue drops are tracked globally and reported via diagnostics
 * 
 * For CRITICAL messages (MidiCore queries), return value MUST be checked
 * and retries implemented - see Services/midicore_query/midicore_query.c
 */
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
  // Handle USB Device MIDI ports (USB_PORT0-3 = cables 0-3)
  if (out_node >= ROUTER_NODE_USB_PORT0 && out_node <= ROUTER_NODE_USB_PORT3) {
    uint8_t cable = (uint8_t)(out_node - ROUTER_NODE_USB_PORT0); // 0-3
    
    // Handle SysEx messages
    if (msg->type == ROUTER_MSG_SYSEX && msg->data && msg->len > 0) {
      usb_midi_send_sysex(msg->data, msg->len, cable);
      return 0;
    }
    
    // Map status byte to correct CIN (Code Index Number)
    uint8_t cin = 0x09; // Default: Note On
    uint8_t status = msg->b0 & 0xF0;
    
    // Add cable number to CIN (cable in upper 4 bits)
    uint8_t cin_with_cable = (cable << 4);
    
    if (msg->type == ROUTER_MSG_1B) {
      // System Real-Time (0xF8-0xFF) or single-byte System Common
      cin = 0x0F;
      usb_midi_send_packet(cin_with_cable | cin, msg->b0, 0, 0);
    } else if (msg->type == ROUTER_MSG_2B) {
      // Program Change or Channel Pressure
      if (status == 0xC0) cin = 0x0C;      // Program Change
      else if (status == 0xD0) cin = 0x0D; // Channel Pressure
      else cin = 0x02;                      // System Common 2-byte
      usb_midi_send_packet(cin_with_cable | cin, msg->b0, msg->b1, 0);
    } else {
      // 3-byte channel voice messages
      if (status == 0x80) cin = 0x08;      // Note Off
      else if (status == 0x90) cin = 0x09; // Note On
      else if (status == 0xA0) cin = 0x0A; // Poly KeyPressure
      else if (status == 0xB0) cin = 0x0B; // Control Change
      else if (status == 0xE0) cin = 0x0E; // Pitch Bend
      else cin = 0x03;                      // System Common 3-byte
      usb_midi_send_packet(cin_with_cable | cin, msg->b0, msg->b1, msg->b2);
    }
    return 0;
  }
  
  // Legacy ROUTER_NODE_USB_OUT support (defaults to cable 0)
  if (out_node == ROUTER_NODE_USB_OUT) {
    // Handle SysEx messages
    if (msg->type == ROUTER_MSG_SYSEX && msg->data && msg->len > 0) {
      usb_midi_send_sysex(msg->data, msg->len, 0);
      return 0;
    }
    
    // Map status byte to correct CIN
    uint8_t cin = 0x09; // Default: Note On
    uint8_t status = msg->b0 & 0xF0;
    
    if (msg->type == ROUTER_MSG_1B) {
      // System Real-Time (0xF8-0xFF) or single-byte System Common
      cin = 0x0F;
      usb_midi_send_packet(cin, msg->b0, 0, 0);
    } else if (msg->type == ROUTER_MSG_2B) {
      // Program Change or Channel Pressure
      if (status == 0xC0) cin = 0x0C;      // Program Change
      else if (status == 0xD0) cin = 0x0D; // Channel Pressure
      else cin = 0x02;                      // System Common 2-byte
      usb_midi_send_packet(cin, msg->b0, msg->b1, 0);
    } else {
      // 3-byte channel voice messages
      if (status == 0x80) cin = 0x08;      // Note Off
      else if (status == 0x90) cin = 0x09; // Note On
      else if (status == 0xA0) cin = 0x0A; // Poly KeyPressure
      else if (status == 0xB0) cin = 0x0B; // Control Change
      else if (status == 0xE0) cin = 0x0E; // Pitch Bend
      else cin = 0x03;                      // System Common 3-byte
      usb_midi_send_packet(cin, msg->b0, msg->b1, msg->b2);
    }
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
