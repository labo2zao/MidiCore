#pragma once
#include <stdint.h>
#include "usbh_core.h"
#include <stddef.h>

extern USBH_ClassTypeDef USBH_MIDI_Class;

#ifdef __cplusplus
extern "C" {
#endif

// Wrapper around STM32 USB Host middleware + our MIDI class.
// If USB Host middleware is not generated in your CubeMX project, this module
// becomes a no-op stub (firmware still compiles).

void usb_host_midi_init(void);
void usb_host_midi_task(void);

// Send a 3-byte MIDI message (status,data1,data2). Returns 0 on success.
int usb_host_midi_send3(uint8_t status, uint8_t d1, uint8_t d2);

// Receive one 3-byte MIDI message. Returns 0 if a message was written.
int usb_host_midi_recv3(uint8_t *status, uint8_t *d1, uint8_t *d2);

#ifdef __cplusplus
}
#endif
