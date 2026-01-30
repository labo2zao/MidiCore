# Production Build Configuration

This document explains how to configure MidiCore for production builds, focusing on excluding test infrastructure to save flash and RAM.

## Overview

MidiCore includes extensive test infrastructure for development and validation. However, this test code should **NOT** be included in production builds to:
- Save flash memory (~50-100KB)
- Save RAM (~10-20KB)
- Reduce attack surface
- Improve boot time
- Prevent accidental test execution

## Configuration

All test-related code is controlled by the `MODULE_ENABLE_TEST` flag in `Config/module_config.h`.

### Production Build (Default)

```c
// Config/module_config.h
#ifndef MODULE_ENABLE_TEST
#define MODULE_ENABLE_TEST 0  // Disabled in production
#endif
```

With `MODULE_ENABLE_TEST=0`:
- ✅ All test modules excluded from compilation
- ✅ Test CLI commands unavailable
- ✅ Test infrastructure code removed
- ✅ Stub functions provided for compatibility
- ✅ No runtime overhead

### Development Build

```c
// Config/module_config.h
#ifndef MODULE_ENABLE_TEST
#define MODULE_ENABLE_TEST 1  // Enabled for development
#endif
```

With `MODULE_ENABLE_TEST=1`:
- ✅ Full test infrastructure available
- ✅ Test CLI commands enabled
- ✅ Runtime test execution
- ✅ Module validation

## Affected Modules

### Services Layer

**Services/test/**
- `test.h` / `test.c` - Test module service
- `test_cli.h` / `test_cli.c` - Test CLI commands

When `MODULE_ENABLE_TEST=0`:
- All functions become inline stubs
- No code compiled
- Zero flash/RAM overhead

### App Layer

**App/tests/**
- `module_tests.h` / `module_tests.c` - Test framework
- `tests_common.h` - Test utilities and macros
- `test_router_example.c` - Example refactored test
- `test_*.c` - Individual test modules

When `MODULE_ENABLE_TEST=0`:
- Test files excluded from build
- No test execution possible
- Test descriptors not compiled

## Code Patterns

### Conditional Compilation in Headers

```c
#pragma once

#include "Config/module_config.h"

#if MODULE_ENABLE_TEST

// Full API declarations
int test_init(void);
int test_run(const char* name, int32_t duration_ms);
// ... more functions ...

#else  // !MODULE_ENABLE_TEST

// Stub implementations
static inline int test_init(void) { return 0; }
static inline int test_run(const char* name, int32_t duration_ms) { 
  (void)name; (void)duration_ms; 
  return -1; 
}
// ... more stubs ...

#endif  // MODULE_ENABLE_TEST
```

### Conditional Compilation in Source Files

```c
#include "Config/module_config.h"

#if MODULE_ENABLE_TEST

#include "Services/test/test.h"
#include "App/tests/module_tests.h"
// ... other includes ...

// Full implementation
int test_init(void) {
  // ... actual implementation ...
}

// ... more functions ...

#endif  // MODULE_ENABLE_TEST
```

### Application Integration

```c
// App/app_init.c
#include "Services/test/test.h"
#include "Services/test/test_cli.h"

void app_init(void) {
  // ... other initialization ...
  
  // Test module automatically excluded in production
  test_init();           // Becomes no-op stub when MODULE_ENABLE_TEST=0
  test_cli_init();       // Becomes no-op stub when MODULE_ENABLE_TEST=0
  
  // ... rest of initialization ...
}
```

**No `#ifdef` guards needed in application code!** The stub functions handle it automatically.

## Build System Integration

### Makefile Configuration

```makefile
# Conditional test source inclusion
ifeq ($(MODULE_ENABLE_TEST), 1)
  # Include test sources
  SOURCES += Services/test/test.c
  SOURCES += Services/test/test_cli.c
  SOURCES += App/tests/module_tests.c
  SOURCES += App/tests/test_router_example.c
  # ... more test files ...
else
  # Exclude all test sources
  # (headers still provide stub functions)
endif
```

### STM32CubeIDE Configuration

1. Right-click on test files
2. Select **Resource Configurations** → **Exclude from Build**
3. Check boxes for **Release** configuration
4. Leave unchecked for **Debug** configuration

Or use preprocessor exclusion:
- Project Properties → C/C++ Build → Settings → Preprocessor
- Add `MODULE_ENABLE_TEST=0` to Release configuration
- Add `MODULE_ENABLE_TEST=1` to Debug configuration

## Memory Savings

Typical savings with `MODULE_ENABLE_TEST=0`:

| Component | Flash Saved | RAM Saved |
|-----------|-------------|-----------|
| module_tests.c (8182 lines) | ~45KB | ~5KB |
| test.c service | ~3KB | ~2KB |
| test_cli.c | ~2KB | ~1KB |
| tests_common.h macros | ~5KB | - |
| Test descriptors & data | ~8KB | ~3KB |
| **Total** | **~63KB** | **~11KB** |

*Note: Actual savings depend on compiler optimization level and specific test implementations.*

## CLI Commands in Production

When `MODULE_ENABLE_TEST=0`, test commands are excluded but other CLI commands remain available:

**Available:**
- ✅ `module list` / `module info`
- ✅ `module enable` / `module disable`
- ✅ `module get` / `module set`
- ✅ `config load` / `config save`
- ✅ `router` commands
- ✅ `looper` commands
- ✅ All production module CLIs

**Excluded:**
- ❌ `test list`
- ❌ `test run`
- ❌ `test stop`
- ❌ `test status`

Test commands gracefully return "command not found" when excluded.

## Verification

### Compile-Time Verification

```bash
# Check if MODULE_ENABLE_TEST is defined
grep "MODULE_ENABLE_TEST" Config/module_config.h

# Verify test files are excluded from build
arm-none-eabi-nm firmware.elf | grep test_
# Should return nothing if properly excluded
```

### Runtime Verification

```bash
# Connect to MIOS Terminal
# Try test command
test list

# Expected production response:
# ERROR: Command not found: test
```

### Build Size Comparison

```bash
# Build with tests enabled
make clean
MODULE_ENABLE_TEST=1 make
ls -lh firmware.elf  # Note size

# Build with tests disabled
make clean
MODULE_ENABLE_TEST=0 make
ls -lh firmware.elf  # Should be ~63KB smaller
```

## Best Practices

### 1. Always Use Conditional Compilation

```c
// ✅ GOOD: Test code properly guarded
#if MODULE_ENABLE_TEST
void internal_test_function(void) {
  // Test implementation
}
#endif

// ❌ BAD: Test code always compiled
void internal_test_function(void) {
  #if MODULE_ENABLE_TEST
  // Test implementation
  #endif
}
```

### 2. Use Inline Stubs in Headers

```c
// ✅ GOOD: Inline stub has zero overhead
#else  // !MODULE_ENABLE_TEST
static inline int test_init(void) { return 0; }
#endif

// ❌ BAD: Function call overhead even when disabled
int test_init(void);  // Always declared, conditionally implemented
```

### 3. Don't Use Test Functions in Production Code

```c
// ❌ BAD: Production code calling test function
void production_function(void) {
  test_validate_state();  // Only exists in test builds!
}

// ✅ GOOD: Production code is independent
void production_function(void) {
  // Production logic only
}
```

### 4. Keep Test Files Separate

```
✅ GOOD:
App/tests/test_router.c       # Test code
Services/router/router.c      # Production code

❌ BAD:
Services/router/router.c      # Mixed test and production code
```

## Troubleshooting

### Build Error: Undefined Reference to test_*

**Cause:** Test function called in production code without guards.

**Solution:** Either:
1. Remove test call from production code
2. Add `#if MODULE_ENABLE_TEST` guards around the call
3. Use stub function that works in both modes

### Link Error: Multiple Definition of test_*

**Cause:** Test source file included in production build.

**Solution:**
1. Add `#if MODULE_ENABLE_TEST` guard at top of .c file
2. Exclude file from build in Release configuration
3. Check Makefile conditional inclusion

### Runtime: Test Command Not Found

**Expected behavior in production.** Test commands are intentionally excluded.

If you need test commands:
1. Set `MODULE_ENABLE_TEST=1` in `module_config.h`
2. Rebuild firmware
3. Flash new firmware to device

## Summary

- **Production:** `MODULE_ENABLE_TEST=0` (default)
  - Saves ~63KB flash, ~11KB RAM
  - Zero test overhead
  - Stub functions provide compatibility
  
- **Development:** `MODULE_ENABLE_TEST=1`
  - Full test infrastructure
  - Runtime test execution
  - CLI test commands

- **Integration:** No `#ifdef` guards needed in app code
  - Stub functions handle it automatically
  - Clean, maintainable application code

The test infrastructure is designed to be completely transparent to production builds while providing comprehensive testing capabilities during development.
