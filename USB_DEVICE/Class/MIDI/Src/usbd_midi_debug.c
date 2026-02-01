/**
 ******************************************************************************
 * @file    usbd_midi_debug.c
 * @brief   USB MIDI Debug Instrumentation - UART Version
 * @author  MidiCore
 ******************************************************************************
 * Uses existing test_debug system with automatic baudrate configuration
 * Enable by uncommenting #define USBD_MIDI_DEBUG in usbd_midi_debug.h
 * 
 * MIOS32 PRINCIPLES:
 * - NO printf / snprintf / vsnprintf (causes stack overflow!)
 * - Fixed string outputs only via dbg_print/dbg_print_hex8/etc.
 * 
 * Uses TEST_DEBUG_UART_PORT and TEST_DEBUG_UART_BAUD from test_debug.h
 * Default: UART2 at 115200 baud
 ******************************************************************************
 */
#include "CONFIG/module_config.h"  // For MODULE_ENABLE_USB_MIDI_DEBUG

#ifdef USBD_MIDI_DEBUG

#include "USB_DEVICE/Class/MIDI/Inc/usbd_midi_debug.h"
#include "App/tests/test_debug.h"

/* NO stdio.h - we don't use printf! */

/**
 * @brief  Log USB setup request to UART (MIOS32-style, no printf)
 */
void USBD_MIDI_DebugSetupRequest(uint8_t bmRequest, uint8_t bRequest, uint16_t wValue, uint16_t wIndex, uint16_t wLength)
{
    dbg_print("USB Setup: bmReq=0x");
    dbg_print_hex8(bmRequest);
    dbg_print(" bReq=0x");
    dbg_print_hex8(bRequest);
    dbg_print(" wVal=0x");
    dbg_print_hex16(wValue);
    dbg_print(" wIdx=0x");
    dbg_print_hex16(wIndex);
    dbg_print(" wLen=");
    dbg_print_uint(wLength);
    dbg_print("\r\n");
    
    /* Decode common requests */
    if (bRequest == 0x06) {  /* GET_DESCRIPTOR */
        uint8_t desc_type = (wValue >> 8) & 0xFF;
        uint8_t desc_index = wValue & 0xFF;
        dbg_print("  -> GET_DESCRIPTOR: type=");
        dbg_print_uint(desc_type);
        dbg_print(" index=");
        dbg_print_uint(desc_index);
        dbg_print("\r\n");
        
        switch (desc_type) {
            case 1: dbg_print("     (DEVICE descriptor)\r\n"); break;
            case 2: dbg_print("     (CONFIGURATION descriptor)\r\n"); break;
            case 3: dbg_print("     (STRING descriptor)\r\n"); break;
            default: dbg_print("     (Unknown type)\r\n"); break;
        }
    }
    else if (bRequest == 0x05) {  /* SET_ADDRESS */
        dbg_print("  -> SET_ADDRESS: ");
        dbg_print_uint(wValue & 0x7F);
        dbg_print("\r\n");
    }
    else if (bRequest == 0x09) {  /* SET_CONFIGURATION */
        dbg_print("  -> SET_CONFIGURATION: ");
        dbg_print_uint(wValue & 0xFF);
        dbg_print("\r\n");
    }
}

/**
 * @brief  Dump descriptor bytes to UART (MIOS32-style, no printf)
 */
void USBD_MIDI_DebugDescriptor(const char *name, const uint8_t *data, uint16_t len)
{
    dbg_print("Descriptor [");
    dbg_print(name);
    dbg_print("]: ");
    dbg_print_uint(len);
    dbg_print(" bytes (0x");
    dbg_print_hex8((uint8_t)len);
    dbg_print(")\r\n");
    
    /* Dump first 64 bytes in hex */
    uint16_t dump_len = (len > 64) ? 64 : len;
    for (uint16_t i = 0; i < dump_len; i++) {
        if (i % 16 == 0) {
            dbg_print("  ");
            dbg_print_hex16(i);
            dbg_print(": ");
        }
        dbg_print_hex8(data[i]);
        dbg_putc(' ');
        if ((i + 1) % 16 == 0) {
            dbg_print("\r\n");
        }
    }
    if (dump_len % 16 != 0) {
        dbg_print("\r\n");
    }
    
    if (len > 64) {
        dbg_print("  ... (");
        dbg_print_uint(len - 64);
        dbg_print(" more bytes)\r\n");
    }
}

/**
 * @brief  Log enumeration state to UART (MIOS32-style, no printf)
 */
void USBD_MIDI_DebugState(const char *state)
{
    dbg_print("USB State: ");
    dbg_print(state);
    dbg_print("\r\n");
}

#endif /* USBD_MIDI_DEBUG */
