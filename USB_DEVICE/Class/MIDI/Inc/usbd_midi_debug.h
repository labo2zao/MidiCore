/**
 ******************************************************************************
 * @file    usbd_midi_debug.h
 * @brief   USB MIDI Debug Instrumentation Header
 ******************************************************************************
 * SIMPLE UART DEBUG - No build flags needed!
 * 
 * To enable debug output (choose ONE method):
 * 
 * METHOD 1: Via module_config.h (recommended for project integration)
 *   - Open Config/module_config.h
 *   - Set MODULE_ENABLE_USB_MIDI_DEBUG to 1
 *   - Rebuild
 * 
 * METHOD 2: Directly in this file (quick testing)
 *   - Uncomment the line below: #define USBD_MIDI_DEBUG
 *   - Rebuild
 * 
 * Requirements:
 *   - printf must be redirected to UART
 *   - UART terminal connected (e.g., 115200 baud)
 * 
 * Debug output will appear on your UART terminal showing:
 *   - USB setup requests from Windows
 *   - Descriptor data being sent
 *   - Enumeration state changes
 ******************************************************************************
 */

#ifndef __USBD_MIDI_DEBUG_H
#define __USBD_MIDI_DEBUG_H

#include <stdint.h>

/* Check module configuration first */
#ifdef MODULE_ENABLE_USB_MIDI_DEBUG
  #if MODULE_ENABLE_USB_MIDI_DEBUG
    #define USBD_MIDI_DEBUG
  #endif
#endif



#ifdef USBD_MIDI_DEBUG

#include <stdio.h>  /* For printf - make sure UART printf is working */
#include "App/tests/test_debug.h"  /* For dbg_print (dbg_printf removed) */

void USBD_MIDI_DebugSetupRequest(uint8_t bmRequest, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength);
void USBD_MIDI_DebugDescriptor(const char *name, const uint8_t *data, uint16_t len);
void USBD_MIDI_DebugState(const char *state);

#define DEBUG_SETUP(bmReq, bReq, wVal, wIdx, wLen) USBD_MIDI_DebugSetupRequest(bmReq, bReq, wVal, wIdx, wLen)
#define DEBUG_DESCRIPTOR(name, data, len) USBD_MIDI_DebugDescriptor(name, data, len)
#define DEBUG_STATE(state) USBD_MIDI_DebugState(state)
/* DEBUG_PRINT disabled - dbg_printf removed to prevent stack overflow */
#define DEBUG_PRINT(...)

#else

#define DEBUG_SETUP(bmReq, bReq, wVal, wIdx, wLen)
#define DEBUG_DESCRIPTOR(name, data, len)
#define DEBUG_STATE(state)
#define DEBUG_PRINT(...)

#endif /* USBD_MIDI_DEBUG */

#endif /* __USBD_MIDI_DEBUG_H */
