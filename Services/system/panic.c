#include "Services/system/panic.h"
// Include main.h for portable STM32 HAL (F4/F7/H7 compatibility)
#include "main.h"

static volatile uint32_t g_panic_code = 0;

/**
 * @brief Record panic code - DOES NOT BLOCK
 * 
 * This function just records the panic code for later inspection.
 * The caller decides what to do next (reset, loop, etc.)
 * 
 * IMPORTANT: This function intentionally does NOT enter an infinite loop
 * to allow caller to perform cleanup (set safe_mode, reset, etc.) before
 * any final action.
 */
void panic_set(uint32_t code)
{
  g_panic_code = code;
  /* Keep variable alive for debugger inspection */
  (void)g_panic_code;
}
