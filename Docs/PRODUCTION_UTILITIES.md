# Production Utilities: Performance Monitoring & Runtime Configuration

## Overview

Phase 4 makes all Phase 3 advanced features available in production mode by creating reusable service modules that can be used anywhere in the codebase, not just in tests.

## New Services

### 1. Performance Monitor (`Services/performance/`)

Production-ready performance monitoring and benchmarking system.

**Features:**
- Millisecond-precision timing for any operation
- Automatic statistics tracking (calls, average, min, max)
- CSV export for offline analysis  
- UART reporting for live monitoring
- Support for up to 32 concurrent tracked operations
- Zero heap allocation

**Use Cases:**
- Monitor MIDI processing latency
- Track looper operation timing
- Measure UI responsiveness
- Identify performance bottlenecks
- Export data for support/debugging

**API:**
```c
// Initialize
perf_monitor_init();

// Register operations to track
perf_metric_id_t midi = perf_monitor_register("MIDI_Process");
perf_metric_id_t ui = perf_monitor_register("UI_Update");

// Measure operation
perf_monitor_start(midi);
process_midi_events();
uint32_t duration = perf_monitor_end(midi);

// Or record directly
perf_monitor_record("Config_Load", 150);

// Query metrics
const perf_metrics_t* metrics = perf_monitor_get(midi);
uint32_t avg_ms = perf_monitor_get_average(midi);

// Report to UART
perf_monitor_report_uart();

// Export to CSV
perf_monitor_save_csv("0:/performance.csv");

// Reset statistics
perf_monitor_reset();
```

**Output Example:**
```
==============================================
       PERFORMANCE METRICS
==============================================
Operation                 Calls    Avg(ms)  Min(ms)  Max(ms)
--------------------------------------------------------------
MIDI_Process                247       12        8       45
UI_Update                   150       16       15       23
Config_Load                   1      150      150      150
==============================================
```

**CSV Export:**
```csv
Operation,Calls,Average_ms,Min_ms,Max_ms,Total_ms
MIDI_Process,247,12,8,45,2964
UI_Update,150,16,15,23,2400
Config_Load,1,150,150,150,150
```

---

### 2. Runtime Configuration (`Services/config/runtime_config.h`)

INI-style configuration management for runtime behavior modification.

**Features:**
- Load/save configurations from SD card
- No recompilation needed for config changes
- Type-safe value getters (string, int, bool, float)
- Change notification callbacks
- Support for up to 64 configuration entries
- Human-readable INI format

**Use Cases:**
- User-configurable parameters
- Per-device configurations
- A/B testing different behaviors
- Field-updateable settings
- Debug mode toggles

**API:**
```c
// Initialize and load
runtime_config_init();
runtime_config_load("0:/midicore.cfg");

// Get values with defaults
int32_t tempo = runtime_config_get_int("tempo", 120);
uint8_t enable_metronome = runtime_config_get_bool("metronome.enabled", 1);
float volume = runtime_config_get_float("master_volume", 0.8);
const char* theme = runtime_config_get_string("ui.theme", "dark");

// Set values
runtime_config_set_int("tempo", 140);
runtime_config_set_bool("metronome.enabled", 0);
runtime_config_set_float("master_volume", 0.9);

// Save changes
runtime_config_save("0:/midicore.cfg");

// Check existence
if (runtime_config_exists("experimental.feature_x")) {
  // Enable feature
}

// Register callback for changes
void on_config_change(const char* key, const char* old, const char* new) {
  printf("Config changed: %s = %s (was %s)\n", key, new, old);
}
runtime_config_set_change_callback(on_config_change);

// Management
runtime_config_delete("obsolete_setting");
runtime_config_clear();
runtime_config_print();
```

**Config File Example (`midicore.cfg`):**
```ini
# MidiCore Configuration
# User-editable settings

tempo=120
master_volume=0.8

[metronome]
enabled=1
sound=click

[ui]
theme=dark
brightness=80

[midi]
channel=1
velocity_curve=linear

[experimental]
feature_x=0
```

---

## Integration with Testing

The test utilities in `App/tests/test_config_runtime.c` have been refactored to use these production services:

**Before (test-only):**
- All code in `App/tests/`
- Only available during testing
- Tightly coupled to test framework

**After (production-ready):**
- Core functionality in `Services/`
- Available everywhere (tests + production)
- Clean separation of concerns
- Test code uses same APIs as production

---

## Usage Examples

### Example 1: Monitor Critical Operations

```c
#include "Services/performance/perf_monitor.h"

void app_init(void) {
  perf_monitor_init();
}

void midi_event_handler(void) {
  static perf_metric_id_t midi_id = 0;
  if (midi_id == 0) {
    midi_id = perf_monitor_register("MIDI_Handler");
  }
  
  perf_monitor_start(midi_id);
  
  // Process MIDI event
  handle_note_on();
  update_arpeggiator();
  trigger_envelopes();
  
  perf_monitor_end(midi_id);
  
  // Check if too slow
  uint32_t avg = perf_monitor_get_average(midi_id);
  if (avg > 10) {  // More than 10ms average
    log_warning("MIDI processing slow: %lu ms", avg);
  }
}

void periodic_diagnostics(void) {
  perf_monitor_report_uart();
  perf_monitor_save_csv("0:/diagnostics.csv");
}
```

### Example 2: User-Configurable Behavior

```c
#include "Services/config/runtime_config.h"

void app_init(void) {
  runtime_config_init();
  
  // Load user config (or use defaults)
  if (runtime_config_load("0:/user_config.ini") < 0) {
    // No config file, use defaults
    runtime_config_set_int("tempo", 120);
    runtime_config_set_bool("metronome", 1);
    runtime_config_save("0:/user_config.ini");
  }
}

void setup_metronome(void) {
  int32_t tempo = runtime_config_get_int("tempo", 120);
  uint8_t enabled = runtime_config_get_bool("metronome", 1);
  
  metronome_set_tempo(tempo);
  metronome_set_enabled(enabled);
}

void user_changed_tempo(int32_t new_tempo) {
  runtime_config_set_int("tempo", new_tempo);
  runtime_config_save("0:/user_config.ini");
  setup_metronome();  // Apply immediately
}
```

### Example 3: A/B Testing New Features

```c
void check_experimental_features(void) {
  runtime_config_load("0:/config.ini");
  
  if (runtime_config_get_bool("experimental.new_algo", 0)) {
    use_new_algorithm();
  } else {
    use_stable_algorithm();
  }
  
  if (runtime_config_get_bool("experimental.ui_v2", 0)) {
    enable_new_ui();
  }
}
```

---

## Memory Usage

**Performance Monitor:**
- Per-metric: 36 bytes × 32 metrics = 1,152 bytes
- Total: ~1.2 KB

**Runtime Config:**
- Per-entry: 192 bytes × 64 entries = 12,288 bytes
- Total: ~12 KB

Both services use static allocation, no heap required.

---

## Build Integration

Add to your build system:

```makefile
# In Makefile or CMakeLists.txt
SOURCES += Services/performance/perf_monitor.c
SOURCES += Services/config/runtime_config.c

INCLUDES += -IServices/performance
INCLUDES += -IServices/config
```

---

## Module Enable Flags

To control inclusion in builds:

```c
// In Config/module_config.h or similar
#define MODULE_ENABLE_PERF_MONITOR 1
#define MODULE_ENABLE_RUNTIME_CONFIG 1
```

Wrap service code:
```c
#ifdef MODULE_ENABLE_PERF_MONITOR
  perf_monitor_init();
  // ...
#endif
```

---

## Migration Guide

### For Existing Test Code

**Old (test-only):**
```c
#include "App/tests/test_config_runtime.h"
test_perf_start(test_id);
test_perf_end(test_id);
```

**New (production-ready):**
```c
#include "Services/performance/perf_monitor.h"
perf_monitor_start(metric_id);
perf_monitor_end(metric_id);
```

### For New Production Code

Simply include and use:
```c
#include "Services/performance/perf_monitor.h"
#include "Services/config/runtime_config.h"

// Use anywhere in your codebase
```

---

## Best Practices

**Performance Monitoring:**
1. Register metrics once (static or at init)
2. Start/end pairs must be balanced
3. Don't monitor trivial operations (<1ms)
4. Export data periodically, not on every call
5. Reset statistics when needed (e.g., after export)

**Runtime Configuration:**
1. Provide sensible defaults
2. Validate values after loading
3. Save only when necessary (SD card wear)
4. Use callbacks for immediate effect
5. Document config file format for users

---

## Related Files

**Implementation:**
- `Services/performance/perf_monitor.h` - Performance API
- `Services/performance/perf_monitor.c` - Implementation
- `Services/config/runtime_config.h` - Config API  
- `Services/config/runtime_config.c` - Implementation

**Tests:**
- `App/tests/test_config_runtime.h` - Test-specific wrappers
- `App/tests/test_config_runtime.c` - Test utilities

**Documentation:**
- This file (PRODUCTION_UTILITIES.md)

---

## Version History

- **v1.0** (2026-01-24) - Initial release
  - Performance monitoring service
  - Runtime configuration service
  - Production-ready refactoring of Phase 3 features

## Authors

- Implementation: Copilot Coding Agent
- Integration: MidiCore Project Team
