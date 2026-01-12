
#include "Services/usb_host_midi/usb_host_midi.h"
#include "Services/usb_host_midi/usbh_midi.h"

#include "usbh_core.h"

#if defined(USBH_MIDI_PRESENT) && USBH_MIDI_PRESENT

// USB Host handle is defined in CubeMX-generated USB_HOST/App/usb_host.c
extern USBH_HandleTypeDef hUsbHostFS;

void usb_host_midi_init(void)
{
  // For now, nothing specific to do here.
  // MX_USB_HOST_Init() is called from main.c before the default task starts.
}

void usb_host_midi_task(void)
{
  // Pump the USB Host state machine.
  USBH_Process(&hUsbHostFS);
}

int usb_host_midi_send3(uint8_t status, uint8_t d1, uint8_t d2)
{
  // We only support simple 3-byte Channel Voice messages here (Note On/Off, CC, etc.).
  uint8_t cin = (uint8_t)((status & 0xF0u) >> 4); // matches CIN for most 3-byte channel messages

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

#else // !USBH_MIDI_PRESENT

// Stub implementations when USB Host MIDI is not present in the project.

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

#endif // USBH_MIDI_PRESENT
