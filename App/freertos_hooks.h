#ifndef FREERTOS_HOOKS_H
#define FREERTOS_HOOKS_H

#include "FreeRTOS.h"
#include "task.h"

// Hook function declarations
void vApplicationMallocFailedHook(void);
void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName);
void vApplicationIdleHook(void);

#endif /* FREERTOS_HOOKS_H */
