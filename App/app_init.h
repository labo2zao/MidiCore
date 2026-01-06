#pragma once

/**
 * Call once after CubeMX init (GPIO/SPI/RTOS) and before/after osKernelStart depending on your setup.
 * Recommended: call AFTER osKernelInitialize and BEFORE osKernelStart.
 *
 * This function initializes:
 * - spibus
 * - ainser64 hal
 * - AIN service
 * - OLED driver
 * and starts the AinTask (5ms tick) + a small OLED demo task.
 */
void app_init_and_start(void);
