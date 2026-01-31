#include "App/app_entry.h"

#include "App/app_init.h"
#include "App/tests/test_debug.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * @note This runs in the context of the CubeMX default task.
 *       It should do one-time init and start other tasks, then never return.
 * 
 * @important This function MUST NOT RETURN to match test mode behavior.
 *            Test mode runs forever in its test loop without returning.
 *            Production mode must also run forever to avoid DefaultTask exit.
 */
void app_entry_start(void)
{
  static uint8_t started = 0u;
  
  dbg_printf("\r\n");
  dbg_printf("[APP_ENTRY] ===========================================\r\n");
  dbg_printf("[APP_ENTRY] app_entry_start() ENTRY - DefaultTask init\r\n");
  dbg_printf("[APP_ENTRY] ===========================================\r\n");
  
  if (started) {
    // Already started - keep this task alive
    dbg_printf("[APP_ENTRY] WARNING: Re-entry detected! Entering keep-alive loop\r\n");
    for(;;) {
      osDelay(1000);
    }
  }
  started = 1u;
  
  dbg_printf("[APP_ENTRY] First entry - calling app_init_and_start()\r\n");
  
  // One-time init + task creation
  app_init_and_start();
  
  dbg_printf("[APP_ENTRY] app_init_and_start() returned successfully\r\n");
  dbg_printf("[APP_ENTRY] Entering infinite idle loop - DefaultTask will sleep here\r\n");
  dbg_printf("[APP_ENTRY] All application work happens in dedicated tasks\r\n");
  
  // Print stack usage before entering idle loop
  TaskHandle_t handle = xTaskGetCurrentTaskHandle();
  UBaseType_t high_water = uxTaskGetStackHighWaterMark(handle);
  dbg_printf("[APP_ENTRY] DefaultTask stack: %lu bytes free (high water mark)\r\n", 
             (unsigned long)(high_water * sizeof(StackType_t)));
  
  dbg_printf("[APP_ENTRY] ===========================================\r\n");
  dbg_printf("[APP_ENTRY] ENTERING INFINITE LOOP - should never exit!\r\n");
  dbg_printf("[APP_ENTRY] ===========================================\r\n");
  dbg_printf("\r\n");
  
  // CRITICAL: Never return from this function!
  // Test mode stays in infinite loop without returning.
  // Production mode must do the same to prevent DefaultTask from exiting.
  // This task now just sleeps - all work happens in other tasks.
  uint32_t loop_count = 0;
  for(;;) {
    osDelay(1000);  // 1 second idle loop
    
    // Heartbeat every 60 seconds to prove loop is running
    loop_count++;
    if (loop_count % 60 == 0) {
      high_water = uxTaskGetStackHighWaterMark(handle);
      dbg_printf("[APP_ENTRY] Still alive! Loop count: %lu, Stack free: %lu bytes\r\n",
                 (unsigned long)loop_count, (unsigned long)(high_water * sizeof(StackType_t)));
    }
  }
  
  // SHOULD NEVER REACH HERE!
  dbg_printf("[APP_ENTRY] FATAL: Infinite loop exited! This is impossible!\r\n");
}
