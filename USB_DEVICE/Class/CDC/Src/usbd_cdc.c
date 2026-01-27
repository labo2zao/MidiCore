/**
 ******************************************************************************
 * @file    usbd_cdc.c
 * @author  MidiCore Project
 * @brief   USB CDC Device Class implementation - ACM (Abstract Control Model)
 ******************************************************************************
 * @attention
 *
 * Based on USB Device Class Definition for Communications Devices v1.2
 * Implements Virtual COM Port functionality
 *
 * This file is protected from CubeMX regeneration - do not modify markers
 *
 ******************************************************************************
 */

#include "../Inc/usbd_cdc.h"
#include "usbd_ctlreq.h"
#include <string.h>

/* Private function prototypes */
static uint8_t USBD_CDC_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_CDC_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_CDC_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_CDC_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_CDC_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_CDC_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t *USBD_CDC_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_CDC_GetHSCfgDesc(uint16_t *length);
static uint8_t *USBD_CDC_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t *USBD_CDC_GetDeviceQualifierDesc(uint16_t *length);

/* USB CDC Class Callbacks */
USBD_ClassTypeDef USBD_CDC = 
{
  USBD_CDC_Init,
  USBD_CDC_DeInit,
  USBD_CDC_Setup,
  NULL,  /* EP0_TxSent */
  USBD_CDC_EP0_RxReady,
  USBD_CDC_DataIn,
  USBD_CDC_DataOut,
  NULL,  /* SOF */
  NULL,  /* IsoINIncomplete */
  NULL,  /* IsoOUTIncomplete */
  USBD_CDC_GetFSCfgDesc,
  USBD_CDC_GetHSCfgDesc,
  USBD_CDC_GetOtherSpeedCfgDesc,
  USBD_CDC_GetDeviceQualifierDesc,
};

/* USB CDC Device Configuration Descriptor */
/* For composite device with MIDI, descriptors are built in usbd_desc.c */
/* This is a standalone CDC descriptor for reference */
__ALIGN_BEGIN static uint8_t USBD_CDC_CfgDesc[67] __ALIGN_END =
{
  /* Configuration Descriptor */
  0x09,                                    /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,             /* bDescriptorType: Configuration */
  67,                                      /* wTotalLength */
  0x00,
  0x02,                                    /* bNumInterfaces: 2 interfaces (Control + Data) */
  0x01,                                    /* bConfigurationValue: Configuration value */
  0x00,                                    /* iConfiguration: Index of string descriptor */
  0x80,                                    /* bmAttributes: Bus Powered */
  0xFA,                                    /* MaxPower 500 mA */
  
  /*---------------------------------------------------------------------------*/
  /* Interface Descriptor 0: CDC Communication Interface */
  0x09,                                    /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,                 /* bDescriptorType: Interface */
  0x00,                                    /* bInterfaceNumber: Number of Interface */
  0x00,                                    /* bAlternateSetting: Alternate setting */
  0x01,                                    /* bNumEndpoints: One endpoint used (Interrupt IN) */
  0x02,                                    /* bInterfaceClass: Communication Interface Class */
  0x02,                                    /* bInterfaceSubClass: Abstract Control Model */
  0x01,                                    /* bInterfaceProtocol: Common AT commands */
  0x00,                                    /* iInterface */
  
  /* Header Functional Descriptor */
  0x05,                                    /* bLength: Endpoint Descriptor size */
  0x24,                                    /* bDescriptorType: CS_INTERFACE */
  0x00,                                    /* bDescriptorSubtype: Header Func Desc */
  0x10,                                    /* bcdCDC: spec release number */
  0x01,
  
  /* Call Management Functional Descriptor */
  0x05,                                    /* bFunctionLength */
  0x24,                                    /* bDescriptorType: CS_INTERFACE */
  0x01,                                    /* bDescriptorSubtype: Call Management Func Desc */
  0x00,                                    /* bmCapabilities: D0+D1 */
  0x01,                                    /* bDataInterface: 1 */
  
  /* ACM Functional Descriptor */
  0x04,                                    /* bFunctionLength */
  0x24,                                    /* bDescriptorType: CS_INTERFACE */
  0x02,                                    /* bDescriptorSubtype: Abstract Control Management desc */
  0x02,                                    /* bmCapabilities: D1 (SET_LINE_CODING, SET_CONTROL_LINE_STATE, GET_LINE_CODING, SERIAL_STATE) */
  
  /* Union Functional Descriptor */
  0x05,                                    /* bFunctionLength */
  0x24,                                    /* bDescriptorType: CS_INTERFACE */
  0x06,                                    /* bDescriptorSubtype: Union func desc */
  0x00,                                    /* bMasterInterface: Communication class interface */
  0x01,                                    /* bSlaveInterface0: Data Class Interface */
  
  /* Endpoint Descriptor: Interrupt IN (Command) */
  0x07,                                    /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                  /* bDescriptorType: Endpoint */
  CDC_CMD_EP,                              /* bEndpointAddress: IN Endpoint 3 */
  0x03,                                    /* bmAttributes: Interrupt */
  LOBYTE(CDC_CMD_PACKET_SIZE),             /* wMaxPacketSize */
  HIBYTE(CDC_CMD_PACKET_SIZE),
  0x10,                                    /* bInterval: Polling interval (16 ms) */
  
  /*---------------------------------------------------------------------------*/
  /* Data class interface descriptor */
  0x09,                                    /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_INTERFACE,                 /* bDescriptorType: Interface */
  0x01,                                    /* bInterfaceNumber: Number of Interface */
  0x00,                                    /* bAlternateSetting: Alternate setting */
  0x02,                                    /* bNumEndpoints: Two endpoints used (Bulk IN/OUT) */
  0x0A,                                    /* bInterfaceClass: CDC Data Class */
  0x00,                                    /* bInterfaceSubClass */
  0x00,                                    /* bInterfaceProtocol */
  0x00,                                    /* iInterface */
  
  /* Endpoint Descriptor: Bulk OUT */
  0x07,                                    /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                  /* bDescriptorType: Endpoint */
  CDC_OUT_EP,                              /* bEndpointAddress: OUT Endpoint 2 */
  0x02,                                    /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),     /* wMaxPacketSize */
  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
  0x00,                                    /* bInterval */
  
  /* Endpoint Descriptor: Bulk IN */
  0x07,                                    /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                  /* bDescriptorType: Endpoint */
  CDC_IN_EP,                               /* bEndpointAddress: IN Endpoint 2 */
  0x02,                                    /* bmAttributes: Bulk */
  LOBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),     /* wMaxPacketSize */
  HIBYTE(CDC_DATA_FS_MAX_PACKET_SIZE),
  0x00                                     /* bInterval */
};

/* USB CDC device qualifier descriptor (for high-speed capable devices) */
__ALIGN_BEGIN static uint8_t USBD_CDC_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/* Private variables */
static USBD_CDC_ItfTypeDef *pCDC_Fops = NULL;

/**
 * @brief  Initialize the CDC interface
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_CDC_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);
  
  /* Allocate class data structure */
  USBD_CDC_HandleTypeDef *hcdc;
  
  hcdc = (USBD_CDC_HandleTypeDef *)USBD_malloc(sizeof(USBD_CDC_HandleTypeDef));
  
  if (hcdc == NULL) {
    pdev->pClassData = NULL;
    return USBD_EMEM;
  }
  
  memset(hcdc, 0, sizeof(USBD_CDC_HandleTypeDef));
  pdev->pClassData = (void *)hcdc;
  
  /* Open CDC endpoints */
  USBD_LL_OpenEP(pdev, CDC_IN_EP, USBD_EP_TYPE_BULK, CDC_DATA_FS_MAX_PACKET_SIZE);
  pdev->ep_in[CDC_IN_EP & 0xFU].is_used = 1U;
  
  USBD_LL_OpenEP(pdev, CDC_OUT_EP, USBD_EP_TYPE_BULK, CDC_DATA_FS_MAX_PACKET_SIZE);
  pdev->ep_out[CDC_OUT_EP & 0xFU].is_used = 1U;
  
  USBD_LL_OpenEP(pdev, CDC_CMD_EP, USBD_EP_TYPE_INTR, CDC_CMD_PACKET_SIZE);
  pdev->ep_in[CDC_CMD_EP & 0xFU].is_used = 1U;
  
  /* Initialize physical interface */
  if (pCDC_Fops != NULL && pCDC_Fops->Init != NULL) {
    pCDC_Fops->Init();
  }
  
  /* Initialize line coding (default 115200 8N1) */
  hcdc->line_coding.bitrate = 115200;
  hcdc->line_coding.format = 0;      /* 1 stop bit */
  hcdc->line_coding.parity = 0;      /* No parity */
  hcdc->line_coding.databits = 8;    /* 8 data bits */
  
  hcdc->tx_state = 0U;
  hcdc->rx_state = 0U;
  
  /* Prepare OUT endpoint to receive first packet */
  USBD_LL_PrepareReceive(pdev, CDC_OUT_EP, hcdc->data_out, CDC_DATA_FS_MAX_PACKET_SIZE);
  
  return USBD_OK;
}

/**
 * @brief  DeInitialize the CDC layer
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_CDC_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);
  
  /* Close CDC endpoints */
  USBD_LL_CloseEP(pdev, CDC_IN_EP);
  pdev->ep_in[CDC_IN_EP & 0xFU].is_used = 0U;
  
  USBD_LL_CloseEP(pdev, CDC_OUT_EP);
  pdev->ep_out[CDC_OUT_EP & 0xFU].is_used = 0U;
  
  USBD_LL_CloseEP(pdev, CDC_CMD_EP);
  pdev->ep_in[CDC_CMD_EP & 0xFU].is_used = 0U;
  
  /* DeInit physical interface */
  if (pCDC_Fops != NULL && pCDC_Fops->DeInit != NULL) {
    pCDC_Fops->DeInit();
  }
  
  /* Free class data */
  if (pdev->pClassData != NULL) {
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }
  
  return USBD_OK;
}

/**
 * @brief  Handle the CDC specific requests
 * @param  pdev: instance
 * @param  req: usb requests
 * @retval status
 */
static uint8_t USBD_CDC_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;
  uint16_t len = 0;
  uint8_t *pbuf = NULL;
  uint8_t ret = USBD_OK;
  
  if (hcdc == NULL) {
    return USBD_FAIL;
  }
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK) {
    case USB_REQ_TYPE_CLASS:
      if (req->wLength > 0U) {
        if ((req->bmRequest & 0x80U) != 0U) {
          /* Device-to-Host request */
          if (pCDC_Fops != NULL && pCDC_Fops->Control != NULL) {
            pCDC_Fops->Control(req->bRequest, (uint8_t *)hcdc->data_out, req->wLength);
          }
          
          len = (uint16_t)((req->wLength < CDC_DATA_FS_MAX_PACKET_SIZE) ? req->wLength : CDC_DATA_FS_MAX_PACKET_SIZE);
          
          if (req->bRequest == CDC_GET_LINE_CODING) {
            pbuf = (uint8_t *)&hcdc->line_coding;
            len = sizeof(USBD_CDC_LineCodingTypeDef);
          }
          
          if (pbuf != NULL) {
            USBD_CtlSendData(pdev, pbuf, len);
          }
        } else {
          /* Host-to-Device request */
          hcdc->cmd_data[0] = req->bRequest;
          hcdc->data_out_length = req->wLength;
          
          USBD_CtlPrepareRx(pdev, hcdc->data_out, req->wLength);
        }
      } else {
        /* No data stage - handle command immediately */
        if (pCDC_Fops != NULL && pCDC_Fops->Control != NULL) {
          pCDC_Fops->Control(req->bRequest, (uint8_t *)req, 0U);
        }
        
        if (req->bRequest == CDC_SET_CONTROL_LINE_STATE) {
          hcdc->control_line_state = (uint16_t)req->wValue;
        }
      }
      break;
      
    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest) {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED) {
            USBD_CtlSendData(pdev, (uint8_t *)&hcdc->rx_state, 2U);
          } else {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;
          
        case USB_REQ_GET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED) {
            USBD_CtlSendData(pdev, (uint8_t *)&hcdc->rx_state, 1U);
          } else {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;
          
        case USB_REQ_SET_INTERFACE:
          if (pdev->dev_state != USBD_STATE_CONFIGURED) {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;
          
        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;
      
    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }
  
  return ret;
}

/**
 * @brief  Handle data IN stage
 * @param  pdev: device instance
 * @param  epnum: endpoint number
 * @retval status
 */
static uint8_t USBD_CDC_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;
  
  if (hcdc == NULL) {
    return USBD_FAIL;
  }
  
  if ((pdev->ep_in[epnum & 0xFU].total_length > 0U) && 
      ((pdev->ep_in[epnum & 0xFU].total_length % CDC_DATA_FS_MAX_PACKET_SIZE) == 0U)) {
    /* Send ZLP (Zero Length Packet) */
    pdev->ep_in[epnum & 0xFU].total_length = 0U;
    USBD_LL_Transmit(pdev, epnum, NULL, 0U);
  } else {
    hcdc->tx_state = 0U;
    
    if (pCDC_Fops != NULL && pCDC_Fops->TransmitCplt != NULL) {
      pCDC_Fops->TransmitCplt(hcdc->data_in, &hcdc->data_in_length, epnum);
    }
  }
  
  return USBD_OK;
}

/**
 * @brief  Handle data OUT stage
 * @param  pdev: device instance
 * @param  epnum: endpoint number
 * @retval status
 */
static uint8_t USBD_CDC_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;
  
  if (hcdc == NULL) {
    return USBD_FAIL;
  }
  
  /* Get received data length */
  hcdc->data_out_length = USBD_LL_GetRxDataSize(pdev, epnum);
  
  /* Call receive callback */
  if (pCDC_Fops != NULL && pCDC_Fops->Receive != NULL) {
    pCDC_Fops->Receive(hcdc->data_out, &hcdc->data_out_length);
  }
  
  return USBD_OK;
}

/**
 * @brief  Handle EP0 Rx Ready event
 * @param  pdev: device instance
 * @retval status
 */
static uint8_t USBD_CDC_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;
  
  if (hcdc == NULL) {
    return USBD_FAIL;
  }
  
  if (hcdc->cmd_data[0] == CDC_SET_LINE_CODING) {
    /* Copy received line coding */
    memcpy(&hcdc->line_coding, hcdc->data_out, sizeof(USBD_CDC_LineCodingTypeDef));
    
    if (pCDC_Fops != NULL && pCDC_Fops->Control != NULL) {
      pCDC_Fops->Control(hcdc->cmd_data[0], (uint8_t *)hcdc->data_out, hcdc->data_out_length);
    }
  }
  
  return USBD_OK;
}

/**
 * @brief  Return configuration descriptor
 * @param  length : pointer to data length
 * @retval pointer to descriptor buffer
 */
static uint8_t *USBD_CDC_GetFSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_CDC_CfgDesc);
  return USBD_CDC_CfgDesc;
}

/**
 * @brief  Return configuration descriptor for high speed
 * @param  length : pointer to data length
 * @retval pointer to descriptor buffer
 */
static uint8_t *USBD_CDC_GetHSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_CDC_CfgDesc);
  return USBD_CDC_CfgDesc;
}

/**
 * @brief  Return other speed configuration descriptor
 * @param  length : pointer to data length
 * @retval pointer to descriptor buffer
 */
static uint8_t *USBD_CDC_GetOtherSpeedCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_CDC_CfgDesc);
  return USBD_CDC_CfgDesc;
}

/**
 * @brief  Return Device Qualifier descriptor
 * @param  length : pointer to data length
 * @retval pointer to descriptor buffer
 */
static uint8_t *USBD_CDC_GetDeviceQualifierDesc(uint16_t *length)
{
  *length = sizeof(USBD_CDC_DeviceQualifierDesc);
  return USBD_CDC_DeviceQualifierDesc;
}

/* ============================================================================
 * Public API Functions
 * ============================================================================ */

/**
 * @brief  Register CDC interface callbacks
 * @param  pdev: device instance
 * @param  fops: CDC interface callback
 * @retval status
 */
uint8_t USBD_CDC_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_CDC_ItfTypeDef *fops)
{
  if (fops == NULL) {
    return USBD_FAIL;
  }
  
  pCDC_Fops = fops;
  return USBD_OK;
}

/**
 * @brief  Set transmit buffer
 * @param  pdev: device instance
 * @param  pbuff: Tx buffer
 * @param  length: buffer length
 * @retval status
 */
uint8_t USBD_CDC_SetTxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint32_t length)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;
  
  if (hcdc == NULL) {
    return USBD_FAIL;
  }
  
  hcdc->data_in_length = length;
  /* Note: This function stores the length only. The actual buffer is managed
   * internally by the class driver. The pbuff parameter is not used as data
   * is copied to the internal buffer in USBD_CDC_TransmitData(). */
  
  return USBD_OK;
}

/**
 * @brief  Set receive buffer
 * @param  pdev: device instance
 * @param  pbuff: Rx buffer
 * @retval status
 */
uint8_t USBD_CDC_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;
  
  if (hcdc == NULL) {
    return USBD_FAIL;
  }
  
  /* Note: Application should manage its own buffer */
  /* This function is provided for compatibility but buffer is managed internally */
  UNUSED(pbuff);
  
  return USBD_OK;
}

/**
 * @brief  Prepare receive next packet
 * @param  pdev: device instance
 * @retval status
 */
uint8_t USBD_CDC_ReceivePacket(USBD_HandleTypeDef *pdev)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;
  
  if (hcdc == NULL) {
    return USBD_FAIL;
  }
  
  /* Prepare OUT endpoint to receive next packet */
  USBD_LL_PrepareReceive(pdev, CDC_OUT_EP, hcdc->data_out, CDC_DATA_FS_MAX_PACKET_SIZE);
  
  return USBD_OK;
}

/**
 * @brief  Transmit packet
 * @param  pdev: device instance
 * @retval status
 */
uint8_t USBD_CDC_TransmitPacket(USBD_HandleTypeDef *pdev)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;
  
  if (hcdc == NULL) {
    return USBD_FAIL;
  }
  
  if (hcdc->tx_state == 0U) {
    hcdc->tx_state = 1U;
    
    pdev->ep_in[CDC_IN_EP & 0xFU].total_length = hcdc->data_in_length;
    
    USBD_LL_Transmit(pdev, CDC_IN_EP, hcdc->data_in, hcdc->data_in_length);
    
    return USBD_OK;
  } else {
    return USBD_BUSY;
  }
}

/**
 * @brief  Transmit data via CDC
 * @param  pdev: device instance
 * @param  buf: data buffer
 * @param  length: data length
 * @retval status
 */
uint8_t USBD_CDC_TransmitData(USBD_HandleTypeDef *pdev, uint8_t *buf, uint16_t length)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;
  
  if (hcdc == NULL) {
    return USBD_FAIL;
  }
  
  if (hcdc->tx_state != 0U) {
    return USBD_BUSY;
  }
  
  /* Copy data to internal buffer */
  if (length > CDC_DATA_IN_MAX_PACKET_SIZE) {
    length = CDC_DATA_IN_MAX_PACKET_SIZE;
  }
  
  memcpy(hcdc->data_in, buf, length);
  hcdc->data_in_length = length;
  
  /* Transmit */
  return USBD_CDC_TransmitPacket(pdev);
}

/**
 * @brief  Check if CDC is connected (DTR active)
 * @param  pdev: device instance
 * @retval 1 if connected, 0 otherwise
 */
uint8_t USBD_CDC_IsConnected(USBD_HandleTypeDef *pdev)
{
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)pdev->pClassData;
  
  if (hcdc == NULL) {
    return 0;
  }
  
  /* Check if device is configured and DTR (Data Terminal Ready) is active */
  /* DTR is bit 0 of control_line_state */
  return (pdev->dev_state == USBD_STATE_CONFIGURED) && ((hcdc->control_line_state & 0x01) != 0);
}
