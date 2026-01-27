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

/* Composite descriptor - statically built for MIDI + CDC */
/* Total size calculation:
 * - Configuration Header: 9 bytes
 * - MIDI IAD: 8 bytes
 * - MIDI Interfaces (AC + MS): ~206 bytes (includes IAD already in MIDI descriptor)
 * - CDC IAD: 8 bytes  
 * - CDC Interfaces (Control + Data): 58 bytes (without config header)
 * Total: 9 + 206 + 8 + 58 = 281 bytes (rounded to 300 for safety)
 */
#define USB_COMPOSITE_CONFIG_DESC_SIZE  512  /* Buffer size for building descriptor */
static uint8_t USBD_COMPOSITE_CfgDesc[USB_COMPOSITE_CONFIG_DESC_SIZE] __ALIGN_BEGIN __ALIGN_END;
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
 * @brief  Build composite configuration descriptor for MIDI + CDC
 * @param  length: pointer to data length
 * @retval pointer to descriptor buffer
 * 
 * Descriptor structure:
 * 1. Configuration Descriptor (9 bytes)
 * 2. MIDI Function (IAD + Audio Control + MIDI Streaming + Endpoints)
 * 3. CDC Function (IAD + Communication Interface + Data Interface + Endpoints)
 * 
 * Interface assignments:
 * - Interface 0: Audio Control (MIDI)
 * - Interface 1: MIDI Streaming
 * - Interface 2: CDC Communication (Control)
 * - Interface 3: CDC Data
 */
static uint8_t *USBD_COMPOSITE_GetFSCfgDesc(uint16_t *length)
{
  if (composite_desc_len == 0) {
    /* Build descriptor on first call */
    uint8_t *ptr = USBD_COMPOSITE_CfgDesc;
    uint16_t total_len = 0;
    
    /* Get individual class descriptors */
    uint16_t midi_len = 0;
    uint8_t *midi_desc = USBD_MIDI.GetFSConfigDescriptor(&midi_len);
    
#if MODULE_ENABLE_USB_CDC
    uint16_t cdc_len = 0;
    uint8_t *cdc_desc = USBD_CDC.GetFSConfigDescriptor(&cdc_len);
    
    /* ===================================================================
     * STEP 1: Configuration Descriptor Header
     * =================================================================== */
    ptr[0] = 0x09;                          /* bLength */
    ptr[1] = USB_DESC_TYPE_CONFIGURATION;   /* bDescriptorType */
    /* wTotalLength - will be filled later */
    ptr[2] = 0x00;  /* Placeholder */
    ptr[3] = 0x00;  /* Placeholder */
    ptr[4] = 0x04;                          /* bNumInterfaces: 4 (MIDI:2 + CDC:2) */
    ptr[5] = 0x01;                          /* bConfigurationValue */
    ptr[6] = 0x00;                          /* iConfiguration */
    ptr[7] = 0x80;                          /* bmAttributes: Bus Powered */
    ptr[8] = 0xFA;                          /* MaxPower: 500mA */
    ptr += 9;
    total_len = 9;
    
    /* ===================================================================
     * STEP 2: Copy MIDI Function (includes IAD + interfaces + endpoints)
     * Skip the config header (9 bytes) from MIDI descriptor
     * =================================================================== */
    uint16_t midi_function_len = midi_len - 9;  /* Exclude config header */
    memcpy(ptr, midi_desc + 9, midi_function_len);
    ptr += midi_function_len;
    total_len += midi_function_len;
    
    /* ===================================================================
     * STEP 3: Add IAD for CDC Function
     * =================================================================== */
    ptr[0] = 0x08;                          /* bLength */
    ptr[1] = 0x0B;                          /* bDescriptorType: IAD */
    ptr[2] = 0x02;                          /* bFirstInterface: 2 (CDC Control) */
    ptr[3] = 0x02;                          /* bInterfaceCount: 2 (Control + Data) */
    ptr[4] = 0x02;                          /* bFunctionClass: CDC */
    ptr[5] = 0x02;                          /* bFunctionSubClass: ACM */
    ptr[6] = 0x00;                          /* bFunctionProtocol */
    ptr[7] = 0x00;                          /* iFunction */
    ptr += 8;
    total_len += 8;
    
    /* ===================================================================
     * STEP 4: Copy CDC interfaces with hardcoded interface numbers
     * Instead of adjusting in-place, rebuild CDC descriptors with correct values
     * =================================================================== */
    
    /* Build CDC Control Interface (Interface 2) */
    ptr[0] = 0x09;  /* bLength */
    ptr[1] = USB_DESC_TYPE_INTERFACE;  /* bDescriptorType */
    ptr[2] = 0x02;  /* bInterfaceNumber: 2 (not 0!) */
    ptr[3] = 0x00;  /* bAlternateSetting */
    ptr[4] = 0x01;  /* bNumEndpoints: 1 (Interrupt IN) */
    ptr[5] = 0x02;  /* bInterfaceClass: CDC */
    ptr[6] = 0x02;  /* bInterfaceSubClass: ACM */
    ptr[7] = 0x01;  /* bInterfaceProtocol: AT commands */
    ptr[8] = 0x00;  /* iInterface */
    ptr += 9;
    total_len += 9;
    
    /* Header Functional Descriptor */
    ptr[0] = 0x05;  /* bLength */
    ptr[1] = 0x24;  /* bDescriptorType: CS_INTERFACE */
    ptr[2] = 0x00;  /* bDescriptorSubtype: Header */
    ptr[3] = 0x10;  /* bcdCDC: 1.10 */
    ptr[4] = 0x01;
    ptr += 5;
    total_len += 5;
    
    /* Call Management Functional Descriptor */
    ptr[0] = 0x05;  /* bLength */
    ptr[1] = 0x24;  /* bDescriptorType: CS_INTERFACE */
    ptr[2] = 0x01;  /* bDescriptorSubtype: Call Management */
    ptr[3] = 0x00;  /* bmCapabilities */
    ptr[4] = 0x03;  /* bDataInterface: 3 (not 1!) */
    ptr += 5;
    total_len += 5;
    
    /* ACM Functional Descriptor */
    ptr[0] = 0x04;  /* bLength */
    ptr[1] = 0x24;  /* bDescriptorType: CS_INTERFACE */
    ptr[2] = 0x02;  /* bDescriptorSubtype: ACM */
    ptr[3] = 0x02;  /* bmCapabilities */
    ptr += 4;
    total_len += 4;
    
    /* Union Functional Descriptor */
    ptr[0] = 0x05;  /* bLength */
    ptr[1] = 0x24;  /* bDescriptorType: CS_INTERFACE */
    ptr[2] = 0x06;  /* bDescriptorSubtype: Union */
    ptr[3] = 0x02;  /* bControlInterface: 2 (not 0!) */
    ptr[4] = 0x03;  /* bSubordinateInterface: 3 (not 1!) */
    ptr += 5;
    total_len += 5;
    
    /* Endpoint Descriptor: Interrupt IN (Command) */
    ptr[0] = 0x07;  /* bLength */
    ptr[1] = USB_DESC_TYPE_ENDPOINT;  /* bDescriptorType */
    ptr[2] = 0x83;  /* bEndpointAddress: IN Endpoint 3 */
    ptr[3] = 0x03;  /* bmAttributes: Interrupt */
    ptr[4] = 0x08;  /* wMaxPacketSize: 8 bytes */
    ptr[5] = 0x00;
    ptr[6] = 0x10;  /* bInterval: 16 ms */
    ptr += 7;
    total_len += 7;
    
    /* Build CDC Data Interface (Interface 3) */
    ptr[0] = 0x09;  /* bLength */
    ptr[1] = USB_DESC_TYPE_INTERFACE;  /* bDescriptorType */
    ptr[2] = 0x03;  /* bInterfaceNumber: 3 (not 1!) */
    ptr[3] = 0x00;  /* bAlternateSetting */
    ptr[4] = 0x02;  /* bNumEndpoints: 2 (Bulk IN/OUT) */
    ptr[5] = 0x0A;  /* bInterfaceClass: CDC Data */
    ptr[6] = 0x00;  /* bInterfaceSubClass */
    ptr[7] = 0x00;  /* bInterfaceProtocol */
    ptr[8] = 0x00;  /* iInterface */
    ptr += 9;
    total_len += 9;
    
    /* Endpoint Descriptor: Bulk OUT */
    ptr[0] = 0x07;  /* bLength */
    ptr[1] = USB_DESC_TYPE_ENDPOINT;  /* bDescriptorType */
    ptr[2] = 0x02;  /* bEndpointAddress: OUT Endpoint 2 */
    ptr[3] = 0x02;  /* bmAttributes: Bulk */
    ptr[4] = 0x40;  /* wMaxPacketSize: 64 bytes */
    ptr[5] = 0x00;
    ptr[6] = 0x00;  /* bInterval */
    ptr += 7;
    total_len += 7;
    
    /* Endpoint Descriptor: Bulk IN */
    ptr[0] = 0x07;  /* bLength */
    ptr[1] = USB_DESC_TYPE_ENDPOINT;  /* bDescriptorType */
    ptr[2] = 0x82;  /* bEndpointAddress: IN Endpoint 2 */
    ptr[3] = 0x02;  /* bmAttributes: Bulk */
    ptr[4] = 0x40;  /* wMaxPacketSize: 64 bytes */
    ptr[5] = 0x00;
    ptr[6] = 0x00;  /* bInterval */
    ptr += 7;
    total_len += 7;
    
    /* ===================================================================
     * STEP 5: Update total length in configuration descriptor
     * =================================================================== */
    USBD_COMPOSITE_CfgDesc[2] = LOBYTE(total_len);
    USBD_COMPOSITE_CfgDesc[3] = HIBYTE(total_len);
    
    /* ===================================================================
     * STEP 6: Validate descriptor structure (prevent freeze)
     * =================================================================== */
    uint16_t parsed_len = 0;
    uint8_t *validate_ptr = USBD_COMPOSITE_CfgDesc;
    uint8_t valid = 1;
    
    while (parsed_len < total_len && valid) {
      uint8_t desc_len = validate_ptr[parsed_len];
      
      /* Check for zero-length descriptor (causes infinite loop) */
      if (desc_len == 0) {
        valid = 0;
        break;
      }
      
      /* Check for descriptor extending past buffer */
      if (parsed_len + desc_len > total_len) {
        valid = 0;
        break;
      }
      
      /* Check for invalid descriptor length (must be at least 2) */
      if (desc_len < 2) {
        valid = 0;
        break;
      }
      
      parsed_len += desc_len;
    }
    
    /* If validation failed, reset and return NULL */
    if (!valid || parsed_len != total_len) {
      /* Descriptor validation FAILED - fall back to MIDI-only mode */
      /* This prevents device freeze if CDC descriptor is malformed */
      memcpy(USBD_COMPOSITE_CfgDesc, midi_desc, midi_len);
      composite_desc_len = midi_len;
      *length = midi_len;
      return USBD_COMPOSITE_CfgDesc;
    }
    
    composite_desc_len = total_len;
    
#else /* !MODULE_ENABLE_USB_CDC - MIDI only mode */
    /* Just copy MIDI descriptor as-is */
    memcpy(USBD_COMPOSITE_CfgDesc, midi_desc, midi_len);
    composite_desc_len = midi_len;
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
