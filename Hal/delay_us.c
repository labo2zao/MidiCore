#include "Hal/delay_us.h"
// Include main.h for portable STM32 HAL (F4/F7/H7 compatibility)
#include "main.h"

static uint8_t dwt_inited = 0;

static void dwt_init(void) {
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  dwt_inited = 1;
}

void delay_us(uint32_t us) {
  if (!dwt_inited) dwt_init();
  uint32_t start = DWT->CYCCNT;
  uint32_t cycles = (HAL_RCC_GetHCLKFreq() / 1000000u) * us;
  while ((DWT->CYCCNT - start) < cycles) { }
}
