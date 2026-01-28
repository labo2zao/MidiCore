#include "cmsis_os2.h"
#include "Services/midi/midi_din.h"
#include "Services/looper/looper.h"
#include "Services/ui/ui.h"
#include "Services/midi/midi_delayq.h"
#include "Services/expression/expression.h"
#include "Services/usb_midi/usb_midi.h"
#include "Services/usb_cdc/usb_cdc.h"

// Call this from app_init_and_start() if you want a dedicated task.
static void MidiIOTask(void *argument) {
  (void)argument;
  uint32_t ui_ms = 0;
  midi_delayq_init();
  expression_init();
  for (;;) {
    /* CRITICAL: Process USB MIDI RX queue in task context (NOT interrupt!)
     * This handles MIOS32 queries, router processing, and TX responses safely */
    usb_midi_process_rx_queue();
    
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
