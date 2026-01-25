# Test Code Separation - Production Mode Cleanup

## Overview

This document tracks the systematic separation of test/debug code from production code to ensure clean production builds without unnecessary test overhead.

**Goal**: Production builds should NOT compile or link any test-specific code, saving Flash/RAM and improving code clarity.

## Separation Strategy

### 1. Conditional Compilation Pattern
```c
// In headers (.h files):
#ifdef MODULE_TEST_ENABLED  // Or specific test like MODULE_TEST_OLED
void test_function(void);
#endif

// In implementation (.c files):
#ifdef MODULE_TEST_ENABLED
void test_function(void) {
    // Test implementation
}
#endif
```

### 2. File Organization
- **Production code**: Regular files (e.g., `ain.c`, `midi_din.c`)
- **Test code**: Separate test files (e.g., `ain_test.c`, `midi_din_test.c` in `/App/tests/`)
- **Debug tasks**: Conditionally compiled (e.g., `ain_raw_debug_task.c`, `midi_din_debug_task.c`)

## Module-by-Module Status

### ✅ Core App Files

| File | Type | Status | Guard | Notes |
|------|------|--------|-------|-------|
| `ain_raw_debug_task.c/h` | Debug Task | ⚠️ TODO | Needs `#ifdef MODULE_TEST_AIN_RAW` | AIN raw value debug output |
| `midi_din_debug_task.c/h` | Debug Task | ⚠️ TODO | Needs `#ifdef MODULE_TEST_MIDI_DIN_DEBUG` | MIDI DIN traffic debug |
| `din_selftest.c/h` | Test | ⚠️ TODO | Needs `#ifdef MODULE_TEST_DIN_SELFTEST` | DIN self-test (deprecated?) |

### ✅ HAL Layer

| File | Function | Status | Guard | Notes |
|------|----------|--------|-------|-------|
| `oled_ssd1322.h` | `oled_init()` | ✅ DONE | Already marked "FOR TESTING ONLY" | Simple MIOS32 test init |
| `oled_ssd1322.h` | `oled_init_progressive()` | ✅ DONE | Already marked "FOR TESTING ONLY" | Debug step-by-step init |
| `oled_ssd1322.h` | `oled_test_*()` functions | ⚠️ TODO | Need `#ifdef MODULE_TEST_OLED` | 9 test pattern functions |
| `oled_ssd1322.c` | Test function implementations | ⚠️ TODO | Need `#ifdef MODULE_TEST_OLED` | Implementations of above |

### ✅ Services Layer

| File | Function | Status | Guard | Notes |
|------|----------|--------|-------|-------|
| `looper.h` | Test/debug functions | ⚠️ TODO | Scan for test functions | Check looper |
| `ain.h` | `ain_debug_*()` functions | ⚠️ TODO | Need `#ifdef MODULE_TEST_AIN` | Debug output functions |
| `usb_midi.h` | `usb_midi_debug_*()` | ⚠️ TODO | Scan file | Check for debug functions |
| `midi_filter.h` | Example functions | ⚠️ TODO | Need `#ifdef MODULE_EXAMPLES` | Example code |
| `channelizer.h` | Example functions | ⚠️ TODO | Need `#ifdef MODULE_EXAMPLES` | Example code |
| `quantizer_example.c` | Examples | ✅ OK | Separate file | Already isolated |
| `swing_example.c` | Examples | ✅ OK | Separate file | Already isolated |
| `strum_example.c` | Examples | ✅ OK | Separate file | Already isolated |
| `test_strum.c` | Tests | ✅ OK | Separate file | Already isolated |
| `velocity_compressor_test.c` | Tests | ✅ OK | Separate file | Already isolated |

### ✅ UI Layer

| File | Function | Status | Guard | Notes |
|------|----------|--------|-------|-------|
| `ui_page_oled_test.c/h` | OLED test page | ⚠️ TODO | Need `#ifdef MODULE_TEST_OLED` | Entire UI page for testing |
| `ui.c` | Test page integration | ⚠️ TODO | Need conditional in page array | Don't register test pages |

### ✅ Test Directory `/App/tests/`

All files in `/App/tests/` are test-only and should NOT be compiled in production:

| File | Purpose | Status | Notes |
|------|---------|--------|-------|
| `module_tests.c/h` | Test framework | ✅ OK | Only compiled when `MODULE_TEST_*` defined |
| `app_test_*.c/h` | Specific tests | ✅ OK | Only compiled for specific tests |
| `test_*.c/h` | Various tests | ✅ OK | Only compiled when needed |

**Production rule**: The entire `/App/tests/` directory should be excluded from production builds.

## Implementation Phases

### Phase 1: Core Separation (Priority 1) ⚠️ IN PROGRESS
- [ ] Guard `ain_raw_debug_task.c/h` with `#ifdef MODULE_TEST_AIN_RAW`
- [ ] Guard `midi_din_debug_task.c/h` with `#ifdef MODULE_TEST_MIDI_DIN_DEBUG`
- [ ] Guard `din_selftest.c/h` with `#ifdef MODULE_TEST_DIN_SELFTEST`
- [ ] Conditionally exclude from `app_init.c`

### Phase 2: OLED Test Functions (Priority 2)
- [ ] Guard `oled_test_*()` declarations in `oled_ssd1322.h`
- [ ] Guard `oled_test_*()` implementations in `oled_ssd1322.c`
- [ ] Guard `oled_init()` and `oled_init_progressive()` (keep declarations visible but mark deprecated)
- [ ] Guard `ui_page_oled_test.c/h` entirely
- [ ] Remove test page registration from `ui.c` when `MODULE_TEST_OLED` not defined

### Phase 3: Service Layer Debug Functions (Priority 3)
- [ ] Scan and guard debug functions in `ain.h/ain.c`
- [ ] Scan and guard debug functions in `looper.h/looper.c`
- [ ] Scan and guard debug functions in `usb_midi.h/usb_midi.c`
- [ ] Scan other services for debug/test functions

### Phase 4: Example Code (Priority 4)
- [ ] Example files are OK (separate .c files)
- [ ] Ensure examples not linked in production builds via Makefile/project settings

### Phase 5: Documentation & Verification (Priority 5)
- [ ] Update `module_config.h` with clear test enable/disable section
- [ ] Document which `MODULE_TEST_*` flags exist and what they enable
- [ ] Verify production build has NO test code in .map file
- [ ] Calculate Flash/RAM savings from test exclusion

## Expected Benefits

### Flash Savings (Estimated)
- OLED test patterns: ~2-3 KB
- Debug tasks: ~1-2 KB each
- Test page UI: ~1-2 KB
- **Total estimate**: 5-10 KB Flash savings

### RAM Savings (Estimated)
- Debug task stacks (if tasks removed): ~2-4 KB
- Test page state: ~0.5 KB
- **Total estimate**: 2-5 KB RAM savings

### Code Quality Benefits
- Cleaner production code
- Faster builds (fewer files)
- Easier to audit production functionality
- Clear separation of concerns

## Build Verification Checklist

After implementation, verify:

- [ ] Production build (all `MODULE_TEST_*` = 0) compiles successfully
- [ ] Production `.map` shows NO symbols from test files
- [ ] Test build (specific `MODULE_TEST_*` = 1) still works
- [ ] No increase in production binary size
- [ ] RAM/Flash usage matches expectations

## Notes

- **Deprecated code**: `din_selftest` may be deprecated - verify with user before guarding
- **OLED init functions**: Keep `oled_init()` and `oled_init_progressive()` declarations visible but clearly marked as test-only - some tests may use them
- **UI test pages**: These should be completely excluded from production - they're only for development/debugging
- **Example code**: Keep in separate files, don't guard - useful for documentation even if not compiled in production

## Status Summary

**Overall Progress**: 10% (Documentation phase)

**Next Action**: Phase 1 implementation - guard debug tasks

**Target**: 100% separation for production release
