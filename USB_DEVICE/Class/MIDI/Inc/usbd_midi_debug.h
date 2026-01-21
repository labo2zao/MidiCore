/**
 ******************************************************************************
 * @file    usbd_midi_debug.h
 * @brief   USB MIDI Debug Instrumentation Header
 ******************************************************************************
 * SIMPLE UART DEBUG - No build flags needed!
 * 
 * To enable debug output:
 * 1. Uncomment the line below: #define USBD_MIDI_DEBUG
 * 2. Make sure you have printf working via UART (retarget printf to UART)
 * 3. Rebuild and flash
 * 
 * Debug output will appear on your UART terminal (e.g., 115200 baud)
 ******************************************************************************
 */

#ifndef __USBD_MIDI_DEBUG_H
#define __USBD_MIDI_DEBUG_H

#include <stdint.h>

/* ============================================ */
/* ENABLE DEBUG HERE - Just uncomment this line */
/* ============================================ */
// #define USBD_MIDI_DEBUG
/* ============================================ */

#ifdef USBD_MIDI_DEBUG

#include <stdio.h>  /* For printf - make sure UART printf is working */

void USBD_MIDI_DebugSetupRequest(uint8_t bmRequest, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength);
void USBD_MIDI_DebugDescriptor(const char *name, const uint8_t *data, uint16_t len);
void USBD_MIDI_DebugState(const char *state);

#define DEBUG_SETUP(bmReq, bReq, wVal, wIdx, wLen) USBD_MIDI_DebugSetupRequest(bmReq, bReq, wVal, wIdx, wLen)
#define DEBUG_DESCRIPTOR(name, data, len) USBD_MIDI_DebugDescriptor(name, data, len)
#define DEBUG_STATE(state) USBD_MIDI_DebugState(state)

#else

#define DEBUG_SETUP(bmReq, bReq, wVal, wIdx, wLen)
#define DEBUG_DESCRIPTOR(name, data, len)
#define DEBUG_STATE(state)

#endif /* USBD_MIDI_DEBUG */

#endif /* __USBD_MIDI_DEBUG_H */
