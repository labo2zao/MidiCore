/**
  ******************************************************************************
  * @file    usb_device.h
  * @brief   Header for usb_device.c file.
  * @author  MidiCore (MIOS32-inspired USB MIDI Device)
  ******************************************************************************
  */

#ifndef __USB_DEVICE__H__
#define __USB_DEVICE__H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "usbd_def.h"

/** @addtogroup USB_DEVICE_Library
  * @{
  */

/** USB Device handle */
extern USBD_HandleTypeDef hUsbDeviceFS;

/**
  * @brief Initialize the USB Device Library
  * @retval None
  */
void MX_USB_DEVICE_Init(void);

/**
  * @brief DeInitialize the USB Device Library
  * @retval None
  */
void MX_USB_DEVICE_DeInit(void);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USB_DEVICE__H__ */
