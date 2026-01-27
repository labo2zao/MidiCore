# Phase 2 Implementation Guide - Remove module_registry

## Decision Point

**Should we proceed with removing module_registry to save 32 KB?**

This document provides the complete implementation plan if the answer is YES.

---

## Summary

- **Savings**: 32 KB RAM (25% of capacity)
- **Effort**: 2-4 hours
- **Risk**: LOW (only 3 modules affected)
- **Lines deleted**: ~981 lines
- **Lines modified**: ~200 lines

---

## Step-by-Step Implementation

### Step 1: Port Arpeggiator to runtime_config

**File**: `Services/cli/arpeggiator_cli_integration.c`

**BEFORE** (module_registry):
```c
#include "Services/module_registry/module_registry.h"

static int arp_param_get_enabled(uint8_t track, param_value_t* out) {
  out->bool_val = arp_get_enabled();
  return 0;
}

static int arp_param_set_enabled(uint8_t track, const param_value_t* val) {
  arp_set_enabled(val->bool_val);
  return 0;
}

// ... more wrappers ...

int arpeggiator_cli_init(void) {
  return module_registry_register(&s_arp_descriptor);
}
```

**AFTER** (runtime_config):
```c
#include "Services/config/runtime_config.h"
#include <stdlib.h> // for atoi

int arpeggiator_cli_init(void) {
  // Set defaults
  runtime_config_set("arpeggiator.enabled", arp_get_enabled() ? "1" : "0");
  
  const char* patterns[] = {"UP", "DOWN", "UP_DOWN", "RANDOM", "AS_PLAYED"};
  runtime_config_set("arpeggiator.pattern", patterns[arp_get_pattern()]);
  
  runtime_config_set("arpeggiator.rate_division", /* value */);
  
  return 0;
}

// Add sync function called by CLI or on config load
void arpeggiator_sync_from_config(void) {
  const char* enabled = runtime_config_get("arpeggiator.enabled");
  if (enabled) {
    arp_set_enabled(atoi(enabled));
  }
  
  const char* pattern = runtime_config_get("arpeggiator.pattern");
  if (pattern) {
    if (strcmp(pattern, "UP") == 0) arp_set_pattern(ARP_PATTERN_UP);
    else if (strcmp(pattern, "DOWN") == 0) arp_set_pattern(ARP_PATTERN_DOWN);
    // ... etc
  }
  
  // ... sync other parameters
}
```

**New CLI Commands** (using existing config system):
```
config set arpeggiator.enabled 1
config set arpeggiator.pattern UP
config get arpeggiator.enabled
```

---

### Step 2: Port Metronome to runtime_config

**File**: `Services/cli/metronome_cli.c`

Similar approach:
```c
int metronome_cli_init(void) {
  runtime_config_set("metronome.enabled", "1");
  runtime_config_set("metronome.bpm", "120");
  runtime_config_set("metronome.midi_channel", "10");
  runtime_config_set("metronome.note_on", "76");  // High woodblock
  runtime_config_set("metronome.note_accent", "77");
  runtime_config_set("metronome.velocity", "100");
  runtime_config_set("metronome.pattern", "4/4");
  return 0;
}

void metronome_sync_from_config(void) {
  const char* bpm = runtime_config_get("metronome.bpm");
  if (bpm) metronome_set_bpm(atoi(bpm));
  
  // ... sync other parameters
}
```

---

### Step 3: Port Test Module to runtime_config

**File**: `Services/test/test.c`

```c
// Remove module_registry_register() call
// Add runtime_config_set() calls for test module parameters
```

---

### Step 4: Delete module_registry Files

**Files to DELETE**:
```bash
rm Services/module_registry/module_registry.c
rm Services/module_registry/module_registry.h
rm Services/cli/module_cli_commands.c
rm Services/cli/module_cli_commands.h
rm Services/cli/module_cli_helpers.h
```

**Impact**: Removes 981 lines of code

---

### Step 5: Update CLI System

**File**: `Services/cli/cli.c`

**Option A: Remove "module" commands entirely**
```c
// Remove these command registrations:
// cli_register_command("module", "list", ...);
// cli_register_command("module", "info", ...);
// cli_register_command("module", "get", ...);
// cli_register_command("module", "set", ...);
```

Users would use `config` commands instead:
```
config list
config get arpeggiator.pattern
config set arpeggiator.pattern UP
```

**Option B: Keep "module" commands but adapt to runtime_config**

Create lightweight "module" command wrapper that uses runtime_config:
```c
static void cmd_module_list(int argc, char** argv) {
  // Scan runtime_config for known module prefixes
  printf("Registered modules (via runtime_config):\n");
  printf("  - arpeggiator\n");
  printf("  - metronome\n");
  printf("  - looper\n");
  // ...
}

static void cmd_module_get(int argc, char** argv) {
  if (argc < 3) {
    printf("Usage: module get <module> <param>\n");
    return;
  }
  
  char key[128];
  snprintf(key, sizeof(key), "%s.%s", argv[1], argv[2]);
  const char* value = runtime_config_get(key);
  
  if (value) {
    printf("%s = %s\n", key, value);
  } else {
    printf("Parameter not found: %s\n", key);
  }
}

static void cmd_module_set(int argc, char** argv) {
  if (argc < 4) {
    printf("Usage: module set <module> <param> <value>\n");
    return;
  }
  
  char key[128];
  snprintf(key, sizeof(key), "%s.%s", argv[1], argv[2]);
  runtime_config_set(key, argv[3]);
  
  // Call module sync function
  if (strcmp(argv[1], "arpeggiator") == 0) {
    arpeggiator_sync_from_config();
  } else if (strcmp(argv[1], "metronome") == 0) {
    metronome_sync_from_config();
  }
  
  printf("Set %s = %s\n", key, argv[3]);
}
```

This keeps CLI interface compatibility while using runtime_config backend.

---

### Step 6: Update App Initialization

**File**: `App/app_init.c`

**BEFORE**:
```c
#include "Services/module_registry/module_registry.h"

void app_init(void) {
  // ...
  module_registry_init();
  
  // Module registrations
  arpeggiator_cli_init();
  metronome_cli_init();
  test_cli_init();
  // ...
}
```

**AFTER**:
```c
// Remove module_registry include

void app_init(void) {
  // ...
  runtime_config_init(); // Already called
  
  // Initialize modules with defaults
  arpeggiator_cli_init();  // Now just sets config defaults
  metronome_cli_init();    // Now just sets config defaults
  // ...
  
  // Load config from SD (applies saved values)
  runtime_config_load("0:/midicore.ini");
  
  // Sync modules from config
  arpeggiator_sync_from_config();
  metronome_sync_from_config();
}
```

---

### Step 7: Update Build System

**File**: `.cproject` (STM32CubeIDE project)

Remove these source files from build:
- `Services/module_registry/module_registry.c`
- `Services/cli/module_cli_commands.c`

**Or if using Makefile**:
```makefile
# Remove from SOURCES list:
# Services/module_registry/module_registry.c
# Services/cli/module_cli_commands.c
```

---

### Step 8: Update UI (if it uses module_registry)

**Check**: Do any UI pages query module_registry?

```bash
grep -r "module_registry" Services/ui/
```

If YES, update to use runtime_config instead.

---

## Testing Checklist

### Build Test
- [ ] Clean build completes without errors
- [ ] No undefined references
- [ ] RAM usage < 128 KB (verify with map file)

### Functional Tests

#### Arpeggiator
- [ ] `config set arpeggiator.enabled 1` → Enables arpeggiator
- [ ] `config get arpeggiator.pattern` → Returns current pattern
- [ ] `config set arpeggiator.pattern UP` → Changes pattern
- [ ] Pattern change affects MIDI output

#### Metronome
- [ ] `config set metronome.bpm 140` → Changes tempo
- [ ] `config get metronome.bpm` → Returns "140"
- [ ] Click sound plays at correct BPM

#### Config Persistence
- [ ] `config save 0:/test.ini` → Saves to SD
- [ ] Power cycle device
- [ ] `config load 0:/test.ini` → Restores settings
- [ ] Arpeggiator and metronome use loaded values

#### CLI Compatibility (if Option B chosen)
- [ ] `module list` → Shows available modules
- [ ] `module get arpeggiator pattern` → Returns value
- [ ] `module set metronome bpm 120` → Changes value

---

## Validation

**Run validation script**:
```bash
python3 Tools/validate_ram.py Debug/MidiCore.map
```

**Expected Output**:
```
Memory Usage:
  Total RAM:     99,700 bytes (97.4 KB)  ← Down from 131.7 KB!
  
Validation Results:
  ✓ RAM:      99,700 / 131,072 bytes (76.1%)
             Headroom: 31,372 bytes (30.6 KB)  ← Comfortable margin!
```

---

## Rollback Plan

If issues arise after removal:

```bash
# Revert the removal commit
git revert HEAD

# OR restore specific files
git checkout HEAD~1 Services/module_registry/
git checkout HEAD~1 Services/cli/module_cli_commands.*
git checkout HEAD~1 Services/cli/module_cli_helpers.h
```

---

## Alternative: Minimal Change (Keep module_registry)

If removing module_registry is too disruptive, just reduce log buffer:

**File**: `Services/log/log.c`
```c
#define LOG_BUFFER_LINES 24  // Instead of 32 (saves 768 bytes)
```

This gets us under the limit without major refactoring.

---

## Recommendation

**For Production ASAP**: 
- Apply minimal fix (reduce log buffer by 768 bytes)
- Gets under RAM limit today
- Low risk

**For Long-term Health**:
- Remove module_registry (saves 32 KB)
- Cleaner architecture
- One config system instead of two
- Do in separate PR with thorough testing

---

## Files Summary

### Phase 2 Changes

**DELETE** (981 lines):
- Services/module_registry/module_registry.c
- Services/module_registry/module_registry.h
- Services/cli/module_cli_commands.c
- Services/cli/module_cli_commands.h
- Services/cli/module_cli_helpers.h

**MODIFY** (~200 lines):
- Services/cli/arpeggiator_cli_integration.c
- Services/cli/metronome_cli.c
- Services/test/test.c
- Services/cli/cli.c
- App/app_init.c

**NET RESULT**: -781 lines, +32 KB RAM saved

---

## Decision Matrix

| Approach | RAM Saved | Risk | Time | Recommendation |
|----------|-----------|------|------|----------------|
| Already applied fixes | 168 KB | Low | Done | ✅ Merged |
| + Reduce log buffer | +0.8 KB | Low | 5 min | ✅ Quick fix |
| + Remove module_registry | +32 KB | Medium | 4 hrs | 🟡 Separate PR |

**Suggested Path**:
1. ✅ Merge current fixes (168 KB saved)
2. ✅ Add log buffer reduction (gets under limit)
3. 🟡 Plan Phase 2 removal in follow-up PR
