/**
 * @file freertos_hooks.c
 * @brief FreeRTOS error hooks - MIOS32-style (no printf in ISR context!)
 * 
 * CRITICAL: These hooks run in ISR/kernel context with minimal stack!
 * NO printf/dbg_printf allowed - uses 500+ bytes of stack!
 * 
 * BEHAVIOR controlled by PANIC_AUTO_RESET in project_config.h:
 * - PANIC_AUTO_RESET=0: HALT for debugging (attach debugger to inspect)
 * - PANIC_AUTO_RESET=1: AUTO-RESET for production (system recovers)
 */

#include "Config/project_config.h"
#include "Services/system/panic.h"
#include "Services/safe/safe_mode.h"

#include "FreeRTOS.h"
#include "task.h"

/* CMSIS for NVIC_SystemReset() */
#include "stm32f4xx.h"

/* Counters visible in debugger */
volatile uint32_t g_stack_overflow_count = 0;
volatile uint32_t g_malloc_fail_count = 0;

/* Task name that caused overflow - visible in debugger */
volatile const char* g_overflow_task_name = NULL;

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  (void)xTask;
  
  /* Save task name for debugger inspection */
  g_overflow_task_name = pcTaskName;
  
  /* Increment counter - visible in debugger */
  g_stack_overflow_count++;
  
  /* Set safe_mode so next boot can detect the failure */
  safe_mode_set_forced(1u);
  
#if PANIC_AUTO_RESET
  /* PRODUCTION MODE: Auto-reset for recovery */
  NVIC_SystemReset();
#endif
  
  /* DEBUG MODE (or if reset fails): Halt for debugger inspection
   * Attach debugger and check:
   * - g_overflow_task_name: which task overflowed
   * - g_stack_overflow_count: how many times
   */
  panic_set(PANIC_STACK_OVERFLOW);
}

void vApplicationMallocFailedHook(void) {
  /* Increment counter - visible in debugger */
  g_malloc_fail_count++;
  
  /* Set safe_mode so next boot can detect the failure */
  safe_mode_set_forced(1u);
  
#if PANIC_AUTO_RESET
  /* PRODUCTION MODE: Auto-reset for recovery */
  NVIC_SystemReset();
#endif
  
  /* DEBUG MODE (or if reset fails): Halt for debugger inspection
   * Attach debugger and check:
   * - g_malloc_fail_count: how many times
   * - FreeRTOS heap state
   */
  panic_set(PANIC_MALLOC_FAILED);
}
