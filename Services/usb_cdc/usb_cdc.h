/**
 ******************************************************************************
 * @file    usb_cdc.h
 * @brief   USB CDC (Virtual COM Port / ACM) Service API
 * @author  MidiCore Project
 ******************************************************************************
 * @attention
 *
 * USB CDC ACM (Abstract Control Model) Virtual COM Port service
 * Compatible with MIOS32 USB_CDC API and MIOS Studio
 * 
 * Features:
 * - Virtual COM port for terminal/debug communication
 * - Composite device support (CDC + MIDI concurrent)
 * - Receive callback mechanism for incoming data
 * - Connection state detection
 * - MIOS32-compatible shim functions
 * 
 * Integration:
 * - Enable MODULE_ENABLE_USB_CDC in Config/module_config.h
 * - Configure CubeMX with USB_OTG_FS
 * - Call usb_cdc_init() during startup
 * - Register receive callback with usb_cdc_register_receive_callback()
 * 
 * MIOS32 Compatibility:
 * - MIOS32_USB_CDC_Init() maps to usb_cdc_init()
 * - MIOS32_USB_CDC_SendBlock() maps to usb_cdc_send()
 * - Similar callback registration pattern
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
 * MIOS32 Compatibility Shims
 * ============================================================================ */

/**
 * @brief MIOS32-compatible initialization
 * 
 * Maps to usb_cdc_init() for MIOS32 code compatibility.
 * MIOS32 applications can use this name directly.
 * 
 * @return 0 on success (MIOS32 convention)
 */
static inline int32_t MIOS32_USB_CDC_Init(void) {
  usb_cdc_init();
  return USB_CDC_OK;
}

/**
 * @brief MIOS32-compatible block send
 * @param buf Pointer to data buffer
 * @param len Number of bytes to send
 * @return 0 on success, negative on error (MIOS32 convention)
 * 
 * Maps to usb_cdc_send() for MIOS32 code compatibility.
 * Note: Return convention differs (MIOS32 returns 0 on success,
 * while usb_cdc_send returns bytes sent). This shim adapts the return value.
 */
static inline int32_t MIOS32_USB_CDC_SendBlock(const uint8_t *buf, uint32_t len) {
  int32_t result = usb_cdc_send(buf, len);
  return (result >= 0) ? USB_CDC_OK : result;
}

/**
 * @brief MIOS32-compatible receive callback registration
 * @param callback Function to call when data is received
 * @return 0 on success (MIOS32 convention)
 * 
 * Maps to usb_cdc_register_receive_callback() for MIOS32 compatibility.
 */
static inline int32_t MIOS32_USB_CDC_RegisterRxCallback(usb_cdc_rx_callback_t callback) {
  usb_cdc_register_receive_callback(callback);
  return USB_CDC_OK;
}

/**
 * @brief MIOS32-compatible connection check
 * @return 1 if connected, 0 otherwise
 * 
 * Maps to usb_cdc_is_connected() for MIOS32 compatibility.
 */
static inline uint8_t MIOS32_USB_CDC_IsConnected(void) {
  return usb_cdc_is_connected();
}

#ifdef __cplusplus
}
#endif

#endif /* __USB_CDC_H */
