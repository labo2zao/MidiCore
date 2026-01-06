#include "Services/system/boot_reason.h"

#if __has_include("stm32f4xx_hal.h")
  #include "stm32f4xx_hal.h"
  #define BR_HAS_HAL 1
#else
  #define BR_HAS_HAL 0
#endif

static boot_reason_t g_reason = BOOT_REASON_UNKNOWN;

void boot_reason_init(void) {
#if BR_HAS_HAL
  uint32_t csr = RCC->CSR;
  // detect reset flags
  if (csr & RCC_CSR_IWDGRSTF) g_reason = BOOT_REASON_WATCHDOG;
  else if (csr & RCC_CSR_BORRSTF) g_reason = BOOT_REASON_BROWNOUT;
  else if (csr & RCC_CSR_SFTRSTF) g_reason = BOOT_REASON_SOFTWARE;
  else if (csr & RCC_CSR_PORRSTF) g_reason = BOOT_REASON_POWERON;
  else g_reason = BOOT_REASON_UNKNOWN;

  // clear flags
  __HAL_RCC_CLEAR_RESET_FLAGS();
#else
  g_reason = BOOT_REASON_UNKNOWN;
#endif
}

boot_reason_t boot_reason_get(void) { return g_reason; }
