/**
 ******************************************************************************
 * @file    usbd_msc.h
 * @brief   USB MSC (Mass Storage Class) - Bulk-Only Transport
 * @author  MidiCore Project
 ******************************************************************************
 * @attention
 *
 * USB Mass Storage Class - Bulk-Only Transport (BOT) Protocol
 * Based on USB Mass Storage Class Specification 1.0
 * 
 * Note: This is a simplified stub implementation for structure.
 * Full implementation requires SCSI command handling.
 *
 ******************************************************************************
 */

#ifndef __USBD_MSC_H
#define __USBD_MSC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_ioreq.h"

/* MSC Class Codes */
#define USB_MSC_CLASS                           0x08
#define MSC_SUBCLASS_SCSI_TRANSPARENT           0x06
#define MSC_PROTOCOL_BULK_ONLY                  0x50

/* MSC Endpoints - Must not conflict with MIDI (0x01) and CDC (0x02, 0x83) */
#define MSC_IN_EP                               0x84  /* Endpoint 4 IN */
#define MSC_OUT_EP                              0x04  /* Endpoint 4 OUT */

/* MSC Endpoint Sizes */
#define MSC_DATA_FS_MAX_PACKET_SIZE             64    /* Full Speed: 64 bytes */

/* MSC Device Class Handle (stub) */
typedef struct {
  uint8_t  bot_state;
} USBD_MSC_HandleTypeDef;

/* Callback function prototypes (stub) */
typedef struct {
  int8_t (*Init)(uint8_t lun);
  int8_t (*GetCapacity)(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
  int8_t (*IsReady)(uint8_t lun);
  int8_t (*Read)(uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
  int8_t (*Write)(uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
} USBD_MSC_ItfTypeDef;

/* External Class Definition */
extern USBD_ClassTypeDef USBD_MSC;
extern USBD_MSC_ItfTypeDef USBD_MSC_fops;

/* Function Prototypes */
uint8_t USBD_MSC_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MSC_ItfTypeDef *fops);

#ifdef __cplusplus
}
#endif

#endif /* __USBD_MSC_H */
