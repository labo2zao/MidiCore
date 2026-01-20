# Protecting FreeRTOS Tasks When Regenerating with CubeMX

## The Problem

When you regenerate code with STM32CubeMX, it can overwrite your custom code if you don't place it in the correct "USER CODE" sections. This guide shows you how to protect your FreeRTOS tasks and other custom code.

---

## How CubeMX Protects User Code

CubeMX uses special comment markers to preserve your code:

```c
/* USER CODE BEGIN [section_name] */
   Your custom code here - THIS WILL BE PRESERVED
/* USER CODE END [section_name] */
```

**Everything between these markers is preserved** when you regenerate code.
**Everything outside these markers will be overwritten.**

---

## FreeRTOS Task Protection

### 1. Task Functions (Implementation)

Your FreeRTOS task implementations should ALWAYS be inside USER CODE sections:

**✅ CORRECT** - This will be preserved:
```c
/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Main production task
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_HOST */
  MX_USB_HOST_Init();
  /* USER CODE BEGIN 5 */
  
  // ✅ Your custom task code here - PRESERVED
  usb_host_midi_init();
  
  module_test_t selected_test = module_tests_get_compile_time_selection();
  
  if (selected_test != MODULE_TEST_NONE) {
    module_tests_run(selected_test);
  } else {
    // Normal application code
    while(1) {
      usb_host_midi_task();
      osDelay(1);
    }
  }
  
  /* USER CODE END 5 */
}
```

**❌ WRONG** - This will be overwritten:
```c
void StartDefaultTask(void *argument)
{
  /* init code for USB_HOST */
  MX_USB_HOST_Init();
  /* USER CODE BEGIN 5 */
  /* USER CODE END 5 */
  
  // ❌ Code here will be LOST on regeneration!
  usb_host_midi_init();
  while(1) {
    usb_host_midi_task();
    osDelay(1);
  }
}
```

---

### 2. Creating Additional FreeRTOS Tasks

If you want to create additional tasks, add them in the `RTOS_THREADS` section:

```c
/* USER CODE BEGIN RTOS_THREADS */

// ✅ Create additional tasks here - PRESERVED
osThreadAttr_t usbMidiTask_attributes = {
  .name = "usbMidiTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
usbMidiTaskHandle = osThreadNew(StartUsbMidiTask, NULL, &usbMidiTask_attributes);

osThreadAttr_t routerTask_attributes = {
  .name = "routerTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};
routerTaskHandle = osThreadNew(StartRouterTask, NULL, &routerTask_attributes);

/* USER CODE END RTOS_THREADS */
```

---

### 3. Task Function Declarations

If you create new tasks, declare them in the private function prototypes section:

```c
/* USER CODE BEGIN PFP */

// ✅ Declare your task functions here - PRESERVED
void StartUsbMidiTask(void *argument);
void StartRouterTask(void *argument);

/* USER CODE END PFP */
```

---

### 4. Task Handles (Global Variables)

If you need handles for your tasks, declare them in the private variables section:

```c
/* USER CODE BEGIN PV */

// ✅ Your task handles here - PRESERVED
osThreadId_t usbMidiTaskHandle;
osThreadId_t routerTaskHandle;

/* USER CODE END PV */
```

---

## Complete Example: Adding a USB MIDI Task

Here's a complete example of adding a USB MIDI processing task:

### Step 1: Add Task Handle and Declaration

```c
/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

osThreadId_t usbMidiTaskHandle;  // ✅ Preserved

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

void StartUsbMidiTask(void *argument);  // ✅ Preserved

/* USER CODE END PFP */
```

### Step 2: Create Task in RTOS_THREADS Section

```c
/* USER CODE BEGIN RTOS_THREADS */

// Create USB MIDI processing task
const osThreadAttr_t usbMidiTask_attributes = {
  .name = "usbMidiTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
usbMidiTaskHandle = osThreadNew(StartUsbMidiTask, NULL, &usbMidiTask_attributes);

/* USER CODE END RTOS_THREADS */
```

### Step 3: Implement Task Function

```c
/* USER CODE BEGIN Header_StartUsbMidiTask */
/**
 * @brief USB MIDI processing task
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartUsbMidiTask */
void StartUsbMidiTask(void *argument)
{
  /* USER CODE BEGIN StartUsbMidiTask */
  
  // Task initialization
  usb_midi_init();
  
  // Task loop
  for(;;)
  {
    // Process USB MIDI messages
    usb_midi_rx_packet(NULL);  // Your processing here
    
    // Yield to other tasks
    osDelay(1);
  }
  
  /* USER CODE END StartUsbMidiTask */
}
```

---

## Important USER CODE Sections in main.c

Here are the key USER CODE sections you should know:

| Section | Location | Purpose |
|---------|----------|---------|
| `USER CODE BEGIN Includes` | After default includes | Add your header files |
| `USER CODE BEGIN PV` | Private variables | Add global variables |
| `USER CODE BEGIN PFP` | Function prototypes | Declare your functions |
| `USER CODE BEGIN 0` | Before main() | Add helper functions |
| `USER CODE BEGIN 1` | Start of main() | Early initialization |
| `USER CODE BEGIN 2` | After peripheral init | Late initialization |
| `USER CODE BEGIN RTOS_THREADS` | FreeRTOS setup | Create additional tasks |
| `USER CODE BEGIN 5` | Inside task function | Task implementation |

---

## Best Practices

### ✅ DO:
- **Always** put custom code inside USER CODE sections
- Keep task implementations between `/* USER CODE BEGIN 5 */` and `/* USER CODE END 5 */`
- Create new tasks in `RTOS_THREADS` section
- Test after each CubeMX regeneration

### ❌ DON'T:
- Add code outside USER CODE sections
- Modify CubeMX-generated code directly
- Delete USER CODE markers
- Assume regeneration won't affect your code

---

## Verifying Protection After CubeMX Regeneration

After regenerating with CubeMX:

1. **Check your task functions** still contain your code
2. **Compile** the project to catch any issues
3. **Search** for your custom function calls (like `usb_midi_init()`)
4. **Test** that tasks still work as expected

---

## Quick Checklist

Before regenerating with CubeMX:

- [ ] All custom code is inside USER CODE sections
- [ ] FreeRTOS task implementations are in `USER CODE BEGIN 5` sections
- [ ] New tasks created in `RTOS_THREADS` section
- [ ] Task handles declared in `USER CODE BEGIN PV`
- [ ] Function prototypes in `USER CODE BEGIN PFP`
- [ ] Backed up important changes (optional but recommended)

---

## Example: Current StartDefaultTask

Your current `StartDefaultTask` is already properly protected:

```c
void StartDefaultTask(void *argument)
{
  /* init code for USB_HOST */
  MX_USB_HOST_Init();
  /* USER CODE BEGIN 5 */    ← Protection starts here
  
  // ✅ All this code is SAFE from regeneration
  usb_host_midi_init();
  
  module_test_t selected_test = module_tests_get_compile_time_selection();
  
  if (selected_test != MODULE_TEST_NONE) {
    module_tests_run(selected_test);
  } else {
    // Run production application
    run_production_app();
  }
  
  /* USER CODE END 5 */      ← Protection ends here
}
```

**Result**: All your custom initialization and task logic is protected! ✅

---

## Summary

1. **Always use USER CODE sections** - CubeMX preserves everything inside them
2. **FreeRTOS tasks** go in `USER CODE BEGIN 5` section
3. **New tasks** created in `RTOS_THREADS` section
4. **Declarations** go in `PFP` and `PV` sections
5. **Test after regeneration** to verify everything works

Your FreeRTOS tasks are now safe from CubeMX regeneration! 🎉
