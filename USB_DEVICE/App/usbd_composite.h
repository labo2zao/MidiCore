/**
 ******************************************************************************
 * @file    usbd_composite.h
 * @brief   USB Composite Device Class (MIDI + CDC)
 * @author  MidiCore Project
 ******************************************************************************
 * @attention
 *
 * Composite USB Device Class - Combines MIDI and CDC
 * Manages both classes in a single USB device configuration
 *
 ******************************************************************************
 */

#ifndef __USBD_COMPOSITE_H
#define __USBD_COMPOSITE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_ioreq.h"
#include "Config/module_config.h"

/* External composite class definition */
extern USBD_ClassTypeDef USBD_COMPOSITE;

/* Function to get active class based on interface number */
USBD_ClassTypeDef* USBD_COMPOSITE_GetClass(uint8_t interface_num);

#ifdef __cplusplus
}
#endif

#endif /* __USBD_COMPOSITE_H */
