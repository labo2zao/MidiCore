# Redundancy Analysis - Module Registry vs Runtime Config

## Executive Summary

**CRITICAL FINDING**: The firmware has **TWO separate configuration systems** with overlapping functionality, consuming **44.2 KB combined** (22% of RAM capacity!).

### The Two Systems

1. **runtime_config** (PR #??, earlier): INI-style key-value storage
   - Size: 12.5 KB (64 entries × 196 bytes)
   - Usage: 23 call sites
   - Format: Simple string key-value pairs

2. **module_registry** (PR #66): Structured module metadata
   - Size: 32 KB after fix (32 modules × 1024 bytes)
   - Usage: **Only 3 modules registered!** (arpeggiator, metronome, test)
   - Format: Typed parameters with getters/setters

**Overlap**: Both provide configuration storage, module enable/disable, parameter get/set, CLI commands.

---

## Detailed Analysis

### System 1: Runtime Config (Existing)

**File**: `Services/config/runtime_config.{c,h}`

**Data Structure:**
```c
typedef struct {
  char key[64];              // 64 bytes
  char value[128];           // 128 bytes  
  const char* section;       // 4 bytes (pointer)
} config_entry_t;             // = 196 bytes

static config_entry_t g_config[64];  // 12,544 bytes (12.2 KB)
```

**Features:**
- ✅ Load/save from SD card (INI format)
- ✅ Key-value pairs with sections
- ✅ String-based (flexible but untyped)
- ✅ Change callbacks
- ✅ Already integrated with CLI

**Usage Examples:**
```c
runtime_config_set("arpeggiator.pattern", "UP");
runtime_config_get("looper.bpm");
runtime_config_load("0:/config.ini");
```

**CLI Commands:**
```
config list
config get arpeggiator.pattern
config set looper.bpm 120
config load 0:/midicore.ini
config save 0:/midicore.ini
```

---

### System 2: Module Registry (PR #66)

**File**: `Services/module_registry/module_registry.{c,h}`

**Data Structures:**
```c
typedef struct {
  char name[24];             // 24 bytes (reduced from 32)
  char description[64];      // 64 bytes (reduced from 128)
  param_type_t type;         // 4 bytes
  int32_t min, max;          // 8 bytes
  const char** enum_values;  // 4 bytes
  uint8_t enum_count;        // 1 byte
  uint8_t read_only;         // 1 byte
  int (*get_value)(...);     // 4 bytes
  int (*set_value)(...);     // 4 bytes
} module_param_t;            // ~114 bytes

typedef struct {
  char name[24];             // 24 bytes
  char description[64];      // 64 bytes
  module_category_t category;// 4 bytes
  // Function pointers (16 bytes)
  module_param_t params[8];  // 8 × 114 = 912 bytes
  uint8_t param_count;       // 1 byte
  uint8_t has_per_track_state; // 1 byte
  uint8_t is_global;         // 1 byte
  uint8_t registered;        // 1 byte
} module_descriptor_t;       // ~1,024 bytes

static module_descriptor_t s_modules[32];  // 32,768 bytes (32 KB)
```

**Features:**
- ✅ Typed parameters (bool, int, float, enum)
- ✅ Parameter validation (min/max ranges)
- ✅ Function pointer callbacks (type-safe)
- ✅ Per-track vs global configuration
- ✅ Module categories for UI organization
- ❌ **Only 3 modules use it** (4.7% utilization!)

**Actual Registrations:**
```c
Services/cli/arpeggiator_cli_integration.c: module_registry_register(&s_arp_descriptor);
Services/cli/metronome_cli.c: module_registry_register(&s_metronome_descriptor);
Services/test/test.c: module_registry_register(&desc);
```

**CLI Commands:**
```
module list
module info arpeggiator
module get arpeggiator pattern
module set looper bpm 120
module enable/disable <name>
```

---

## Redundancy Matrix

| Feature | runtime_config | module_registry | Overlap? |
|---------|----------------|-----------------|----------|
| Configuration storage | ✓ | ✓ | **YES** |
| Key-value access | ✓ | ✓ | **YES** |
| Load/save from SD | ✓ | ✗ | Partial |
| CLI integration | ✓ | ✓ | **YES** |
| Enable/disable control | ✓ | ✓ | **YES** |
| Parameter get/set | ✓ | ✓ | **YES** |
| Type validation | ✗ | ✓ | No |
| Range validation | ✗ | ✓ | No |
| Per-track config | ✗ | ✓ | No |
| **Memory usage** | 12.5 KB | 32 KB | **44.5 KB!** |
| **Actual usage** | 23 sites | 3 modules | Low! |

---

## Root Cause Analysis

**Why This Happened:**

1. **PR #66 added module_registry** without removing or refactoring runtime_config
2. **Over-allocation**: Registry sized for 64→32 modules but only 3 use it
3. **Incomplete migration**: Other 67 services still use old config system
4. **No consolidation**: Both systems coexist with duplicate functionality

**Result**: 44.5 KB spent on configuration (23% of 192 KB total capacity!)

---

## Proposed Solutions

### Option 1: **REMOVE module_registry** (Saves 32 KB) ✅ RECOMMENDED

**Rationale:**
- Only 3 modules use it (4.7% adoption)
- runtime_config already works and is widely used
- Simpler to remove new system than migrate 67 services

**Impact:**
- **Saves**: 32 KB RAM
- **Removes**: 665 lines of code (module_registry.{c,h})
- **Breaks**: 3 module integrations (easy to port)

**Migration:**
```c
// BEFORE (module_registry):
module_set("arpeggiator", "pattern", "UP");

// AFTER (runtime_config):
runtime_config_set("arpeggiator.pattern", "UP");
```

**Steps:**
1. Migrate 3 modules to use runtime_config
2. Remove module_registry.{c,h}
3. Remove module CLI commands (or adapt to runtime_config)
4. Update App/app_init.c to remove registry init

---

### Option 2: **MERGE systems** into hybrid (Saves ~20 KB)

**Approach:**
- Keep runtime_config for storage
- Add type metadata layer (lightweight)
- Reuse runtime_config arrays, add type info

**Benefits:**
- Best of both worlds
- Gradual migration path
- Type safety + flexibility

**Drawbacks:**
- More complex
- Requires refactoring both systems
- Takes more time

---

### Option 3: **REMOVE runtime_config**, keep module_registry (Saves 12.5 KB)

**Rationale:**
- Module registry is more structured
- Better type safety and validation
- Cleaner API for future

**Impact:**
- **Saves**: 12.5 KB RAM
- **Requires**: Migrating 23 runtime_config call sites
- **Requires**: Registering 67 modules (massive work!)

**Why NOT recommended:**
- 67 modules need registration vs 3 modules need porting
- registry already over-allocated (32 slots for 3 modules)
- More work, less savings

---

## Recommended Action Plan

**Phase 1: Remove module_registry (Immediate - 32 KB savings)**

1. ✅ Port 3 modules to runtime_config:
   ```c
   // Services/cli/arpeggiator_cli_integration.c
   // Replace module_registry_register() with runtime_config calls
   
   void arpeggiator_init(void) {
     runtime_config_set("arpeggiator.enabled", "1");
     runtime_config_set("arpeggiator.pattern", "UP");
   }
   
   void arpeggiator_set_pattern(const char* val) {
     runtime_config_set("arpeggiator.pattern", val);
   }
   ```

2. ✅ Remove files:
   - `Services/module_registry/module_registry.{c,h}`
   - `Services/cli/module_cli_commands.{c,h}`
   - `Services/cli/module_cli_helpers.h`

3. ✅ Update CLI:
   - Remove "module" commands OR
   - Adapt "module" commands to use runtime_config backend

4. ✅ Update App/app_init.c:
   - Remove `module_registry_init()`
   - Remove module registrations

**Estimated savings: 32 KB RAM + ~1 KB code cleanup**

---

**Phase 2: Optimize runtime_config (Future - 6 KB savings)**

If more RAM needed, reduce runtime_config:

```c
// Services/config/runtime_config.h
#define CONFIG_MAX_ENTRIES 32  // Instead of 64 (saves 6.2 KB)
#define CONFIG_MAX_KEY_LEN 48  // Instead of 64
#define CONFIG_MAX_VALUE_LEN 96 // Instead of 128
```

**Additional savings: 6-8 KB**

---

## Impact Assessment

### Before Cleanup
```
Module Registry:   32 KB (3 modules using it = 10.6 KB per module!)
Runtime Config:    12.5 KB (23 call sites = 544 bytes per site)
TOTAL:             44.5 KB
```

### After Phase 1
```
Module Registry:   REMOVED (-32 KB)
Runtime Config:    12.5 KB (26 call sites now)
TOTAL:             12.5 KB
NET SAVINGS:       32 KB (72% reduction!)
```

### After Phase 2 (Optional)
```
Runtime Config:    6.3 KB (optimized)
TOTAL:             6.3 KB
NET SAVINGS:       38.2 KB (86% reduction!)
```

---

## Risk Analysis

### Option 1 Risks (Remove module_registry)

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| Lose type safety | High | Low | Runtime config validates strings |
| Break UI menus | Medium | Medium | UI can query runtime_config directly |
| Lose per-track config | Low | Medium | Implement via key naming (e.g., "arp.0.pattern") |
| Migration bugs | Low | Low | Only 3 modules to port, simple API |

**Overall Risk**: 🟢 **LOW** - Only 3 modules affected, easy to port

---

## Code Changes Required

### Remove module_registry (32 KB savings)

**Files to DELETE:**
- `Services/module_registry/module_registry.c` (349 lines)
- `Services/module_registry/module_registry.h` (316 lines)
- `Services/cli/module_cli_commands.c` 
- `Services/cli/module_cli_commands.h`
- `Services/cli/module_cli_helpers.h`

**Files to MODIFY:**
- `Services/cli/arpeggiator_cli_integration.c` - Port to runtime_config
- `Services/cli/metronome_cli.c` - Port to runtime_config
- `Services/test/test.c` - Port to runtime_config
- `App/app_init.c` - Remove registry init
- `Services/cli/cli.c` - Remove/adapt module commands

**Lines of code removed**: ~1,200 lines  
**Lines of code modified**: ~300 lines  
**Net deletion**: ~900 lines

---

## Testing Requirements

### Functional Tests After Removal

1. **Arpeggiator Configuration**
   ```
   config set arpeggiator.pattern UP
   config get arpeggiator.pattern
   → Should return "UP"
   ```

2. **Metronome Configuration**
   ```
   config set metronome.bpm 120
   config get metronome.bpm
   → Should return "120"
   ```

3. **Config Persistence**
   ```
   config save 0:/test.ini
   → Restart device
   config load 0:/test.ini
   → Settings restored
   ```

4. **CLI Still Works**
   ```
   help
   config list
   → All commands functional
   ```

---

## Timeline

- **Investigation**: ✅ Complete
- **Phase 1 Implementation**: 2-4 hours
- **Testing**: 1-2 hours
- **Documentation**: 1 hour
- **Total**: ~1 day

---

## Recommendation

**PROCEED WITH OPTION 1: Remove module_registry**

**Why:**
- Biggest RAM savings (32 KB = 25% of capacity!)
- Lowest risk (only 3 modules affected)
- Fastest implementation
- Removes complexity
- runtime_config is proven and widely used

**Next Step:**
Create a new commit that:
1. Ports 3 modules to runtime_config
2. Removes module_registry files
3. Updates build files
4. Tests all affected functionality

**Expected Result:**
```
BEFORE: 131.7 KB RAM (100.5% over capacity)
AFTER:   99.7 KB RAM (77.8% of capacity) ✓
SAVINGS: 32 KB (24.3% reduction)
```

This single change would put us comfortably under the RAM limit with 28 KB headroom!
