/**
 ******************************************************************************
 * @file    usbd_cdc_if.h
 * @brief   Header for usbd_cdc_if.c file
 * @author  MidiCore Project
 ******************************************************************************
 * @attention
 *
 * USB CDC Interface Layer
 * Connects USB CDC class driver to Services/usb_cdc service layer
 *
 * This file is protected from CubeMX regeneration - do not modify markers
 *
 ******************************************************************************
 */

#ifndef __USBD_CDC_IF_H
#define __USBD_CDC_IF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_cdc.h"

/* External interface structure for CDC callbacks */
extern USBD_CDC_ItfTypeDef USBD_CDC_fops;

/* Internal callback from service layer */
extern void usb_cdc_rx_callback_internal(const uint8_t *buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* __USBD_CDC_IF_H */
