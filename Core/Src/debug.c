#include "main.h"
#include "stm32f4xx_hal.h"
#include <string.h>
#include <stdint.h>

// UART2 handle défini dans main.c
extern UART_HandleTypeDef huart2;

void dbg_print(const char *s)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)s, strlen(s), 100);
}
