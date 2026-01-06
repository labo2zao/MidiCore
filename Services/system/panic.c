#include "Services/system/panic.h"
#include "stm32f4xx_hal.h"

static volatile uint32_t g_panic_code = 0;

void panic_set(uint32_t code)
{
  g_panic_code = code;
  (void)g_panic_code;

  __disable_irq();
  while(1) { __NOP(); }
}
