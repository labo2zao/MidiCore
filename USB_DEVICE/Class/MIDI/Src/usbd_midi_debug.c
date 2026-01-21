/**
 ******************************************************************************
 * @file    usbd_midi_debug.c
 * @brief   USB MIDI Debug Instrumentation - UART Version
 * @author  MidiCore
 ******************************************************************************
 * Simple UART debug output using printf
 * Enable by uncommenting #define USBD_MIDI_DEBUG in usbd_midi_debug.h
 * 
 * Make sure printf is redirected to UART in your project!
 ******************************************************************************
 */

#ifdef USBD_MIDI_DEBUG

#include "USB_DEVICE/Class/MIDI/Inc/usbd_midi_debug.h"
#include <stdio.h>

/**
 * @brief  Log USB setup request to UART
 */
void USBD_MIDI_DebugSetupRequest(uint8_t bmRequest, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength)
{
    printf("USB Setup: bmReq=0x%02X bReq=0x%02X wVal=0x%04X wIdx=0x%04X wLen=%u\r\n",
           bmRequest, bRequest, wValue, wIndex, wLength);
    
    /* Decode common requests */
    if (bRequest == 0x06) {  /* GET_DESCRIPTOR */
        uint8_t desc_type = (wValue >> 8) & 0xFF;
        uint8_t desc_index = wValue & 0xFF;
        printf("  -> GET_DESCRIPTOR: type=%u index=%u\r\n", desc_type, desc_index);
        
        switch (desc_type) {
            case 1: printf("     (DEVICE descriptor)\r\n"); break;
            case 2: printf("     (CONFIGURATION descriptor)\r\n"); break;
            case 3: printf("     (STRING descriptor)\r\n"); break;
            default: printf("     (Unknown type)\r\n"); break;
        }
    }
    else if (bRequest == 0x05) {  /* SET_ADDRESS */
        printf("  -> SET_ADDRESS: %u\r\n", wValue & 0x7F);
    }
    else if (bRequest == 0x09) {  /* SET_CONFIGURATION */
        printf("  -> SET_CONFIGURATION: %u\r\n", wValue & 0xFF);
    }
}

/**
 * @brief  Dump descriptor bytes to UART
 */
void USBD_MIDI_DebugDescriptor(const char *name, const uint8_t *data, uint16_t len)
{
    printf("Descriptor [%s]: %u bytes (0x%02X)\r\n", name, len, len);
    
    /* Dump first 64 bytes in hex */
    uint16_t dump_len = (len > 64) ? 64 : len;
    for (uint16_t i = 0; i < dump_len; i++) {
        if (i % 16 == 0) {
            printf("  %04X: ", i);
        }
        printf("%02X ", data[i]);
        if ((i + 1) % 16 == 0) {
            printf("\r\n");
        }
    }
    if (dump_len % 16 != 0) {
        printf("\r\n");
    }
    
    if (len > 64) {
        printf("  ... (%u more bytes)\r\n", len - 64);
    }
}

/**
 * @brief  Log enumeration state to UART
 */
void USBD_MIDI_DebugState(const char *state)
{
    printf("USB State: %s\r\n", state);
}

#endif /* USBD_MIDI_DEBUG */
