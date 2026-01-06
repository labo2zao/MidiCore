#include "Services/watchdog/watchdog.h"
#include "stm32f4xx_hal.h"
#include "Services/system/safe_mode.h"
#include "Services/ui/ui_status.h"

void watchdog_panic_code(uint32_t code)
{
  (void)code;
  safe_mode_set_forced(1);
  ui_set_status_line("WDT PANIC -> reset");
  NVIC_SystemReset();
}
