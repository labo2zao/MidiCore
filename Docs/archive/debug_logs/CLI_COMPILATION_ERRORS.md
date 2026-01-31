# MidiCore CLI Files - Compilation Error Report

**Generated:** 2026-01-28  
**Project:** MidiCore STM32F407VG Firmware  
**Scope:** Services/cli/*_cli.c Files

---

## Executive Summary

**Total Errors Found: 4 CRITICAL + Multiple MEDIUM severity issues**

The compilation will **FAIL** due to:
1. **CRITICAL:** Missing module descriptor in `router_cli.c` (linker error)
2. **CRITICAL:** 14 undefined function references in `metronome_cli.c` (linker errors)
3. **MEDIUM:** Inconsistent registration patterns across multiple files
4. **MEDIUM:** Missing parameter getter/setter implementations

---

## CRITICAL ERROR #1: router_cli.c - Missing Module Descriptor

### File Location
- **File:** `Services/cli/router_cli.c`
- **Lines:** 290-317 (end of file)
- **Type:** Linker Error - Undefined Reference

### Problem Description

The `router_cli.c` file is missing the standard `module_descriptor_t` structure that all other CLI modules define. This breaks the module registry system.

### Current State
```c
// router_cli.c Lines 305-317
int router_cli_register(void) {
  return cli_register_command(
    "router",
    cmd_router,
    "Control MIDI routing matrix",
    "router <matrix|enable|disable|channel|label|info|test> [args...]",
    "midi"
  );
}
// FILE ENDS - NO module_descriptor_t definition
```

### Expected State (Pattern from other CLI files)
```c
// MISSING: The following should exist at the end of router_cli.c

static int router_cli_enable(uint8_t track) {
  (void)track;
  return 0;
}

static int router_cli_disable(uint8_t track) {
  (void)track;
  return 0;
}

static int router_cli_get_status(uint8_t track) {
  (void)track;
  return MODULE_STATUS_ENABLED;
}

static int router_cli_init(void) {
  return 0;
}

static module_descriptor_t s_router_descriptor = {
  .name = "router",
  .description = "MIDI Router",
  .category = MODULE_CATEGORY_ROUTING,
  .init = router_cli_init,
  .enable = router_cli_enable,
  .disable = router_cli_disable,
  .get_status = router_cli_get_status,
  .has_per_track_state = 0,
  .is_global = 1
};
```

### Impact
- **Linker Error:** `undefined reference to 's_router_descriptor'`
- **Compilation Will Fail** when trying to build the complete firmware
- The module registry system expects this descriptor to register the router module

### Fix Required
Add the missing module descriptor and control functions to `router_cli.c` (approximately 30-50 lines)

---

## CRITICAL ERROR #2: metronome_cli.c - 14 Undefined Function References

### File Location
- **File:** `Services/cli/metronome_cli.c`
- **Lines:** 21, 29, 40, 48, 59, 67, 78, 86, 97, 105, 116, 124, 171, 270
- **Type:** Linker Error - Undefined References

### Problem Description

`metronome_cli.c` defines parameter getter/setter functions that are NEVER implemented. It only declares stubs that reference functions from `metronome.h`, but those functions don't exist.

### Missing Functions (14 total)

#### Parameter Getter Functions
1. **Line 21:** `metronome_param_get_midi_channel()`
   - Called in setup function (Line 202)
   - Never implemented in metronome.h or metronome.c

2. **Line 40:** `metronome_param_get_accent_note()`
   - Called in setup function (Line 212)
   - Never implemented

3. **Line 59:** `metronome_param_get_regular_note()`
   - Called in setup function (Line 222)
   - Never implemented

4. **Line 78:** `metronome_param_get_accent_velocity()`
   - Called in setup function (Line 232)
   - Never implemented

5. **Line 97:** `metronome_param_get_regular_velocity()`
   - Called in setup function (Line 242)
   - Never implemented

6. **Line 116:** `metronome_param_get_mode()`
   - Called in setup function (Line 192)
   - Never implemented

#### Parameter Setter Functions
7. **Line 29:** `metronome_param_set_midi_channel()`
   - Called in setup function (Line 203)
   - Never implemented

8. **Line 48:** `metronome_param_set_accent_note()`
   - Called in setup function (Line 213)
   - Never implemented

9. **Line 67:** `metronome_param_set_regular_note()`
   - Called in setup function (Line 223)
   - Never implemented

10. **Line 86:** `metronome_param_set_accent_velocity()`
    - Called in setup function (Line 233)
    - Never implemented

11. **Line 105:** `metronome_param_set_regular_velocity()`
    - Called in setup function (Line 243)
    - Never implemented

12. **Line 124:** `metronome_param_set_mode()`
    - Called in setup function (Line 193)
    - Never implemented

#### Registration Functions
13. **Line 171:** `setup_metronome_parameters()`
    - Defined locally but references non-existent functions above

14. **Line 270:** `metronome_register_cli()`
    - Defined at line 270
    - Uses non-existent parameter functions
    - Should call `module_registry_register(&s_metronome_descriptor)`

### Actual Functions in metronome.h
```c
void metronome_cancel_count_in(void);
void metronome_get_config(metronome_cfg_t* cfg);
int metronome_get_enabled(void);
void metronome_init(void);
int metronome_is_count_in_active(void);
void metronome_set_config(const metronome_cfg_t* cfg);
void metronome_set_enabled(int enabled);
void metronome_start_count_in(uint8_t count);
void metronome_sync_tempo(uint16_t bpm);
void metronome_tick_1ms(void);
```

**None of these match the parameter getter/setter pattern expected in metronome_cli.c**

### Setup Function Issue
```c
// metronome_cli.c Lines 171-255
static void setup_metronome_parameters(void) {
  module_param_t params[] = {
    {
      .name = "enabled",
      .description = "Enable metronome",
      .type = PARAM_TYPE_BOOL,
      .get_value = metronome_param_get_enabled,  // EXISTS in header
      .set_value = metronome_param_set_enabled   // DOES NOT EXIST
    },
    {
      .name = "mode",
      .description = "Output mode (0=OFF, 1=MIDI, 2=AUDIO)",
      .type = PARAM_TYPE_ENUM,
      .get_value = metronome_param_get_mode,     // DOES NOT EXIST
      .set_value = metronome_param_set_mode      // DOES NOT EXIST
    },
    // ... more missing functions ...
  };
}
```

### Impact
- **14 Linker Errors:** `undefined reference to 'metronome_param_get_*'`
- **Compilation Will Fail**
- The module descriptor cannot be properly set up without these functions

### Fix Required (Choose one approach)

**Option A: Implement missing functions in metronome.c**
```c
// Add to Services/metronome/metronome.h
int metronome_param_get_mode(uint8_t track, param_value_t* out);
int metronome_param_set_mode(uint8_t track, const param_value_t* val);
// ... etc for all 14 functions
```

**Option B: Use DEFINE_PARAM_* macros properly**
Replace manual function definitions with macro-generated code

**Option C: Fix at source - Update metronome.c to provide proper parameter interface**

---

## MEDIUM SEVERITY ERROR #3: Incomplete Module Registration Pattern

### Files Affected
- `Services/cli/ain_cli.c` (Line 65-67)
- `Services/cli/looper_cli.c` (Line 346-349)
- `Services/cli/metronome_cli.c` (Line 270-273)
- `Services/cli/cc_smoother_cli.c` (Line 108-111)
- And potentially others

### Problem Description

Multiple CLI files follow an inconsistent pattern for module registration. Some use the `MODULE_REGISTER()` macro, while others use custom `_register_cli()` functions.

### Pattern Inconsistency

**Pattern A (ain_cli.c):**
```c
int ain_register_cli(void) {
  setup_ain_parameters();
  return module_registry_register(&s_ain_descriptor);
}
```

**Pattern B (looper_cli.c):**
```c
int looper_register_cli(void) {
  setup_looper_parameters();
  return module_registry_register(&s_looper_descriptor);
}
```

**Pattern C (router_cli.c) - COMPLETELY DIFFERENT:**
```c
int router_cli_register(void) {
  return cli_register_command("router", cmd_router, ...);
}
// NO module_registry_register() call!
// NO MODULE_REGISTER() macro!
```

### Impact
- Different modules use different initialization patterns
- Makes code harder to maintain
- May hide bugs if initialization order matters
- CLI modules won't be properly registered with the module system

### Recommendation
Standardize all CLI files to use the same pattern:
```c
// Standard pattern for all modules
static void setup_<module>_parameters(void) { ... }

int <module>_register_cli(void) {
  setup_<module>_parameters();
  return module_registry_register(&s_<module>_descriptor);
}
```

---

## MEDIUM SEVERITY ERROR #4: Inconsistent Implementation Patterns

### Pattern Variations Detected

**Pattern 1: Simple modules (ain_cli.c, bootloader_cli.c)**
- Use `DEFINE_MODULE_CONTROL_*` macros
- Use simple parameter setup
- Minimal code

**Pattern 2: Complex modules (looper_cli.c, metronome_cli.c)**
- Manual parameter structure definitions
- Complex setup functions
- More code, more potential for errors

**Pattern 3: Non-standard (router_cli.c)**
- Pure CLI command handler
- No module descriptor
- Different registration mechanism

### Impact
- Code inconsistency reduces maintainability
- Developers may not follow the correct pattern
- Harder to add new modules
- Potential for hidden bugs

### Recommendation
1. Choose one pattern as standard
2. Update all existing modules to match
3. Document the standard pattern
4. Add build-time consistency checks

---

## Summary Error Table

| File Name | Lines | Error Type | Severity | Linker Error? |
|-----------|-------|-----------|----------|--------------|
| router_cli.c | 290-317 | Missing module descriptor | CRITICAL | ✓ YES |
| metronome_cli.c | 21,29,40,48,59,67,78,86,97,105,116,124,171,270 | 14 undefined functions | CRITICAL | ✓ YES |
| ain_cli.c | 65-67 | Incomplete registration | MEDIUM | - |
| looper_cli.c | 346-349 | Incomplete registration | MEDIUM | - |
| cc_smoother_cli.c | 108-111 | Incomplete registration | MEDIUM | - |
| Multiple | Various | Inconsistent patterns | MEDIUM | - |

---

## Build Impact

### Current State
- **Compilation:** Will FAIL due to linker errors
- **Errors:** 2 critical undefined references
- **Build cannot complete**

### Required Fixes (Priority Order)

### Priority 1 - CRITICAL (Must fix to compile)
1. [ ] Add module descriptor to `router_cli.c`
   - Add ~35 lines of code to end of file
   - Follow pattern from `ain_cli.c`

2. [ ] Implement 14 missing functions in `metronome_cli.c` or `metronome.c`
   - Add parameter getter/setter functions
   - OR modify metronome_cli.c to use macros instead

### Priority 2 - HIGH (Required for proper function)
3. [ ] Verify all parameter functions are implemented
4. [ ] Test module registration during startup
5. [ ] Ensure all `_register_cli()` functions are called during init

### Priority 3 - MEDIUM (Code Quality)
6. [ ] Standardize registration patterns
7. [ ] Add consistency checks to build system
8. [ ] Document CLI module creation guidelines

---

## Compilation Commands

### To Trigger These Errors (with ARM toolchain installed)
```bash
cd /home/runner/work/MidiCore/MidiCore
make clean
make all  # Will fail with linker errors
```

### Expected Error Output (Example)
```
arm-none-eabi-gcc: error: undefined reference to 's_router_descriptor'
arm-none-eabi-gcc: error: undefined reference to 'metronome_param_get_mode'
arm-none-eabi-gcc: error: undefined reference to 'metronome_param_set_mode'
... (13 more undefined references)
collect2: error: ld returned 1 exit status
```

---

## Files Requiring Modification

### Critical Changes Required
1. **Services/cli/router_cli.c** - Add module descriptor (35 lines)
2. **Services/cli/metronome_cli.c** or **Services/metronome/metronome.c** - Add 14 functions

### Secondary Changes (After Critical Fixes)
3. **Services/cli/ain_cli.c** - Verify registration
4. **Services/cli/looper_cli.c** - Verify registration
5. **Services/cli/cc_smoother_cli.c** - Verify registration
6. Other CLI files - Standardize patterns

---

## References

### Module Descriptor Template
See `Services/cli/module_cli_helpers.h` for macro definitions and patterns.

### Standard Module Pattern
Review `Services/cli/ain_cli.c` for the recommended pattern to follow.

### Header Files Involved
- `Services/cli/module_cli_helpers.h` - Helper macros
- `Services/module_registry/module_registry.h` - Registry definitions
- Individual service headers (e.g., `Services/metronome/metronome.h`)

---

**End of Report**
