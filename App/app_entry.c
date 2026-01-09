#include "App/app_entry.h"

#include "App/app_init.h"
#include "cmsis_os.h"

/**
 * @note This runs in the context of the CubeMX default task.
 *       It should do one-time init and start other tasks.
 */
void app_entry_start(void)
{
  static uint8_t started = 0u;
  if (started) { return; }
  started = 1u;

  // One-time init + task creation
  //app_init_and_start(); // TODO faire fonctionner cette fonction

  // If you want: lower this task priority or delete it once init is done.
  // osThreadExit(); // uncomment if you want to end the default task after init
}
