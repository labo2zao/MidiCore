/**
 ******************************************************************************
 * @file    usb_msc.h
 * @brief   USB Mass Storage Class (MSC) Service API
 * @author  MidiCore Project
 ******************************************************************************
 * @attention
 *
 * USB MSC - Exposes SD card as USB Mass Storage device
 * Compatible with MIOS Studio for direct SD card file editing
 * 
 * Features:
 * - SD card exposed as removable disk
 * - Safe arbitration with firmware SD access
 * - Automatic locking when host mounts
 * - MIOS Studio integration
 * 
 * Integration:
 * - Enable MODULE_ENABLE_USB_MSC in Config/module_config.h
 * - Configure CubeMX with USB_OTG_FS
 * - Call usb_msc_init() during startup
 * - Monitor connection state with usb_msc_is_mounted()
 * 
 ******************************************************************************
 */

#ifndef __USB_MSC_H
#define __USB_MSC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Return codes for USB MSC operations */
#define USB_MSC_OK              0
#define USB_MSC_ERROR           -1
#define USB_MSC_BUSY            -2
#define USB_MSC_NOT_READY       -3

/**
 * @brief Initialize USB MSC service
 * 
 * Call once during startup after USB Device is initialized.
 * Registers MSC interface callbacks with USB stack.
 * 
 * @note Must be called after MX_USB_DEVICE_Init() and FATFS initialization
 */
void usb_msc_init(void);

/**
 * @brief Check if USB MSC is mounted by host
 * @return 1 if mounted, 0 otherwise
 * 
 * When mounted:
 * - Host has recognized device as mass storage
 * - Firmware should NOT access SD card
 * - All file operations suspended until unmount
 */
uint8_t usb_msc_is_mounted(void);

/**
 * @brief Register callback for mount/unmount events
 * @param callback Function to call when mount state changes
 * 
 * Callback receives: mounted (1=mounted, 0=unmounted)
 * Use this to pause/resume firmware SD card operations
 */
typedef void (*usb_msc_mount_callback_t)(uint8_t mounted);
void usb_msc_register_mount_callback(usb_msc_mount_callback_t callback);

/**
 * @brief Check if SD card is safe to access by firmware
 * @return 1 if safe, 0 if USB has control
 * 
 * Always check this before any SD card operations in firmware
 */
uint8_t usb_msc_can_access_sd(void);

/* ============================================================================
 * MidiCore Compatibility Shims
 * ============================================================================ */

/**
 * @brief MIOS32-compatible initialization
 * @return 0 on success (MidiCore convention)
 */
static inline int32_t MIOS32_USB_MSC_Init(void) {
  usb_msc_init();
  return USB_MSC_OK;
}

/**
 * @brief MIOS32-compatible mount status check
 * @return 1 if mounted, 0 otherwise
 */
static inline uint8_t MIOS32_USB_MSC_IsMounted(void) {
  return usb_msc_is_mounted();
}

#ifdef __cplusplus
}
#endif

#endif /* __USB_MSC_H */
