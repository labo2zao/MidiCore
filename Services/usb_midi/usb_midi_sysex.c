#include "Services/usb_midi/usb_midi_sysex.h"
#include "Services/usb_midi/usb_midi.h"

#ifdef ENABLE_USBD_MIDI

// USB-MIDI SysEx CIN values:
// 0x4: SysEx start/continue with 3 bytes
// 0x5: SysEx end with 1 byte
// 0x6: SysEx end with 2 bytes
// 0x7: SysEx end with 3 bytes
void usb_midi_send_sysex(const uint8_t* data, size_t len) {
  if (!data || len == 0) return;

  size_t i = 0;
  while (i < len) {
    size_t rem = len - i;

    if (rem >= 3) {
      // If exactly 3 bytes remain and last is F7 => end with 3 bytes
      if (rem == 3 && data[i+2] == 0xF7) {
        usb_midi_send_packet(0x07, data[i], data[i+1], data[i+2]);
        return;
      }
      usb_midi_send_packet(0x04, data[i], data[i+1], data[i+2]);
      i += 3;
      continue;
    }

    // rem is 1 or 2: must be end packets
    if (rem == 1) {
      usb_midi_send_packet(0x05, data[i], 0, 0);
      return;
    } else {
      usb_midi_send_packet(0x06, data[i], data[i+1], 0);
      return;
    }
  }
}

#else
void usb_midi_send_sysex(const uint8_t* data, size_t len) { (void)data; (void)len; }
#endif
