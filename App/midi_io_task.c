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
  }
}

void app_start_midi_io_task(void) {
  const osThreadAttr_t attr = {
    .name = "MidiIO",
    .priority = osPriorityAboveNormal,
    .stack_size = 1024
  };
  (void)osThreadNew(MidiIOTask, NULL, &attr);
}
