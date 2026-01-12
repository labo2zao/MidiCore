#include "Services/system/panic.h"
// Include main.h for portable STM32 HAL (F4/F7/H7 compatibility)
#include "main.h"

static volatile uint32_t g_panic_code = 0;

void panic_set(uint32_t code)
{
  g_panic_code = code;
  (void)g_panic_code;

  __disable_irq();
  while(1) { __NOP(); }
}
