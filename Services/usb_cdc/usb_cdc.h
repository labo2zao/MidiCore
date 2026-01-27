/**
 ******************************************************************************
 * @file    usb_cdc.h
 * @brief   USB CDC (Virtual COM Port / ACM) Service API
 * @author  MidiCore Project
 ******************************************************************************
 * @attention
 *
 * USB CDC ACM (Abstract Control Model) Virtual COM Port service
 * 
 * Original clean-room implementation for MidiCore.
 * Compatible with MIOS Studio and standard terminal applications.
 * Licensed for commercial use.
 * 
 * Features:
 * - Virtual COM port for terminal/debug communication
 * - Composite device support (CDC + MIDI concurrent)
 * - Receive callback mechanism for incoming data
 * - Connection state detection
 * - MIOS Studio compatible terminal functions
 * 
 * Integration:
 * - Enable MODULE_ENABLE_USB_CDC in Config/module_config.h
 * - Configure CubeMX with USB_OTG_FS
 * - Call usb_cdc_init() during startup
 * - Register receive callback with usb_cdc_register_receive_callback()
 * 
 ******************************************************************************
 */

#ifndef __USB_CDC_H
#define __USB_CDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Return codes for USB CDC operations */
#define USB_CDC_OK              0
#define USB_CDC_ERROR           -1
#define USB_CDC_BUSY            -2
#define USB_CDC_NOT_READY       -3

/**
 * @brief Receive callback function type
 * @param buf Pointer to received data buffer
 * @param len Number of bytes received
 * 
 * Called when data is received from USB CDC interface
 * Implementation should be quick and non-blocking
 */
typedef void (*usb_cdc_rx_callback_t)(const uint8_t *buf, uint32_t len);

/* ============================================================================
 * Service API Functions
 * ============================================================================ */

/**
 * @brief Initialize USB CDC service
 * 
 * Call once during startup after USB Device is initialized.
 * Registers CDC interface callbacks with USB stack.
 * 
 * @note Must be called after MX_USB_DEVICE_Init()
 */
void usb_cdc_init(void);

/**
 * @brief Send data via USB CDC
 * @param buf Pointer to data buffer to send
 * @param len Number of bytes to send
 * @return Number of bytes sent, or negative error code
 * 
 * Return values:
 * - >= 0: Number of bytes successfully sent
 * - USB_CDC_ERROR: General error
 * - USB_CDC_BUSY: Previous transmission still in progress
 * - USB_CDC_NOT_READY: USB not connected or CDC not ready
 * 
 * @note Non-blocking. Returns immediately even if transmission is pending.
 * @note Maximum length per call depends on USB packet size (typically 64 bytes)
 */
int32_t usb_cdc_send(const uint8_t *buf, uint32_t len);

/**
 * @brief Register callback for received data
 * @param callback Function to call when data is received
 * 
 * Register a callback that will be called from USB interrupt context
 * when data is received. Only one callback can be registered.
 * 
 * @note Callback is called from interrupt context - keep it fast
 * @note Pass NULL to unregister callback
 */
void usb_cdc_register_receive_callback(usb_cdc_rx_callback_t callback);

/**
 * @brief Check if USB CDC is connected and ready
 * @return 1 if connected and ready, 0 otherwise
 * 
 * Returns true when:
 * - USB device is enumerated
 * - CDC interface is configured
 * - Host has opened the COM port
 */
uint8_t usb_cdc_is_connected(void);

/* ============================================================================
 * Terminal Compatibility API (MIOS-Studio Compatible)
 * ============================================================================
 * 
 * This API provides terminal/debugging functionality compatible with MIOS Studio
 * and other USB CDC terminal applications, without using any MIOS32 code.
 * 
 * Designed for commercial use with clean-room implementation.
 */

/**
 * @brief Initialize USB CDC for terminal use
 * @return 0 on success, negative on error
 * 
 * Terminal-compatible initialization function.
 * Call after USB Device initialization.
 */
static inline int32_t USB_CDC_TerminalInit(void) {
  usb_cdc_init();
  return USB_CDC_OK;
}

/**
 * @brief Check if terminal is connected
 * @return 1 if connected, 0 otherwise
 * 
 * Use this to check if a terminal application (MIOS Studio, PuTTY, etc.)
 * is connected before sending data.
 */
static inline int32_t USB_CDC_TerminalAvailable(void) {
  return usb_cdc_is_connected() ? 1 : 0;
}

/**
 * @brief Send single byte to terminal (non-blocking)
 * @param byte Byte to send
 * @return 0 on success, -1 on error, -2 if busy
 * 
 * Non-blocking byte transmission for terminal output.
 */
static inline int32_t USB_CDC_TerminalPutChar(uint8_t byte) {
  int32_t result = usb_cdc_send(&byte, 1);
  if (result == 1) return 0;
  if (result == USB_CDC_BUSY) return -2;
  return -1;
}

/**
 * @brief Send string to terminal (non-blocking)
 * @param str Null-terminated string
 * @return 0 on success, negative on error
 * 
 * Convenience function for terminal string output.
 */
static inline int32_t USB_CDC_TerminalPutString(const char *str) {
  if (!str) return -1;
  uint32_t len = 0;
  while (str[len]) len++;
  int32_t result = usb_cdc_send((const uint8_t*)str, len);
  return (result == (int32_t)len) ? 0 : -1;
}

/**
 * @brief Send data buffer to terminal (non-blocking)
 * @param buffer Pointer to data buffer
 * @param length Number of bytes to send
 * @return Number of bytes sent on success, negative on error
 * 
 * Non-blocking buffer transmission for terminal output.
 */
static inline int32_t USB_CDC_TerminalWrite(const uint8_t *buffer, uint32_t length) {
  return usb_cdc_send(buffer, length);
}

/**
 * @brief Register callback for received terminal data
 * @param callback Function to call when data received
 * @return 0 on success
 * 
 * Register a callback to handle data received from the terminal.
 * Callback is called from USB interrupt context.
 */
static inline int32_t USB_CDC_TerminalRegisterRxCallback(usb_cdc_rx_callback_t callback) {
  usb_cdc_register_receive_callback(callback);
  return USB_CDC_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* __USB_CDC_H */
