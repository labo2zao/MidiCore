/**
  ******************************************************************************
  * @file    usbd_composite_builder.c
  * @author  MidiCore Project
  * @brief   USB Composite Device Builder Implementation
  ******************************************************************************
  * @attention
  *
  * This file provides minimal stub implementation for composite USB devices.
  * 
  * For MidiCore, the composite device (MIDI + CDC) is built using the
  * existing usbd_composite.c file in USB_DEVICE/App/, which provides a
  * complete implementation combining MIDI and CDC classes.
  *
  * This stub is provided to satisfy compilation requirements when
  * USE_USBD_COMPOSITE is defined.
  *
  ******************************************************************************
  */

#include "usbd_composite_builder.h"

#ifdef USE_USBD_COMPOSITE

/* Private variables */
static USBD_CompositeClassTypeDef composite_classes[USBD_COMPOSITE_MAX_CLASSES];
static uint8_t composite_class_count = 0;

/**
  * @brief  Add a class to the composite device
  * @param  pdev: Device handle
  * @param  pclass: Class structure
  * @param  cfgidx: Configuration index
  * @retval Status
  */
uint8_t USBD_Composite_AddClass(USBD_HandleTypeDef *pdev, 
                                 USBD_ClassTypeDef *pclass,
                                 uint8_t cfgidx)
{
  if (composite_class_count >= USBD_COMPOSITE_MAX_CLASSES)
  {
    return USBD_FAIL;
  }
  
  /* Store class information */
  composite_classes[composite_class_count].ClassId = composite_class_count;
  composite_classes[composite_class_count].Active = 1;
  composite_classes[composite_class_count].ClassType = cfgidx;
  composite_classes[composite_class_count].pClassData = pclass;
  
  composite_class_count++;
  
  return USBD_OK;
}

/**
  * @brief  Get composite device configuration descriptor
  * @param  pdev: Device handle
  * @param  length: Pointer to descriptor length
  * @retval Descriptor pointer
  * 
  * @note   This is a stub. Actual descriptor is built by usbd_composite.c
  */
uint8_t USBD_Composite_GetDescriptor(USBD_HandleTypeDef *pdev, 
                                      uint16_t *length)
{
  /* Composite descriptor is handled by usbd_composite.c */
  /* This is just a stub to satisfy linker */
  (void)pdev;
  *length = 0;
  return USBD_OK;
}

#endif /* USE_USBD_COMPOSITE */
