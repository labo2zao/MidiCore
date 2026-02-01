/**
 * @file app_entry.c
 * @brief MIOS32-style application entry point
 * 
 * MIOS32 ARCHITECTURE:
 * - NO printf/snprintf/vsnprintf (causes stack overflow)
 * - Silent initialization
 * - Debug via MIOS Studio terminal (SysEx protocol)
 */

#include "App/app_entry.h"
#include "App/app_init.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"

/* Global counters for debugger visibility (no printf!) */
volatile uint32_t g_app_entry_count = 0;
volatile uint32_t g_app_loop_count = 0;
volatile uint32_t g_app_stack_free = 0;

/**
 * @note This runs in the context of the CubeMX default task.
 *       It should do one-time init and start other tasks, then never return.
 * 
 * @important This function MUST NOT RETURN to match test mode behavior.
 *            Test mode runs forever in its test loop without returning.
 *            Production mode must also run forever to avoid DefaultTask exit.
 * 
 * MIOS32-STYLE: NO printf - use debugger to view g_app_* variables
 */
void app_entry_start(void)
{
  static uint8_t started = 0u;
  
  g_app_entry_count++;
  
  if (started) {
    /* Re-entry detected - keep alive without logging */
    for(;;) {
      osDelay(1000);
    }
  }
  started = 1u;
  
  /* One-time init + task creation */
  app_init_and_start();
  
  /* Get stack info for debugger */
  TaskHandle_t handle = xTaskGetCurrentTaskHandle();
  UBaseType_t high_water = uxTaskGetStackHighWaterMark(handle);
  g_app_stack_free = (uint32_t)(high_water * sizeof(StackType_t));
  
  /* CRITICAL: Never return from this function!
   * Test mode stays in infinite loop without returning.
   * Production mode must do the same to prevent DefaultTask from exiting.
   * This task now just sleeps - all work happens in other tasks.
   */
  for(;;) {
    osDelay(1000);  /* 1 second idle loop */
    g_app_loop_count++;
    
    /* Update stack info for debugger every 60 seconds */
    if ((g_app_loop_count % 60) == 0) {
      high_water = uxTaskGetStackHighWaterMark(handle);
      g_app_stack_free = (uint32_t)(high_water * sizeof(StackType_t));
    }
  }
  
  /* SHOULD NEVER REACH HERE! */
}
