# Bootloader RAM Usage and Optimization Guide

## RAM Usage Overview

The bootloader is designed to have minimal RAM footprint. Understanding the memory layout is crucial when RAM is constrained.

### Bootloader RAM Usage (When Active)

The bootloader only runs during:
1. System startup (checks for bootloader entry conditions)
2. Firmware update mode (stays in bootloader)

**Static RAM Usage:**
```
sysex_tx_buffer:     272 bytes  (256 + 16 for SysEx responses)
Stack during ops:    ~200 bytes  (flash operations)
Total:               ~500 bytes
```

**Key Point**: Once bootloader jumps to application, this RAM is **released and available** to your application!

### Application RAM Usage (Normal Operation)

When your application is running:
- **Bootloader code**: Not in RAM (in flash at 0x08000000)
- **Bootloader RAM**: Freed and available for application use
- **Only if integrated**: `bootloader_app.c` uses ~50 bytes for SysEx detection

## RAM Optimization Strategies

### 1. Disable Bootloader Integration (If Not Needed)

If you don't need to enter bootloader from running application:

**In `Config/module_config.h`:**
```c
#define MODULE_ENABLE_BOOTLOADER 0  // Disable bootloader integration
```

This removes:
- `bootloader_app.c` compilation (~50 bytes RAM)
- Bootloader entry detection code
- SysEx handler helpers

**You can still use the bootloader** - just enter it via:
- Power-on with no valid application
- Hardware button press
- JTAG/SWD reset into bootloader

### 2. Reduce SysEx Buffer Size (Advanced)

If you need bootloader integration but want to save RAM, reduce the buffer:

**In `Services/bootloader/bootloader_protocol.h`:**
```c
// Original:
#define SYSEX_MAX_DATA_SIZE         256

// Reduced (use smaller blocks):
#define SYSEX_MAX_DATA_SIZE         128  // Saves 128 bytes
// or
#define SYSEX_MAX_DATA_SIZE         64   // Saves 192 bytes
```

**Trade-offs:**
- Smaller blocks = more transfer overhead
- 64 bytes still efficient for USB MIDI
- Update Python tool: `--block-size 64`

### 3. Optimize Application Memory

The bootloader doesn't impact application RAM much. Focus on application optimization:

#### Check Current RAM Usage

```bash
arm-none-eabi-size -A MidiCore.elf

# Look for:
# .data    - Initialized data (RAM)
# .bss     - Uninitialized data (RAM)
# .heap    - Dynamic allocation
# .stack   - Stack size
```

#### Common RAM Hogs in MIDI Applications

**Large Static Buffers:**
```c
// BAD: Wastes RAM even if rarely used
static uint8_t midi_buffer[4096];

// GOOD: Use smaller, multiple buffers or dynamic allocation
#define MIDI_BUFFER_SIZE 256
static uint8_t midi_buffer[MIDI_BUFFER_SIZE];
```

**MIDI Router Buffers:**
```c
// Check Services/router/ for buffer sizes
// Reduce if you have fewer MIDI channels
```

**Looper/Recorder Buffers:**
```c
// Services/looper/ may use significant RAM
// Consider reducing track count or event buffers
```

**USB MIDI Buffers:**
```c
// Check USB middleware for buffer sizes
// May be configurable in CubeMX
```

### 4. Use Flash for Constant Data

Move constant data from RAM to flash:

```c
// BAD: Stored in RAM
static const char* error_messages[] = {
    "Error 1", "Error 2", ...
};

// GOOD: Stored in flash
static const char* const error_messages[] PROGMEM = {
    "Error 1", "Error 2", ...
};
```

### 5. Stack Size Optimization

**Check linker script:**
```ld
_Min_Stack_Size = 0x400; /* 1KB - may be too much or too little */
```

**Find optimal size:**
1. Enable stack overflow detection in FreeRTOS
2. Monitor high water mark: `uxTaskGetStackHighWaterMark()`
3. Reduce until you see issues, then add 20% margin

### 6. Heap Size Optimization

```ld
_Min_Heap_Size = 0x200; /* 512 bytes */
```

If not using `malloc()`/`new`:
- Reduce heap to minimum (0x100 or less)
- Use static allocation for FreeRTOS tasks

### 7. FreeRTOS Task Stack Sizes

```c
// Check task creation in your code
xTaskCreate(task_func, "Task", 
    256,  // Stack size in words (1KB) - reduce if possible
    NULL, priority, &handle);
```

**Monitor stack usage:**
```c
void monitor_task_stacks(void) {
    TaskHandle_t tasks[10];
    int count = uxTaskGetNumberOfTasks();
    
    for (int i = 0; i < count; i++) {
        UBaseType_t watermark = uxTaskGetStackHighWaterMark(tasks[i]);
        // If watermark > 50% of stack, can reduce task stack size
    }
}
```

## Bootloader-Specific Memory Layout

### Memory Map with Bootloader

```
RAM Layout (128KB total on STM32F407):

0x20000000 ┌──────────────────────┐
           │   .data (globals)    │
           ├──────────────────────┤
           │   .bss (uninit)      │
           ├──────────────────────┤
           │   Heap               │
           ├──────────────────────┤
           │   Stack (grows down) │
0x20020000 └──────────────────────┘

Bootloader Magic RAM:
0x2001FFF0 (last 16 bytes) - Reserved for bootloader entry flag
```

**Reserved RAM:**
- Last 16 bytes: Bootloader magic value (preserved across reset)
- Your application should avoid: `0x2001FFF0 - 0x20020000`

### Flash Layout with Bootloader

```
Flash (1MB total):

0x08000000 ┌──────────────────────┐
           │   Bootloader (32KB)  │  ← Code runs here during update
0x08008000 ├──────────────────────┤
           │   Vector Table       │  ← Application starts here
           │   Application Code   │
           │   Application Data   │
           │   (992KB)            │
0x08100000 └──────────────────────┘
```

## Memory-Constrained Build Configuration

If RAM is critically low, use this configuration:

**1. Minimal Bootloader Integration (`Config/module_config.h`):**
```c
#define MODULE_ENABLE_BOOTLOADER 0  // Disable in-app bootloader entry
```

**2. Reduce Buffer Sizes (`Services/bootloader/bootloader_protocol.h`):**
```c
#define SYSEX_MAX_DATA_SIZE 64  // Minimum viable size
```

**3. Optimize Linker Script (`STM32F407VGTX_FLASH_APP.ld`):**
```ld
_Min_Heap_Size = 0x100;   /* Reduce if not using malloc */
_Min_Stack_Size = 0x300;  /* Reduce but monitor for overflow */
```

**4. Upload Tool Configuration:**
```bash
python3 Tools/upload_firmware.py firmware.bin --block-size 64
```

## Measuring RAM Usage

### Build-Time Analysis

```bash
# After building
arm-none-eabi-size -A MidiCore.elf

# Output example:
section         size      addr
.data           2048   0x20000000  # Initialized data in RAM
.bss           10240   0x20000800  # Uninitialized data in RAM
# Total RAM = .data + .bss = 12288 bytes (12KB)
```

### Runtime Monitoring

Add to your application:

```c
#include "FreeRTOS.h"
#include "task.h"

void print_memory_stats(void) {
    // Heap
    size_t free_heap = xPortGetFreeHeapSize();
    size_t min_free_heap = xPortGetMinimumEverFreeHeapSize();
    
    printf("Free heap: %u bytes\n", free_heap);
    printf("Min free heap: %u bytes\n", min_free_heap);
    
    // Tasks
    char buffer[512];
    vTaskList(buffer);
    printf("Task list:\n%s\n", buffer);
}
```

## Troubleshooting RAM Issues

### Symptom: Hard Fault or Crashes

**Possible causes:**
1. Stack overflow
2. Heap exhaustion
3. Buffer overrun

**Debug steps:**
```c
// 1. Enable stack overflow checking
#define configCHECK_FOR_STACK_OVERFLOW 2

// 2. Implement overflow hook
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    printf("Stack overflow in task: %s\n", pcTaskName);
    while(1); // Trap
}

// 3. Implement malloc failed hook
void vApplicationMallocFailedHook(void) {
    printf("Malloc failed!\n");
    while(1); // Trap
}
```

### Symptom: Bootloader Won't Start

**Check:**
1. Magic RAM location not corrupted: `0x2001FFF0`
2. Bootloader has enough stack (runs before main app)
3. Flash operations have temp buffers

### Symptom: Application Unstable After Bootloader

**Verify:**
1. Vector table relocated: `SCB->VTOR = 0x08008000`
2. Application linker script uses correct origin
3. No global variables at bootloader magic RAM address

## Best Practices

1. **Profile First**: Use `arm-none-eabi-size` and runtime monitoring
2. **Optimize Application**: Bootloader RAM impact is minimal (<500 bytes when active)
3. **Test Thoroughly**: After reducing buffers, test complete update cycle
4. **Document Changes**: Note any reduced buffer sizes for troubleshooting
5. **Keep Margins**: Don't run at 100% RAM usage (keep 10-20% free)

## Example: Extreme RAM Optimization

If you need every byte:

```c
// In module_config.h - disable bootloader app integration
#define MODULE_ENABLE_BOOTLOADER 0

// In bootloader_protocol.h - minimal buffer
#define SYSEX_MAX_DATA_SIZE 64

// In linker script - tight memory
_Min_Heap_Size = 0x100;
_Min_Stack_Size = 0x200;
```

**Result:**
- Bootloader still works (enter via power-on without app)
- Saves ~400+ bytes RAM in application
- Upload takes slightly longer (smaller blocks)

## Conclusion

The bootloader is designed to be RAM-friendly:
- Minimal static allocation (~500 bytes)
- Only active during updates
- RAM freed when application runs
- Fully optional integration

For RAM-constrained systems, focus on:
1. Application buffer optimization
2. FreeRTOS task stack tuning
3. Disabling unused modules
4. Moving const data to flash

The bootloader should not be a significant RAM concern in most applications.
