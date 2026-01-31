#include "App/app_entry.h"

#include "App/app_init.h"
#include "cmsis_os.h"

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
  if (started) {
    // Already started - keep this task alive
    for(;;) {
      osDelay(1000);
    }
  }
  started = 1u;

  // One-time init + task creation
  app_init_and_start();

  // CRITICAL: Never return from this function!
  // Test mode stays in infinite loop without returning.
  // Production mode must do the same to prevent DefaultTask from exiting.
  // This task now just sleeps - all work happens in other tasks.
  for(;;) {
    osDelay(1000);  // 1 second idle loop
  }
  
  // SHOULD NEVER REACH HERE!
}
