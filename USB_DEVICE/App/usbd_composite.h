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

/* Composite class data helpers */
void *USBD_COMPOSITE_GetClassData(const USBD_ClassTypeDef *class_handler);
void *USBD_COMPOSITE_SwitchClassData(USBD_HandleTypeDef *pdev, void *new_data);

#ifdef __cplusplus
}
#endif

#endif /* __USBD_COMPOSITE_H */
