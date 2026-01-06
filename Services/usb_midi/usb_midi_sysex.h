#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Send a full SysEx message over USB MIDI (F0 ... F7). Safe to call even if USB MIDI disabled (no-op). */
void usb_midi_send_sysex(const uint8_t* data, size_t len);

#ifdef __cplusplus
}
#endif
