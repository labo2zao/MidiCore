#include "Services/watchdog/watchdog.h"
// Include main.h for portable STM32 HAL (F4/F7/H7 compatibility)
#include "main.h"
#include "Services/system/safe_mode.h"
#include "Services/ui/ui_status.h"

void watchdog_panic_code(uint32_t code)
{
  (void)code;
  safe_mode_set_forced(1);
  ui_set_status_line("WDT PANIC -> reset");
  NVIC_SystemReset();
}

void watchdog_init(void)
{
#ifdef WATCHDOG_ENABLE
  // Hardware watchdog setup can be implemented here once the HAL IWDG module
  // is enabled. Keeping the hook in place ensures the symbol is available for
  // builds that want to turn the watchdog on without breaking existing
  // configurations.
#endif
}
