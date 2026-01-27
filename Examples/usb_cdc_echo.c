/**
 ******************************************************************************
 * @file    usb_cdc_echo.c
 * @brief   USB CDC Echo Example
 * @author  MidiCore Project
 ******************************************************************************
 * @attention
 *
 * Simple example demonstrating USB CDC (Virtual COM Port) functionality
 * 
 * Features:
 * - Sends a welcome banner when CDC connects
 * - Echoes back any received data
 * - Demonstrates basic CDC send/receive operations
 * 
 * Usage:
 * 1. Enable MODULE_ENABLE_USB_CDC in Config/module_config.h
 * 2. Include this file in your build (or copy functions to your application)
 * 3. Call usb_cdc_echo_init() after usb_cdc_init()
 * 4. Connect to device via serial terminal
 * 5. Type characters - they will be echoed back
 * 
 ******************************************************************************
 */

#include "Services/usb_cdc/usb_cdc.h"
#include <string.h>

#if MODULE_ENABLE_USB_CDC

/* Private variables */
static uint8_t connection_state = 0;

/* Welcome banner sent when CDC connects */
static const char welcome_banner[] = 
    "\r\n"
    "═══════════════════════════════════════════════════════════════\r\n"
    "  MidiCore USB CDC Virtual COM Port\r\n"
    "  Firmware Version: 1.0.0\r\n"
    "  Build Date: " __DATE__ " " __TIME__ "\r\n"
    "═══════════════════════════════════════════════════════════════\r\n"
    "\r\n"
    "USB CDC Echo Test Active\r\n"
    "Type characters to see them echoed back...\r\n"
    "\r\n";

/**
 * @brief CDC receive callback - echoes back received data
 * @param buf Pointer to received data buffer
 * @param len Number of bytes received
 */
static void cdc_echo_rx_callback(const uint8_t *buf, uint32_t len)
{
    /* Echo received data back to host */
    usb_cdc_send(buf, len);
    
    /* Optional: Add local echo with formatting */
    /* Example: Convert lowercase to uppercase before echoing */
    /*
    uint8_t echo_buf[64];
    if (len > sizeof(echo_buf)) len = sizeof(echo_buf);
    
    for (uint32_t i = 0; i < len; i++) {
        if (buf[i] >= 'a' && buf[i] <= 'z') {
            echo_buf[i] = buf[i] - 32;  // Convert to uppercase
        } else {
            echo_buf[i] = buf[i];
        }
    }
    
    usb_cdc_send(echo_buf, len);
    */
}

/**
 * @brief CDC connection monitor task
 * 
 * Call this periodically (e.g., from FreeRTOS task or main loop)
 * to detect CDC connection changes and send welcome banner
 * 
 * @note This is optional - can be omitted if you don't need banner
 */
void usb_cdc_echo_task(void)
{
    uint8_t current_state = usb_cdc_is_connected();
    
    /* Detect connection state change */
    if (current_state != connection_state) {
        connection_state = current_state;
        
        if (current_state) {
            /* CDC just connected - send welcome banner */
            usb_cdc_send((const uint8_t *)welcome_banner, strlen(welcome_banner));
        }
    }
}

/**
 * @brief Initialize USB CDC echo example
 * 
 * Call this after usb_cdc_init() to set up echo functionality
 */
void usb_cdc_echo_init(void)
{
    /* Register receive callback */
    usb_cdc_register_receive_callback(cdc_echo_rx_callback);
    
    /* Initialize connection state */
    connection_state = usb_cdc_is_connected();
}

/**
 * @brief Send a custom message via CDC
 * @param msg Null-terminated string to send
 * @return Number of bytes sent or error code
 * 
 * Helper function to send strings via CDC
 */
int32_t usb_cdc_echo_send_message(const char *msg)
{
    if (msg == NULL) {
        return USB_CDC_ERROR;
    }
    
    return usb_cdc_send((const uint8_t *)msg, strlen(msg));
}

/* ============================================================================
 * Integration Example
 * ============================================================================ */

#if 0  /* Example code - enable in your application */

/* In your main.c or application initialization: */

#include "Examples/usb_cdc_echo.c"

void app_init(void)
{
    /* ... other initialization ... */
    
    #if MODULE_ENABLE_USB_CDC
    /* Initialize USB CDC service */
    usb_cdc_init();
    
    /* Initialize echo example */
    usb_cdc_echo_init();
    #endif
}

/* In your FreeRTOS task or main loop: */

void app_task(void)
{
    while (1)
    {
        #if MODULE_ENABLE_USB_CDC
        /* Monitor CDC connection and send banner */
        usb_cdc_echo_task();
        #endif
        
        /* ... other task code ... */
        
        osDelay(100);  /* Run every 100ms */
    }
}

/* Send custom messages: */

void send_debug_message(void)
{
    #if MODULE_ENABLE_USB_CDC
    usb_cdc_echo_send_message("Debug: System initialized OK\r\n");
    #endif
}

#endif /* Example code */

#else /* !MODULE_ENABLE_USB_CDC */

/* Stub implementations when CDC is disabled */
void usb_cdc_echo_init(void) {}
void usb_cdc_echo_task(void) {}
int32_t usb_cdc_echo_send_message(const char *msg) { (void)msg; return -1; }

#endif /* MODULE_ENABLE_USB_CDC */
