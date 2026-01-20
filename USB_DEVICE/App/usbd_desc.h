/**
  ******************************************************************************
  * @file    usbd_desc.h
  * @brief   Header for usbd_desc.c
  * @author  MidiCore
  ******************************************************************************
  */

#ifndef __USBD_DESC__H__
#define __USBD_DESC__H__

#ifdef __cplusplus
 extern "C" {
#endif

#include "usbd_def.h"

/* Descriptor indexes */
#define USBD_IDX_MFC_STR            0x01
#define USBD_IDX_PRODUCT_STR        0x02
#define USBD_IDX_SERIAL_STR         0x03
#define USBD_IDX_CONFIG_STR         0x04
#define USBD_IDX_INTERFACE_STR      0x05

#define USB_SIZ_STRING_SERIAL       0x1A

/** Descriptor for the Device */
extern USBD_DescriptorsTypeDef FS_Desc;

#ifdef __cplusplus
}
#endif

#endif /* __USBD_DESC__H__ */
