#include "cmsis_os2.h"
#include "Services/midi/midi_din.h"
#include "Services/looper/looper.h"
#include "Services/ui/ui.h"
#include "Services/midi/midi_delayq.h"
#include "Services/expression/expression.h"
#include "Services/usb_midi/usb_midi.h"
#include "Services/usb_cdc/usb_cdc.h"
#include "Services/midicore_query/midicore_query.h"
#include "App/tests/test_debug.h"

/* USB MIDI RX debug hook for production mode
 * This overrides the weak symbol in usb_midi.c to provide RX packet visibility
 * when MODULE_DEBUG_MIDICORE_QUERIES is enabled.
 * Test mode has its own implementation in module_tests.c
 * 
 * CRITICAL: Must match test mode behavior to prevent stack overflow and timing issues
 */
#if defined(MODULE_TEST_USB_DEVICE_MIDI) || MODULE_DEBUG_MIDICORE_QUERIES
void usb_midi_rx_debug_hook(const uint8_t packet4[4])
{
  uint8_t cin = packet4[0] & 0x0F;
  
  /* CRITICAL: Skip SysEx packets (CIN 0x4-0x7) to match test mode behavior
   * Test mode skips SysEx logging to avoid stack overflow and timing delays.
   * MIOS Studio queries are SysEx format - logging them causes:
   * 1. Stack overflow (dbg_printf uses ~256 bytes per call)
   * 2. Timing delays that break query processing
   * 3. MIOS Studio timeout waiting for response
   * 
   * This is THE difference that prevented MIOS Studio recognition in production!
   */
  if (cin >= 0x04 && cin <= 0x07) {
    return; // Skip SysEx like test mode does
  }
  
  /* Log regular MIDI messages only (Note On/Off, CC, Program Change, etc.)
   * This provides visibility without overwhelming the system during query processing */
  uint8_t cable = (packet4[0] >> 4) & 0x0F;
  dbg_printf("[USB-RX] Cable:%u CIN:0x%X Data:[%02X %02X %02X %02X]\r\n",
             cable, cin, packet4[0], packet4[1], packet4[2], packet4[3]);
}
#endif

// Call this from app_init_and_start() if you want a dedicated task.
static void MidiIOTask(void *argument) {
  (void)argument;
  
  dbg_printf("[MIDI-TASK] MidiIOTask started\r\n");
  
  // Initialize delay queue and expression
  midi_delayq_init();
  expression_init();
  
  dbg_printf("[MIDI-TASK] Init complete, entering main loop\r\n");
  
  // MIOS Studio recognition diagnostics
  uint32_t diagnostic_counter = 0;
  uint32_t last_diagnostic_time = 0;
  
  // Check USB MIDI status immediately
  extern bool usb_midi_get_tx_status(uint32_t *queue_size, uint32_t *queue_used, uint32_t *queue_drops);
  uint32_t queue_size = 0, queue_used = 0, queue_drops = 0;
  bool midi_ready = usb_midi_get_tx_status(&queue_size, &queue_used, &queue_drops);
  
  dbg_printf("[MIDI-TASK] USB MIDI Status:\r\n");
  dbg_printf("  Ready: %s\r\n", midi_ready ? "YES" : "NO");
  dbg_printf("  TX Queue Size: %lu packets\r\n", (unsigned long)queue_size);
  dbg_printf("  TX Queue Used: %lu packets\r\n", (unsigned long)queue_used);
  dbg_printf("  TX Queue Drops: %lu packets\r\n", (unsigned long)queue_drops);
  
  if (!midi_ready) {
    dbg_printf("[MIDI-TASK] WARNING: USB MIDI not ready! Check USB enumeration.\r\n");
  }
  
  dbg_printf("[MIDI-TASK] MidiCore query processing enabled\r\n");
  dbg_printf("[MIDI-TASK] For MIOS Studio recognition:\r\n");
  dbg_printf("  1. Connect USB cable\r\n");
  dbg_printf("  2. Open MIOS Studio\r\n");
  dbg_printf("  3. Watch for query messages below\r\n");
  dbg_printf("  4. Device should appear in MIOS Studio device list\r\n");
  
  // Send test message to MIOS Studio terminal to verify communication
  dbg_printf("[MIDI-TASK] Sending test message to MIOS Studio terminal...\r\n");
  bool test_sent = midicore_debug_send_message("*** MidiCore MIOS Terminal Test ***\r\n", 0);
  if (test_sent) {
    dbg_printf("[MIDI-TASK] Test message sent successfully\r\n");
    dbg_printf("[MIDI-TASK] Check MIOS Studio Terminal window for the message\r\n");
  } else {
    dbg_printf("[MIDI-TASK] ERROR: Failed to send test message (USB MIDI not ready?)\r\n");
  };
  
  /* CRITICAL: Allow USB enumeration to complete before processing queries
   * Test mode has 500ms of delays (5 messages * 100ms each) before main loop.
   * This timing is ESSENTIAL for MIOS Studio recognition to work reliably.
   * Without this delay, firmware enters main loop before USB fully enumerated,
   * causing first queries from MIOS Studio to be missed or fail.
   * USB composite enumeration (MIDI + CDC) typically takes 200-500ms. */
  dbg_printf("[MIDI-TASK] Waiting for USB enumeration to complete (500ms)...\r\n");
  osDelay(500);
  dbg_printf("[MIDI-TASK] USB enumeration complete, ready for MIOS Studio queries\r\n");
  
  uint32_t ui_ms = 0;
  
  for (;;) {
    /* CRITICAL: Process USB MIDI RX queue in task context (NOT interrupt!)
     * This handles MidiCore queries, router processing, and TX responses safely */
    usb_midi_process_rx_queue();
    
    /* CRITICAL: Process MidiCore queries queued from ISR
     * This enables MIOS Studio detection and terminal communication
     * MUST match test mode - only call if MODULE_ENABLE_USB_MIDI enabled */
#if MODULE_ENABLE_USB_MIDI
    midicore_query_process_queued();
#endif
    
    /* CRITICAL: Process USB CDC RX queue in task context (NOT interrupt!)
     * This handles MIOS Studio terminal data safely
     * MUST match test mode - only call if MODULE_ENABLE_USB_CDC enabled */
#if MODULE_ENABLE_USB_CDC
    usb_cdc_process_rx_queue();
#endif
    
    midi_din_tick();
    
    /* Call 1ms tick functions 10 times to maintain timing accuracy
     * while using 10ms loop delay (matches test mode for MIOS Studio compatibility) */
    for (int i = 0; i < 10; i++) {
      looper_tick_1ms();
      midi_delayq_tick_1ms();
      expression_tick_1ms();
      ui_ms++;
      if ((ui_ms % 20u) == 0u) ui_tick_20ms();
    }
    
    /* CRITICAL: 10ms delay matches MODULE_TEST_USB_DEVICE_MIDI timing
     * This is REQUIRED for MIOS Studio recognition to work reliably.
     * 1ms delay causes USB TX queue timing issues and recognition failure. */
    osDelay(10);
    
    // Periodic diagnostic output (every 10 seconds)
    diagnostic_counter++;
    uint32_t now = osKernelGetTickCount();
    if (now - last_diagnostic_time >= 10000) {
      last_diagnostic_time = now;
      
      // Check USB MIDI status
      midi_ready = usb_midi_get_tx_status(&queue_size, &queue_used, &queue_drops);
      
      dbg_printf("[MIDI-TASK] ===== Heartbeat #%lu =====\r\n", (unsigned long)(now / 10000));
      dbg_printf("[MIDI-TASK] Status: Task running, processing queries\r\n");
      dbg_printf("[MIDI-TASK] USB MIDI: %s | Queue: %lu/%lu used | Drops: %lu\r\n",
                 midi_ready ? "READY" : "NOT_READY",
                 (unsigned long)queue_used,
                 (unsigned long)queue_size,
                 (unsigned long)queue_drops);
      
      if (!midi_ready) {
        dbg_printf("[MIDI-TASK] ERROR: USB MIDI still not ready after %lu seconds!\r\n",
                   (unsigned long)(now / 1000));
        dbg_printf("[MIDI-TASK]        Check USB cable connection and enumeration\r\n");
      } else {
        dbg_printf("[MIDI-TASK] Waiting for MIOS Studio to send queries...\r\n");
        dbg_printf("[MIDI-TASK] NOTE: If no [USB-RX] messages appear above:\r\n");
        dbg_printf("[MIDI-TASK]       1. MIOS Studio may not be open\r\n");
        dbg_printf("[MIDI-TASK]       2. MIOS Studio may not see the device\r\n");
        dbg_printf("[MIDI-TASK]       3. USB enumeration issue on host side\r\n");
      }
      
      if (queue_drops > 0) {
        dbg_printf("[MIDI-TASK] WARNING: TX queue has dropped %lu packets!\r\n",
                   (unsigned long)queue_drops);
      }
      
      dbg_printf("[MIDI-TASK] ========================\r\n");
      
      diagnostic_counter = 0;
    }
  }
}

void app_start_midi_io_task(void) {
  const osThreadAttr_t attr = {
    .name = "MidiIO",
    .priority = osPriorityAboveNormal,
    .stack_size = 2048  // 2KB - processes multiple queues (USB MIDI, CDC, MidiCore queries)
  };
  (void)osThreadNew(MidiIOTask, NULL, &attr);
}
