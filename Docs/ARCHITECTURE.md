# MidiCore Architecture Guide

## MIOS32-Like FreeRTOS Design

MidiCore follows MIOS32 design principles for deterministic, stack-safe, and maintainable embedded firmware.

---

## Core Philosophy

### 1. FreeRTOS is a Scheduler, Not an Architecture
- Avoid "one task per feature"
- Prefer cooperative execution over preemptive fragmentation
- Tasks are for scheduling, not for organizing code

### 2. Minimal Task Model
- **ONE main task** (`MidiCore_MainTask`) handles all application logic
- Logic lives in **services**, not tasks
- Services are simple, predictable, and non-blocking

### 3. Stability Over Cleverness
- Fewer tasks = fewer stacks = less RAM
- Fewer hidden states = easier debugging
- Deterministic timing = predictable behavior

---

## Task Architecture

### Required Tasks

| Task | Stack | Priority | Purpose |
|------|-------|----------|---------|
| `MidiCore_MainTask` | 5KB | Normal | Main cooperative scheduler |

### Optional Tasks (Only if Justified)

| Task | Stack | Priority | Purpose |
|------|-------|----------|---------|
| `IO_Task` | 2KB | AboveNormal | USB/MIDI buffering only |
| `USBH_Process_OS` | - | - | USB Host library (required) |

### Removed Tasks (Logic Moved to Services)

| Old Task | New Service Function |
|----------|---------------------|
| `AinTask` | `ain_tick_5ms()` called from main |
| `AinMidiTask` | `ain_midi_process_events()` |
| `OledDemoTask` | `ui_tick_20ms()` |
| `CliTask` | `cli_task()` called from main |
| `CalibrationTask` | State machine in calibration service |
| `PressureTask` | `pressure_service_tick()` |
| `StackMonitorTask` | Periodic check in main task |

---

## Service Model

### Design Principles

Services are called from the main task tick and must be:
- **Non-blocking**: Return quickly, never wait
- **Bounded time**: Predictable worst-case execution
- **No RTOS API**: Don't call FreeRTOS functions
- **No HAL calls**: Hardware access through drivers
- **No malloc/free**: Static allocation only
- **No logging**: Debug output only in development

### Service Tick Intervals

```c
#define MIDICORE_TICK_AIN         5     /* Every 5ms */
#define MIDICORE_TICK_PRESSURE    5     /* Every 5ms */
#define MIDICORE_TICK_MIDI        1     /* Every 1ms */
#define MIDICORE_TICK_EXPRESSION  1     /* Every 1ms */
#define MIDICORE_TICK_UI          20    /* Every 20ms */
#define MIDICORE_TICK_CLI         5     /* Every 5ms */
#define MIDICORE_TICK_WATCHDOG    100   /* Every 100ms */
```

### Main Task Loop Structure

```c
for (;;) {
    uint32_t tick = s_tick_count;
    
    /* Priority 1: Time-critical (every tick) */
    midi_io_service_tick(tick);
    expression_service_tick(tick);
    
    /* Priority 2: Regular (every 5ms) */
    if ((tick % 5) == 0) {
        ain_service_tick(tick);
        pressure_service_tick(tick);
        cli_service_tick(tick);
    }
    
    /* Priority 3: UI (every 20ms) */
    if ((tick % 20) == 0) {
        ui_service_tick(tick);
    }
    
    /* Priority 4: Background (infrequent) */
    if ((tick % 100) == 0) {
        watchdog_service_tick(tick);
    }
    
    s_tick_count++;
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
}
```

---

## Memory & Stack Discipline

### Stack Rules
- No local buffers > 64 bytes
- No deep call chains
- No printf in critical paths
- Measure stack usage with `uxTaskGetStackHighWaterMark()`

### Memory Rules
- Prefer static allocation
- No dynamic allocation in real-time paths
- Use static ring buffers for I/O

### Stack Sizes

| Component | Size | Rationale |
|-----------|------|-----------|
| Main Task | 5KB | Services + moderate call depth |
| CLI (if separate) | 8KB | Printf formatting overhead |
| ISR | Minimal | Push to queues only |

---

## ISR Rules

### Interrupts MUST:
- Be minimal (< 10µs)
- Push to ring buffers/queues
- Notify tasks if needed

### Interrupts MUST NEVER:
- Log or print
- Call services
- Allocate memory
- Format strings
- Block

---

## Layering

```
┌─────────────────────────────────────────┐
│  Application (MidiCore_MainTask)        │
├─────────────────────────────────────────┤
│  Services (midi, ain, ui, cli, etc.)    │
│  - Pure logic, no HAL, no RTOS          │
├─────────────────────────────────────────┤
│  Drivers (HAL wrappers)                 │
│  - Hardware access only                 │
├─────────────────────────────────────────┤
│  HAL / CMSIS / FreeRTOS                 │
└─────────────────────────────────────────┘
```

---

## CLI Design (MIOS32-Compatible)

### Requirements
- No printf/snprintf in output (fixed strings)
- No dedicated task (processed in main tick)
- Line-based, ASCII-only
- Input buffered via USB CDC/MIDI SysEx

### Command Format
```
help              # Show help
status            # Show system status
ain               # Show AIN values
calibrate         # Run calibration
reboot            # Restart system
```

---

## Migration Guide

### From Task-Based to Service-Based

1. **Identify task's core logic** (the loop body)
2. **Extract to service function** (e.g., `xxx_service_tick()`)
3. **Remove osDelay()** - timing controlled by main task
4. **Remove FreeRTOS API calls** - service is pure logic
5. **Add to main task tick** at appropriate interval
6. **Delete old task file** or keep for legacy mode

### Example: Pressure Task Migration

**Before (task-based):**
```c
static void PressureTask(void* arg) {
    for(;;) {
        if (cfg->enable) {
            pressure_read_once(&raw);
            expression_set_raw(raw);
        }
        osDelay(5);
    }
}
```

**After (service-based):**
```c
void pressure_service_tick(uint32_t tick) {
    (void)tick;
    if (cfg->enable) {
        pressure_read_once(&raw);
        expression_set_raw(raw);
    }
}
// Called every 5ms from main task
```

---

## References

- **MIOS32**: http://www.midibox.org/mios32/
- **MIOS32 GitHub**: https://github.com/midibox/mios32
- **FreeRTOS Best Practices**: https://www.freertos.org/
