/**
 * @file app_test_task.h
 * @brief Dedicated FreeRTOS task for module testing
 * 
 * This provides a clean separation between production code (StartDefaultTask)
 * and test code (StartTestTask).
 */

#ifndef APP_TEST_TASK_H
#define APP_TEST_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Test task entry point
 * 
 * This function is the entry point for the dedicated test task.
 * It will be called as a FreeRTOS task function.
 * 
 * @param argument FreeRTOS task argument (unused)
 * 
 * @note This function runs module tests based on compile-time configuration.
 * Define MODULE_TEST_xxx at compile time to select a test, or run the default
 * production code if no test is selected.
 */
void StartTestTask(void* argument);

/**
 * @brief Create and start the test task
 * 
 * Call this to create the test task as a FreeRTOS thread.
 * The task will start automatically once the scheduler is running.
 * 
 * @return 0 on success, negative on error
 */
int app_test_task_create(void);

#ifdef __cplusplus
}
#endif

#endif // APP_TEST_TASK_H
