/**
  ******************************************************************************
  * @file    usb_device.c
  * @brief   This file implements the USB Device
  * @author  MidiCore (MIOS32-inspired USB MIDI Device - 4 ports)
  ******************************************************************************
  */

#include "usb_device.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "USB_DEVICE/Class/MIDI/Inc/usbd_midi.h"  /* Custom MIDI class - protected from CubeMX regen */
#include "Config/module_config.h"
#include "main.h"  /* For Error_Handler */

#if MODULE_ENABLE_USB_CDC
#include "USB_DEVICE/Class/CDC/Inc/usbd_cdc.h"    /* Custom CDC class - protected from CubeMX regen */
#endif

/* USB Device Core handle declaration */
USBD_HandleTypeDef hUsbDeviceFS;

/**
  * @brief  Initialize the USB device
  * @retval None
  */
void MX_USB_DEVICE_Init(void)
{
  /* Init Device Library, add supported class and start the library. */
  if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK)
  {
    Error_Handler();
  }
  
#if MODULE_ENABLE_USB_CDC
  /* Composite device mode: Register MIDI and CDC classes */
  /* Note: Composite API requires USE_USBD_COMPOSITE to be defined */
  
  /* Register the MIDI class (interfaces 0-1) */
  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_MIDI) != USBD_OK)
  {
    Error_Handler();
  }
  
  /* Register the CDC class (interfaces 2-3) */
  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC) != USBD_OK)
  {
    Error_Handler();
  }
  
  /* TODO: When USE_USBD_COMPOSITE is fully tested, use:
   * USBD_RegisterClassComposite(&hUsbDeviceFS, &USBD_MIDI, CLASS_TYPE_AUDIO, NULL);
   * USBD_RegisterClassComposite(&hUsbDeviceFS, &USBD_CDC, CLASS_TYPE_CDC, NULL);
   */
  
#else
  /* Single class mode: MIDI only */
  if (USBD_RegisterClass(&hUsbDeviceFS, &USBD_MIDI) != USBD_OK)
  {
    Error_Handler();
  }
#endif
  
  /* Start Device Process */
  if (USBD_Start(&hUsbDeviceFS) != USBD_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  De-Initialize the USB device
  * @retval None
  */
void MX_USB_DEVICE_DeInit(void)
{
  /* Stop Device */
  USBD_Stop(&hUsbDeviceFS);
  
  /* DeInit Device Library */
  USBD_DeInit(&hUsbDeviceFS);
}
