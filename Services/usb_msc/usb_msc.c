/**
 ******************************************************************************
 * @file    usb_msc.c
 * @brief   USB Mass Storage Class (MSC) Service Implementation
 * @author  MidiCore Project
 ******************************************************************************
 * @attention
 *
 * USB MSC service implementation with SD card arbitration
 * Prevents SD corruption by locking access when USB host mounts
 * 
 ******************************************************************************
 */

#include "Services/usb_msc/usb_msc.h"
#include "Config/module_config.h"
#include <string.h>

#if MODULE_ENABLE_USB_MSC

/* Include USB Device stack and MSC class */
#include "usb_device.h"
#include "USB_DEVICE/Class/MSC/Inc/usbd_msc.h"

/* External USB Device handle */
extern USBD_HandleTypeDef hUsbDeviceFS;

/* Private variables */
static usb_msc_mount_callback_t mount_callback = NULL;
static uint8_t is_mounted = 0;
static uint8_t is_initialized = 0;

/* ============================================================================
 * Service API Implementation
 * ============================================================================ */

void usb_msc_init(void) {
  if (is_initialized) {
    return; /* Already initialized */
  }
  
  /* MSC interface is registered in usb_device.c via composite class */
  /* No additional registration needed here */
  
  is_initialized = 1;
  is_mounted = 0;
}

uint8_t usb_msc_is_mounted(void) {
  return is_mounted;
}

void usb_msc_register_mount_callback(usb_msc_mount_callback_t callback) {
  mount_callback = callback;
}

uint8_t usb_msc_can_access_sd(void) {
  /* SD is safe to access if USB is not mounted */
  return !is_mounted;
}

/* ============================================================================
 * Internal Callbacks (called from usbd_msc_if.c)
 * ============================================================================ */

/**
 * @brief Internal callback for mount state change
 * @param mounted 1 if mounted, 0 if unmounted
 * 
 * Called from USB interrupt context via usbd_msc driver
 */
void usb_msc_mount_callback_internal(uint8_t mounted) {
  uint8_t old_state = is_mounted;
  is_mounted = mounted;
  
  /* Notify application if state changed */
  if (old_state != mounted && mount_callback != NULL) {
    mount_callback(mounted);
  }
}

#else /* !MODULE_ENABLE_USB_MSC */

/* Stub implementations when MSC is disabled */
void usb_msc_init(void) {}
uint8_t usb_msc_is_mounted(void) { return 0; }
void usb_msc_register_mount_callback(usb_msc_mount_callback_t callback) { (void)callback; }
uint8_t usb_msc_can_access_sd(void) { return 1; }
void usb_msc_mount_callback_internal(uint8_t mounted) { (void)mounted; }

#endif /* MODULE_ENABLE_USB_MSC */
