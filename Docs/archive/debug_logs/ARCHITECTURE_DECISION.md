# Architecture Decision - Module Configuration System

## Current Situation

We have TWO configuration systems:

### System A: runtime_config (Simple)
- **Storage**: String key-value pairs in static array
- **Type Safety**: None (everything is strings)
- **Validation**: None
- **Usage**: 23 call sites across codebase
- **RAM**: 12.5 KB
- **Pros**: Simple, flexible, widely used
- **Cons**: No type safety, no validation, string parsing overhead

### System B: module_registry (Structured)
- **Storage**: Typed parameter descriptors with metadata
- **Type Safety**: Strong (bool, int, float, enum)
- **Validation**: Min/max ranges, enum values
- **Usage**: 3 modules registered
- **RAM**: 32 KB (but 197 KB originally!)
- **Pros**: Type safety, validation, structured, per-track support, CLI/UI friendly
- **Cons**: More complex, RAM intensive, low adoption

## Architecture Decision: Choose module_registry as Foundation

### Why module_registry is Better Long-term

**1. Type Safety**
```c
// runtime_config (NO type safety)
runtime_config_set("bpm", "abc");  // Silently broken!
int bpm = atoi(runtime_config_get("bpm"));  // Returns 0, no error

// module_registry (TYPED)
param_value_t val = {.int_val = 999};
module_set_param("metronome", "bpm", &val);  // Validates 30-300 range!
```

**2. Validation Built-in**
```c
// No need for manual validation in every module
module_param_t bpm_param = {
  .type = PARAM_TYPE_INT,
  .min = 30,
  .max = 300,  // Automatic range checking!
};
```

**3. Better CLI Integration**
```c
// CLI can auto-generate help from metadata
module info metronome
  Parameters:
    bpm (int): 30-300, current: 120
    pattern (enum): 4/4, 3/4, 6/8
    enabled (bool): true/false
```

**4. Per-Track Configuration**
```c
// Essential for multi-track systems
module set arpeggiator pattern UP 0   // Track 0
module set arpeggiator pattern DOWN 1  // Track 1
```

**5. UI Auto-Generation**
```c
// UI can build menus automatically from registry
for (each module) {
  display_module_name();
  for (each param) {
    display_param_with_type();  // Knows it's a slider, dropdown, toggle
  }
}
```

---

## Recommended Architecture: "Optimized module_registry"

### Strategy: Keep module_registry but Make it Efficient

**Problem with current implementation:**
- Static arrays sized for 64 modules (only 3 used)
- Large descriptors with embedded parameter arrays
- 197 KB → 32 KB (after our fix) but still wasteful

**Solution: Dynamic Registration with Shared Storage**

```c
// NEW APPROACH: Separate storage for descriptors and parameters

// Lightweight module descriptor (no embedded params)
typedef struct {
  const char* name;           // Pointer (4 bytes)
  const char* description;    // Pointer (4 bytes)
  module_category_t category; // 4 bytes
  // Function pointers (16 bytes)
  module_param_t* params;     // Pointer to params (4 bytes)
  uint8_t param_count;        // 1 byte
  uint8_t flags;              // 1 byte
} module_descriptor_t;        // = 34 bytes!

// Parameters stored separately
static module_param_t g_all_params[128];  // Shared pool
static module_descriptor_t g_modules[32];

// Registration stores pointers, not copies
int module_registry_register(const module_descriptor_t* desc) {
  // Store pointer, not full copy!
  g_modules[count].name = desc->name;
  g_modules[count].params = desc->params;  // Reference, not copy
}
```

**RAM Savings:**
```
OLD: 32 modules × 1024 bytes = 32 KB
NEW: 32 modules × 34 bytes = 1 KB (!!)
     + 128 params × 120 bytes = 15 KB
     = 16 KB total (50% saving!)
```

---

## Migration Plan: Port runtime_config Users to module_registry

### Phase 1: Optimize module_registry (Save 16 KB)

**File**: `Services/module_registry/module_registry.h`

**NEW Structure:**
```c
// Efficient descriptor (34 bytes vs 1024 bytes)
typedef struct {
  const char* name;              // 4 bytes (pointer to const string)
  const char* description;       // 4 bytes (pointer to const string)
  module_category_t category;    // 4 bytes
  
  // Function pointers (16 bytes total)
  int (*init)(void);
  int (*enable)(uint8_t track);
  int (*disable)(uint8_t track);
  int (*get_status)(uint8_t track);
  
  module_param_t* params;        // 4 bytes (pointer to param array)
  uint8_t param_count;           // 1 byte
  uint8_t flags;                 // 1 byte
} module_descriptor_t;           // = 34 bytes

// Shared parameter pool
#define MAX_TOTAL_PARAMS 128     // Across all modules
static module_param_t g_param_pool[MAX_TOTAL_PARAMS];
static uint8_t g_param_pool_used = 0;

// Module registry (much smaller!)
#define MODULE_REGISTRY_MAX_MODULES 32
static module_descriptor_t g_modules[MODULE_REGISTRY_MAX_MODULES];
```

**Registration becomes:**
```c
// In arpeggiator_cli_integration.c
static const char arp_name[] = "arpeggiator";
static const char arp_desc[] = "Pattern-based note arpeggiator";

static module_param_t arp_params[] = {
  {.name = "enabled", .type = PARAM_TYPE_BOOL, ...},
  {.name = "pattern", .type = PARAM_TYPE_ENUM, ...},
  {.name = "rate", .type = PARAM_TYPE_INT, .min = 1, .max = 32, ...},
};

static const module_descriptor_t arp_descriptor = {
  .name = arp_name,          // Pointer!
  .description = arp_desc,    // Pointer!
  .category = MODULE_CATEGORY_EFFECT,
  .params = arp_params,       // Pointer to const array!
  .param_count = 3,
  // ...
};

module_registry_register(&arp_descriptor);  // Just stores pointers
```

**Savings: 32 KB → 16 KB (16 KB freed!)**

---

### Phase 2: Migrate Key Modules to module_registry

**Priority modules to register (based on CLI usage):**

1. **Looper** (core feature)
2. **Router** (MIDI routing)
3. **MIDI DIN** (communication)
4. **AIN/AINSER** (analog input)
5. **Expression** (bellows/pressure)
6. **Zones** (keyboard configuration)
7. **Patch System** (presets)
8. **UI** (display settings)

**Template for migration:**
```c
// For any module currently using runtime_config

// OLD (runtime_config):
runtime_config_set("looper.bpm", "120");
const char* bpm = runtime_config_get("looper.bpm");

// NEW (module_registry):
static module_param_t looper_params[] = {
  {
    .name = "bpm",
    .description = "Beats per minute",
    .type = PARAM_TYPE_INT,
    .min = 30,
    .max = 300,
    .get_value = looper_get_bpm_wrapper,
    .set_value = looper_set_bpm_wrapper,
  },
  // ... more params
};

static const module_descriptor_t looper_descriptor = {
  .name = "looper",
  .description = "MIDI looper/sequencer",
  .category = MODULE_CATEGORY_LOOPER,
  .params = looper_params,
  .param_count = sizeof(looper_params) / sizeof(looper_params[0]),
  .init = looper_init,
  .enable = looper_enable,
  .disable = looper_disable,
};

// Register at init
module_registry_register(&looper_descriptor);
```

---

### Phase 3: Deprecate runtime_config (Saves 12.5 KB)

Once all modules migrated:
```c
// Remove Services/config/runtime_config.{c,h}
// Keep only module_registry as single source of truth
```

**Total savings: 16 KB + 12.5 KB = 28.5 KB**

---

## Implementation Roadmap

### Step 1: Optimize module_registry Structure ✅ DO THIS

**Files to modify:**
1. `Services/module_registry/module_registry.h` - New efficient structs
2. `Services/module_registry/module_registry.c` - Registration logic

**Changes:**
- Descriptor: 1024 bytes → 34 bytes (97% reduction!)
- Use pointers instead of embedded arrays
- Separate parameter pool

**Impact:**
- Saves 16 KB immediately
- Makes system scalable
- No functional changes

**Effort**: 2-3 hours

---

### Step 2: Register Core Modules (Iterative)

**Week 1 - Essential:**
- Looper
- Router  
- MIDI DIN
- AINSER64

**Week 2 - Important:**
- Expression
- Zones
- Patch System
- UI Settings

**Week 3 - Effects:**
- Arpeggiator ✅ (already done)
- Metronome ✅ (already done)
- Harmonizer
- Quantizer
- LiveFX

**Per module effort**: 30-60 minutes

---

### Step 3: Remove runtime_config

**When**: After 80%+ modules registered

**Steps:**
1. Verify all critical paths use module_registry
2. Remove runtime_config.{c,h}
3. Update build system
4. Test CLI commands work

**Effort**: 2-3 hours

---

## Benefits Summary

### Technical Benefits

| Aspect | Before | After | Benefit |
|--------|--------|-------|---------|
| Type Safety | None | Strong | ✅ Prevents bugs |
| Validation | Manual | Automatic | ✅ Less code |
| RAM Usage | 44.5 KB | 16 KB | ✅ 28.5 KB saved |
| CLI Help | Manual | Auto-gen | ✅ Self-documenting |
| UI Menus | Hardcoded | Auto-gen | ✅ Maintainable |
| Per-track | No | Yes | ✅ Multi-track ready |

### User Benefits

1. **CLI remains powerful**: All commands work, better help
2. **Type safety**: Prevents configuration errors
3. **Auto-discovery**: `module list` shows all available features
4. **Consistent interface**: Same pattern for all modules
5. **Future-proof**: Easy to add new parameters

---

## Code Example: Efficient Registration

```c
// ============================================================================
// LOOPER MODULE REGISTRATION (Efficient Version)
// ============================================================================

// Names and descriptions stored as const (in FLASH, not RAM!)
static const char looper_name[] = "looper";
static const char looper_desc[] = "4-track MIDI looper with quantization";

// Parameter wrappers
static int looper_get_bpm(uint8_t track, param_value_t* out) {
  out->int_val = looper_get_bpm();
  return 0;
}

static int looper_set_bpm(uint8_t track, const param_value_t* val) {
  if (val->int_val < 30 || val->int_val > 300) return -1;
  looper_set_bpm(val->int_val);
  return 0;
}

// Parameters (stored in FLASH as const)
static const module_param_t looper_params[] = {
  {
    .name = "bpm",
    .description = "Tempo in beats per minute",
    .type = PARAM_TYPE_INT,
    .min = 30,
    .max = 300,
    .get_value = looper_get_bpm,
    .set_value = looper_set_bpm,
  },
  {
    .name = "quantize",
    .description = "Quantization grid",
    .type = PARAM_TYPE_ENUM,
    .enum_values = (const char*[]){"OFF", "1/4", "1/8", "1/16"},
    .enum_count = 4,
    .get_value = looper_get_quantize,
    .set_value = looper_set_quantize,
  },
  {
    .name = "metronome",
    .description = "Click track enabled",
    .type = PARAM_TYPE_BOOL,
    .get_value = looper_get_metronome,
    .set_value = looper_set_metronome,
  },
  // ... more params
};

// Descriptor (stored in FLASH as const)
static const module_descriptor_t looper_descriptor = {
  .name = looper_name,              // Pointer (4 bytes)
  .description = looper_desc,        // Pointer (4 bytes)
  .category = MODULE_CATEGORY_LOOPER,// 4 bytes
  .init = looper_init,              // 4 bytes
  .enable = looper_enable,          // 4 bytes
  .disable = looper_disable,        // 4 bytes
  .get_status = looper_get_status,  // 4 bytes
  .params = (module_param_t*)looper_params,  // Pointer (4 bytes)
  .param_count = sizeof(looper_params) / sizeof(looper_params[0]),  // 1 byte
  .flags = MODULE_FLAG_GLOBAL,      // 1 byte
};                                   // Total: 34 bytes in RAM!

// Registration (called once at init)
int looper_register_cli(void) {
  return module_registry_register(&looper_descriptor);
}

// RAM usage:
// - Descriptor reference in registry: 34 bytes
// - Params and strings: 0 bytes (stored in FLASH as const!)
// Total RAM: 34 bytes per module!
```

---

## Decision Matrix

| Option | RAM Saved | Effort | Long-term | Risk |
|--------|-----------|--------|-----------|------|
| A: Remove module_registry | 32 KB | Low | ❌ Bad | Low |
| B: Remove runtime_config | 12.5 KB | High | ✅ Good | Medium |
| C: Optimize module_registry | 16 KB | Medium | ✅ Best | Low |

**RECOMMENDATION: Option C**
- Optimize module_registry structure (saves 16 KB immediately)
- Gradually migrate modules from runtime_config
- Eventually remove runtime_config (saves 12.5 KB more)
- Total: 28.5 KB saved + better architecture

---

## Next Steps - What to Implement?

1. **Immediate** (this PR):
   - Keep current array size reductions (✅ done)
   - Add this architecture document

2. **Follow-up PR #1** (Optimize structure):
   - Implement efficient module_registry structure
   - Migrate 3 existing modules to new structure
   - Test CLI commands
   - Save 16 KB

3. **Follow-up PRs #2-4** (Migrate modules):
   - Register 5-10 modules per PR
   - Test each batch
   - Update documentation

4. **Final PR** (Remove runtime_config):
   - Remove old system
   - Save 12.5 KB more
   - Clean up

**Total timeline: 3-4 weeks**

---

## Questions for User

1. **Should we proceed with efficient module_registry architecture?**
   - Saves 16 KB immediately
   - Better long-term design
   - Keeps CLI powerful

2. **Which modules to prioritize for migration?**
   - Looper? Router? MIDI? AIN?

3. **Timeline acceptable?**
   - Can do iteratively over 3-4 weeks
   - Or faster if needed?

---

**RECOMMENDATION: Implement optimized module_registry in next PR**
- Best architecture for embedded MIDI system
- Keeps CLI functionality
- Saves RAM progressively
- Type-safe and future-proof
