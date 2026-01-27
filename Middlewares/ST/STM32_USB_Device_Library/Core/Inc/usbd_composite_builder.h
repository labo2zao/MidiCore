/**
  ******************************************************************************
  * @file    usbd_composite_builder.h
  * @author  MidiCore Project
  * @brief   USB Composite Device Builder - STM32 Middleware Compatible
  ******************************************************************************
  * @attention
  *
  * This file provides the composite device builder required by STM32 USB
  * middleware when USE_USBD_COMPOSITE is defined.
  *
  * The middleware expects specific functions and structures which are
  * provided here and delegate to the actual composite implementation
  * in USB_DEVICE/App/usbd_composite.c
  *
  * Original implementation for MidiCore - commercially licensable.
  *
  ******************************************************************************
  */

#ifndef __USBD_COMPOSITE_BUILDER_H
#define __USBD_COMPOSITE_BUILDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_def.h"

#ifdef USE_USBD_COMPOSITE

/* Composite Device Class Structure (Required by middleware) */
extern USBD_ClassTypeDef USBD_CMPSIT;

/* Composite Builder Functions (Required by middleware) */
uint8_t USBD_CMPSIT_AddClass(USBD_HandleTypeDef *pdev,
                              USBD_ClassTypeDef *pclass,
                              USBD_CompositeClassTypeDef class_type,
                              uint8_t cfgidx);

uint8_t USBD_CMPST_ClearConfDesc(USBD_HandleTypeDef *pdev);

/* Configuration Descriptor Getters */
uint8_t *USBD_CMPSIT_GetFSConfigDescriptor(uint16_t *length);
uint8_t *USBD_CMPSIT_GetHSConfigDescriptor(uint16_t *length);

#endif /* USE_USBD_COMPOSITE */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_COMPOSITE_BUILDER_H */
