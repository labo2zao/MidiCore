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
  
  // CRITICAL FIX: Minimize operations in this hook!
  // Any blocking operation (USB CDC, UI updates) can prevent cleanup
  // and cause infinite loop in prvCheckTasksWaitingTermination()
  
  // Try to log (but don't wait if USB CDC not ready)
  dbg_printf("[FATAL] Stack overflow #%lu in task: %s\r\n", 
             (unsigned long)s_overflow_count, pcTaskName);
  
  // Set panic state (these should be non-blocking)
  panic_set(PANIC_STACK_OVERFLOW);
  safe_mode_set_forced(1u);
  
  // FORCE IMMEDIATE RESET - don't call other functions that might block
  // Don't use ui_set_status_line or watchdog_panic - they might wait
  NVIC_SystemReset();
  
  // REMOVED: for(;;) after reset - this is unreachable dead code
  // If reset fails, we'll just continue and let the system crash naturally
}

void vApplicationMallocFailedHook(void) {
  // Compteur pour détecter les échecs répétés
  static volatile uint32_t s_malloc_fail_count = 0;
  s_malloc_fail_count++;
  
  // CRITICAL FIX: Minimize operations in this hook!
  // Any blocking can prevent cleanup
  
  dbg_printf("[FATAL] Malloc failed! Count: %lu - heap exhausted\r\n", 
             (unsigned long)s_malloc_fail_count);
  
  // Set panic state (non-blocking)
  panic_set(PANIC_MALLOC_FAILED);
  safe_mode_set_forced(1u);
  
  // FORCE IMMEDIATE RESET - don't wait for anything
  NVIC_SystemReset();
  
  // REMOVED: for(;;) after reset - unreachable dead code
}
