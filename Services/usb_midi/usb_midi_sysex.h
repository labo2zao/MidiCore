#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Send a full SysEx message over USB MIDI (F0 ... F7)
 * @param data Pointer to SysEx data (must start with 0xF0 and end with 0xF7)
 * @param len Length of SysEx data in bytes
 * @param cable Cable number (0-3)
 * 
 * Automatically packetizes the SysEx message into USB MIDI packets with correct CINs:
 * - CIN 0x04: SysEx start/continue (3 bytes, no F7)
 * - CIN 0x05: SysEx ends with 1 byte (contains F7)
 * - CIN 0x06: SysEx ends with 2 bytes (last is F7)
 * - CIN 0x07: SysEx ends with 3 bytes (last is F7)
 * 
 * Safe to call even if USB MIDI disabled (no-op).
 * 
 * @return true if all packets queued successfully, false if any packet dropped (TX queue full)
 */
bool usb_midi_send_sysex(const uint8_t* data, size_t len, uint8_t cable);

#ifdef __cplusplus
}
#endif
