/**
 * @file freertos_hooks.c
 * @brief FreeRTOS error hooks - MIOS32-style (no printf in ISR context!)
 * 
 * CRITICAL: These hooks run in ISR/kernel context with minimal stack!
 * NO printf/dbg_printf allowed - uses 500+ bytes of stack!
 */

#include "Services/system/panic.h"
#include "Services/safe/safe_mode.h"

#include "FreeRTOS.h"
#include "task.h"

/* Counters visible in debugger */
volatile uint32_t g_stack_overflow_count = 0;
volatile uint32_t g_malloc_fail_count = 0;

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  (void)xTask;
  (void)pcTaskName;  /* Available in debugger via stack frame */
  
  /* Increment counter - visible in debugger */
  g_stack_overflow_count++;
  
  /* Set panic state (minimal operations only!) */
  panic_set(PANIC_STACK_OVERFLOW);
  safe_mode_set_forced(1u);
  
  /* FORCE IMMEDIATE RESET - no logging, no delays! */
  NVIC_SystemReset();
}

void vApplicationMallocFailedHook(void) {
  /* Increment counter - visible in debugger */
  g_malloc_fail_count++;
  
  /* Set panic state (minimal operations only!) */
  panic_set(PANIC_MALLOC_FAILED);
  safe_mode_set_forced(1u);
  
  /* FORCE IMMEDIATE RESET - no logging, no delays! */
  NVIC_SystemReset();
}
