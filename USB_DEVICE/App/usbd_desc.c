/**
  ******************************************************************************
  * @file    usbd_desc.c
  * @brief   USB Device descriptors for MIOS32-style 4-port MIDI interface
  * @author  MidiCore
  ******************************************************************************
  * Descriptors define:
  * - Vendor/Product ID (customizable)
  * - Device name: "MidiCore" (4x4 MIDI interface like MIOS32)
  * - 4 virtual MIDI ports (cables 0-3)
  * - USB MIDI 1.0 compliant
  ******************************************************************************
  */

#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_conf.h"
#include "USB_DEVICE/Class/MIDI/Inc/usbd_midi_debug.h"

/* USB VID/PID - Customize these for your product */
#define USBD_VID                      0x16C0  /* Generic VID */
#define USBD_PID_FS                   0x0489  /* MIDI Device PID */
#define USBD_LANGID_STRING            1033    /* English (United States) */
#define USBD_MANUFACTURER_STRING      "MidiCore"
#define USBD_PRODUCT_STRING_FS        "MidiCore 4x4"  /* Like MIOS32 MBHP_CORE_STM32F4 */
#define USBD_CONFIGURATION_STRING_FS  "MIDI Config"
#define USBD_INTERFACE_STRING_FS      "MIDI Interface"

/* Device Descriptor */
__ALIGN_BEGIN uint8_t USBD_FS_DeviceDesc[USB_LEN_DEV_DESC] __ALIGN_END =
{
  0x12,                       /* bLength */
  USB_DESC_TYPE_DEVICE,       /* bDescriptorType */
  0x00, 0x02,                 /* bcdUSB = 2.00 */
  0x00,                       /* bDeviceClass */
  0x00,                       /* bDeviceSubClass */
  0x00,                       /* bDeviceProtocol */
  USB_MAX_EP0_SIZE,           /* bMaxPacketSize */
  LOBYTE(USBD_VID), HIBYTE(USBD_VID),     /* idVendor */
  LOBYTE(USBD_PID_FS), HIBYTE(USBD_PID_FS),  /* idProduct */
  0x00, 0x02,                 /* bcdDevice = 2.00 */
  USBD_IDX_MFC_STR,           /* Index of manufacturer string */
  USBD_IDX_PRODUCT_STR,       /* Index of product string */
  USBD_IDX_SERIAL_STR,        /* Index of serial number string */
  USBD_MAX_NUM_CONFIGURATION  /* bNumConfigurations */
};

/* USB language identifier descriptor */
__ALIGN_BEGIN uint8_t USBD_LangIDDesc[USB_LEN_LANGID_STR_DESC] __ALIGN_END =
{
  USB_LEN_LANGID_STR_DESC,
  USB_DESC_TYPE_STRING,
  LOBYTE(USBD_LANGID_STRING),
  HIBYTE(USBD_LANGID_STRING)
};

/* Internal string descriptor */
__ALIGN_BEGIN uint8_t USBD_StrDesc[USBD_MAX_STR_DESC_SIZ] __ALIGN_END;

/**
  * @brief  Return the device descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t *USBD_FS_DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  DEBUG_PRINT("\r\n>>> DEVICE DESCRIPTOR REQUESTED <<<\r\n");
  UNUSED(speed);
  *length = sizeof(USBD_FS_DeviceDesc);
  DEBUG_PRINT("  Returning Device Descriptor: %u bytes\r\n", *length);
  return USBD_FS_DeviceDesc;
}

/**
  * @brief  Return the LangID string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t *USBD_FS_LangIDStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  DEBUG_PRINT(">>> LANGID STRING DESCRIPTOR REQUESTED <<<\r\n");
  UNUSED(speed);
  *length = sizeof(USBD_LangIDDesc);
  return USBD_LangIDDesc;
}

/**
  * @brief  Return the product string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t *USBD_FS_ProductStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  DEBUG_PRINT(">>> PRODUCT STRING DESCRIPTOR REQUESTED <<<\r\n");
  UNUSED(speed);
  USBD_GetString((uint8_t *)USBD_PRODUCT_STRING_FS, USBD_StrDesc, length);
  return USBD_StrDesc;
}

/**
  * @brief  Return the manufacturer string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t *USBD_FS_ManufacturerStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  DEBUG_PRINT(">>> MANUFACTURER STRING DESCRIPTOR REQUESTED <<<\r\n");
  UNUSED(speed);
  USBD_GetString((uint8_t *)USBD_MANUFACTURER_STRING, USBD_StrDesc, length);
  return USBD_StrDesc;
}

/**
  * @brief  Return the serial number string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t *USBD_FS_SerialStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  DEBUG_PRINT(">>> SERIAL STRING DESCRIPTOR REQUESTED <<<\r\n");
  UNUSED(speed);
  
  /* Use STM32 unique ID as serial number */
  uint32_t deviceserial0, deviceserial1, deviceserial2;
  
  deviceserial0 = *(uint32_t *)UID_BASE;
  deviceserial1 = *(uint32_t *)(UID_BASE + 4);
  deviceserial2 = *(uint32_t *)(UID_BASE + 8);
  
  deviceserial0 += deviceserial2;
  
  if (deviceserial0 != 0)
  {
    /* Convert to hex string */
    USBD_StrDesc[0] = 26;  /* Length */
    USBD_StrDesc[1] = USB_DESC_TYPE_STRING;
    
    uint8_t *pStr = &USBD_StrDesc[2];
    uint32_t val;
    const char hexDigits[] = "0123456789ABCDEF";
    
    for (int i = 0; i < 3; i++)
    {
      val = (i == 0) ? deviceserial0 : ((i == 1) ? deviceserial1 : deviceserial2);
      for (int j = 7; j >= 0; j--)
      {
        *pStr++ = hexDigits[(val >> (j * 4)) & 0xF];
        *pStr++ = 0;
      }
    }
    *length = 26;
  }
  else
  {
    USBD_GetString((uint8_t *)"000000000000", USBD_StrDesc, length);
  }
  
  return USBD_StrDesc;
}

/**
  * @brief  Return the configuration string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t *USBD_FS_ConfigStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  DEBUG_PRINT(">>> CONFIGURATION STRING DESCRIPTOR REQUESTED <<<\r\n");
  UNUSED(speed);
  USBD_GetString((uint8_t *)USBD_CONFIGURATION_STRING_FS, USBD_StrDesc, length);
  return USBD_StrDesc;
}

/**
  * @brief  Return the interface string descriptor
  * @param  speed : Current device speed
  * @param  length : Pointer to data length variable
  * @retval Pointer to descriptor buffer
  */
uint8_t *USBD_FS_InterfaceStrDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
  DEBUG_PRINT(">>> INTERFACE STRING DESCRIPTOR REQUESTED <<<\r\n");
  UNUSED(speed);
  USBD_GetString((uint8_t *)USBD_INTERFACE_STRING_FS, USBD_StrDesc, length);
  return USBD_StrDesc;
}

/* USB Standard Device Descriptor */
USBD_DescriptorsTypeDef FS_Desc =
{
  USBD_FS_DeviceDescriptor,
  USBD_FS_LangIDStrDescriptor,
  USBD_FS_ManufacturerStrDescriptor,
  USBD_FS_ProductStrDescriptor,
  USBD_FS_SerialStrDescriptor,
  USBD_FS_ConfigStrDescriptor,
  USBD_FS_InterfaceStrDescriptor
};
