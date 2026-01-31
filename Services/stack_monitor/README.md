# Stack Monitor Module

## Overview

The Stack Monitor module provides comprehensive runtime monitoring of FreeRTOS task stack usage. It detects stack overflows, tracks high-water marks, and provides alerts when tasks approach their stack limits.

## Features

- **Real-time Monitoring**: Periodic checks of all task stack usage
- **High-Water Mark Tracking**: Tracks minimum free stack (deepest usage point)
- **0xA5 Pattern Detection**: Detects corruption of FreeRTOS stack fill pattern
- **Configurable Thresholds**: Warning and critical alerts at configurable percentages
- **CLI Integration**: Complete command-line interface for inspection
- **Zero Dynamic Memory**: Static allocation for embedded constraints
- **Telemetry Export**: CSV export for logging and analysis

## Architecture

```
Services/stack_monitor/
├── stack_monitor.h          - Public API
├── stack_monitor.c          - Implementation
├── stack_monitor_cli.c      - CLI commands
└── README.md                - This file
```

## Configuration

Edit `stack_monitor.h` or define in `module_config.h`:

```c
#define STACK_MONITOR_ENABLED             1      // Enable/disable monitoring
#define STACK_MONITOR_INTERVAL_MS         5000   // Check interval (ms)
#define STACK_MONITOR_WARNING_THRESHOLD   20     // Warn at 20% free
#define STACK_MONITOR_CRITICAL_THRESHOLD  5      // Critical at 5% free
#define STACK_MONITOR_MAX_TASKS           16     // Max tasks to track
#define STACK_MONITOR_STACK_SIZE          512    // Monitor task stack (bytes)
```

## Usage

### Initialization

Call during system startup (typically in `app_init.c`):

```c
#include "Services/stack_monitor/stack_monitor.h"

void app_init_and_start(void) {
    // ... other init ...
    
    stack_monitor_init();  // Starts monitoring task automatically
    
    // ... continue init ...
}
```

### CLI Commands

#### Show Stack Usage for Current Task
```
> stack
AinTask       :  450/ 1024 bytes ( 44% used,  56% free) [OK]
```

#### Show Stack Usage for Specific Task
```
> stack CliTask

Task: CliTask
  Stack size:    5120 bytes (1280 words)
  Used:          3584 bytes (70%)
  Free:          1536 bytes (30%)
  High-water:    1536 bytes
  Status:        OK
```

#### Show All Tasks
```
> stack_all

=== Stack Usage Report (10 tasks) ===
Task            Used         Total    Used%   Free%  Status
--------------- ------------ -------- ------- ------ ------
defaultTask         8192 B  12288 B      67%     33% OK
CliTask             3584 B   5120 B      70%     30% OK
AinTask              450 B   1024 B      44%     56% OK
OledDemo             512 B   1024 B      50%     50% OK
MidiIOTask          768 B   2048 B      37%     63% OK
...
```

#### Verbose Output
```
> stack_all -v

=== Stack Usage Report (10 tasks) ===
...
  High-water mark: 1536 bytes (384 words)
```

#### Monitor Control
```
> stack_monitor stats

=== Stack Monitor Statistics ===
Total checks:    120
Warnings:        2
Critical alerts: 0
Overflows:       0
Last check:      600523 ms
Interval:        5000 ms
Warn threshold:  20%
Crit threshold:  5%
```

```
> stack_monitor start          # Start monitoring
> stack_monitor stop           # Stop monitoring
> stack_monitor check          # Force immediate check
> stack_monitor export         # Export CSV
```

#### Configuration
```
> stack_monitor config                    # Show current config
> stack_monitor config interval 10000     # Set interval to 10s
> stack_monitor config warning 25         # Set warning to 25%
> stack_monitor config critical 10        # Set critical to 10%
```

### Programmatic API

```c
#include "Services/stack_monitor/stack_monitor.h"

// Get info for current task
stack_info_t info;
stack_monitor_get_info(NULL, &info);
printf("Stack usage: %lu%%\n", info.used_percent);

// Get info by task name
stack_monitor_get_info_by_name("CliTask", &info);

// Get all tasks
stack_info_t tasks[16];
uint32_t count;
stack_monitor_get_all_tasks(tasks, 16, &count);

// Register alert callback
void my_alert_handler(const char* name, const stack_info_t* info, stack_status_t status) {
    if (status == STACK_STATUS_CRITICAL) {
        // Handle critical alert
    }
}
stack_monitor_register_callback(my_alert_handler);
```

## Alert Levels

| Status     | Threshold | Action |
|------------|-----------|--------|
| **OK**     | > warning | Normal operation |
| **WARNING** | ≤ warning | Log warning, continue |
| **CRITICAL** | ≤ critical | Log critical alert, may trigger recovery |
| **OVERFLOW** | Pattern corrupted | Log overflow, immediate recovery needed |

## FreeRTOS Requirements

The following FreeRTOS configuration must be enabled in `FreeRTOSConfig.h`:

```c
#define INCLUDE_uxTaskGetStackHighWaterMark  1
#define configUSE_TRACE_FACILITY             1
#define configCHECK_FOR_STACK_OVERFLOW       2  // Recommended
#define configUSE_STATS_FORMATTING_FUNCTIONS 1  // Optional
```

## Stack Overflow Detection

FreeRTOS fills unused stack with `0xA5A5A5A5` pattern. The monitor detects when:

1. **High-water mark is low**: Less than threshold percentage free
2. **Pattern corruption**: 0xA5 pattern overwritten (indicates overflow)

When overflow detected:
- `STACK_STATUS_OVERFLOW` status set
- Critical alert logged via `dbg_printf`
- Alert callback invoked (if registered)
- Statistics updated

## Memory Usage

- **Static RAM**: ~200 bytes (state + cache)
- **Task Stack**: 512 bytes (configurable)
- **Per-check overhead**: Minimal (enumerate + measure tasks)

## Performance

- **Check interval**: 5 seconds default (configurable)
- **Task priority**: Below normal (doesn't interfere with real-time)
- **Execution time**: < 10ms per check (depends on task count)

## Integration with CLI

Stack monitor commands are registered automatically when `stack_monitor_cli_init()` is called. This is typically done in `cli_module_commands_init()`:

```c
#if MODULE_ENABLE_STACK_MONITOR
  extern int stack_monitor_cli_init(void);
  stack_monitor_cli_init();
#endif
```

## Debugging Stack Issues

### High Stack Usage

If a task shows high usage (>80%):

1. **Check for large local variables**:
   ```c
   void bad_function(void) {
       char buffer[2048];  // 2KB on stack - BAD!
   }
   
   // Fix: use static
   static char s_buffer[2048];
   ```

2. **Check recursion depth**
3. **Check nested function calls**
4. **Increase task stack size** if legitimate

### Stack Overflow

If overflow detected:

1. **Check debug logs** for task name
2. **Review task function** for large locals
3. **Increase stack size** in task creation
4. **Move large buffers to static/heap**

### Example Fix

Before (causes overflow):
```c
void app_init_and_start(void) {
    config_t cfg;              // 500 bytes on stack
    instrument_cfg_t icfg;     // 200 bytes on stack
    zones_cfg_t zcfg;          // 200 bytes on stack
    // ... total 1000+ bytes
}
```

After (no stack impact):
```c
void app_init_and_start(void) {
    static config_t s_cfg;         // In .bss, not stack
    static instrument_cfg_t s_icfg;
    static zones_cfg_t s_zcfg;
    // Stack usage: minimal
}
```

## Testing

### Manual Testing

1. Build with stack monitor enabled
2. Run firmware, open terminal
3. Exercise CLI commands:
   ```
   > stack_all
   > stack_monitor stats
   > stack defaultTask
   ```
4. Verify all tasks report OK status

### Stress Testing

1. Intentionally reduce a task's stack:
   ```c
   const osThreadAttr_t attr = {
       .name = "TestTask",
       .stack_size = 256  // Very small
   };
   ```
2. Run intensive workload
3. Verify monitor detects and alerts:
   ```
   [STACK] WARNING: Task 'TestTask' stack usage high: 85% used
   ```

### Automated Testing

Create a test task with known stack usage:
```c
void test_stack_monitor(void) {
    char big_buffer[1024];  // Force high usage
    memset(big_buffer, 0, sizeof(big_buffer));
    
    stack_info_t info;
    stack_monitor_get_info(NULL, &info);
    assert(info.used_percent > 50);
}
```

## Troubleshooting

### Monitor not running
- Check `STACK_MONITOR_ENABLED` is defined as 1
- Verify `stack_monitor_init()` is called
- Check task creation succeeded (not NULL handle)

### Inaccurate stack sizes
- Stack size estimation may not be perfect for all tasks
- Use known stack sizes from task creation attributes
- High-water marks are always accurate

### Missing tasks
- Increase `STACK_MONITOR_MAX_TASKS` if needed
- Check FreeRTOS task creation succeeded

## References

- FreeRTOS Stack Overflow Detection: https://www.freertos.org/Stacks-and-stack-overflow-checking.html
- uxTaskGetStackHighWaterMark API: https://www.freertos.org/uxTaskGetStackHighWaterMark.html
- Task utilities: https://www.freertos.org/a00021.html
