/**
  ******************************************************************************
  * @file    usbd_composite_builder.c
  * @author  MidiCore Project
  * @brief   USB Composite Device Builder Implementation
  ******************************************************************************
  * @attention
  *
  * This file provides the interface expected by the STM32 USB middleware
  * and delegates to the actual composite device implementation in
  * USB_DEVICE/App/usbd_composite.c
  *
  * Original implementation for MidiCore - commercially licensable.
  *
  ******************************************************************************
  */

#include "usbd_composite_builder.h"

#ifdef USE_USBD_COMPOSITE

#include "usbd_core.h"
#include "USB_DEVICE/App/usbd_composite.h"

/* Private variables */
static uint8_t composite_class_count = 0;

/**
  * @brief  Add a class to the composite device
  * @param  pdev: Device handle
  * @param  pclass: Class to add
  * @param  class_type: Class type enum
  * @param  cfgidx: Configuration index
  * @retval Status
  */
uint8_t USBD_CMPSIT_AddClass(USBD_HandleTypeDef *pdev,
                              USBD_ClassTypeDef *pclass,
                              USBD_CompositeClassTypeDef class_type,
                              uint8_t cfgidx)
{
  (void)pdev;
  (void)pclass;
  (void)class_type;
  (void)cfgidx;
  
  composite_class_count++;
  
  return USBD_OK;
}

/**
  * @brief  Clear composite configuration descriptor
  * @param  pdev: Device handle
  * @retval Status
  */
uint8_t USBD_CMPST_ClearConfDesc(USBD_HandleTypeDef *pdev)
{
  (void)pdev;
  
  composite_class_count = 0;
  
  return USBD_OK;
}

/**
  * @brief  Get FS configuration descriptor
  * @param  length: Pointer to descriptor length
  * @retval Descriptor pointer
  * 
  * Delegates to the real composite implementation.
  */
uint8_t *USBD_CMPSIT_GetFSConfigDescriptor(uint16_t *length)
{
  /* Use the actual composite implementation from USBD_COMPOSITE class */
  if (USBD_COMPOSITE.GetFSConfigDescriptor != NULL) {
    return USBD_COMPOSITE.GetFSConfigDescriptor(length);
  }
  
  /* Fallback if not available */
  *length = 0;
  return NULL;
}

/**
  * @brief  Get HS configuration descriptor
  * @param  length: Pointer to descriptor length
  * @retval Descriptor pointer
  */
uint8_t *USBD_CMPSIT_GetHSConfigDescriptor(uint16_t *length)
{
  /* Use the actual composite implementation from USBD_COMPOSITE class */
  if (USBD_COMPOSITE.GetHSConfigDescriptor != NULL) {
    return USBD_COMPOSITE.GetHSConfigDescriptor(length);
  }
  
  /* Same as FS for Full Speed device */
  return USBD_CMPSIT_GetFSConfigDescriptor(length);
}

/* ========================================================================
   USBD_CMPSIT Class Structure
   
   This is the class structure expected by the STM32 middleware.
   It delegates all operations to the real USBD_COMPOSITE implementation.
   ======================================================================== */

static uint8_t USBD_CMPSIT_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  if (USBD_COMPOSITE.Init != NULL) {
    return USBD_COMPOSITE.Init(pdev, cfgidx);
  }
  return USBD_OK;
}

static uint8_t USBD_CMPSIT_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  if (USBD_COMPOSITE.DeInit != NULL) {
    return USBD_COMPOSITE.DeInit(pdev, cfgidx);
  }
  return USBD_OK;
}

static uint8_t USBD_CMPSIT_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  if (USBD_COMPOSITE.Setup != NULL) {
    return USBD_COMPOSITE.Setup(pdev, req);
  }
  return USBD_OK;
}

static uint8_t USBD_CMPSIT_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  if (USBD_COMPOSITE.DataIn != NULL) {
    return USBD_COMPOSITE.DataIn(pdev, epnum);
  }
  return USBD_OK;
}

static uint8_t USBD_CMPSIT_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  if (USBD_COMPOSITE.DataOut != NULL) {
    return USBD_COMPOSITE.DataOut(pdev, epnum);
  }
  return USBD_OK;
}

static uint8_t USBD_CMPSIT_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  if (USBD_COMPOSITE.EP0_RxReady != NULL) {
    return USBD_COMPOSITE.EP0_RxReady(pdev);
  }
  return USBD_OK;
}

static uint8_t USBD_CMPSIT_EP0_TxSent(USBD_HandleTypeDef *pdev)
{
  if (USBD_COMPOSITE.EP0_TxSent != NULL) {
    return USBD_COMPOSITE.EP0_TxSent(pdev);
  }
  return USBD_OK;
}

static uint8_t USBD_CMPSIT_SOF(USBD_HandleTypeDef *pdev)
{
  if (USBD_COMPOSITE.SOF != NULL) {
    return USBD_COMPOSITE.SOF(pdev);
  }
  return USBD_OK;
}

static uint8_t *USBD_CMPSIT_GetFSCfgDesc(uint16_t *length)
{
  return USBD_CMPSIT_GetFSConfigDescriptor(length);
}

/* Composite Device Class Structure (Required by middleware) */
USBD_ClassTypeDef USBD_CMPSIT =
{
  USBD_CMPSIT_Init,
  USBD_CMPSIT_DeInit,
  USBD_CMPSIT_Setup,
  USBD_CMPSIT_EP0_TxSent,
  USBD_CMPSIT_EP0_RxReady,
  USBD_CMPSIT_DataIn,
  USBD_CMPSIT_DataOut,
  USBD_CMPSIT_SOF,
  NULL, /* IsoINIncomplete */
  NULL, /* IsoOUTIncomplete */
  NULL, /* GetHSConfigDescriptor */
  USBD_CMPSIT_GetFSCfgDesc, /* GetFSConfigDescriptor */
  NULL, /* GetOtherSpeedConfigDescriptor */
  NULL, /* GetDeviceQualifierDescriptor */
};

#endif /* USE_USBD_COMPOSITE */

