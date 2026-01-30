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
  
  // CRITICAL: Ne JAMAIS boucler dans ce hook!
  // Cela empêche FreeRTOS de nettoyer la tâche et cause une boucle
  // infinie dans prvCheckTasksWaitingTermination()
  
  // Si c'est le premier overflow, essayer de capturer l'état
  if (s_overflow_count == 1) {
    panic_set(PANIC_STACK_OVERFLOW);
    safe_mode_set_forced(1u);
    ui_set_status_line("PANIC STK");
    watchdog_panic();
    __BKPT(0);  // Breakpoint pour debug
  }
  
  // Forcer reset immédiat pour éviter la boucle infinie
  dbg_printf("[FATAL] Forcing immediate system reset to prevent hang\r\n");
  NVIC_SystemReset();
  
  // Ne devrait jamais arriver ici
  for(;;) { }
}

void vApplicationMallocFailedHook(void) {
  // Compteur pour détecter les échecs répétés
  static volatile uint32_t s_malloc_fail_count = 0;
  s_malloc_fail_count++;
  
  dbg_printf("[FATAL] Malloc failed! Count: %lu\r\n", (unsigned long)s_malloc_fail_count);
  dbg_printf("[FATAL] FreeRTOS heap exhausted - check configTOTAL_HEAP_SIZE\r\n");
  
  // Premier échec : capturer l'état
  if (s_malloc_fail_count == 1) {
    panic_set(PANIC_MALLOC_FAILED);
    safe_mode_set_forced(1u);
    ui_set_status_line("PANIC MAL");
    watchdog_panic();
    __BKPT(0);  // Breakpoint pour debug
  }
  
  // Forcer reset immédiat pour éviter comportement indéfini
  dbg_printf("[FATAL] Forcing immediate system reset\r\n");
  NVIC_SystemReset();
  
  // Ne devrait jamais arriver ici
  for(;;) { }
}
