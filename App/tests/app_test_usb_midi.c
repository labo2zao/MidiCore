#include "App/tests/app_test_usb_midi.h"
#include "App/tests/test_debug.h"
#include "Config/module_config.h"

#include "cmsis_os2.h"
#include "main.h"

#if MODULE_ENABLE_USB_MIDI
#include "Services/usb_midi/usb_midi.h"
#endif

#include <string.h>

// =============================================================================
// CONFIGURATION
// =============================================================================

#ifndef APP_TEST_USB_MIDI_SEND_INTERVAL
#define APP_TEST_USB_MIDI_SEND_INTERVAL 2000  // ms between test messages
#endif

#ifndef APP_TEST_USB_MIDI_BASE_NOTE
#define APP_TEST_USB_MIDI_BASE_NOTE 60  // Middle C
#endif

#ifndef APP_TEST_USB_MIDI_CHANNEL
#define APP_TEST_USB_MIDI_CHANNEL 0  // Channel 1 (0-based)
#endif

#ifndef APP_TEST_USB_MIDI_VELOCITY
#define APP_TEST_USB_MIDI_VELOCITY 100
#endif

#ifndef APP_TEST_USB_MIDI_CABLE
#define APP_TEST_USB_MIDI_CABLE 0  // Cable 0 (USB MIDI Port 1)
#endif

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * @brief Get MIDI message type name from status byte
 */
static const char* get_midi_msg_type(uint8_t status)
{
  uint8_t msg_type = status & 0xF0;
  
  switch (msg_type) {
    case 0x80: return "Note Off";
    case 0x90: return "Note On";
    case 0xA0: return "Poly Aftertouch";
    case 0xB0: return "CC";
    case 0xC0: return "Program Change";
    case 0xD0: return "Ch Aftertouch";
    case 0xE0: return "Pitch Bend";
    case 0xF0: return "System";
    default: return "Unknown";
  }
}

/**
 * @brief Print received USB MIDI packet to UART
 */
static void print_usb_midi_packet(const uint8_t packet4[4])
{
  uint8_t cable = (packet4[0] >> 4) & 0x0F;
  uint8_t cin = packet4[0] & 0x0F;
  uint8_t status = packet4[1];
  uint8_t data1 = packet4[2];
  uint8_t data2 = packet4[3];
  
  // Print basic packet info
  dbg_print("[RX] Cable:");
  dbg_print_uint(cable);
  dbg_print(" ");
  dbg_print_hex8(status);
  dbg_print(" ");
  dbg_print_hex8(data1);
  dbg_print(" ");
  dbg_print_hex8(data2);
  
  // Print decoded message type
  dbg_print(" (");
  dbg_print(get_midi_msg_type(status));
  
  uint8_t channel = (status & 0x0F) + 1;  // 1-based for display
  dbg_print(" Ch:");
  dbg_print_uint(channel);
  
  // Print message-specific data
  uint8_t msg_type = status & 0xF0;
  if (msg_type == 0x80 || msg_type == 0x90) {
    dbg_print(" Note:");
    dbg_print_uint(data1);
    dbg_print(" Vel:");
    dbg_print_uint(data2);
  } else if (msg_type == 0xB0) {
    dbg_print(" CC:");
    dbg_print_uint(data1);
    dbg_print(" Val:");
    dbg_print_uint(data2);
  } else if (msg_type == 0xC0) {
    dbg_print(" Prog:");
    dbg_print_uint(data1);
  } else if (msg_type == 0xD0) {
    dbg_print(" Pressure:");
    dbg_print_uint(data1);
  } else if (msg_type == 0xE0) {
    uint16_t bend_value = (uint16_t)data1 | ((uint16_t)data2 << 7);
    dbg_print(" Bend:");
    dbg_print_uint(bend_value);
  }
  
  dbg_print(")\r\n");
}

// =============================================================================
// USB MIDI RECEIVE CALLBACK (intercepts packets before router)
// =============================================================================
// Note: usb_midi_rx_debug_hook is defined in module_tests.c to avoid duplication

// =============================================================================
// USB MIDI SEND TEST FUNCTIONS
// =============================================================================

/**
 * @brief Debug trace for TX failures
 * 
 * Note: This is called from interrupt context, so we build the complete
 * message in a buffer and send it as a single atomic write to avoid
 * fragmentation when using USB CDC.
 */
void test_debug_tx_trace(uint8_t code)
{
  char buffer[80];
  const char* msg;
  
  switch(code) {
    case 0x01: msg = "Class data NULL or not ready"; break;
    case 0x02: msg = "Queue empty"; break;
    case 0x03: msg = "Endpoint BUSY"; break;
    case 0xFF: msg = "Queue FULL!"; break;
    default: msg = "Unknown"; break;
  }
  
  // Build complete message in buffer for atomic send
  int len = snprintf(buffer, sizeof(buffer), "[TX-DBG] Code:%02X %s\r\n", code, msg);
  
  if (len > 0 && len < (int)sizeof(buffer)) {
    dbg_print(buffer);
  }
}

/**
 * @brief Debug trace for packet queueing
 * 
 * Note: This is called from interrupt context, so we build the complete
 * message in a buffer and send it as a single atomic write to avoid
 * fragmentation when using USB CDC.
 */
void test_debug_tx_packet_queued(uint8_t cin, uint8_t b0)
{
  char buffer[40];
  
  // Build complete message in buffer for atomic send
  int len = snprintf(buffer, sizeof(buffer), "[TX-QUEUE] CIN:%02X B0:%02X\r\n", cin, b0);
  
  if (len > 0 && len < (int)sizeof(buffer)) {
    dbg_print(buffer);
  }
}

/**
 * @brief Send a test MIDI Note On message via USB
 */
static void send_test_note_on(void)
{
#if MODULE_ENABLE_USB_MIDI
  uint8_t cable = APP_TEST_USB_MIDI_CABLE;
  uint8_t status = 0x90 | (APP_TEST_USB_MIDI_CHANNEL & 0x0F);
  uint8_t note = APP_TEST_USB_MIDI_BASE_NOTE;
  uint8_t velocity = APP_TEST_USB_MIDI_VELOCITY;
  
  // CIN = 0x9 for Note On
  uint8_t cin = (cable << 4) | 0x09;
  
  dbg_print("[TX] Sending test Note On: Cable:");
  dbg_print_uint(cable);
  dbg_print(" ");
  dbg_print_hex8(status);
  dbg_print(" ");
  dbg_print_hex8(note);
  dbg_print(" ");
  dbg_print_hex8(velocity);
  dbg_print(" -> Calling usb_midi_send_packet()...\r\n");
  
  usb_midi_send_packet(cin, status, note, velocity);
  
  dbg_print("[TX] ...packet queued\r\n");
#endif
}

/**
 * @brief Send a test MIDI Note Off message via USB
 */
static void send_test_note_off(void)
{
#if MODULE_ENABLE_USB_MIDI
  uint8_t cable = APP_TEST_USB_MIDI_CABLE;
  uint8_t status = 0x80 | (APP_TEST_USB_MIDI_CHANNEL & 0x0F);
  uint8_t note = APP_TEST_USB_MIDI_BASE_NOTE;
  uint8_t velocity = 0;
  
  // CIN = 0x8 for Note Off
  uint8_t cin = (cable << 4) | 0x08;
  
  dbg_print("[TX] Sending test Note Off: Cable:");
  dbg_print_uint(cable);
  dbg_print(" ");
  dbg_print_hex8(status);
  dbg_print(" ");
  dbg_print_hex8(note);
  dbg_print(" ");
  dbg_print_hex8(velocity);
  dbg_print("\r\n");
  
  usb_midi_send_packet(cin, status, note, velocity);
#endif
}

/**
 * @brief Send a test MIDI CC message via USB
 * @param cc_number CC number (0-127)
 * @param cc_value CC value (0-127)
 */
void app_test_usb_midi_send_cc(uint8_t cc_number, uint8_t cc_value)
{
#if MODULE_ENABLE_USB_MIDI
  uint8_t cable = APP_TEST_USB_MIDI_CABLE;
  uint8_t status = 0xB0 | (APP_TEST_USB_MIDI_CHANNEL & 0x0F);
  
  // CIN = 0xB for Control Change
  uint8_t cin = (cable << 4) | 0x0B;
  
  dbg_print("[TX] Sending CC: Cable:");
  dbg_print_uint(cable);
  dbg_print(" ");
  dbg_print_hex8(status);
  dbg_print(" ");
  dbg_print_hex8(cc_number);
  dbg_print(" ");
  dbg_print_hex8(cc_value);
  dbg_print("\r\n");
  
  usb_midi_send_packet(cin, status, cc_number, cc_value);
#endif
}

/**
 * @brief Send a generic 3-byte MIDI message via USB
 * @param status MIDI status byte (includes channel)
 * @param data1 First data byte
 * @param data2 Second data byte
 */
void app_test_usb_midi_send3(uint8_t status, uint8_t data1, uint8_t data2)
{
#if MODULE_ENABLE_USB_MIDI
  uint8_t cable = APP_TEST_USB_MIDI_CABLE;
  uint8_t msg_type = (status >> 4) & 0x0F;
  
  // Build CIN from message type
  // For channel messages (0x8-0xE), the CIN matches the message type
  // For system messages (0xF), specific handling would be needed
  uint8_t cin = (cable << 4) | msg_type;
  
  dbg_print("[TX] Sending MIDI: Cable:");
  dbg_print_uint(cable);
  dbg_print(" ");
  dbg_print_hex8(status);
  dbg_print(" ");
  dbg_print_hex8(data1);
  dbg_print(" ");
  dbg_print_hex8(data2);
  dbg_print("\r\n");
  
  usb_midi_send_packet(cin, status, data1, data2);
#endif
}

// =============================================================================
// MAIN TEST FUNCTION
// =============================================================================

void app_test_usb_midi_run_forever(void)
{
#if MODULE_ENABLE_USB_MIDI
  // Initialize debug UART
  test_debug_init();
  
  // Print test header
  dbg_print_test_header("USB MIDI Device Test");
  dbg_print("USB Device MIDI: Enabled\r\n");
  dbg_printf("Debug UART: UART%d (%d baud)\r\n", 
             TEST_DEBUG_UART_PORT + 1, TEST_DEBUG_UART_BAUD);
  dbg_printf("Test send interval: %d ms\r\n", APP_TEST_USB_MIDI_SEND_INTERVAL);
  dbg_printf("Test channel: %d\r\n", APP_TEST_USB_MIDI_CHANNEL + 1);
  dbg_printf("Test note: %d\r\n", APP_TEST_USB_MIDI_BASE_NOTE);
  dbg_printf("USB Cable: %d\r\n", APP_TEST_USB_MIDI_CABLE);
  dbg_print_separator();
  
  // Note: usb_midi_init() is already called in main.c before RTOS starts
  // No need to call it again here
  
  dbg_print("Test started. Waiting for USB MIDI data from DAW...\r\n");
  dbg_print("Sending test MIDI messages every ");
  dbg_print_uint(APP_TEST_USB_MIDI_SEND_INTERVAL);
  dbg_print(" ms\r\n");
  dbg_print_separator();
  
  uint32_t last_send_time = 0;
  uint8_t note_state = 0;  // 0=off, 1=on
  
  // Main test loop
  for (;;) {
    uint32_t now = osKernelGetTickCount();
    
    // Periodically send test MIDI messages
    if (now - last_send_time >= APP_TEST_USB_MIDI_SEND_INTERVAL) {
      last_send_time = now;
      
      if (note_state == 0) {
        // Send Note On
        send_test_note_on();
        note_state = 1;
      } else {
        // Send Note Off
        send_test_note_off();
        note_state = 0;
      }
    }
    
    // Small delay to avoid CPU overload
    osDelay(10);
  }
  
#else
  // USB MIDI not enabled
  test_debug_init();
  dbg_print_test_header("USB MIDI Device Test");
  dbg_print("ERROR: USB MIDI not enabled!\r\n");
  dbg_print("Enable MODULE_ENABLE_USB_MIDI in Config/module_config.h\r\n");
  
  for (;;) {
    osDelay(1000);
  }
#endif
}
