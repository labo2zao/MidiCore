#include "cmsis_os2.h"
#include "Services/looper/looper.h"

#ifdef LOOPER_SELFTEST
static void LooperSelfTestTask(void *argument) {
  (void)argument;
  osDelay(1000);
  looper_set_quant(0, LOOPER_QUANT_1_16);
  looper_set_loop_beats(0, 4);
  for (;;) {
    looper_set_state(0, LOOPER_STATE_REC);
    osDelay(7000);
    looper_set_state(0, LOOPER_STATE_PLAY);
    osDelay(8000);
    looper_set_state(0, LOOPER_STATE_STOP);
    osDelay(2000);
  }
}
void app_start_looper_selftest(void) {
  const osThreadAttr_t attr = { .name="LooperTest", .priority=osPriorityLow, .stack_size=1024 };
  (void)osThreadNew(LooperSelfTestTask, NULL, &attr);
}
#else
void app_start_looper_selftest(void) {}
#endif
