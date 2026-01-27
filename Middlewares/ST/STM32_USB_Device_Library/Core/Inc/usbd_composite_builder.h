/**
  ******************************************************************************
  * @file    usbd_composite_builder.h
  * @author  MidiCore Project
  * @brief   USB Composite Device Builder Header
  ******************************************************************************
  * @attention
  *
  * This file provides support for building composite USB devices
  * (e.g., MIDI + CDC) by combining multiple device classes.
  *
  * This is a minimal stub to satisfy compilation requirements.
  * The actual composite device configuration is handled by the
  * individual class drivers (MIDI, CDC) and the main descriptor file.
  *
  ******************************************************************************
  */

#ifndef __USBD_COMPOSITE_BUILDER_H
#define __USBD_COMPOSITE_BUILDER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_def.h"
#include "usbd_core.h"

#ifdef USE_USBD_COMPOSITE

/* Composite Device Configuration */
#define USBD_COMPOSITE_MAX_CLASSES    4

/* Composite Device Class Structure */
typedef struct {
  uint8_t ClassId;
  uint8_t Active;
  uint8_t ClassType;
  void    *pClassData;
} USBD_CompositeClassTypeDef;

/* Composite Device Builder Functions */
uint8_t USBD_Composite_AddClass(USBD_HandleTypeDef *pdev, 
                                 USBD_ClassTypeDef *pclass,
                                 uint8_t cfgidx);

uint8_t USBD_Composite_GetDescriptor(USBD_HandleTypeDef *pdev, 
                                      uint16_t *length);

#endif /* USE_USBD_COMPOSITE */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_COMPOSITE_BUILDER_H */
