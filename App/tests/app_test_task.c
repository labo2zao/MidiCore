/**
 * @file app_test_task.c
 * @brief Implementation of dedicated test task
 */

#include "App/tests/app_test_task.h"
#include "App/tests/module_tests.h"
#include "cmsis_os2.h"

// Test task handle
static osThreadId_t testTaskHandle = NULL;

// Test task attributes
static const osThreadAttr_t testTask_attributes = {
  .name = "testTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/**
 * @brief Test task implementation
 */
void StartTestTask(void* argument)
{
  (void)argument;
  
  // Initialize test framework
  module_tests_init();
  
  // Check if a test was selected at compile time
  module_test_t selected_test = module_tests_get_compile_time_selection();
  
  if (selected_test != MODULE_TEST_NONE) {
    // Run the selected test
    // Note: most tests run forever and don't return
    module_tests_run(selected_test);
  }
  
  // If we get here, no test was selected or test returned
  // Just idle
  for (;;) {
    osDelay(1000);
  }
}

/**
 * @brief Create the test task
 */
int app_test_task_create(void)
{
  if (testTaskHandle != NULL) {
    // Task already created
    return -1;
  }
  
  testTaskHandle = osThreadNew(StartTestTask, NULL, &testTask_attributes);
  
  if (testTaskHandle == NULL) {
    return -1;
  }
  
  return 0;
}
