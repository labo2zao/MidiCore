#include "cmsis_os2.h"
#include "Services/midi/midi_din.h"
#include "Services/looper/looper.h"
#include "Services/ui/ui.h"
#include "Services/midi/midi_delayq.h"
#include "Services/expression/expression.h"
#include "Services/usb_midi/usb_midi.h"
#include "Services/usb_cdc/usb_cdc.h"
#include "Services/mios32_query/mios32_query.h"
#include "App/tests/test_debug.h"

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
  
  dbg_printf("[MIDI-TASK] MIOS32 query processing enabled\r\n");
  dbg_printf("[MIDI-TASK] For MIOS Studio recognition:\r\n");
  dbg_printf("  1. Connect USB cable\r\n");
  dbg_printf("  2. Open MIOS Studio\r\n");
  dbg_printf("  3. Watch for query messages below\r\n");
  dbg_printf("  4. Device should appear in MIOS Studio device list\r\n");
  
  // Send test message to MIOS Studio terminal to verify communication
  dbg_printf("[MIDI-TASK] Sending test message to MIOS Studio terminal...\r\n");
  bool test_sent = mios32_debug_send_message("*** MidiCore MIOS Terminal Test ***\r\n", 0);
  if (test_sent) {
    dbg_printf("[MIDI-TASK] Test message sent successfully\r\n");
    dbg_printf("[MIDI-TASK] Check MIOS Studio Terminal window for the message\r\n");
  } else {
    dbg_printf("[MIDI-TASK] ERROR: Failed to send test message (USB MIDI not ready?)\r\n");
  };
  
  uint32_t ui_ms = 0;
  
  for (;;) {
    /* CRITICAL: Process USB MIDI RX queue in task context (NOT interrupt!)
     * This handles MIOS32 queries, router processing, and TX responses safely */
    usb_midi_process_rx_queue();
    
    /* CRITICAL: Process MIOS32 queries queued from ISR
     * This enables MIOS Studio detection and terminal communication */
    mios32_query_process_queued();
    
    /* CRITICAL: Process USB CDC RX queue in task context (NOT interrupt!)
     * This handles MIOS Studio terminal data safely */
    usb_cdc_process_rx_queue();
    
    midi_din_tick();
    looper_tick_1ms();
    midi_delayq_tick_1ms();
    expression_tick_1ms();
    osDelay(1);
    ui_ms++;
    if ((ui_ms % 20u) == 0u) ui_tick_20ms();
    
    // Periodic diagnostic output (every 10 seconds)
    diagnostic_counter++;
    uint32_t now = osKernelGetTickCount();
    if (now - last_diagnostic_time >= 10000) {
      last_diagnostic_time = now;
      
      // Check USB MIDI status
      midi_ready = usb_midi_get_tx_status(&queue_size, &queue_used, &queue_drops);
      
      dbg_printf("[MIDI-TASK] Heartbeat: Task running OK (loops: %lu)\r\n", (unsigned long)diagnostic_counter);
      dbg_printf("[MIDI-TASK] USB MIDI: %s, Queue: %lu/%lu, Drops: %lu\r\n",
                 midi_ready ? "READY" : "NOT_READY",
                 (unsigned long)queue_used,
                 (unsigned long)queue_size,
                 (unsigned long)queue_drops);
      
      if (!midi_ready) {
        dbg_printf("[MIDI-TASK] ERROR: USB MIDI still not ready after %lu seconds!\r\n",
                   (unsigned long)(now / 1000));
      }
      
      if (queue_drops > 0) {
        dbg_printf("[MIDI-TASK] WARNING: TX queue has dropped %lu packets!\r\n",
                   (unsigned long)queue_drops);
      }
      
      diagnostic_counter = 0;
    }
  }
}

void app_start_midi_io_task(void) {
  const osThreadAttr_t attr = {
    .name = "MidiIO",
    .priority = osPriorityAboveNormal,
    .stack_size = 2048  // 2KB - processes multiple queues (USB MIDI, CDC, MIOS32 queries)
  };
  (void)osThreadNew(MidiIOTask, NULL, &attr);
}
