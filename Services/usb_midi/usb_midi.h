#pragma once
#include <stdint.h>

/**
 * @file usb_midi.h
 * @brief USB MIDI Device transport layer (4-port support)
 * 
 * Provides USB Device MIDI functionality for MidiCore with 4 virtual ports
 * (cables 0-3) like MIOS32. Routes messages to/from router nodes.
 * 
 * Integration:
 *  - Enable MODULE_ENABLE_USB_MIDI in Config/module_config.h
 *  - Configure CubeMX with USB_OTG_FS in OTG or Device mode
 *  - Our custom MIDI Device class handles 4 ports automatically
 *  - Each cable (0-3) maps to a router node (ROUTER_NODE_USB_PORT0-3)
 * 
 * MIOS32 Compatibility:
 *  - Similar to MIOS32_USB_MIDI layer
 *  - Cable numbers in packets like MIOS32 (upper 4 bits)
 *  - Multi-port routing like MIOS32 USB0-USB3
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize USB Device MIDI (4 ports)
 * Call once during startup after USB Device is initialized
 */
void usb_midi_init(void);

/**
 * @brief Send a USB MIDI packet
 * @param cin Cable Index Number (bits 7-4: cable 0-3, bits 3-0: code index)
 * @param b0 First MIDI byte (usually status)
 * @param b1 Second MIDI byte
 * @param b2 Third MIDI byte
 * 
 * Example: Send Note On (cable 0, channel 1)
 *   usb_midi_send_packet(0x09, 0x90, 0x3C, 0x7F);
 * 
 * Example: Send CC (cable 2, channel 3)
 *   usb_midi_send_packet(0x2B, 0xB2, 0x07, 0x64);
 */
void usb_midi_send_packet(uint8_t cin, uint8_t b0, uint8_t b1, uint8_t b2);

/**
 * @brief Process received USB MIDI packet (internal callback)
 * @param packet4 4-byte USB MIDI packet [header, b0, b1, b2]
 * 
 * Routes packet to appropriate router node based on cable number:
 *  - Cable 0 → ROUTER_NODE_USB_PORT0
 *  - Cable 1 → ROUTER_NODE_USB_PORT1
 *  - Cable 2 → ROUTER_NODE_USB_PORT2
 *  - Cable 3 → ROUTER_NODE_USB_PORT3
 */
void usb_midi_rx_packet(const uint8_t packet4[4]);

#ifdef __cplusplus
}
#endif
