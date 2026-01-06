#include "Services/system/panic.h"
#include "Services/safe/safe_mode.h"
#include "Services/ui/ui.h"
#include "Services/watchdog/watchdog.h"

#include "FreeRTOS.h"
#include "task.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  (void)xTask;
  (void)pcTaskName;
  panic_set(PANIC_STACK_OVERFLOW);
  safe_mode_set_forced(1u);
  ui_set_status_line("PANIC STK");
  watchdog_panic();
  for(;;) { }
}

void vApplicationMallocFailedHook(void) {
  panic_set(PANIC_MALLOC_FAILED);
  safe_mode_set_forced(1u);
  ui_set_status_line("PANIC MAL");
  watchdog_panic();
  for(;;) { }
}
