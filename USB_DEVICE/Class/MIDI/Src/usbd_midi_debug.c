/**
 ******************************************************************************
 * @file    usbd_midi_debug.c
 * @brief   USB MIDI Debug Instrumentation
 * @author  MidiCore
 ******************************************************************************
 * Debug helpers to trace USB enumeration and descriptor requests
 * Enable by defining USBD_MIDI_DEBUG in your build
 ******************************************************************************
 */

#ifdef USBD_MIDI_DEBUG

#include <stdio.h>
#include <string.h>

/* Debug output buffer */
static char debug_buffer[256];

/**
 * @brief  Log USB setup request
 */
void USBD_MIDI_DebugSetupRequest(uint8_t bmRequest, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength)
{
    snprintf(debug_buffer, sizeof(debug_buffer),
             "USB Setup: bmRequest=0x%02X, bRequest=0x%02X, wValue=0x%04X, wIndex=0x%04X, wLength=%u\r\n",
             bmRequest, bRequest, wValue, wIndex, wLength);
    
    /* Output via ITM/SWO or UART depending on your debug setup */
    /* For ITM: */
    for (char *p = debug_buffer; *p; p++) {
        ITM_SendChar(*p);
    }
}

/**
 * @brief  Log descriptor being sent
 */
void USBD_MIDI_DebugDescriptor(const char *name, const uint8_t *data, uint16_t len)
{
    snprintf(debug_buffer, sizeof(debug_buffer), "Descriptor %s: %u bytes\r\n", name, len);
    for (char *p = debug_buffer; *p; p++) {
        ITM_SendChar(*p);
    }
    
    /* Dump first 32 bytes */
    for (int i = 0; i < len && i < 32; i++) {
        if (i % 16 == 0) {
            snprintf(debug_buffer, sizeof(debug_buffer), "  %04X: ", i);
            for (char *p = debug_buffer; *p; p++) ITM_SendChar(*p);
        }
        snprintf(debug_buffer, sizeof(debug_buffer), "%02X ", data[i]);
        for (char *p = debug_buffer; *p; p++) ITM_SendChar(*p);
        if ((i + 1) % 16 == 0) {
            ITM_SendChar('\r');
            ITM_SendChar('\n');
        }
    }
    ITM_SendChar('\r');
    ITM_SendChar('\n');
}

/**
 * @brief  Log enumeration state
 */
void USBD_MIDI_DebugState(const char *state)
{
    snprintf(debug_buffer, sizeof(debug_buffer), "USB State: %s\r\n", state);
    for (char *p = debug_buffer; *p; p++) {
        ITM_SendChar(*p);
    }
}

#endif /* USBD_MIDI_DEBUG */
