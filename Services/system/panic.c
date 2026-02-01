#include "Services/system/panic.h"
// Include main.h for portable STM32 HAL (F4/F7/H7 compatibility)
#include "main.h"

static volatile uint32_t g_panic_code = 0;

/**
 * @brief Get the current panic code (for inspection after reset)
 */
uint32_t panic_get(void)
{
  return g_panic_code;
}

/**
 * @brief Record panic code and HALT the system
 * 
 * IMPORTANT: This function does NOT return!
 * Do all critical operations (safe_mode_set, etc.) BEFORE calling this.
 * 
 * The infinite loop allows a debugger to:
 * 1. Attach to the halted processor
 * 2. Inspect g_panic_code and other globals
 * 3. Examine stack frames and fault registers
 * 
 * For production auto-recovery, call NVIC_SystemReset() BEFORE panic_set()
 * or use watchdog timeout to force reset.
 */
void panic_set(uint32_t code)
{
  g_panic_code = code;
  
  /* Disable interrupts to prevent further corruption */
  __disable_irq();
  
  /* Infinite loop - attach debugger to inspect state */
  while(1) { 
    __NOP(); 
  }
}
