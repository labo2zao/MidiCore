#include "Services/usb_midi/usb_midi_sysex.h"
#include "Services/usb_midi/usb_midi.h"
#include "Config/module_config.h"

#if MODULE_ENABLE_USB_MIDI

// USB-MIDI SysEx CIN values per USB MIDI 1.0 spec:
// 0x4: SysEx start or continue (3 bytes, no F7)
// 0x5: SysEx ends with 1 byte (single byte OR ends with 1 byte containing F7)
// 0x6: SysEx ends with 2 bytes (last byte is F7)
// 0x7: SysEx ends with 3 bytes (last byte is F7)
bool usb_midi_send_sysex(const uint8_t* data, size_t len, uint8_t cable) {
  if (!data || len == 0) return false;
  
  // Validate cable number (0-3)
  if (cable > 3) cable = 0;
  
  // Validate: must start with F0
  if (data[0] != 0xF0) return false;
  
  // Validate: must end with F7
  if (data[len-1] != 0xF7) return false;
  
  // Build cable-specific CIN (cable in upper 4 bits)
  uint8_t cable_nibble = (cable << 4);
  
  bool all_sent = true;  // Track if all packets sent successfully

  size_t i = 0;
  while (i < len) {
    size_t rem = len - i;

    // Check if we're in the last packet (contains F7)
    // Search for F7 in the next 1-3 bytes
    uint8_t f7_pos = 0;
    uint8_t found_f7 = 0;
    for (uint8_t j = 0; j < 3 && (i + j) < len; j++) {
      if (data[i + j] == 0xF7) {
        f7_pos = j + 1; // Position 1-3
        found_f7 = 1;
        break;
      }
    }

    if (found_f7) {
      // This is the final packet - use appropriate end CIN
      bool sent = false;
      if (f7_pos == 1) {
        // F7 is first byte: SysEx ends with 1 byte
        sent = usb_midi_send_packet(cable_nibble | 0x05, data[i], 0, 0);
      } else if (f7_pos == 2) {
        // F7 is second byte: SysEx ends with 2 bytes
        sent = usb_midi_send_packet(cable_nibble | 0x06, data[i], data[i+1], 0);
      } else { // f7_pos == 3
        // F7 is third byte: SysEx ends with 3 bytes
        sent = usb_midi_send_packet(cable_nibble | 0x07, data[i], data[i+1], data[i+2]);
      }
      if (!sent) all_sent = false;
      return all_sent; // Done - return overall success status
    }

    // No F7 in next 3 bytes: send as continue packet
    if (rem >= 3) {
      bool sent = usb_midi_send_packet(cable_nibble | 0x04, data[i], data[i+1], data[i+2]);
      if (!sent) all_sent = false;
      i += 3;
    } else {
      // This shouldn't happen if SysEx is properly formed (ends with F7)
      // But handle gracefully: send remaining bytes as end packet
      bool sent = false;
      if (rem == 1) {
        sent = usb_midi_send_packet(cable_nibble | 0x05, data[i], 0, 0);
      } else { // rem == 2
        sent = usb_midi_send_packet(cable_nibble | 0x06, data[i], data[i+1], 0);
      }
      if (!sent) all_sent = false;
      return all_sent;
    }
  }
  
  return all_sent;
}

#else
bool usb_midi_send_sysex(const uint8_t* data, size_t len, uint8_t cable) { 
  (void)data; (void)len; (void)cable; 
  return false;  /* USB MIDI disabled - cannot send */
}
#endif
