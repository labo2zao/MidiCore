# Crash Debugging Guide for MidiCore

## How to See and Diagnose Crashes in Debug Mode

This guide explains how to detect, analyze, and fix crashes in the MidiCore firmware running on STM32F407 with FreeRTOS.

---

## 1. Hard Fault Detection

### Setting Up Hard Fault Handler

**Add breakpoint in `Core/Src/stm32f4xx_it.c`:**

```c
void HardFault_Handler(void)
{
  // Set breakpoint HERE
  while (1)
  {
  }
}
```

**When debugger stops at hard fault:**
1. Examine call stack in debugger
2. Check CPU registers (especially PC, LR, SP)
3. Look at disassembly to see faulting instruction

### Hard Fault Registers (Cortex-M4)

```c
// Read these in GDB or debugger watch window:
SCB->CFSR   // Configurable Fault Status Register
SCB->HFSR   // Hard Fault Status Register  
SCB->DFSR   // Debug Fault Status Register
SCB->AFSR   // Auxiliary Fault Status Register
SCB->BFAR   // Bus Fault Address Register
SCB->MMFAR  // MemManage Fault Address Register
```

**Common fault causes:**
- `IBUSERR`: Instruction bus error (PC points to invalid memory)
- `DBUSERR`: Data bus error (accessing invalid memory)
- `MUNSTKERR`: MemManage fault on exception return (stack corruption)
- `IMPRECISERR`: Imprecise bus fault (previous instruction caused it)

---

## 2. FreeRTOS Task Crashes

### prvTaskExitError Detection

**What it means:** A FreeRTOS task returned from its function instead of running forever in an infinite loop.

**How to detect:**

1. **In OpenOCD/GDB:** Set breakpoint on `prvTaskExitError`:
   ```gdb
   b prvTaskExitError
   ```

2. **Check task list** when debugger stops:
   ```gdb
   info threads
   ```

3. **Examine which task crashed:**
   - Look at call stack
   - Check `pxCurrentTCB->pcTaskName`

**Example from MidiCore:**
```c
// WRONG - Task will exit and crash:
static void MidiIOTask(void *argument)
{
  // Do initialization
  midicore_debug_send_message(...);  // If this fails, task exits!
}

// CORRECT - Task runs forever:
static void MidiIOTask(void *argument)
{
  // Do initialization
  for (;;) {  // Infinite loop required!
    // Task work
    osDelay(10);
  }
}
```

### Real Fix Applied (Commit bc3e91c)

**Problem:** MidiIOTask called `midicore_debug_send_message()` before USB enumeration completed, causing hard fault → task exit → prvTaskExitError.

**Solution:** Wait for USB enumeration before sending:
```c
// Wait for USB to be ready
osDelay(500);  // USB enumeration time

// NOW safe to send
midicore_debug_send_message(...);

// Enter infinite loop
for (;;) {
  // Main task loop
}
```

---

## 3. OpenOCD/GDB Debugging

### Connecting to Running Target

```bash
# Terminal 1: Start OpenOCD
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg

# Terminal 2: Connect GDB
arm-none-eabi-gdb build/MidiCore.elf
(gdb) target remote localhost:3333
(gdb) monitor reset halt
(gdb) load
(gdb) monitor reset init
(gdb) continue
```

### Useful GDB Commands

```gdb
# Break on hard fault
b HardFault_Handler

# Break on FreeRTOS task exit
b prvTaskExitError

# Examine memory
x/16x 0x20000000   # Show 16 words at address

# Show backtrace
bt

# Show all threads (FreeRTOS tasks)
info threads

# Switch to thread
thread 3

# Show task control blocks
p *pxCurrentTCB
```

### Memory Inspection for Crashes

```gdb
# Check for 0xa5a5a5xx pattern (heap corruption)
x/16x 0xa5a5a5d9   # Will fail if address is corrupted

# Check stack pointer is valid
p $sp
x/32x $sp

# Check program counter
p $pc
x/i $pc   # Disassemble instruction at PC
```

---

## 4. UART Debug Output Analysis

### Fatal Error Messages

**Look for these in UART output:**

```
[FATAL] Stack overflow #1 in task: MidiIO
[FATAL] Heap exhausted
[ERROR] Task creation failed
prvTaskExitError reached
```

### Stack Overflow Detection

**Symptom in FreeRTOS task list:**
```
Task name: qzbfjhbsdkbldvmjkdbgnbjkjng   // Corrupted name!
Event object: 0x2270036c
```

**What this means:**
- Stack overflow wrote into adjacent task control block
- Task name memory corrupted with garbage
- Indicates stack too small for task

**Real fix applied (Commit 3d44131):**
```c
// BEFORE: 1KB stack (too small)
osThreadDef(MidiIOTask, MidiIOTask, osPriorityNormal, 0, 1024);

// AFTER: 2KB stack (sufficient)
osThreadDef(MidiIOTask, MidiIOTask, osPriorityNormal, 0, 2048);
```

### Heap Diagnostics

**Look for heap warnings:**
```
[HEAP] After CLI task: 4104 bytes free    // TOO LOW!
[HEAP] Task creation may fail
```

**Check heap usage:**
```c
// In code:
size_t free_heap = xPortGetFreeHeapSize();
size_t min_heap = xPortGetMinimumEverFreeHeapSize();

dbg_printf("[HEAP] Free: %u Min: %u\r\n", free_heap, min_heap);
```

---

## 5. Stack Overflow Detection

### Compile-Time Settings

**In `FreeRTOSConfig.h`:**
```c
#define configCHECK_FOR_STACK_OVERFLOW  2  // Enable stack overflow detection
```

**Stack overflow hook (add to `main.c`):**
```c
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  dbg_printf("[FATAL] Stack overflow in task: %s\r\n", pcTaskName);
  while(1);  // Halt for debugger
}
```

### Runtime Stack Monitoring

**Use the `stack_free` CLI command:**
```
midicore> stack_free

Task Stack Free Space:
  DefaultTask:     10234 / 16384 bytes (62% free)
  CliTask:          4567 /  8192 bytes (56% free)
  MidiIOTask:        512 /  2048 bytes (25% free)  ← Warning!
  StackMon:          800 /  1024 bytes (78% free)
```

**Check programmatically:**
```c
UBaseType_t stack_free = uxTaskGetStackHighWaterMark(NULL);
dbg_printf("[STACK] Free: %u words\r\n", stack_free * 4);
```

---

## 6. Heap Corruption Detection (0xa5a5a5xx Pattern)

### What 0xa5a5a5xx Means

The `0xa5` pattern is `tskSTACK_FILL_BYTE` used by FreeRTOS to initialize unused stack memory.

**When you see these addresses in OpenOCD:**
```
Error: Failed to read memory at 0xa5a5a5d9
Error: Failed to read memory at 0xa5a5a5f1
Error: Failed to read memory at 0xa5a5a5a5
```

**It means:**
1. Heap exhausted during task creation
2. Task Control Block (TCB) partially allocated
3. TCB filled with 0xa5 pattern
4. Scheduler tries to use corrupted TCB pointers
5. Crashes accessing invalid 0xa5a5a5xx addresses

### Real Fix Applied (Commit a7f4fb0)

**Problem:** Only 4104 bytes heap free, needed ~3800 for remaining tasks.

**Solution:** Increase heap from 36KB to 40KB:
```c
// FreeRTOSConfig.h
#define configTOTAL_HEAP_SIZE  ((size_t)(40*1024))  // Was 36KB
```

**After fix:**
```
[HEAP] After CLI task: 8200 bytes free  // Safe margin!
```

---

## 7. Common Crash Patterns in MidiCore

### Pattern 1: USB Timing Crash

**Symptoms:**
- prvTaskExitError
- Crash during task initialization
- UART shows task started but then stops

**Cause:** Calling USB MIDI functions before USB enumeration complete.

**Fix:** Wait 500ms after USB initialization:
```c
// Wait for USB composite enumeration
osDelay(500);

// NOW safe to use USB MIDI
usb_midi_send_packet(...);
```

### Pattern 2: Stack Overflow

**Symptoms:**
- Corrupted task names in debugger
- Random crashes in different places
- Task watchdog triggers

**Cause:** Task stack too small for actual usage.

**Fix:** Increase stack size in task definition.

### Pattern 3: Heap Exhaustion

**Symptoms:**
- 0xa5a5a5xx addresses in OpenOCD
- Tasks fail to create
- System unstable after some tasks start

**Cause:** Not enough heap for all task allocations.

**Fix:** Increase `configTOTAL_HEAP_SIZE`.

### Pattern 4: SysEx Logging Overflow

**Symptoms:**
- System works but then crashes under load
- UART output stops mid-message
- Happens when MIOS Studio connected

**Cause:** Logging every SysEx packet causes stack overflow (256 bytes/call).

**Fix:** Skip SysEx packets in debug hook:
```c
void usb_midi_rx_debug_hook(const uint8_t packet4[4])
{
  uint8_t cin = packet4[0] & 0x0F;
  
  // Skip SysEx to prevent stack overflow
  if (cin >= 0x04 && cin <= 0x07) {
    return;
  }
  
  // Log non-SysEx packets
  dbg_printf("[USB-RX] ...\r\n");
}
```

---

## 8. Debugging Workflow

### Step-by-Step Process

1. **Reproduce the crash** with UART connected:
   ```
   - Monitor UART at 115200 baud
   - Note last message before crash
   - Look for error patterns
   ```

2. **Check UART output** for clues:
   ```
   - [FATAL] messages
   - Stack overflow warnings
   - Heap diagnostics
   - Last task that ran
   ```

3. **Connect debugger** and reproduce:
   ```
   - Set breakpoint on HardFault_Handler
   - Set breakpoint on prvTaskExitError
   - Run and wait for crash
   ```

4. **When debugger stops:**
   ```gdb
   # Get backtrace
   bt
   
   # Check current task
   p *pxCurrentTCB
   
   # Check all tasks
   info threads
   
   # Examine crash location
   x/i $pc
   ```

5. **Analyze crash cause:**
   - Stack overflow? Check uxTaskGetStackHighWaterMark
   - Heap exhaustion? Check xPortGetFreeHeapSize
   - Invalid memory access? Check address in BFAR/MMFAR
   - USB timing? Check if USB enumeration complete

6. **Apply fix** and verify:
   - Test multiple times
   - Monitor heap/stack usage
   - Check system stays stable

---

## 9. Tools and Utilities

### Built-in Diagnostics

**CLI Commands:**
```bash
stack_free      # Show stack usage for all tasks
hooks           # Show MidiCore hooks statistics
help            # List all commands
```

**Debug Macros:**
```c
MODULE_DEBUG_MIDICORE_QUERIES  // Enable query debug output
MODULE_DEBUG_OUTPUT            // Set debug output destination
```

### External Tools

**OpenOCD:** Hardware debugging
**GDB:** Software debugging and analysis
**PuTTY/TeraTerm:** UART terminal monitoring
**STM32CubeIDE:** Integrated debugging environment

---

## 10. Quick Reference

### Most Common Issues

| Symptom | Likely Cause | Check |
|---------|-------------|-------|
| prvTaskExitError | Task returned instead of looping | Task has `for(;;)` loop? |
| Corrupted task names | Stack overflow | Increase task stack size |
| 0xa5a5a5xx addresses | Heap exhaustion | Increase configTOTAL_HEAP_SIZE |
| Random crashes | USB timing | Wait 500ms after USB init |
| Crash on query | SysEx logging | Skip SysEx in debug hook |

### Critical Settings

```c
// FreeRTOSConfig.h
#define configTOTAL_HEAP_SIZE            ((size_t)(40*1024))  // 40KB
#define configCHECK_FOR_STACK_OVERFLOW   2

// Task stack sizes (in words)
#define MIDI_IO_TASK_STACK_SIZE          512   // 2KB
#define CLI_TASK_STACK_SIZE              2048  // 8KB
#define DEFAULT_TASK_STACK_SIZE          4096  // 16KB
```

---

## Real Fixes Applied to This Project

1. **Commit a7f4fb0:** Heap 36KB → 40KB (fixed 0xa5a5a5xx errors)
2. **Commit 3d44131:** MidiIOTask stack 1KB → 2KB (fixed overflow)
3. **Commit bc3e91c:** USB timing fix (fixed prvTaskExitError)
4. **Commit 925ccde:** Skip SysEx logging (fixed query crashes)
5. **Commit 5163b9c:** Unified debug hook (eliminated duplication)

All these fixes came from systematic crash debugging using the techniques in this guide!

---

## Summary

**To see crashes in debug mode:**

1. Monitor UART output (115200 baud)
2. Set breakpoints on HardFault_Handler and prvTaskExitError
3. Use OpenOCD/GDB to connect to running target
4. Analyze crash location, registers, and memory
5. Check for common patterns (stack, heap, timing)
6. Apply targeted fix
7. Verify stability

**The key is systematic investigation - crashes leave evidence!**
