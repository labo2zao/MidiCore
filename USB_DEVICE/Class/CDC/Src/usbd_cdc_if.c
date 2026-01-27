/**
 ******************************************************************************
 * @file    usbd_cdc_if.c
 * @brief   USB CDC Interface implementation
 * @author  MidiCore Project
 ******************************************************************************
 * @attention
 *
 * USB CDC Interface Layer - Connects CDC class driver to service layer
 * Maps USB stack callbacks to Services/usb_cdc API
 *
 * This file is protected from CubeMX regeneration - do not modify markers
 *
 ******************************************************************************
 */

#include "../Inc/usbd_cdc_if.h"
#include "Config/module_config.h"

#if MODULE_ENABLE_USB_CDC

/* Private function prototypes */
static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t *buf, uint32_t *len);
static int8_t CDC_TransmitCplt_FS(uint8_t *buf, uint32_t *len, uint8_t epnum);

/* CDC Interface callbacks structure */
USBD_CDC_ItfTypeDef USBD_CDC_fops =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS,
  CDC_TransmitCplt_FS
};

/* External USB Device handle */
extern USBD_HandleTypeDef hUsbDeviceFS;

/* External callback to service layer */
extern void usb_cdc_rx_callback_internal(const uint8_t *buf, uint32_t len);

/* ============================================================================
 * CDC Interface Implementation
 * ============================================================================ */

/**
 * @brief  Initialize CDC interface
 * @retval USBD_OK if all operations are OK, else USBD_FAIL
 */
static int8_t CDC_Init_FS(void)
{
  /* No hardware initialization needed - USB handles everything */
  return USBD_OK;
}

/**
 * @brief  De-Initialize CDC interface
 * @retval USBD_OK if all operations are OK, else USBD_FAIL
 */
static int8_t CDC_DeInit_FS(void)
{
  /* No hardware de-initialization needed */
  return USBD_OK;
}

/**
 * @brief  Manage CDC control requests
 * @param  cmd: Command code
 * @param  pbuf: Buffer containing command data (request parameters)
 * @param  length: Number of data to be sent (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK, else USBD_FAIL
 */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length)
{
  UNUSED(pbuf);
  UNUSED(length);
  
  switch (cmd) {
    case CDC_SEND_ENCAPSULATED_COMMAND:
      /* Not used - placeholder for future */
      break;
      
    case CDC_GET_ENCAPSULATED_RESPONSE:
      /* Not used - placeholder for future */
      break;
      
    case CDC_SET_COMM_FEATURE:
      /* Not used - placeholder for future */
      break;
      
    case CDC_GET_COMM_FEATURE:
      /* Not used - placeholder for future */
      break;
      
    case CDC_CLEAR_COMM_FEATURE:
      /* Not used - placeholder for future */
      break;
      
    case CDC_SET_LINE_CODING:
      /* Line coding parameters received - accept any (virtual port) */
      break;
      
    case CDC_GET_LINE_CODING:
      /* Line coding parameters requested - handled by class driver */
      break;
      
    case CDC_SET_CONTROL_LINE_STATE:
      /* Control line state changed (DTR/RTS) - handled by class driver */
      break;
      
    case CDC_SEND_BREAK:
      /* Send break signal - not applicable for virtual port */
      break;
      
    default:
      break;
  }
  
  return USBD_OK;
}

/**
 * @brief  Data received over USB OUT endpoint
 * @param  buf: Buffer of data to be received
 * @param  len: Number of data received (in bytes)
 * @retval Result of the operation: USBD_OK if all operations are OK, else USBD_FAIL
 */
static int8_t CDC_Receive_FS(uint8_t *buf, uint32_t *len)
{
  /* Forward received data to service layer callback */
  usb_cdc_rx_callback_internal(buf, *len);
  
  /* Prepare for next reception */
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  
  return USBD_OK;
}

/**
 * @brief  Data transmit complete callback
 * @param  buf: Buffer of data to be sent
 * @param  len: Number of data to be sent (in bytes)
 * @param  epnum: Endpoint number
 * @retval Result of the operation: USBD_OK if all operations are OK, else USBD_FAIL
 */
static int8_t CDC_TransmitCplt_FS(uint8_t *buf, uint32_t *len, uint8_t epnum)
{
  UNUSED(buf);
  UNUSED(len);
  UNUSED(epnum);
  
  /* Transmission complete - no action needed in this implementation */
  /* Application can track completion if needed via callback extension */
  
  return USBD_OK;
}

#else /* !MODULE_ENABLE_USB_CDC */

/* Stub implementations when CDC is disabled - safe no-op functions */
static int8_t CDC_Init_Stub(void) { return USBD_OK; }
static int8_t CDC_DeInit_Stub(void) { return USBD_OK; }
static int8_t CDC_Control_Stub(uint8_t cmd, uint8_t *pbuf, uint16_t length) { 
  (void)cmd; (void)pbuf; (void)length; return USBD_OK; 
}
static int8_t CDC_Receive_Stub(uint8_t *buf, uint32_t *len) { 
  (void)buf; (void)len; return USBD_OK; 
}
static int8_t CDC_TransmitCplt_Stub(uint8_t *buf, uint32_t *len, uint8_t epnum) { 
  (void)buf; (void)len; (void)epnum; return USBD_OK; 
}

USBD_CDC_ItfTypeDef USBD_CDC_fops = {
  CDC_Init_Stub,
  CDC_DeInit_Stub,
  CDC_Control_Stub,
  CDC_Receive_Stub,
  CDC_TransmitCplt_Stub
};

#endif /* MODULE_ENABLE_USB_CDC */
