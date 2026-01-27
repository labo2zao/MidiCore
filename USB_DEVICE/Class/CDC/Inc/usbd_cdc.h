/**
 ******************************************************************************
 * @file    usbd_cdc.h
 * @author  MidiCore Project
 * @brief   Header file for the usbd_cdc.c file
 ******************************************************************************
 * @attention
 *
 * USB CDC Device Class - ACM (Abstract Control Model)
 * Implements Virtual COM Port functionality
 * Based on USB Device Class Definition for Communications Devices v1.2
 *
 * This file is protected from CubeMX regeneration - do not modify markers
 *
 ******************************************************************************
 */

#ifndef __USBD_CDC_H
#define __USBD_CDC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_ioreq.h"

/* CDC Device Class Codes */
#define USB_DEVICE_CLASS_CDC                    0x02
#define CDC_COMMUNICATION_INTERFACE_CLASS       0x02

/* CDC Subclass Codes */
#define CDC_ABSTRACT_CONTROL_MODEL              0x02

/* CDC Protocol Codes */
#define CDC_PROTOCOL_COMMON_AT_COMMANDS         0x01
#define CDC_PROTOCOL_VENDOR_SPECIFIC            0xFF

/* CDC Descriptor Types */
#define CDC_DESCRIPTOR_TYPE_CS_INTERFACE        0x24
#define CDC_DESCRIPTOR_TYPE_CS_ENDPOINT         0x25

/* CDC Descriptor Subtypes */
#define CDC_DESCRIPTOR_SUBTYPE_HEADER           0x00
#define CDC_DESCRIPTOR_SUBTYPE_CALL_MANAGEMENT  0x01
#define CDC_DESCRIPTOR_SUBTYPE_ACM              0x02
#define CDC_DESCRIPTOR_SUBTYPE_UNION            0x06

/* CDC Requests */
#define CDC_SEND_ENCAPSULATED_COMMAND           0x00
#define CDC_GET_ENCAPSULATED_RESPONSE           0x01
#define CDC_SET_COMM_FEATURE                    0x02
#define CDC_GET_COMM_FEATURE                    0x03
#define CDC_CLEAR_COMM_FEATURE                  0x04
#define CDC_SET_LINE_CODING                     0x20
#define CDC_GET_LINE_CODING                     0x21
#define CDC_SET_CONTROL_LINE_STATE              0x22
#define CDC_SEND_BREAK                          0x23

/* CDC Endpoint Numbers - Must not conflict with MIDI (EP 0x01/0x81) */
#define CDC_IN_EP                               0x82  /* Endpoint 2 IN (Bulk Data) */
#define CDC_OUT_EP                              0x02  /* Endpoint 2 OUT (Bulk Data) */
#define CDC_CMD_EP                              0x83  /* Endpoint 3 IN (Interrupt for notifications) */

/* CDC Endpoint Sizes */
#define CDC_DATA_FS_MAX_PACKET_SIZE             64    /* Full Speed: 64 bytes max for bulk */
#define CDC_CMD_PACKET_SIZE                     8     /* Interrupt endpoint for notifications */

/* CDC Buffer Sizes */
#define CDC_DATA_OUT_MAX_PACKET_SIZE            CDC_DATA_FS_MAX_PACKET_SIZE
#define CDC_DATA_IN_MAX_PACKET_SIZE             CDC_DATA_FS_MAX_PACKET_SIZE

/* Line Coding Structure (format of SET_LINE_CODING / GET_LINE_CODING data) */
typedef struct {
  uint32_t bitrate;    /* Baud rate in bits per second */
  uint8_t  format;     /* Stop bits: 0=1, 1=1.5, 2=2 */
  uint8_t  parity;     /* Parity: 0=None, 1=Odd, 2=Even, 3=Mark, 4=Space */
  uint8_t  databits;   /* Data bits: 5, 6, 7, 8, 16 */
} __attribute__((packed)) USBD_CDC_LineCodingTypeDef;

/* CDC Device Class Handle */
typedef struct {
  uint8_t  data_out[CDC_DATA_OUT_MAX_PACKET_SIZE];
  uint8_t  data_in[CDC_DATA_IN_MAX_PACKET_SIZE];
  uint8_t  cmd_data[CDC_CMD_PACKET_SIZE];
  uint32_t data_out_length;
  uint32_t data_in_length;
  uint8_t  tx_state;
  uint8_t  rx_state;
  USBD_CDC_LineCodingTypeDef line_coding;
  uint16_t control_line_state; /* DTR/RTS state bitmap */
} USBD_CDC_HandleTypeDef;

/* Callback function prototypes */
typedef struct {
  int8_t (*Init)(void);
  int8_t (*DeInit)(void);
  int8_t (*Control)(uint8_t cmd, uint8_t *pbuf, uint16_t length);
  int8_t (*Receive)(uint8_t *buf, uint32_t *len);
  int8_t (*TransmitCplt)(uint8_t *buf, uint32_t *len, uint8_t epnum);
} USBD_CDC_ItfTypeDef;

/* External Class Definition */
extern USBD_ClassTypeDef USBD_CDC;
extern USBD_CDC_ItfTypeDef USBD_CDC_fops;

/* Function Prototypes */
uint8_t USBD_CDC_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_CDC_ItfTypeDef *fops);
uint8_t USBD_CDC_SetTxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff, uint32_t length);
uint8_t USBD_CDC_SetRxBuffer(USBD_HandleTypeDef *pdev, uint8_t *pbuff);
uint8_t USBD_CDC_ReceivePacket(USBD_HandleTypeDef *pdev);
uint8_t USBD_CDC_TransmitPacket(USBD_HandleTypeDef *pdev);
uint8_t USBD_CDC_TransmitData(USBD_HandleTypeDef *pdev, uint8_t *buf, uint16_t length);
uint8_t USBD_CDC_IsConnected(USBD_HandleTypeDef *pdev);

#ifdef __cplusplus
}
#endif

#endif  /* __USBD_CDC_H */
