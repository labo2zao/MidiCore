/**
  ******************************************************************************
  * @file    usbd_composite_builder.c
  * @author  MidiCore Project
  * @brief   USB Composite Device Builder Implementation
  ******************************************************************************
  * @attention
  *
  * Minimal implementation of STM32 USB composite device builder.
  * 
  * The actual composite device (MIDI + CDC) is built in:
  * USB_DEVICE/App/usbd_composite.c
  *
  * This file provides the interface expected by the STM32 USB middleware.
  * Original implementation for MidiCore - commercially licensable.
  *
  ******************************************************************************
  */

#include "usbd_composite_builder.h"

#ifdef USE_USBD_COMPOSITE

#include "usbd_core.h"

/* External reference to actual composite descriptor (if implemented elsewhere) */
extern uint8_t *USBD_Composite_GetFSConfigDescriptor(uint16_t *length) __attribute__((weak));

/* Forward declarations for composite class callbacks */
static uint8_t USBD_CMPSIT_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_CMPSIT_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_CMPSIT_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_CMPSIT_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_CMPSIT_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_CMPSIT_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_CMPSIT_EP0_TxSent(USBD_HandleTypeDef *pdev);
static uint8_t USBD_CMPSIT_SOF(USBD_HandleTypeDef *pdev);
static uint8_t *USBD_CMPSIT_GetFSCfgDesc(uint16_t *length);

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
  */
uint8_t *USBD_CMPSIT_GetFSConfigDescriptor(uint16_t *length)
{
  /* Delegate to actual composite implementation if available */
  if (USBD_Composite_GetFSConfigDescriptor != NULL) {
    return USBD_Composite_GetFSConfigDescriptor(length);
  }
  
  /* Fallback: return empty descriptor */
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
  /* Same as FS for Full Speed device */
  return USBD_CMPSIT_GetFSConfigDescriptor(length);
}

/* ========================================================================
   Composite Class Callbacks (Stub Implementation)
   Real implementation handled by individual class drivers
   ======================================================================== */

static uint8_t USBD_CMPSIT_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  (void)pdev;
  (void)cfgidx;
  return USBD_OK;
}

static uint8_t USBD_CMPSIT_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  (void)pdev;
  (void)cfgidx;
  return USBD_OK;
}

static uint8_t USBD_CMPSIT_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  (void)pdev;
  (void)req;
  return USBD_OK;
}

static uint8_t USBD_CMPSIT_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  (void)pdev;
  (void)epnum;
  return USBD_OK;
}

static uint8_t USBD_CMPSIT_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  (void)pdev;
  (void)epnum;
  return USBD_OK;
}

static uint8_t USBD_CMPSIT_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  (void)pdev;
  return USBD_OK;
}

static uint8_t USBD_CMPSIT_EP0_TxSent(USBD_HandleTypeDef *pdev)
{
  (void)pdev;
  return USBD_OK;
}

static uint8_t USBD_CMPSIT_SOF(USBD_HandleTypeDef *pdev)
{
  (void)pdev;
  return USBD_OK;
}

static uint8_t *USBD_CMPSIT_GetFSCfgDesc(uint16_t *length)
{
  return USBD_CMPSIT_GetFSConfigDescriptor(length);
}

#endif /* USE_USBD_COMPOSITE */
