
#include "Services/usb_host_midi/usb_host_midi.h"
#include "Services/usb_host_midi/usbh_midi.h"
#include "Services/router/router.h"
#include "Config/module_config.h"

#include "usbh_core.h"

#if defined(USBH_MIDI_PRESENT) && USBH_MIDI_PRESENT && MODULE_ENABLE_USBH_MIDI

// USB Host handle is defined in CubeMX-generated USB_HOST/App/usb_host.c
extern USBH_HandleTypeDef hUsbHostFS;

void usb_host_midi_init(void)
{
  // For now, nothing specific to do here.
  // MX_USB_HOST_Init() is called from main.c before the default task starts.
}

void usb_host_midi_task(void)
{
  // Pump the USB Host state machine (MIOS32 style periodic polling)
  USBH_Process(&hUsbHostFS);
  
  // Process incoming MIDI packets from USB Host device
  uint8_t buf[64]; // Buffer for multiple packets (like MIOS32)
  uint16_t used = 0;
  
  if (USBH_MIDI_Recv(&hUsbHostFS, buf, sizeof(buf), &used) == 0 && used >= 4) {
    // Process all received 4-byte packets
    for (uint16_t i = 0; (i + 3) < used; i += 4) {
      // Extract cable number and message (MIOS32 style)
      uint8_t header = buf[i];
      uint8_t cable = (header >> 4) & 0x0F;
      uint8_t cin = header & 0x0F;
      
      // Skip invalid packets
      if (cin == 0x00) continue;
      
      // Route to router (USB Host uses dedicated nodes)
      router_msg_t msg;
      msg.type = ROUTER_MSG_3B;
      msg.b0 = buf[i + 1];
      msg.b1 = buf[i + 2];
      msg.b2 = buf[i + 3];
      
      // All USB Host messages go to USBH_IN node
      // (In MIOS32, USB Host can also support multiple cables)
      router_process(ROUTER_NODE_USBH_IN, &msg);
    }
  }
}

int usb_host_midi_send3(uint8_t status, uint8_t d1, uint8_t d2)
{
  // Send 3-byte MIDI message to USB Host device
  // Cable 0 by default (can be extended for multi-cable like MIOS32)
  uint8_t cin = (uint8_t)((status & 0xF0u) >> 4);
  
  uint8_t pkt[4];
  pkt[0] = (uint8_t)((0u << 4) | (cin & 0x0Fu)); // cable 0
  pkt[1] = status;
  pkt[2] = d1;
  pkt[3] = d2;
  
  return USBH_MIDI_Send(&hUsbHostFS, pkt, sizeof(pkt));
}

int usb_host_midi_recv3(uint8_t *status, uint8_t *d1, uint8_t *d2)
{
  if (!status || !d1 || !d2)
    return -1;
  
  uint8_t buf[4];
  uint16_t used = 0;
  if (USBH_MIDI_Recv(&hUsbHostFS, buf, sizeof(buf), &used) != 0 || used < 4u) {
    return -1;
  }
  
  *status = buf[1];
  *d1     = buf[2];
  *d2     = buf[3];
  return 0;
}

#else // !USBH_MIDI_PRESENT || !MODULE_ENABLE_USBH_MIDI

// Stub implementations when USB Host MIDI is not present or not enabled

void usb_host_midi_init(void) { }
void usb_host_midi_task(void) { }

int usb_host_midi_send3(uint8_t status, uint8_t d1, uint8_t d2)
{
  (void)status; (void)d1; (void)d2;
  return -1;
}

int usb_host_midi_recv3(uint8_t *status, uint8_t *d1, uint8_t *d2)
{
  (void)status; (void)d1; (void)d2;
  return -1;
}

#endif // USBH_MIDI_PRESENT && MODULE_ENABLE_USBH_MIDI
