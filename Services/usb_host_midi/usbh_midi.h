#pragma once

#if defined(__has_include)
#  if __has_include("usbh_core.h")
#    include "usbh_core.h"
#    define USBH_MIDI_PRESENT 1
#  else
#    define USBH_MIDI_PRESENT 0
#  endif
#else
#  define USBH_MIDI_PRESENT 0
#endif

#if USBH_MIDI_PRESENT

#ifdef __cplusplus
extern "C" {
#endif

// Minimal USB MIDI Host class (Audio/MIDIStreaming, bulk IN/OUT).
extern USBH_ClassTypeDef USBH_MIDI_Class;

// Send raw USB-MIDI event packets (4-byte packets). Returns 0 on success.
int USBH_MIDI_Send(USBH_HandleTypeDef *phost, const uint8_t *data, uint16_t len);

// Receive one or more USB-MIDI event packets into out[]. out_used returns bytes written.
int USBH_MIDI_Recv(USBH_HandleTypeDef *phost, uint8_t *out, uint16_t out_len, uint16_t *out_used);

#ifdef __cplusplus
}
#endif

#endif // USBH_MIDI_PRESENT
