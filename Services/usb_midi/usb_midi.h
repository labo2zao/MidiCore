#pragma once
#include <stdint.h>
#include <stdbool.h>

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
 * MidiCore Compatibility:
 *  - Similar to MIOS32_USB_MIDI layer
 *  - Cable numbers in packets like MidiCore (upper 4 bits)
 *  - Multi-port routing like MidiCore USB0-USB3
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
 * 
 * @return true if packet was queued successfully, false if TX queue full (packet dropped)
 */
bool usb_midi_send_packet(uint8_t cin, uint8_t b0, uint8_t b1, uint8_t b2);

/**
 * @brief Process received USB MIDI packet (internal callback - called from interrupt)
 * @param packet4 4-byte USB MIDI packet [header, b0, b1, b2]
 * 
 * CRITICAL: This is called from USB interrupt context. It ONLY queues the packet
 * for deferred processing. Actual processing happens in usb_midi_process_rx_queue().
 * 
 * DO NOT call this directly - it's automatically called by USB MIDI class.
 */
void usb_midi_rx_packet(const uint8_t packet4[4]);

/**
 * @brief Process queued RX packets - MUST be called from task context!
 * 
 * Call this regularly from main loop or dedicated USB MIDI task. It processes
 * all queued RX packets, handles SysEx assembly, MidiCore queries, and routing.
 * 
 * CRITICAL: Do NOT call from interrupt context! This function does heavy
 * processing including router operations and TX responses.
 * 
 * Example usage in main loop:
 *   while(1) {
 *     usb_midi_process_rx_queue();  // Process received MIDI
 *     // ... other tasks ...
 *   }
 */
void usb_midi_process_rx_queue(void);

/**
 * @brief Get USB MIDI TX queue status (for diagnostics)
 * @param queue_size Output: Total queue capacity in packets
 * @param queue_used Output: Current packets in queue
 * @param queue_drops Output: Total packets dropped due to full queue
 * @return true if USB MIDI is ready for transmission, false otherwise
 */
bool usb_midi_get_tx_status(uint32_t *queue_size, uint32_t *queue_used, uint32_t *queue_drops);

#ifdef __cplusplus
}
#endif
