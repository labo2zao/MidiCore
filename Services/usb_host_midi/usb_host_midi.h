#pragma once
#include <stdint.h>
#include "usbh_core.h"
#include <stddef.h>

extern USBH_ClassTypeDef USBH_MIDI_Class;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file usb_host_midi.h
 * @brief USB Host MIDI transport layer
 * 
 * Provides USB Host MIDI functionality for MidiCore to read external
 * USB MIDI devices (keyboards, controllers, etc.) via OTG adapter.
 * 
 * Like MIOS32, this operates in USB Host mode when an OTG adapter is
 * connected to the micro-USB port. Automatically processes incoming
 * MIDI packets and routes them to ROUTER_NODE_USBH_IN.
 * 
 * Integration:
 *  - Enable MODULE_ENABLE_USBH_MIDI in Config/module_config.h
 *  - Configure CubeMX with USB_OTG_FS in OTG or Host mode
 *  - Connect OTG adapter + USB MIDI device
 *  - Power via USB Debug socket (important for Host mode!)
 * 
 * MidiCore Compatibility:
 *  - Similar to MIOS32_USB_MIDI Host mode
 *  - Automatic packet reception and routing
 *  - Cable number support (can be extended)
 */

/**
 * @brief Initialize USB Host MIDI
 * Call once during startup after USB Host is initialized
 */
void usb_host_midi_init(void);

/**
 * @brief USB Host MIDI task (call periodically)
 * 
 * This pumps the USB Host state machine and processes incoming
 * MIDI packets from connected USB MIDI devices. Should be called
 * continuously in the main loop or a dedicated task.
 * 
 * Like MIOS32, this automatically routes received packets to the
 * router system (ROUTER_NODE_USBH_IN).
 */
void usb_host_midi_task(void);

/**
 * @brief Send a 3-byte MIDI message to USB Host device
 * @param status MIDI status byte
 * @param d1 First data byte
 * @param d2 Second data byte
 * @return 0 on success, -1 on failure
 * 
 * Example: Send Note On to connected USB keyboard
 *   usb_host_midi_send3(0x90, 0x3C, 0x7F);
 */
int usb_host_midi_send3(uint8_t status, uint8_t d1, uint8_t d2);

/**
 * @brief Receive one 3-byte MIDI message from USB Host device
 * @param status Pointer to store status byte
 * @param d1 Pointer to store first data byte
 * @param d2 Pointer to store second data byte
 * @return 0 if message received, -1 if no message available
 * 
 * Note: usb_host_midi_task() automatically routes messages to router,
 * so direct polling is usually not necessary.
 */
int usb_host_midi_recv3(uint8_t *status, uint8_t *d1, uint8_t *d2);

#ifdef __cplusplus
}
#endif
