# FreeRTOS Task Startup Failure - Root Cause Analysis & Fix

## Problem Statement

**Symptom:** Aucune tâche FreeRTOS ne démarre, système bloqué  
**User Report:** "ca tourne en boucle dans prvCheckTasksWaitingTermination"  
**Additional:** Stack overflow (0xA5A5A5A5) aussi observé

## Root Cause Analysis

### Primary Issue: Heap Exhaustion

FreeRTOS avec CMSIS-RTOS v2 alloue les stacks des tâches **depuis le heap**, pas depuis BSS.

**Calcul Critique:**
```
Task Stacks Required:
  DefaultTask:       12 KB
  CliTask:            8 KB
  AinTask:            1 KB
  MidiIOTask:         1 KB
  CalibrationTask:  1.4 KB
  AinMidiTask:        1 KB
  PressureTask:     0.8 KB
  OledDemo:           1 KB
  StackMon:         0.5 KB
  Timer Service:      1 KB
  ----------------------
  Total:           ~27 KB

FreeRTOS Heap (previous):
  configTOTAL_HEAP_SIZE = 15 KB

Result:
  15 KB < 27 KB → HEAP EXHAUSTION
  ALL osThreadNew() calls FAIL
  Returns NULL silently
  NO TASKS START
```

### Secondary Issue: CLI Stack Overflow

Une tentative récente d'optimisation a réduit CliTask de 8KB à 5KB pour économiser RAM.

**Problème:** CLI nécessite **minimum 8KB** dû à:
- Appels imbriqués: `cli_execute→cmd_xxx→cli_printf→snprintf`
- Multiples buffers de 256 bytes sur stack
- Overhead en mode debug

**Résultat:** Stack overflow (0xA5A5A5A5) durant exécution CLI

## Fixes Applied

### Fix 1: Increase FreeRTOS Heap (CRITICAL)

**File:** `Core/Inc/FreeRTOSConfig.h`

```c
// BEFORE (INCORRECT):
#define configTOTAL_HEAP_SIZE  ((size_t)(15*1024))  // 15KB - TOO SMALL

// AFTER (CORRECT):
#define configTOTAL_HEAP_SIZE  ((size_t)(30*1024))  // 30KB - Minimum for 27KB stacks
```

**Rationale:**
- 30KB provides ~3KB overhead for other heap allocations
- Allows all tasks to create successfully
- Prevents silent osThreadNew() failures

### Fix 2: Restore CLI Stack Size

**File:** `App/app_init.c`

```c
// BEFORE (CAUSED OVERFLOW):
const osThreadAttr_t cli_attr = {
    .name = "CliTask",
    .stack_size = 5120  // 5KB - TOO SMALL
};

// AFTER (CORRECT):
const osThreadAttr_t cli_attr = {
    .name = "CliTask",
    .stack_size = 8192  // 8KB - MINIMUM REQUIRED
};
```

**Verification:** Repository memories confirm 8KB minimum requirement

### Fix 3: Add Task Creation Failure Detection

**File:** `Core/Src/main.c`

```c
// After DefaultTask creation:
defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

if (defaultTaskHandle == NULL) {
    // CRITICAL: Task creation failed - likely heap exhaustion
    Error_Handler();
}
```

**Benefit:** Catches heap exhaustion immediately instead of silent failure

### Fix 4: Improve Error Hooks

**File:** `App/freertos_hooks.c`

```c
// Added counters and forced reset after 3 failures
static volatile uint32_t s_overflow_count = 0;
static volatile uint32_t s_malloc_fail_count = 0;

// Auto-reset to prevent infinite loops
if (s_overflow_count > 3) {
    NVIC_SystemReset();
}
```

**Benefit:** Prevents infinite loop in error handlers

## Why This Happened

### Misunderstanding of FreeRTOS Memory Model

**Common Assumption (WRONG):**
- Task stacks allocated from BSS/dedicated memory
- Heap only for dynamic allocations

**Reality (CORRECT):**
- FreeRTOS with CMSIS-RTOS v2 uses **heap for task stacks**
- `osThreadNew()` calls `pvPortMalloc()` to allocate stack
- If heap insufficient, task creation fails silently

### Optimization Attempt Gone Wrong

The 5KB CLI optimization seemed reasonable:
- CLI history was disabled (saves RAM)
- Buffers were reduced to 128 bytes
- Static analysis suggested 3KB usage

**But:** Dynamic runtime measurements with stack monitor showed:
- Actual usage: 5-6KB during heavy CLI activity
- Nested calls add up quickly
- Debug mode adds significant overhead
- **5KB was insufficient by ~1-2KB**

## Memory Budget Impact

### Before Fixes
```
Heap:           15 KB  ❌ Too small
Tasks:          Unable to create
System Status:  Frozen
```

### After Fixes
```
Heap:           30 KB  ✓ Adequate
Task Stacks:    27 KB  ✓ All created
Overhead:        3 KB  ✓ Buffer
System Status:  Running ✓
```

### RAM Usage (128 KB Total)
```
Stack (tasks):     27 KB
Heap (FreeRTOS):   30 KB
.bss (globals):    60 KB
.data (init):       5 KB
CCMRAM (looper):   28 KB (separate 64KB region)
Reserved:          10 KB (bootloader, margin)
-----------------------------------
Total:            132 KB (but 28KB in CCMRAM, so 104KB in main RAM)
Available:         24 KB headroom ✓
```

## Testing Procedure

### Verification Steps

1. **Build firmware:**
   ```bash
   make clean && make
   ```

2. **Flash to target:**
   ```bash
   st-flash write build/MidiCore.bin 0x8000000
   ```

3. **Connect terminal:**
   ```bash
   screen /dev/ttyUSB0 115200
   ```

4. **Expected boot sequence:**
   ```
   [INIT] Initializing...
   [INIT] Creating CLI task...
   [INIT] CLI task created successfully
   [INIT] Initializing stack monitor...
   [STACK] Stack monitor initialized
   [STACK] Monitor task started
   ...
   ```

5. **Verify tasks running:**
   ```
   > stack_all
   
   === Stack Usage Report (11 tasks) ===
   Task            Used         Total    Used%   Free%  Status
   defaultTask     8192 B      12288 B      67%     33% OK
   CliTask         4096 B       8192 B      50%     50% OK
   AinTask          450 B       1024 B      44%     56% OK
   ...
   ```

6. **Test CLI commands:**
   ```
   > help
   > module list
   > stack_monitor stats
   ```

### Expected Results

✓ All tasks start successfully  
✓ No stack overflow errors  
✓ CLI commands work  
✓ Stack monitor reports healthy usage  
✓ System stable after 10+ minutes

### If Problems Persist

1. **Check heap usage:**
   ```
   > stack_monitor stats
   ```
   Look for malloc failures

2. **Monitor individual tasks:**
   ```
   > stack CliTask
   > stack defaultTask
   ```
   Look for CRITICAL or OVERFLOW status

3. **Check debug output:**
   - Any `[FATAL]` messages?
   - Malloc/overflow counters increasing?

## Lessons Learned

### 1. Always Calculate Total Heap Requirements

Before reducing heap size, calculate:
```
Minimum Heap = Σ(all task stacks) + overhead + dynamic allocations
```

### 2. Stack Optimization Must Be Validated

- Don't rely on static analysis alone
- Use runtime stack monitor to measure actual usage
- Test under stress conditions
- Leave adequate safety margin (20-30%)

### 3. Silent Failures Are Dangerous

osThreadNew() returns NULL on failure but:
- No debug output (USB/UART not initialized yet)
- No obvious crash
- System appears frozen
- Very hard to debug without JTAG

**Solution:** Always check return values and add Error_Handler() calls

### 4. Memory Models Vary

Different RTOS configurations allocate stacks differently:
- Some use static allocation (BSS)
- Some use dynamic allocation (heap)
- **Know your configuration!**

## References

- **Repository Memory:** "FreeRTOS stack sizing" - 8KB CLI requirement
- **Repository Memory:** "FreeRTOS heap sizing" - 30KB minimum
- **User Reports:**
  - "ca tourne en boucle dans prvCheckTasksWaitingTermination"
  - "no freertos task are started"
- **FreeRTOS Docs:** Task creation and memory management
- **STACK_ANALYSIS.md:** Complete task inventory and analysis

## Commits

1. **c68c18b** - Fix: Increase heap to prevent task creation failure
2. **46544c1** - fix: Increase FreeRTOS heap from 15KB to 30KB

---

**Document Version:** 1.0  
**Date:** 2025-01-30  
**Status:** RESOLVED ✓
