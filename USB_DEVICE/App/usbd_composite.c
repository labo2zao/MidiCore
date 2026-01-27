/**
 ******************************************************************************
 * @file    usbd_composite.c
 * @brief   USB Composite Device Class Implementation
 * @author  MidiCore Project
 ******************************************************************************
 * @attention
 *
 * Composite USB Device Class - Combines MIDI and CDC
 * Routes callbacks to appropriate class based on interface/endpoint
 *
 ******************************************************************************
 */

#include "usbd_composite.h"
#include "USB_DEVICE/Class/MIDI/Inc/usbd_midi.h"

#if MODULE_ENABLE_USB_CDC
#include "USB_DEVICE/Class/CDC/Inc/usbd_cdc.h"
#endif

#include <string.h>

/* Private function prototypes */
static uint8_t USBD_COMPOSITE_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_COMPOSITE_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_COMPOSITE_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_COMPOSITE_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_COMPOSITE_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_COMPOSITE_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t *USBD_COMPOSITE_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_COMPOSITE_GetHSCfgDesc(uint16_t *length);
static uint8_t *USBD_COMPOSITE_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t *USBD_COMPOSITE_GetDeviceQualifierDesc(uint16_t *length);

/* USB Composite Class Callbacks */
USBD_ClassTypeDef USBD_COMPOSITE = 
{
  USBD_COMPOSITE_Init,
  USBD_COMPOSITE_DeInit,
  USBD_COMPOSITE_Setup,
  NULL,  /* EP0_TxSent */
  USBD_COMPOSITE_EP0_RxReady,
  USBD_COMPOSITE_DataIn,
  USBD_COMPOSITE_DataOut,
  NULL,  /* SOF */
  NULL,  /* IsoINIncomplete */
  NULL,  /* IsoOUTIncomplete */
  USBD_COMPOSITE_GetFSCfgDesc,
  USBD_COMPOSITE_GetHSCfgDesc,
  USBD_COMPOSITE_GetOtherSpeedCfgDesc,
  USBD_COMPOSITE_GetDeviceQualifierDesc,
};

/* Composite descriptor will be built dynamically */
static uint8_t USBD_COMPOSITE_CfgDesc[256] __ALIGN_BEGIN __ALIGN_END;
static uint16_t composite_desc_len = 0;

/**
 * @brief  Initialize composite device
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_COMPOSITE_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  uint8_t ret = USBD_OK;
  
  /* Initialize MIDI class */
  if (USBD_MIDI.Init != NULL) {
    ret = USBD_MIDI.Init(pdev, cfgidx);
    if (ret != USBD_OK) {
      return ret;
    }
  }
  
#if MODULE_ENABLE_USB_CDC
  /* Initialize CDC class */
  if (USBD_CDC.Init != NULL) {
    ret = USBD_CDC.Init(pdev, cfgidx);
  }
#endif
  
  return ret;
}

/**
 * @brief  DeInitialize composite device
 * @param  pdev: device instance
 * @param  cfgidx: Configuration index
 * @retval status
 */
static uint8_t USBD_COMPOSITE_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  /* DeInit MIDI class */
  if (USBD_MIDI.DeInit != NULL) {
    USBD_MIDI.DeInit(pdev, cfgidx);
  }
  
#if MODULE_ENABLE_USB_CDC
  /* DeInit CDC class */
  if (USBD_CDC.DeInit != NULL) {
    USBD_CDC.DeInit(pdev, cfgidx);
  }
#endif
  
  return USBD_OK;
}

/**
 * @brief  Handle setup requests - route to appropriate class
 * @param  pdev: device instance
 * @param  req: usb requests
 * @retval status
 */
static uint8_t USBD_COMPOSITE_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  uint8_t interface = LOBYTE(req->wIndex);
  
  /* MIDI interfaces: 0, 1 */
  if (interface <= 1) {
    if (USBD_MIDI.Setup != NULL) {
      return USBD_MIDI.Setup(pdev, req);
    }
  }
  
#if MODULE_ENABLE_USB_CDC
  /* CDC interfaces: 2, 3 */
  if (interface >= 2 && interface <= 3) {
    if (USBD_CDC.Setup != NULL) {
      return USBD_CDC.Setup(pdev, req);
    }
  }
#endif
  
  return USBD_OK;
}

/**
 * @brief  Handle data IN - route based on endpoint
 * @param  pdev: device instance
 * @param  epnum: endpoint number
 * @retval status
 */
static uint8_t USBD_COMPOSITE_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  /* MIDI IN endpoint: 0x81 (EP1) */
  if (epnum == 0x01) {
    if (USBD_MIDI.DataIn != NULL) {
      return USBD_MIDI.DataIn(pdev, epnum);
    }
  }
  
#if MODULE_ENABLE_USB_CDC
  /* CDC IN endpoints: 0x82 (EP2 data), 0x83 (EP3 command) */
  if (epnum == 0x02 || epnum == 0x03) {
    if (USBD_CDC.DataIn != NULL) {
      return USBD_CDC.DataIn(pdev, epnum);
    }
  }
#endif
  
  return USBD_OK;
}

/**
 * @brief  Handle data OUT - route based on endpoint
 * @param  pdev: device instance
 * @param  epnum: endpoint number
 * @retval status
 */
static uint8_t USBD_COMPOSITE_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  /* MIDI OUT endpoint: 0x01 (EP1) */
  if (epnum == 0x01) {
    if (USBD_MIDI.DataOut != NULL) {
      return USBD_MIDI.DataOut(pdev, epnum);
    }
  }
  
#if MODULE_ENABLE_USB_CDC
  /* CDC OUT endpoint: 0x02 (EP2) */
  if (epnum == 0x02) {
    if (USBD_CDC.DataOut != NULL) {
      return USBD_CDC.DataOut(pdev, epnum);
    }
  }
#endif
  
  return USBD_OK;
}

/**
 * @brief  Handle EP0 RxReady event
 * @param  pdev: device instance
 * @retval status
 */
static uint8_t USBD_COMPOSITE_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  /* Route to both classes - they'll ignore if not relevant */
  if (USBD_MIDI.EP0_RxReady != NULL) {
    USBD_MIDI.EP0_RxReady(pdev);
  }
  
#if MODULE_ENABLE_USB_CDC
  if (USBD_CDC.EP0_RxReady != NULL) {
    USBD_CDC.EP0_RxReady(pdev);
  }
#endif
  
  return USBD_OK;
}

/**
 * @brief  Build composite configuration descriptor
 * @param  length: pointer to data length
 * @retval pointer to descriptor buffer
 */
static uint8_t *USBD_COMPOSITE_GetFSCfgDesc(uint16_t *length)
{
  if (composite_desc_len == 0) {
    /* Build descriptor on first call */
    uint8_t *ptr = USBD_COMPOSITE_CfgDesc;
    uint16_t midi_len = 0;
    uint8_t *midi_desc = USBD_MIDI.GetFSConfigDescriptor(&midi_len);
    
    /* Copy MIDI descriptor (skip config descriptor header, we'll rebuild it) */
    memcpy(ptr, midi_desc, midi_len);
    ptr += midi_len;
    composite_desc_len = midi_len;
    
#if MODULE_ENABLE_USB_CDC
    /* Append CDC descriptor (skip config descriptor header) */
    uint16_t cdc_len = 0;
    uint8_t *cdc_desc = USBD_CDC.GetFSConfigDescriptor(&cdc_len);
    
    /* Copy CDC descriptor parts (skip config header - 9 bytes) */
    memcpy(ptr, cdc_desc + 9, cdc_len - 9);
    composite_desc_len += (cdc_len - 9);
    
    /* Update configuration descriptor header */
    USBD_COMPOSITE_CfgDesc[2] = LOBYTE(composite_desc_len);
    USBD_COMPOSITE_CfgDesc[3] = HIBYTE(composite_desc_len);
    USBD_COMPOSITE_CfgDesc[4] = 4;  /* 4 interfaces total (MIDI: 2, CDC: 2) */
#endif
  }
  
  *length = composite_desc_len;
  return USBD_COMPOSITE_CfgDesc;
}

/**
 * @brief  Return HS configuration descriptor
 * @param  length: pointer to data length
 * @retval pointer to descriptor buffer
 */
static uint8_t *USBD_COMPOSITE_GetHSCfgDesc(uint16_t *length)
{
  return USBD_COMPOSITE_GetFSCfgDesc(length);
}

/**
 * @brief  Return other speed configuration descriptor
 * @param  length: pointer to data length
 * @retval pointer to descriptor buffer
 */
static uint8_t *USBD_COMPOSITE_GetOtherSpeedCfgDesc(uint16_t *length)
{
  return USBD_COMPOSITE_GetFSCfgDesc(length);
}

/**
 * @brief  Return device qualifier descriptor
 * @param  length: pointer to data length
 * @retval pointer to descriptor buffer
 */
static uint8_t *USBD_COMPOSITE_GetDeviceQualifierDesc(uint16_t *length)
{
  /* Use MIDI's device qualifier */
  return USBD_MIDI.GetDeviceQualifierDescriptor(length);
}

/**
 * @brief  Get class handler for specific interface
 * @param  interface_num: interface number
 * @retval pointer to class structure
 */
USBD_ClassTypeDef* USBD_COMPOSITE_GetClass(uint8_t interface_num)
{
  if (interface_num <= 1) {
    return &USBD_MIDI;
  }
  
#if MODULE_ENABLE_USB_CDC
  if (interface_num >= 2 && interface_num <= 3) {
    return &USBD_CDC;
  }
#endif
  
  return NULL;
}
