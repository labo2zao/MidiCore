/**
  ******************************************************************************
  * @file    usbd_midi.h
  * @author  MidiCore Project
  * @brief   Header file for the usbd_midi.c file
  ******************************************************************************
  * @attention
  *
  * USB MIDI Device Class - 4 Port (4x4) Interface
  * Based on USB Device Class Definition for MIDI Devices v1.0
  *
  ******************************************************************************
  */

#ifndef __USBD_MIDI_H
#define __USBD_MIDI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_ioreq.h"

/* MIDI Device Class Codes */
#define USB_DEVICE_CLASS_AUDIO                  0x01
#define AUDIO_SUBCLASS_AUDIOCONTROL             0x01
#define AUDIO_SUBCLASS_MIDISTREAMING            0x03

/* MIDI Descriptor Types */
#define AUDIO_DESCRIPTOR_TYPE_INTERFACE         0x24
#define AUDIO_DESCRIPTOR_TYPE_ENDPOINT          0x25

/* MIDI Jack Types */
#define MIDI_JACK_TYPE_EMBEDDED                 0x01
#define MIDI_JACK_TYPE_EXTERNAL                 0x02

/* MIDI Endpoint Sizes */
#define MIDI_DATA_FS_MAX_PACKET_SIZE            64  /* Full Speed: 64 bytes max */
#define MIDI_DATA_HS_MAX_PACKET_SIZE            512 /* High Speed: 512 bytes max */

/* Number of MIDI Ports (Cables) - 4x4 interface like MIOS32 */
#define MIDI_NUM_PORTS                          4

/* MIDI Endpoints */
#define MIDI_OUT_EP                             0x01  /* Endpoint 1 OUT */
#define MIDI_IN_EP                              0x81  /* Endpoint 1 IN */

/* MIDI Buffer Sizes */
#define MIDI_DATA_OUT_MAX_PACKET_SIZE           (MIDI_DATA_FS_MAX_PACKET_SIZE * MIDI_NUM_PORTS)
#define MIDI_DATA_IN_MAX_PACKET_SIZE            (MIDI_DATA_FS_MAX_PACKET_SIZE * MIDI_NUM_PORTS)

/* USB MIDI Event Packet (4 bytes) */
typedef struct
{
  uint8_t header;  /* Cable Number (4 bits) + Code Index Number (4 bits) */
  uint8_t byte1;
  uint8_t byte2;
  uint8_t byte3;
} __attribute__((packed)) USBD_MIDI_EventPacket_t;

/* MIDI Device Class Handle */
typedef struct
{
  uint8_t  data_out[MIDI_DATA_OUT_MAX_PACKET_SIZE];
  uint8_t  data_in[MIDI_DATA_IN_MAX_PACKET_SIZE];
  uint32_t data_out_length;
  uint32_t data_in_length;
  uint8_t  is_ready;
} USBD_MIDI_HandleTypeDef;

/* Callback function prototypes */
typedef struct
{
  void (*Init)(void);
  void (*DeInit)(void);
  void (*DataOut)(USBD_MIDI_EventPacket_t *packet);
} USBD_MIDI_ItfTypeDef;

/* External Class Definition */
extern USBD_ClassTypeDef USBD_MIDI;

/* Function Prototypes */
uint8_t USBD_MIDI_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MIDI_ItfTypeDef *fops);
uint8_t USBD_MIDI_SendData(USBD_HandleTypeDef *pdev, uint8_t cable, uint8_t *data, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif  /* __USBD_MIDI_H */
