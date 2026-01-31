# Memory Optimization: 50.5 KB RAM Savings

## Summary

This optimization reduces RAM usage by **50.5 KB** through two key changes:

1. **Module Registry Pointer-Based Storage** → saves ~38 KB
2. **Runtime Config Removal** → saves ~12.5 KB

## Changes Made

### 1. Module Registry Optimization (~38 KB saved)

**Problem:**
- Registry stored full `module_descriptor_t` copies (1224 bytes each)
- Array of 32 modules = 39,168 bytes (38 KB)

**Solution:**
- Changed to pointer-based storage
- Array now stores `const module_descriptor_t*` (8 bytes each on ARM Cortex-M4)
- Array of 32 pointers = 256 bytes
- **Savings: 38,912 bytes (38 KB)**

**Implementation:**
- Changed `s_modules` from `module_descriptor_t[32]` to `const module_descriptor_t*[32]`
- Updated `module_registry_register()` to store pointer instead of copying
- Updated all accessor functions to dereference pointers
- Removed `registered` field from descriptor struct (no longer needed)

**Impact:**
- Module descriptors must now be static const in module source files
- No behavior change - pointers are to const data
- Zero-copy registration is faster and more efficient

### 2. Runtime Config Removal (~12.5 KB saved)

**Problem:**
- `runtime_config` system allocated 12,800 bytes (64 entries × 200 bytes)
- Used only for debug/test purposes in CLI
- Not used in production code paths

**Solution:**
- Removed `Services/config/runtime_config.c` and `.h`
- Removed `config` command from CLI
- Disabled save/load buttons in UI (marked as "N/A")
- **Savings: 12,800 bytes (12.5 KB)**

**Impact:**
- No runtime INI-style configuration loading
- Module parameters still available via module_registry
- CLI module commands still functional (e.g., `module metronome enable`)

## Files Modified

### Module Registry
- `Services/module_registry/module_registry.h` - Updated descriptor struct and docs
- `Services/module_registry/module_registry.c` - Pointer-based storage implementation

### Runtime Config Removal
- `Services/config/runtime_config.c` - **DELETED**
- `Services/config/runtime_config.h` - **DELETED**
- `Services/cli/cli.c` - Removed config command
- `Services/cli/cli.h` - Updated documentation
- `Services/ui/ui_page_modules.c` - Disabled config save/load buttons

## Total RAM Savings

| Optimization | Before | After | Savings |
|--------------|--------|-------|---------|
| Module Registry | 39,168 bytes | 256 bytes | 38,912 bytes (38.0 KB) |
| Runtime Config | 12,800 bytes | 0 bytes | 12,800 bytes (12.5 KB) |
| **TOTAL** | **51,968 bytes** | **256 bytes** | **51,712 bytes (50.5 KB)** |

## Migration Notes

### For Module Developers

**Before:**
```c
static module_descriptor_t s_my_module = {
  .name = "mymodule",
  // ... fields ...
};

module_registry_register(&s_my_module);
```

**After (no change needed):**
```c
// Same code works - just stores pointer now
static module_descriptor_t s_my_module = {
  .name = "mymodule",
  // ... fields ...
};

module_registry_register(&s_my_module);
```

**Important:** Module descriptors must remain in scope (static/const).

### For Configuration Users

Runtime configuration via INI files is no longer available. Use module registry instead:

**Old approach (removed):**
```c
runtime_config_set_int("metronome.bpm", 120);
int bpm = runtime_config_get_int("metronome.bpm", 120);
```

**New approach (via module registry):**
```c
// CLI: module metronome set bpm 120
// Code: Use module's direct API
metronome_set_bpm(120);
int bpm = metronome_get_bpm();
```

## Verification

To verify the changes after building:

```bash
arm-none-eabi-size --format=berkeley MidiCore.elf
```

Expected RAM usage (bss + data) should be **~50 KB less** than before.

## Benefits

1. **Reduced RAM pressure** - Critical for STM32F407 (128 KB RAM limit)
2. **Faster module registration** - No memcpy, just pointer storage
3. **Simpler codebase** - Removed unused runtime_config system
4. **Better architecture** - Modules own their configuration data

## References

- See `Services/module_registry/module_registry.h` for API documentation
- See existing modules (metronome, arpeggiator) for usage examples
- Module registry still provides full parameter access via CLI/UI
