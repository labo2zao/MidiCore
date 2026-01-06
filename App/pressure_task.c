#include "App/pressure_task.h"
#include "cmsis_os2.h"
#include "Services/pressure/pressure_i2c.h"
#include "Services/expression/expression.h"

static void PressureTask(void* argument){
  (void)argument;
  for(;;){
    const pressure_cfg_t* c = pressure_get_cfg();
    if(c->enable){
      int32_t raw=0;
      if(pressure_read_once(&raw)==0){
        uint16_t v12 = pressure_to_12b(raw);
        expression_set_raw(v12);
        expression_set_pressure_pa(raw);
      }
      osDelay(c->interval_ms ? c->interval_ms : 5);
    } else {
      osDelay(20);
    }
  }
}

void app_start_pressure_task(void){
  const osThreadAttr_t attr = {
    .name="Pressure",
    .priority=osPriorityNormal,
    .stack_size=768
  };
  (void)osThreadNew(PressureTask, NULL, &attr);
}
