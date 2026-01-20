/**
  ******************************************************************************
  * @file    usbd_midi.c
  * @author  MidiCore Project
  * @brief   USB MIDI Device Class implementation - 4 Port (4x4) Interface
  ******************************************************************************
  * @attention
  *
  * Based on USB Device Class Definition for MIDI Devices v1.0
  * Implements a 4-cable (4x4) MIDI interface like MIOS32
  *
  ******************************************************************************
  */

#include "usbd_midi.h"
#include "usbd_ctlreq.h"
#include <string.h>

/* Calculate descriptor size - needed for compilation */
#define USB_MIDI_CONFIG_DESC_SIZ  (9 + 9 + 9 + 9 + 7 + (6 * MIDI_NUM_PORTS) + (9 * MIDI_NUM_PORTS * 2) + 9 + (5 + MIDI_NUM_PORTS) + 9 + (5 + MIDI_NUM_PORTS))

/* Private function prototypes */
static uint8_t USBD_MIDI_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_MIDI_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_MIDI_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_MIDI_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_MIDI_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_MIDI_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t *USBD_MIDI_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_MIDI_GetHSCfgDesc(uint16_t *length);
static uint8_t *USBD_MIDI_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t *USBD_MIDI_GetDeviceQualifierDesc(uint16_t *length);

/* USB MIDI Class Callbacks */
USBD_ClassTypeDef USBD_MIDI = 
{
  USBD_MIDI_Init,
  USBD_MIDI_DeInit,
  USBD_MIDI_Setup,
  NULL,  /* EP0_TxSent */
  USBD_MIDI_EP0_RxReady,
  USBD_MIDI_DataIn,
  USBD_MIDI_DataOut,
  NULL,  /* SOF */
  NULL,  /* IsoINIncomplete */
  NULL,  /* IsoOUTIncomplete */
  USBD_MIDI_GetFSCfgDesc,
  USBD_MIDI_GetHSCfgDesc,
  USBD_MIDI_GetOtherSpeedCfgDesc,
  USBD_MIDI_GetDeviceQualifierDesc,
};

/* USB MIDI Device Configuration Descriptor - 4 Port (4x4) Interface */
__ALIGN_BEGIN static uint8_t USBD_MIDI_CfgDesc[USB_MIDI_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /* Configuration Descriptor */
  0x09,                                  /* bLength */
  USB_DESC_TYPE_CONFIGURATION,           /* bDescriptorType */
  LOBYTE(USB_MIDI_CONFIG_DESC_SIZ),     /* wTotalLength */
  HIBYTE(USB_MIDI_CONFIG_DESC_SIZ),
  0x02,                                  /* bNumInterfaces: 2 (Audio Control + MIDIStreaming) */
  0x01,                                  /* bConfigurationValue */
  0x00,                                  /* iConfiguration */
  0x80,                                  /* bmAttributes: Bus Powered */
  0xFA,                                  /* MaxPower 500 mA */
  
  /* Standard Audio Control Interface Descriptor */
  0x09,                                  /* bLength */
  USB_DESC_TYPE_INTERFACE,               /* bDescriptorType */
  0x00,                                  /* bInterfaceNumber */
  0x00,                                  /* bAlternateSetting */
  0x00,                                  /* bNumEndpoints */
  USB_DEVICE_CLASS_AUDIO,                /* bInterfaceClass: Audio */
  AUDIO_SUBCLASS_AUDIOCONTROL,           /* bInterfaceSubClass: Audio Control */
  0x00,                                  /* bInterfaceProtocol */
  0x00,                                  /* iInterface */
  
  /* Class-specific Audio Control Interface Descriptor */
  0x09,                                  /* bLength */
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,       /* bDescriptorType */
  0x01,                                  /* bDescriptorSubtype: Header */
  0x00, 0x01,                            /* bcdADC: 1.00 */
  0x09, 0x00,                            /* wTotalLength */
  0x01,                                  /* bInCollection: 1 streaming interface */
  0x01,                                  /* baInterfaceNr(1): MIDIStreaming interface 1 */
  
  /* Standard MIDIStreaming Interface Descriptor */
  0x09,                                  /* bLength */
  USB_DESC_TYPE_INTERFACE,               /* bDescriptorType */
  0x01,                                  /* bInterfaceNumber */
  0x00,                                  /* bAlternateSetting */
  0x02,                                  /* bNumEndpoints: 2 (IN + OUT) */
  USB_DEVICE_CLASS_AUDIO,                /* bInterfaceClass: Audio */
  AUDIO_SUBCLASS_MIDISTREAMING,          /* bInterfaceSubClass: MIDIStreaming */
  0x00,                                  /* bInterfaceProtocol */
  0x00,                                  /* iInterface */
  
  /* Class-specific MIDIStreaming Interface Descriptor */
  0x07,                                  /* bLength */
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,       /* bDescriptorType */
  0x01,                                  /* bDescriptorSubtype: MS_HEADER */
  0x00, 0x01,                            /* bcdMSC: 1.00 */
  LOBYTE(65 + (MIDI_NUM_PORTS * 12)),   /* wTotalLength: Calculate based on jacks */
  HIBYTE(65 + (MIDI_NUM_PORTS * 12)),
  
  /* MIDI IN Jacks - External (4 ports) */
  /* Port 1 */
  0x06,                                  /* bLength */
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,       /* bDescriptorType */
  0x02,                                  /* bDescriptorSubtype: MIDI_IN_JACK */
  MIDI_JACK_TYPE_EXTERNAL,               /* bJackType: External */
  0x01,                                  /* bJackID: 1 */
  0x00,                                  /* iJack */
  
  /* Port 2 */
  0x06,
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,
  0x02,
  MIDI_JACK_TYPE_EXTERNAL,
  0x02,                                  /* bJackID: 2 */
  0x00,
  
  /* Port 3 */
  0x06,
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,
  0x02,
  MIDI_JACK_TYPE_EXTERNAL,
  0x03,                                  /* bJackID: 3 */
  0x00,
  
  /* Port 4 */
  0x06,
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,
  0x02,
  MIDI_JACK_TYPE_EXTERNAL,
  0x04,                                  /* bJackID: 4 */
  0x00,
  
  /* MIDI IN Jacks - Embedded (4 ports) */
  /* Port 1 */
  0x09,                                  /* bLength */
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,       /* bDescriptorType */
  0x02,                                  /* bDescriptorSubtype: MIDI_IN_JACK */
  MIDI_JACK_TYPE_EMBEDDED,               /* bJackType: Embedded */
  0x05,                                  /* bJackID: 5 */
  0x01,                                  /* bNrInputPins */
  0x01,                                  /* baSourceID(1): External Jack 1 */
  0x01,                                  /* baSourcePin(1) */
  0x00,                                  /* iJack */
  
  /* Port 2 */
  0x09,
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,
  0x02,
  MIDI_JACK_TYPE_EMBEDDED,
  0x06,                                  /* bJackID: 6 */
  0x01,
  0x02,                                  /* baSourceID(1): External Jack 2 */
  0x01,
  0x00,
  
  /* Port 3 */
  0x09,
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,
  0x02,
  MIDI_JACK_TYPE_EMBEDDED,
  0x07,                                  /* bJackID: 7 */
  0x01,
  0x03,                                  /* baSourceID(1): External Jack 3 */
  0x01,
  0x00,
  
  /* Port 4 */
  0x09,
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,
  0x02,
  MIDI_JACK_TYPE_EMBEDDED,
  0x08,                                  /* bJackID: 8 */
  0x01,
  0x04,                                  /* baSourceID(1): External Jack 4 */
  0x01,
  0x00,
  
  /* MIDI OUT Jacks - Embedded (4 ports) */
  /* Port 1 */
  0x09,
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,
  0x03,                                  /* bDescriptorSubtype: MIDI_OUT_JACK */
  MIDI_JACK_TYPE_EMBEDDED,
  0x09,                                  /* bJackID: 9 */
  0x01,
  0x05,                                  /* baSourceID(1): Embedded IN Jack 5 */
  0x01,
  0x00,
  
  /* Port 2 */
  0x09,
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,
  0x03,
  MIDI_JACK_TYPE_EMBEDDED,
  0x0A,                                  /* bJackID: 10 */
  0x01,
  0x06,                                  /* baSourceID(1): Embedded IN Jack 6 */
  0x01,
  0x00,
  
  /* Port 3 */
  0x09,
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,
  0x03,
  MIDI_JACK_TYPE_EMBEDDED,
  0x0B,                                  /* bJackID: 11 */
  0x01,
  0x07,                                  /* baSourceID(1): Embedded IN Jack 7 */
  0x01,
  0x00,
  
  /* Port 4 */
  0x09,
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,
  0x03,
  MIDI_JACK_TYPE_EMBEDDED,
  0x0C,                                  /* bJackID: 12 */
  0x01,
  0x08,                                  /* baSourceID(1): Embedded IN Jack 8 */
  0x01,
  0x00,
  
  /* MIDI OUT Jacks - External (4 ports) */
  /* Port 1 */
  0x09,
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,
  0x03,
  MIDI_JACK_TYPE_EXTERNAL,
  0x0D,                                  /* bJackID: 13 */
  0x01,
  0x09,                                  /* baSourceID(1): Embedded OUT Jack 9 */
  0x01,
  0x00,
  
  /* Port 2 */
  0x09,
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,
  0x03,
  MIDI_JACK_TYPE_EXTERNAL,
  0x0E,                                  /* bJackID: 14 */
  0x01,
  0x0A,                                  /* baSourceID(1): Embedded OUT Jack 10 */
  0x01,
  0x00,
  
  /* Port 3 */
  0x09,
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,
  0x03,
  MIDI_JACK_TYPE_EXTERNAL,
  0x0F,                                  /* bJackID: 15 */
  0x01,
  0x0B,                                  /* baSourceID(1): Embedded OUT Jack 11 */
  0x01,
  0x00,
  
  /* Port 4 */
  0x09,
  AUDIO_DESCRIPTOR_TYPE_INTERFACE,
  0x03,
  MIDI_JACK_TYPE_EXTERNAL,
  0x10,                                  /* bJackID: 16 */
  0x01,
  0x0C,                                  /* baSourceID(1): Embedded OUT Jack 12 */
  0x01,
  0x00,
  
  /* Standard Bulk OUT Endpoint Descriptor */
  0x09,                                  /* bLength */
  USB_DESC_TYPE_ENDPOINT,                /* bDescriptorType */
  MIDI_OUT_EP,                           /* bEndpointAddress */
  0x02,                                  /* bmAttributes: Bulk */
  LOBYTE(MIDI_DATA_FS_MAX_PACKET_SIZE),  /* wMaxPacketSize */
  HIBYTE(MIDI_DATA_FS_MAX_PACKET_SIZE),
  0x00,                                  /* bInterval */
  0x00,                                  /* bRefresh */
  0x00,                                  /* bSynchAddress */
  
  /* Class-specific Bulk OUT Endpoint Descriptor */
  0x05 + MIDI_NUM_PORTS,                 /* bLength */
  AUDIO_DESCRIPTOR_TYPE_ENDPOINT,        /* bDescriptorType */
  0x01,                                  /* bDescriptorSubtype: MS_GENERAL */
  MIDI_NUM_PORTS,                        /* bNumEmbMIDIJack: 4 */
  0x05,                                  /* baAssocJackID(1): Embedded IN Jack 5 */
  0x06,                                  /* baAssocJackID(2): Embedded IN Jack 6 */
  0x07,                                  /* baAssocJackID(3): Embedded IN Jack 7 */
  0x08,                                  /* baAssocJackID(4): Embedded IN Jack 8 */
  
  /* Standard Bulk IN Endpoint Descriptor */
  0x09,                                  /* bLength */
  USB_DESC_TYPE_ENDPOINT,                /* bDescriptorType */
  MIDI_IN_EP,                            /* bEndpointAddress */
  0x02,                                  /* bmAttributes: Bulk */
  LOBYTE(MIDI_DATA_FS_MAX_PACKET_SIZE),  /* wMaxPacketSize */
  HIBYTE(MIDI_DATA_FS_MAX_PACKET_SIZE),
  0x00,                                  /* bInterval */
  0x00,                                  /* bRefresh */
  0x00,                                  /* bSynchAddress */
  
  /* Class-specific Bulk IN Endpoint Descriptor */
  0x05 + MIDI_NUM_PORTS,                 /* bLength */
  AUDIO_DESCRIPTOR_TYPE_ENDPOINT,        /* bDescriptorType */
  0x01,                                  /* bDescriptorSubtype: MS_GENERAL */
  MIDI_NUM_PORTS,                        /* bNumEmbMIDIJack: 4 */
  0x09,                                  /* baAssocJackID(1): Embedded OUT Jack 9 */
  0x0A,                                  /* baAssocJackID(2): Embedded OUT Jack 10 */
  0x0B,                                  /* baAssocJackID(3): Embedded OUT Jack 11 */
  0x0C,                                  /* baAssocJackID(4): Embedded OUT Jack 12 */
};

/* USB Standard Device Qualifier Descriptor */
__ALIGN_BEGIN static uint8_t USBD_MIDI_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
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
static USBD_MIDI_HandleTypeDef midi_class_data;
static USBD_MIDI_ItfTypeDef *midi_fops = NULL;

/**
  * @brief  Initialize MIDI interface
  */
static uint8_t USBD_MIDI_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);
  
  /* Open EP OUT */
  USBD_LL_OpenEP(pdev, MIDI_OUT_EP, USBD_EP_TYPE_BULK, MIDI_DATA_FS_MAX_PACKET_SIZE);
  pdev->ep_out[MIDI_OUT_EP & 0xFU].is_used = 1U;
  
  /* Open EP IN */
  USBD_LL_OpenEP(pdev, MIDI_IN_EP, USBD_EP_TYPE_BULK, MIDI_DATA_FS_MAX_PACKET_SIZE);
  pdev->ep_in[MIDI_IN_EP & 0xFU].is_used = 1U;
  
  /* Initialize class data */
  pdev->pClassData = &midi_class_data;
  memset(&midi_class_data, 0, sizeof(USBD_MIDI_HandleTypeDef));
  midi_class_data.is_ready = 1;
  
  /* Prepare Out endpoint to receive next packet */
  USBD_LL_PrepareReceive(pdev, MIDI_OUT_EP, midi_class_data.data_out, MIDI_DATA_FS_MAX_PACKET_SIZE);
  
  /* Call interface Init callback */
  if (midi_fops != NULL && midi_fops->Init != NULL)
  {
    midi_fops->Init();
  }
  
  return USBD_OK;
}

/**
  * @brief  De-Initialize MIDI interface
  */
static uint8_t USBD_MIDI_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);
  
  /* Close EP OUT */
  USBD_LL_CloseEP(pdev, MIDI_OUT_EP);
  pdev->ep_out[MIDI_OUT_EP & 0xFU].is_used = 0U;
  
  /* Close EP IN */
  USBD_LL_CloseEP(pdev, MIDI_IN_EP);
  pdev->ep_in[MIDI_IN_EP & 0xFU].is_used = 0U;
  
  /* Call interface DeInit callback */
  if (midi_fops != NULL && midi_fops->DeInit != NULL)
  {
    midi_fops->DeInit();
  }
  
  pdev->pClassData = NULL;
  midi_class_data.is_ready = 0;
  
  return USBD_OK;
}

/**
  * @brief  Handle MIDI specific requests
  */
static uint8_t USBD_MIDI_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  UNUSED(pdev);
  UNUSED(req);
  
  /* No class-specific requests for MIDI */
  return USBD_OK;
}

/**
  * @brief  Handle data IN stage
  */
static uint8_t USBD_MIDI_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  UNUSED(pdev);
  UNUSED(epnum);
  
  /* TX Complete - ready for next packet */
  return USBD_OK;
}

/**
  * @brief  Handle data OUT stage (receive from host)
  */
static uint8_t USBD_MIDI_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;
  
  if (epnum == (MIDI_OUT_EP & 0x7F))
  {
    /* Get received data length */
    hmidi->data_out_length = USBD_LL_GetRxDataSize(pdev, epnum);
    
    /* Process received MIDI packets (4 bytes each) */
    if (midi_fops != NULL && midi_fops->DataOut != NULL && hmidi->data_out_length > 0)
    {
      uint32_t num_packets = hmidi->data_out_length / 4;
      USBD_MIDI_EventPacket_t *packets = (USBD_MIDI_EventPacket_t *)hmidi->data_out;
      
      for (uint32_t i = 0; i < num_packets; i++)
      {
        midi_fops->DataOut(&packets[i]);
      }
    }
    
    /* Prepare Out endpoint to receive next packet */
    USBD_LL_PrepareReceive(pdev, MIDI_OUT_EP, hmidi->data_out, MIDI_DATA_FS_MAX_PACKET_SIZE);
  }
  
  return USBD_OK;
}

/**
  * @brief  Handle EP0 Rx Ready event
  */
static uint8_t USBD_MIDI_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  UNUSED(pdev);
  return USBD_OK;
}

/**
  * @brief  Get Full Speed configuration descriptor
  */
static uint8_t *USBD_MIDI_GetFSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_MIDI_CfgDesc);
  return USBD_MIDI_CfgDesc;
}

/**
  * @brief  Get High Speed configuration descriptor
  */
static uint8_t *USBD_MIDI_GetHSCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_MIDI_CfgDesc);
  return USBD_MIDI_CfgDesc;
}

/**
  * @brief  Get Other Speed configuration descriptor
  */
static uint8_t *USBD_MIDI_GetOtherSpeedCfgDesc(uint16_t *length)
{
  *length = sizeof(USBD_MIDI_CfgDesc);
  return USBD_MIDI_CfgDesc;
}

/**
  * @brief  Get Device Qualifier descriptor
  */
static uint8_t *USBD_MIDI_GetDeviceQualifierDesc(uint16_t *length)
{
  *length = sizeof(USBD_MIDI_DeviceQualifierDesc);
  return USBD_MIDI_DeviceQualifierDesc;
}

/**
  * @brief  Register MIDI interface callbacks
  */
uint8_t USBD_MIDI_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MIDI_ItfTypeDef *fops)
{
  UNUSED(pdev);
  
  if (fops == NULL)
  {
    return USBD_FAIL;
  }
  
  midi_fops = fops;
  return USBD_OK;
}

/**
  * @brief  Send MIDI data
  * @param  pdev: device instance
  * @param  cable: cable number (0-3 for 4 ports)
  * @param  data: pointer to MIDI message (without cable number)
  * @param  length: length of data (1-3 bytes)
  * @retval status
  */
uint8_t USBD_MIDI_SendData(USBD_HandleTypeDef *pdev, uint8_t cable, uint8_t *data, uint16_t length)
{
  USBD_MIDI_HandleTypeDef *hmidi = (USBD_MIDI_HandleTypeDef *)pdev->pClassData;
  
  if (hmidi == NULL || !hmidi->is_ready || cable >= MIDI_NUM_PORTS)
  {
    return USBD_BUSY;
  }
  
  /* Build USB MIDI packet (4 bytes) */
  uint8_t packet[4];
  uint8_t cin = 0x0F; /* Miscellaneous function code */
  
  /* Determine Code Index Number based on status byte */
  if (length >= 1 && (data[0] & 0x80))
  {
    uint8_t status = data[0] & 0xF0;
    switch (status)
    {
      case 0x80: cin = 0x08; break; /* Note Off */
      case 0x90: cin = 0x09; break; /* Note On */
      case 0xA0: cin = 0x0A; break; /* Poly Aftertouch */
      case 0xB0: cin = 0x0B; break; /* Control Change */
      case 0xC0: cin = 0x0C; break; /* Program Change */
      case 0xD0: cin = 0x0D; break; /* Channel Aftertouch */
      case 0xE0: cin = 0x0E; break; /* Pitch Bend */
      case 0xF0:
        switch (data[0])
        {
          case 0xF0: cin = 0x04; break; /* SysEx Start */
          case 0xF1: cin = 0x02; break; /* MTC Quarter Frame */
          case 0xF2: cin = 0x03; break; /* Song Position */
          case 0xF3: cin = 0x02; break; /* Song Select */
          case 0xF6: cin = 0x05; break; /* Tune Request */
          case 0xF7: cin = 0x05; break; /* SysEx End */
          case 0xF8: cin = 0x0F; break; /* Timing Clock */
          case 0xFA: cin = 0x0F; break; /* Start */
          case 0xFB: cin = 0x0F; break; /* Continue */
          case 0xFC: cin = 0x0F; break; /* Stop */
          case 0xFE: cin = 0x0F; break; /* Active Sensing */
          case 0xFF: cin = 0x0F; break; /* System Reset */
        }
        break;
    }
  }
  
  /* Pack cable number and CIN into header */
  packet[0] = (cable << 4) | cin;
  packet[1] = (length >= 1) ? data[0] : 0;
  packet[2] = (length >= 2) ? data[1] : 0;
  packet[3] = (length >= 3) ? data[2] : 0;
  
  /* Transmit packet */
  USBD_LL_Transmit(pdev, MIDI_IN_EP, packet, 4);
  
  return USBD_OK;
}
