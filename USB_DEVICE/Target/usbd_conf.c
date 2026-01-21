/**
  ******************************************************************************
  * @file    usbd_conf.c
  * @brief   USB Device configuration and callbacks
  * @author  MidiCore (MIOS32-inspired)
  ******************************************************************************
  */

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_midi.h"

/* USB Device handle */
PCD_HandleTypeDef hpcd_USB_OTG_FS;

/*******************************************************************************
                       LL Driver Callbacks (PCD -> USB Device Library)
*******************************************************************************/

/**
  * @brief  Setup stage callback
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_SetupStage((USBD_HandleTypeDef*)hpcd->pData, (uint8_t *)hpcd->Setup);
}

/**
  * @brief  Data Out stage callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_DataOutStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->OUT_ep[epnum].xfer_buff);
}

/**
  * @brief  Data In stage callback
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_DataInStage((USBD_HandleTypeDef*)hpcd->pData, epnum, hpcd->IN_ep[epnum].xfer_buff);
}

/**
  * @brief  SOF callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_SOF((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  Reset callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
{   
  USBD_SpeedTypeDef speed = USBD_SPEED_FULL;
  
  /* Set USB current speed */
  switch (hpcd->Init.speed)
  {
    case PCD_SPEED_HIGH:
      speed = USBD_SPEED_HIGH;
      break;
      
    case PCD_SPEED_FULL:
      speed = USBD_SPEED_FULL;
      break;
      
    default:
      speed = USBD_SPEED_FULL;
      break;
  }
  USBD_LL_SetSpeed((USBD_HandleTypeDef*)hpcd->pData, speed);
  
  /* Reset Device */
  USBD_LL_Reset((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  Suspend callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_Suspend((USBD_HandleTypeDef*)hpcd->pData);
  __HAL_PCD_GATE_PHYCLOCK(hpcd);
  
  /* Enter low power mode */
  if (hpcd->Init.low_power_enable)
  {
    /* Set SLEEPDEEP bit and SleepOnExit of Cortex System Control Register */
    SCB->SCR |= (uint32_t)((uint32_t)(SCB_SCR_SLEEPDEEP_Msk | SCB_SCR_SLEEPONEXIT_Msk));
  }
}

/**
  * @brief  Resume callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_Resume((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  ISOOUTIncomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_IsoOUTIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

/**
  * @brief  ISOINIncomplete callback.
  * @param  hpcd: PCD handle
  * @param  epnum: Endpoint number
  * @retval None
  */
void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
  USBD_LL_IsoINIncomplete((USBD_HandleTypeDef*)hpcd->pData, epnum);
}

/**
  * @brief  Connect callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_DevConnected((USBD_HandleTypeDef*)hpcd->pData);
}

/**
  * @brief  Disconnect callback.
  * @param  hpcd: PCD handle
  * @retval None
  */
void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
{
  USBD_LL_DevDisconnected((USBD_HandleTypeDef*)hpcd->pData);
}

/*******************************************************************************
                       LL Driver Interface (USB Device Library --> PCD)
*******************************************************************************/

/**
  * @brief  Initialize the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
  /* Init USB Ip */
  hpcd_USB_OTG_FS.pData = pdev;
  pdev->pData = &hpcd_USB_OTG_FS;
  
  hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
  hpcd_USB_OTG_FS.Init.dev_endpoints = 4;
  hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
  hpcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;  /* No VBUS sense on STM32F407 */
  hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
  
  if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK)
  {
    Error_Handler();
  }
  
  /* CRITICAL FIX for STM32F407: Force B-Device session valid when VBUS sensing disabled */
  /* This is required because without VBUS detection, the USB core won't start */
  USB_OTG_FS->GCCFG |= USB_OTG_GCCFG_NOVBUSSENS;  /* Disable VBUS sensing */
  USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSBSEN;   /* Disable VBUS "B" sensing */
  USB_OTG_FS->GCCFG &= ~USB_OTG_GCCFG_VBUSASEN;   /* Disable VBUS "A" sensing */
  USB_OTG_FS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOEN; /* Enable B-device valid override */
  USB_OTG_FS->GOTGCTL |= USB_OTG_GOTGCTL_BVALOVAL;/* Force B-session valid */
  
  /* Allocate endpoints for MIDI */
  HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 0x80);
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 0, 0x40);
  HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 1, 0x80);  /* MIDI endpoint */
  
  return USBD_OK;
}

/**
  * @brief  De-Initialize the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;
  
  hal_status = HAL_PCD_DeInit(pdev->pData);
  
  switch (hal_status) {
    case HAL_OK:
      usb_status = USBD_OK;
      break;
    case HAL_ERROR:
      usb_status = USBD_FAIL;
      break;
    case HAL_BUSY:
      usb_status = USBD_BUSY;
      break;
    case HAL_TIMEOUT:
      usb_status = USBD_FAIL;
      break;
    default:
      usb_status = USBD_FAIL;
      break;
  }
  return usb_status;
}

/**
  * @brief  Start the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;
  
  hal_status = HAL_PCD_Start(pdev->pData);
  
  switch (hal_status) {
    case HAL_OK:
      usb_status = USBD_OK;
      break;
    case HAL_ERROR:
      usb_status = USBD_FAIL;
      break;
    case HAL_BUSY:
      usb_status = USBD_BUSY;
      break;
    case HAL_TIMEOUT:
      usb_status = USBD_FAIL;
      break;
    default:
      usb_status = USBD_FAIL;
      break;
  }
  return usb_status;
}

/**
  * @brief  Stop the low level portion of the device driver.
  * @param  pdev: Device handle
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;
  
  hal_status = HAL_PCD_Stop(pdev->pData);
  
  switch (hal_status) {
    case HAL_OK:
      usb_status = USBD_OK;
      break;
    case HAL_ERROR:
      usb_status = USBD_FAIL;
      break;
    case HAL_BUSY:
      usb_status = USBD_BUSY;
      break;
    case HAL_TIMEOUT:
      usb_status = USBD_FAIL;
      break;
    default:
      usb_status = USBD_FAIL;
      break;
  }
  return usb_status;
}

/**
  * @brief  Open and configure an endpoint.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  ep_mps: Endpoint max packet size
  * @param  ep_type: Endpoint type
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t ep_type, uint16_t ep_mps)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;
  
  hal_status = HAL_PCD_EP_Open(pdev->pData, ep_addr, ep_mps, ep_type);
  
  switch (hal_status) {
    case HAL_OK:
      usb_status = USBD_OK;
      break;
    case HAL_ERROR:
      usb_status = USBD_FAIL;
      break;
    case HAL_BUSY:
      usb_status = USBD_BUSY;
      break;
    case HAL_TIMEOUT:
      usb_status = USBD_FAIL;
      break;
    default:
      usb_status = USBD_FAIL;
      break;
  }
  return usb_status;
}

/**
  * @brief  Close an endpoint.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;
  
  hal_status = HAL_PCD_EP_Close(pdev->pData, ep_addr);
  
  switch (hal_status) {
    case HAL_OK:
      usb_status = USBD_OK;
      break;
    case HAL_ERROR:
      usb_status = USBD_FAIL;
      break;
    case HAL_BUSY:
      usb_status = USBD_BUSY;
      break;
    case HAL_TIMEOUT:
      usb_status = USBD_FAIL;
      break;
    default:
      usb_status = USBD_FAIL;
      break;
  }
  return usb_status;
}

/**
  * @brief  Flush an endpoint.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;
  
  hal_status = HAL_PCD_EP_Flush(pdev->pData, ep_addr);
  
  switch (hal_status) {
    case HAL_OK:
      usb_status = USBD_OK;
      break;
    case HAL_ERROR:
      usb_status = USBD_FAIL;
      break;
    case HAL_BUSY:
      usb_status = USBD_BUSY;
      break;
    case HAL_TIMEOUT:
      usb_status = USBD_FAIL;
      break;
    default:
      usb_status = USBD_FAIL;
      break;
  }
  return usb_status;
}

/**
  * @brief  Set a stall condition on an endpoint.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;
  
  hal_status = HAL_PCD_EP_SetStall(pdev->pData, ep_addr);
  
  switch (hal_status) {
    case HAL_OK:
      usb_status = USBD_OK;
      break;
    case HAL_ERROR:
      usb_status = USBD_FAIL;
      break;
    case HAL_BUSY:
      usb_status = USBD_BUSY;
      break;
    case HAL_TIMEOUT:
      usb_status = USBD_FAIL;
      break;
    default:
      usb_status = USBD_FAIL;
      break;
  }
  return usb_status;
}

/**
  * @brief  Clear a stall condition on an endpoint.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;
  
  hal_status = HAL_PCD_EP_ClrStall(pdev->pData, ep_addr);
  
  switch (hal_status) {
    case HAL_OK:
      usb_status = USBD_OK;
      break;
    case HAL_ERROR:
      usb_status = USBD_FAIL;
      break;
    case HAL_BUSY:
      usb_status = USBD_BUSY;
      break;
    case HAL_TIMEOUT:
      usb_status = USBD_FAIL;
      break;
    default:
      usb_status = USBD_FAIL;
      break;
  }
  return usb_status;
}

/**
  * @brief  Return stall condition.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval Stall (1) or not (0)
  */
uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef*) pdev->pData;
  
  if((ep_addr & 0x80) == 0x80)
  {
    return hpcd->IN_ep[ep_addr & 0x7F].is_stall;
  }
  else
  {
    return hpcd->OUT_ep[ep_addr & 0x7F].is_stall;
  }
}

/**
  * @brief  Assign a USB address to the device.
  * @param  pdev: Device handle
  * @param  dev_addr: Device address
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;
  
  hal_status = HAL_PCD_SetAddress(pdev->pData, dev_addr);
  
  switch (hal_status) {
    case HAL_OK:
      usb_status = USBD_OK;
      break;
    case HAL_ERROR:
      usb_status = USBD_FAIL;
      break;
    case HAL_BUSY:
      usb_status = USBD_BUSY;
      break;
    case HAL_TIMEOUT:
      usb_status = USBD_FAIL;
      break;
    default:
      usb_status = USBD_FAIL;
      break;
  }
  return usb_status;
}

/**
  * @brief  Transmit data over an endpoint.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  pbuf: Pointer to data
  * @param  size: Data size
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;
  
  hal_status = HAL_PCD_EP_Transmit(pdev->pData, ep_addr, pbuf, size);
  
  switch (hal_status) {
    case HAL_OK:
      usb_status = USBD_OK;
      break;
    case HAL_ERROR:
      usb_status = USBD_FAIL;
      break;
    case HAL_BUSY:
      usb_status = USBD_BUSY;
      break;
    case HAL_TIMEOUT:
      usb_status = USBD_FAIL;
      break;
    default:
      usb_status = USBD_FAIL;
      break;
  }
  return usb_status;
}

/**
  * @brief  Prepare an endpoint to receive data.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @param  pbuf: Pointer to data
  * @param  size: Data size
  * @retval USBD status
  */
USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep_addr, uint8_t *pbuf, uint32_t size)
{
  HAL_StatusTypeDef hal_status = HAL_OK;
  USBD_StatusTypeDef usb_status = USBD_OK;
  
  hal_status = HAL_PCD_EP_Receive(pdev->pData, ep_addr, pbuf, size);
  
  switch (hal_status) {
    case HAL_OK:
      usb_status = USBD_OK;
      break;
    case HAL_ERROR:
      usb_status = USBD_FAIL;
      break;
    case HAL_BUSY:
      usb_status = USBD_BUSY;
      break;
    case HAL_TIMEOUT:
      usb_status = USBD_FAIL;
      break;
    default:
      usb_status = USBD_FAIL;
      break;
  }
  return usb_status;
}

/**
  * @brief  Return the last transferred packet size.
  * @param  pdev: Device handle
  * @param  ep_addr: Endpoint number
  * @retval Received data size
  */
uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
  return HAL_PCD_EP_GetRxCount((PCD_HandleTypeDef*) pdev->pData, ep_addr);
}

/**
  * @brief  Send LPM message to user layer
  * @param  hpcd: PCD handle
  * @param  msg: LPM message
  * @retval None
  */
void HAL_PCDEx_LPM_Callback(PCD_HandleTypeDef *hpcd, PCD_LPM_MsgTypeDef msg)
{
  /* USER CODE BEGIN LPM_Callback */
  switch (msg)
  {
  case PCD_LPM_L0_ACTIVE:
    break;
    
  case PCD_LPM_L1_ACTIVE:
    break;
    
  default:
    break;
  }
  /* USER CODE END LPM_Callback */
}

/**
  * @brief  Delay routine for the USB Device Library.
  * @param  Delay: Delay in ms
  * @retval None
  */
void USBD_LL_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}

/**
  * @brief  Static memory allocation routine.
  * @param  size: Buffer size
  * @retval Pointer to the allocated buffer
  */
void *USBD_static_malloc(uint32_t size)
{
  static uint32_t mem[(sizeof(USBD_MIDI_HandleTypeDef)/4)+1];/* On 32-bit boundary */
  return mem;
}

/**
  * @brief  Dummy memory free routine.
  * @param  p: Pointer to be freed
  * @retval None
  */
void USBD_static_free(void *p)
{
  /* Nothing to do - static allocation */
}
