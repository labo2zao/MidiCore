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

/* Descriptor indexes already defined in usbd_def.h */
/* Additional defines if needed */
#define USB_SIZ_STRING_SERIAL       0x1A

/** Descriptor for the Device */
extern USBD_DescriptorsTypeDef FS_Desc;

#ifdef __cplusplus
}
#endif

#endif /* __USBD_DESC__H__ */
