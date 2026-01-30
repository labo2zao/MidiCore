#include "Services/system/panic.h"
#include "Services/safe/safe_mode.h"
#include "Services/ui/ui.h"
#include "Services/watchdog/watchdog.h"
#include "App/tests/test_debug.h"

#include "FreeRTOS.h"
#include "task.h"

// Counter to detect repeated overflow (indicates looping)
static volatile uint32_t s_overflow_count = 0;

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  (void)xTask;
  
  // Increment counter to detect if we're looping
  s_overflow_count++;
  
  // Debug: affiche le nom de la tâche en overflow avec compteur
  dbg_printf("[FATAL] Stack overflow #%lu in task: %s\r\n", 
             (unsigned long)s_overflow_count, pcTaskName);
  
  // Si on boucle (plus de 3 overflows), forcer un reset complet
  if (s_overflow_count > 3) {
    dbg_printf("[FATAL] Multiple overflows detected - forcing system reset\r\n");
    NVIC_SystemReset();
  }
  
  // Breakpoint automatique pour debug (seulement au premier overflow)
  if (s_overflow_count == 1) {
    __BKPT(0);
  }
  
  panic_set(PANIC_STACK_OVERFLOW);
  safe_mode_set_forced(1u);
  ui_set_status_line("PANIC STK");
  watchdog_panic();
  
  // Si on arrive ici sans reset, boucler
  for(;;) { }
}

void vApplicationMallocFailedHook(void) {
  // Compteur pour détecter les échecs répétés
  static volatile uint32_t s_malloc_fail_count = 0;
  s_malloc_fail_count++;
  
  dbg_printf("[FATAL] Malloc failed! Count: %lu\r\n", (unsigned long)s_malloc_fail_count);
  dbg_printf("[FATAL] FreeRTOS heap exhausted - check configTOTAL_HEAP_SIZE\r\n");
  
  // Breakpoint automatique
  __BKPT(0);
  
  panic_set(PANIC_MALLOC_FAILED);
  safe_mode_set_forced(1u);
  ui_set_status_line("PANIC MAL");
  watchdog_panic();
  
  // Si multiple échecs, forcer reset
  if (s_malloc_fail_count > 3) {
    NVIC_SystemReset();
  }
  
  for(;;) { }
}
