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
 * MIOS32 Compatibility API
 * ============================================================================ */

/**
 * @brief MIOS32-compatible initialization
 * 
 * Initializes USB CDC interface for MIOS Studio and terminal compatibility.
 * Must be called after USB Device initialization.
 * 
 * @param mode Mode parameter (currently ignored, for MIOS32 API compat)
 * @return 0 on success (MIOS32 convention)
 * 
 * MIOS32 API: MIOS32_USB_COM_Init(u32 mode)
 */
static inline int32_t MIOS32_USB_COM_Init(uint32_t mode) {
  (void)mode;  // Mode parameter not used
  usb_cdc_init();
  return USB_CDC_OK;
}

/**
 * @brief MIOS32-compatible connection check
 * @return 1 if connected, 0 otherwise
 * 
 * MIOS32 API: MIOS32_USB_COM_CheckAvailable(void)
 */
static inline int32_t MIOS32_USB_COM_CheckAvailable(void) {
  return usb_cdc_is_connected() ? 1 : 0;
}

/**
 * @brief MIOS32-compatible single byte transmit (non-blocking)
 * @param usb_com USB COM interface number (0 for MidiCore, only one interface)
 * @param b Byte to transmit
 * @return 0 on success, negative on error
 * 
 * MIOS32 API: MIOS32_USB_COM_TxBufferPut_NonBlocking(u8 usb_com, u8 b)
 */
static inline int32_t MIOS32_USB_COM_TxBufferPut_NonBlocking(uint8_t usb_com, uint8_t b) {
  (void)usb_com;  // Only one interface in MidiCore
  int32_t result = usb_cdc_send(&b, 1);
  return (result == 1) ? 0 : ((result == USB_CDC_BUSY) ? -2 : -1);
}

/**
 * @brief MIOS32-compatible single byte transmit (blocking)
 * @param usb_com USB COM interface number (0 for MidiCore)
 * @param b Byte to transmit
 * @return 0 on success, negative on error
 * 
 * MIOS32 API: MIOS32_USB_COM_TxBufferPut(u8 usb_com, u8 b)
 * 
 * Note: MidiCore implementation is non-blocking, so this is same as non-blocking version
 */
static inline int32_t MIOS32_USB_COM_TxBufferPut(uint8_t usb_com, uint8_t b) {
  return MIOS32_USB_COM_TxBufferPut_NonBlocking(usb_com, b);
}

/**
 * @brief MIOS32-compatible block transmit (non-blocking)
 * @param usb_com USB COM interface number (0 for MidiCore)
 * @param buffer Pointer to data buffer
 * @param len Number of bytes to send
 * @return 0 on success, negative on error
 * 
 * MIOS32 API: MIOS32_USB_COM_TxBufferPutMore_NonBlocking(u8 usb_com, u8 *buffer, u16 len)
 */
static inline int32_t MIOS32_USB_COM_TxBufferPutMore_NonBlocking(uint8_t usb_com, uint8_t *buffer, uint16_t len) {
  (void)usb_com;
  int32_t result = usb_cdc_send(buffer, len);
  return (result == (int32_t)len) ? 0 : ((result == USB_CDC_BUSY) ? -2 : -1);
}

/**
 * @brief MIOS32-compatible block transmit (blocking)
 * @param usb_com USB COM interface number (0 for MidiCore)
 * @param buffer Pointer to data buffer
 * @param len Number of bytes to send
 * @return 0 on success, negative on error
 * 
 * MIOS32 API: MIOS32_USB_COM_TxBufferPutMore(u8 usb_com, u8 *buffer, u16 len)
 * 
 * Note: MidiCore implementation is non-blocking, so this is same as non-blocking version
 */
static inline int32_t MIOS32_USB_COM_TxBufferPutMore(uint8_t usb_com, uint8_t *buffer, uint16_t len) {
  return MIOS32_USB_COM_TxBufferPutMore_NonBlocking(usb_com, buffer, len);
}

/* Legacy MIOS32_USB_CDC_* names for backward compatibility */
#define MIOS32_USB_CDC_Init(void) MIOS32_USB_COM_Init(0)
#define MIOS32_USB_CDC_CheckAvailable() MIOS32_USB_COM_CheckAvailable()
#define MIOS32_USB_CDC_SendBlock(buf, len) MIOS32_USB_COM_TxBufferPutMore(0, (uint8_t*)(buf), (uint16_t)(len))
#define MIOS32_USB_CDC_IsConnected() usb_cdc_is_connected()
#define MIOS32_USB_CDC_RegisterRxCallback(cb) usb_cdc_register_receive_callback(cb)

#ifdef __cplusplus
}
#endif

#endif /* __USB_CDC_H */
