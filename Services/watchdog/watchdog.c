#include "Services/watchdog/watchdog.h"
// Include main.h for portable STM32 HAL (F4/F7/H7 compatibility)
#include "main.h"
#include "Services/system/safe_mode.h"
#include "Services/ui/ui_status.h"
#include <stdbool.h>

/* ============================================================================
 * WATCHDOG CONFIGURATION
 * ============================================================================
 * The Independent Watchdog (IWDG) uses the LSI clock (~32 kHz).
 * 
 * Timeout calculation:
 *   timeout = (prescaler * reload) / LSI_freq
 * 
 * With prescaler=128, reload=625:
 *   timeout = (128 * 625) / 32000 = 2.5 seconds
 * 
 * This gives enough margin for the 100ms service tick while still
 * catching stuck tasks within a reasonable time.
 */

#define IWDG_PRESCALER      IWDG_PRESCALER_128
#define IWDG_RELOAD_VALUE   625U  /* ~2.5s timeout at 32kHz LSI */

/* Static IWDG handle - no need to expose globally */
static IWDG_HandleTypeDef s_hiwdg;
static bool s_watchdog_enabled = false;

void watchdog_panic_code(uint32_t code)
{
  (void)code;
  safe_mode_set_forced(1);
  ui_set_status_line("WDT PANIC -> reset");
  NVIC_SystemReset();
}

void watchdog_init(void)
{
  /* Configure the Independent Watchdog (IWDG)
   * The IWDG is clocked from LSI (~32 kHz on STM32F4)
   * Once started, the IWDG cannot be stopped - it must be refreshed
   * periodically or the system will reset.
   */
  s_hiwdg.Instance = IWDG;
  s_hiwdg.Init.Prescaler = IWDG_PRESCALER;
  s_hiwdg.Init.Reload = IWDG_RELOAD_VALUE;
  
  if (HAL_IWDG_Init(&s_hiwdg) == HAL_OK) {
    s_watchdog_enabled = true;
    ui_set_status_line("IWDG enabled");
  } else {
    /* IWDG init failed - system continues without hardware watchdog
     * This can happen if LSI oscillator is not stable yet.
     * The status line provides visual feedback for debugging. */
    ui_set_status_line("IWDG init FAIL");
  }
}

void watchdog_kick(void)
{
  if (s_watchdog_enabled) {
    HAL_IWDG_Refresh(&s_hiwdg);
  }
}
