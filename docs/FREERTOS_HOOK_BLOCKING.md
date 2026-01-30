# FreeRTOS Hook Blocking Issues - Root Cause Analysis

## Problem Report

**Symptom 1:** `while( uxDeletedTasksWaitingCleanUp > (UBaseType_t) 0U )` boucle infinie  
**Symptom 2:** Timer service task blocked

## Root Cause: Blocking Operations in FreeRTOS Hooks

### The Mechanism

FreeRTOS has an internal cleanup mechanism in the idle task:

```c
// In FreeRTOS tasks.c (idle task):
static void prvCheckTasksWaitingTermination( void ) {
    while( uxDeletedTasksWaitingCleanUp > ( UBaseType_t ) 0U ) {
        // Try to clean up deleted/overflowed tasks
        // But if the task is still blocked in a hook, this never completes!
    }
}
```

**Normal Flow:**
1. Task overflows stack
2. FreeRTOS calls `vApplicationStackOverflowHook()`
3. Hook does minimal logging
4. Hook returns or resets system
5. Idle task cleans up the task
6. `uxDeletedTasksWaitingCleanUp` decrements
7. System continues or resets

**Broken Flow (What Was Happening):**
1. Task overflows stack
2. FreeRTOS calls `vApplicationStackOverflowHook()`
3. Hook calls `ui_set_status_line()` → **BLOCKS waiting for UI task!**
4. Hook calls `watchdog_panic()` → **Delays/waits**
5. Hook calls `__BKPT(0)` → **Stops execution in debugger**
6. Hook makes multiple `dbg_printf()` calls → **Can block on USB CDC**
7. Hook finally calls `NVIC_SystemReset()`
8. But then has `for(;;)` → **Infinite loop if reset delayed**
9. **Idle task NEVER gets to clean up**
10. **System frozen in `prvCheckTasksWaitingTermination()` forever**

### The Fix

**BEFORE (BROKEN):**
```c
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  s_overflow_count++;
  dbg_printf("[FATAL] Stack overflow #%lu in task: %s\r\n", ...);
  
  if (s_overflow_count == 1) {
    panic_set(PANIC_STACK_OVERFLOW);
    safe_mode_set_forced(1u);
    ui_set_status_line("PANIC STK");    // ❌ BLOCKS for UI task!
    watchdog_panic();                   // ❌ Can delay!
    __BKPT(0);                          // ❌ Stops in debugger!
  }
  
  dbg_printf("[FATAL] Forcing reset\r\n");  // ❌ Another block risk
  NVIC_SystemReset();
  for(;;) { }                           // ❌ Infinite loop!
}
```

**AFTER (FIXED):**
```c
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
  s_overflow_count++;
  
  // CRITICAL FIX: Minimize operations in this hook!
  // Any blocking operation (USB CDC, UI updates) can prevent cleanup
  // and cause infinite loop in prvCheckTasksWaitingTermination()
  
  // Try to log (but don't wait if USB CDC not ready)
  dbg_printf("[FATAL] Stack overflow #%lu in task: %s\r\n", ...);
  
  // Set panic state (these should be non-blocking)
  panic_set(PANIC_STACK_OVERFLOW);
  safe_mode_set_forced(1u);
  
  // FORCE IMMEDIATE RESET - don't call other functions that might block
  // Don't use ui_set_status_line or watchdog_panic - they might wait
  NVIC_SystemReset();
  
  // REMOVED: for(;;) after reset - this is unreachable dead code
  // If reset fails, we'll just continue and let the system crash naturally
}
```

### Operations That MUST NOT Be Called From Hooks

❌ **Never Call These From FreeRTOS Hooks:**

1. **UI Functions:**
   - `ui_set_status_line()` - waits for UI task to process
   - `ui_show_message()` - waits for OLED updates
   - Any function that queues to UI task

2. **Watchdog Functions:**
   - `watchdog_panic()` - may introduce delays
   - `watchdog_feed()` - unnecessary in panic state

3. **Breakpoints:**
   - `__BKPT(0)` - stops execution (hangs in debugger)
   - Use debugger's halt-on-exception instead

4. **Multiple Debug Prints:**
   - Multiple `dbg_printf()` calls - USB CDC can block
   - Keep to absolute minimum (1 line max)

5. **Infinite Loops:**
   - `for(;;)` after reset - unreachable dead code
   - If reset fails, let system crash naturally

6. **Delays/Waits:**
   - `osDelay()` - can't be called from hook context anyway
   - `HAL_Delay()` - unnecessary delay before reset
   - Any blocking operation

✅ **Safe Operations in Hooks:**

1. Set global state flags (non-blocking)
2. One minimal log message (best effort)
3. Immediate system reset (`NVIC_SystemReset()`)
4. Nothing else!

### Timer Task Issue

**Problem:** Timer task had only 256 words (1KB) stack - smallest in system

**Fix:** Increased to 512 words (2KB)

```c
// BEFORE:
#define configTIMER_TASK_STACK_DEPTH  256  // Too small!

// AFTER:
#define configTIMER_TASK_STACK_DEPTH  512  // Safe
```

Timer task is a FreeRTOS internal task that handles software timers. If timer callbacks are complex or nested, 1KB is insufficient and causes the task to block.

### Memory Impact

```
Task Stacks After Fix:
  DefaultTask:     12 KB
  CliTask:          8 KB
  Timer Task:       2 KB  ← +1KB increase
  Other tasks:     ~7 KB
  ---------------
  Total:           29 KB

FreeRTOS Heap:     30 KB  ← Still sufficient
```

### Testing

**How to Verify the Fix:**

1. **Induce Stack Overflow:**
   ```c
   void test_overflow(void) {
     char big_array[10000];  // Force stack overflow
     memset(big_array, 0, sizeof(big_array));
   }
   ```

2. **Expected Behavior:**
   - Single log message: `[FATAL] Stack overflow #1 in task: TestTask`
   - Immediate system reset
   - **NO freeze in cleanup loop**
   - **NO infinite loop**

3. **Previous (Broken) Behavior:**
   - Multiple log messages
   - UI update attempt (blocks)
   - Watchdog call (delays)
   - Breakpoint (stops in debugger)
   - System hangs in `prvCheckTasksWaitingTermination()`

### Key Insights

1. **Hooks Run in Critical Context:**
   - FreeRTOS hooks are called from scheduler context
   - They must be absolutely minimal
   - No waiting, no blocking, no inter-task communication

2. **Idle Task Dependency:**
   - Idle task is responsible for cleanup
   - If hook blocks, idle task can't complete cleanup
   - Result: infinite loop in cleanup mechanism

3. **Reset is Best Recovery:**
   - For fatal errors (stack overflow, malloc failure)
   - Best action is immediate reset
   - Don't try to be clever with recovery
   - Don't log excessively

4. **Debug vs Production:**
   - In debug: Use debugger halt-on-exception
   - In production: Use minimal logging + reset
   - Don't use breakpoints in production hooks

### References

- FreeRTOS Documentation: Application Hook Functions
- FreeRTOS Source: `tasks.c` - `prvCheckTasksWaitingTermination()`
- STM32 Reference: NVIC System Reset
- Memory: "FreeRTOS hook blocking prevention"

---

**Remember:** FreeRTOS hooks must be **atomic, non-blocking, and minimal**. Anything else risks system deadlock.
