#pragma once
#include <stdint.h>

/**
 * USB MIDI transport is optional. Your current CubeMX project uses Custom HID.
 * To enable USB MIDI later:
 *  - Switch CubeMX USB Device class to MIDI
 *  - Define ENABLE_USBD_MIDI in project build flags
 *  - Provide usbd_midi_if.* (CubeMX generated)
 */
void usb_midi_init(void);
void usb_midi_send_packet(uint8_t cin, uint8_t b0, uint8_t b1, uint8_t b2);
void usb_midi_rx_packet(const uint8_t packet4[4]);
