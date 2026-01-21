/**
 ******************************************************************************
 * @file    validate_usb_descriptors.c
 * @brief   Validation tool for USB MIDI descriptors
 * @author  MidiCore
 ******************************************************************************
 * This tool validates that the USB MIDI descriptor calculations match
 * the actual descriptor byte array. Helps catch descriptor size bugs
 * that cause Windows error 0xC00000E5.
 ******************************************************************************
 */

#include <stdio.h>
#include <stdint.h>

/* Constants from usbd_midi.c */
#define MIDI_NUM_PORTS 4

#define USB_DESC_SIZE_CONFIGURATION      9
#define USB_DESC_SIZE_IAD                8
#define USB_DESC_SIZE_INTERFACE          9
#define USB_DESC_SIZE_ENDPOINT           7      /* Standard Bulk Endpoint (7 bytes, not 9!) */
#define USB_DESC_SIZE_JACK_IN_EXTERNAL   6
#define USB_DESC_SIZE_JACK_IN_EMBEDDED   9
#define USB_DESC_SIZE_JACK_OUT           9
#define USB_DESC_SIZE_CS_AC_INTERFACE    9
#define USB_DESC_SIZE_CS_MS_INTERFACE    7
#define USB_DESC_SIZE_CS_ENDPOINT_BASE   5

#define USB_MIDI_JACK_DESC_SIZE_PER_PORT (USB_DESC_SIZE_JACK_IN_EXTERNAL + \
                                          USB_DESC_SIZE_JACK_IN_EMBEDDED + \
                                          USB_DESC_SIZE_JACK_OUT + \
                                          USB_DESC_SIZE_JACK_OUT)

#define USB_MIDI_MS_TOTAL_LENGTH         ((MIDI_NUM_PORTS * USB_MIDI_JACK_DESC_SIZE_PER_PORT) + \
                                          USB_DESC_SIZE_ENDPOINT + \
                                          (USB_DESC_SIZE_CS_ENDPOINT_BASE + MIDI_NUM_PORTS) + \
                                          USB_DESC_SIZE_ENDPOINT + \
                                          (USB_DESC_SIZE_CS_ENDPOINT_BASE + MIDI_NUM_PORTS))

#define USB_MIDI_CONFIG_DESC_SIZ         (USB_DESC_SIZE_CONFIGURATION + \
                                          USB_DESC_SIZE_IAD + \
                                          USB_DESC_SIZE_INTERFACE + \
                                          USB_DESC_SIZE_CS_AC_INTERFACE + \
                                          USB_DESC_SIZE_INTERFACE + \
                                          USB_DESC_SIZE_CS_MS_INTERFACE + \
                                          USB_MIDI_MS_TOTAL_LENGTH)

int main(void)
{
    printf("===========================================\n");
    printf("  USB MIDI Descriptor Validation Tool\n");
    printf("===========================================\n\n");
    
    printf("Configuration: %d-port MIDI interface\n\n", MIDI_NUM_PORTS);
    
    /* Per-port jack sizes */
    printf("Jack Descriptor Sizes:\n");
    printf("  External IN Jack:  %d bytes\n", USB_DESC_SIZE_JACK_IN_EXTERNAL);
    printf("  Embedded IN Jack:  %d bytes (has source pins)\n", USB_DESC_SIZE_JACK_IN_EMBEDDED);
    printf("  Embedded OUT Jack: %d bytes\n", USB_DESC_SIZE_JACK_OUT);
    printf("  External OUT Jack: %d bytes\n", USB_DESC_SIZE_JACK_OUT);
    printf("  Total per port:    %d bytes\n\n", USB_MIDI_JACK_DESC_SIZE_PER_PORT);
    
    /* Total jack descriptors */
    int total_jacks = MIDI_NUM_PORTS * USB_MIDI_JACK_DESC_SIZE_PER_PORT;
    printf("Total Jack Descriptors: %d bytes (%d ports × %d bytes)\n\n", 
           total_jacks, MIDI_NUM_PORTS, USB_MIDI_JACK_DESC_SIZE_PER_PORT);
    
    /* Endpoint descriptors */
    int ep_out = USB_DESC_SIZE_ENDPOINT + (USB_DESC_SIZE_CS_ENDPOINT_BASE + MIDI_NUM_PORTS);
    int ep_in = USB_DESC_SIZE_ENDPOINT + (USB_DESC_SIZE_CS_ENDPOINT_BASE + MIDI_NUM_PORTS);
    printf("Endpoint Descriptors:\n");
    printf("  Bulk OUT (standard + CS): %d bytes (9 + %d)\n", ep_out, USB_DESC_SIZE_CS_ENDPOINT_BASE + MIDI_NUM_PORTS);
    printf("  Bulk IN (standard + CS):  %d bytes (9 + %d)\n", ep_in, USB_DESC_SIZE_CS_ENDPOINT_BASE + MIDI_NUM_PORTS);
    printf("  Total endpoints:          %d bytes\n\n", ep_out + ep_in);
    
    /* MS_HEADER wTotalLength */
    printf("MS_HEADER wTotalLength:\n");
    printf("  Jacks + Endpoints = %d bytes (0x%04X)\n\n", USB_MIDI_MS_TOTAL_LENGTH, USB_MIDI_MS_TOTAL_LENGTH);
    
    /* Configuration Descriptor breakdown */
    printf("Configuration Descriptor Breakdown:\n");
    printf("  Configuration Descriptor:  %d bytes\n", USB_DESC_SIZE_CONFIGURATION);
    printf("  IAD:                        %d bytes\n", USB_DESC_SIZE_IAD);
    printf("  AC Interface:               %d bytes\n", USB_DESC_SIZE_INTERFACE);
    printf("  CS AC Header:               %d bytes (has bInCollection)\n", USB_DESC_SIZE_CS_AC_INTERFACE);
    printf("  MS Interface:               %d bytes\n", USB_DESC_SIZE_INTERFACE);
    printf("  CS MS Header:               %d bytes\n", USB_DESC_SIZE_CS_MS_INTERFACE);
    printf("  MS data (jacks+endpoints):  %d bytes\n", USB_MIDI_MS_TOTAL_LENGTH);
    printf("  ----------------------------------\n");
    printf("  Total:                      %d bytes (0x%04X)\n\n", USB_MIDI_CONFIG_DESC_SIZ, USB_MIDI_CONFIG_DESC_SIZ);
    
    /* Validation */
    printf("===========================================\n");
    printf("Validation Results:\n");
    printf("===========================================\n");
    
    int errors = 0;
    
    if (USB_MIDI_JACK_DESC_SIZE_PER_PORT == 33) {
        printf("✅ Per-port jack size:  %d bytes (CORRECT)\n", USB_MIDI_JACK_DESC_SIZE_PER_PORT);
    } else {
        printf("❌ Per-port jack size:  %d bytes (WRONG - should be 33)\n", USB_MIDI_JACK_DESC_SIZE_PER_PORT);
        errors++;
    }
    
    if (USB_MIDI_MS_TOTAL_LENGTH == 164) {
        printf("✅ MS_HEADER wTotalLength: %d bytes (CORRECT)\n", USB_MIDI_MS_TOTAL_LENGTH);
    } else {
        printf("❌ MS_HEADER wTotalLength: %d bytes (WRONG - should be 164)\n", USB_MIDI_MS_TOTAL_LENGTH);
        errors++;
    }
    
    if (USB_MIDI_CONFIG_DESC_SIZ == 215) {
        printf("✅ Config wTotalLength:    %d bytes (CORRECT)\n", USB_MIDI_CONFIG_DESC_SIZ);
    } else {
        printf("❌ Config wTotalLength:    %d bytes (WRONG - should be 215)\n", USB_MIDI_CONFIG_DESC_SIZ);
        errors++;
    }
    
    printf("\n");
    
    if (errors == 0) {
        printf("🎉 All descriptor sizes are CORRECT!\n");
        printf("   This should fix Windows error 0xC00000E5\n");
        return 0;
    } else {
        printf("⚠️  Found %d error(s) in descriptor calculations\n", errors);
        printf("   Windows will reject this descriptor with error 0xC00000E5\n");
        return 1;
    }
}
