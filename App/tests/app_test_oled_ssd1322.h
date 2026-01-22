/**
 * @file app_test_oled_ssd1322.h
 * @brief Header for SSD1322 OLED driver test suite
 */

#ifndef APP_TEST_OLED_SSD1322_H
#define APP_TEST_OLED_SSD1322_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Initialize OLED test suite
 * @return 0 on success, -1 on failure
 */
int test_oled_init(void);

/**
 * @brief Run complete OLED test sequence
 * 
 * This will test:
 * - GPIO pin control
 * - Software SPI communication
 * - OLED initialization
 * - Display patterns
 * 
 * @return 0 on success, negative on failure
 */
int test_oled_run(void);

/**
 * @brief Run minimal OLED test (just unlock + display ON)
 * 
 * Use this to quickly verify if OLED hardware responds
 * 
 * @return 0 on success
 */
int test_oled_minimal(void);

#ifdef __cplusplus
}
#endif

#endif // APP_TEST_OLED_SSD1322_H
