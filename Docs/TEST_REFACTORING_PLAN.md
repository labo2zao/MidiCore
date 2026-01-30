# Test Module Architecture Refactoring Plan

## Current Issues

1. **module_tests.c is 8182 lines** - Too large, hard to maintain
2. **Tests run in infinite loops** - No graceful stop mechanism
3. **Hard-coded test discovery** - Static enum + switch statement + descriptor array
4. **No consistent lifecycle** - Start/stop/pause/resume not properly implemented
5. **Poor test isolation** - All tests in one monolithic file

## Refactoring Goals

### 1. Split Monolithic module_tests.c

**Current Structure:**
```
App/tests/module_tests.c (8182 lines)
├── 25 test enums
├── Name mapping table
├── Giant switch statement (routing)
└── All test implementations inline
```

**Target Structure:**
```
App/tests/
├── module_tests.c (dispatcher only, ~300 lines)
├── test_*.c (individual test files)
│   ├── test_ainser64.c
│   ├── test_din_midi.c
│   ├── test_usb_midi.c
│   ├── test_looper.c
│   ├── test_router.c
│   ├── test_bellows.c
│   └── ... (24 tests)
└── tests_common.h (shared utilities)
```

### 2. Improve Test Lifecycle

**Add to Services/test/test.h:**
```c
// Lifecycle control
int test_start(const char* name);
int test_stop(void);  // Graceful stop
int test_pause(void);
int test_resume(void);
int test_is_stopped(void);  // Check stop flag

// Result tracking
typedef enum {
  TEST_RESULT_RUNNING,
  TEST_RESULT_PASS,
  TEST_RESULT_FAIL,
  TEST_RESULT_TIMEOUT,
  TEST_RESULT_STOPPED
} test_result_status_t;

typedef struct {
  test_result_status_t status;
  uint32_t start_time_ms;
  uint32_t end_time_ms;
  uint32_t duration_ms;
  uint32_t assertions_total;
  uint32_t assertions_passed;
  uint32_t assertions_failed;
  char error_message[128];
} test_result_detail_t;

int test_get_result_detail(test_result_detail_t* out);
```

### 3. Dynamic Test Registration

**Replace static enums with registry:**
```c
// In each test file
typedef struct {
  const char* name;
  const char* description;
  const char* category;  // "hardware", "midi", "system", "effect"
  void (*run)(void);     // Test entry point
  uint32_t timeout_ms;   // 0 = no timeout
} test_descriptor_t;

#define TEST_REGISTER(test_name, desc, cat, timeout) \
  static test_descriptor_t test_##test_name##_descriptor = { \
    .name = #test_name, \
    .description = desc, \
    .category = cat, \
    .run = test_##test_name##_run, \
    .timeout_ms = timeout \
  }; \
  __attribute__((constructor)) \
  static void register_##test_name(void) { \
    test_registry_add(&test_##test_name##_descriptor); \
  }
```

### 4. Test Stop Flag Pattern

**Each test checks stop flag periodically:**
```c
volatile uint8_t g_test_stop_requested = 0;

void test_router_run(void) {
  LOG_INFO("TEST", "Starting router test");
  
  while (!g_test_stop_requested) {
    // Test iteration
    router_test_iteration();
    HAL_Delay(100);
  }
  
  LOG_INFO("TEST", "Router test stopped gracefully");
}
```

### 5. Enhanced CLI Commands

**Update test_cli.c:**
```
test list [category]       - List all or filtered tests
test run <name> [timeout]  - Run test with optional timeout
test stop                  - Stop running test gracefully
test pause                 - Pause running test
test resume                - Resume paused test
test status                - Detailed status with progress
test results               - Show test result history
test clear                 - Clear result history
```

## Implementation Steps

### Phase 1: Core Infrastructure (High Priority)
1. ✅ Add stop flag mechanism to test.c/test.h
2. ✅ Add test_result_detail_t structure
3. ✅ Add pause/resume/stop functions
4. ✅ Create tests_common.h with shared utilities

### Phase 2: Split module_tests.c (High Priority)
1. Create test_*.c files for each test (keep 5-10 most important)
2. Extract test code from module_tests.c
3. Update module_tests.c to dispatch to new files
4. Keep backward compatibility during transition

### Phase 3: Enhanced CLI (Medium Priority)
1. Update test_cli.c with new commands
2. Add test filtering by category
3. Add timeout enforcement
4. Add result aggregation

### Phase 4: Test Registry (Low Priority)
1. Create dynamic test registry
2. Convert tests to use TEST_REGISTER macro
3. Remove static enum + switch statement
4. Auto-discovery of tests

## File Checklist

**New Files to Create:**
- [ ] App/tests/tests_common.h
- [ ] App/tests/test_router.c
- [ ] App/tests/test_looper.c  
- [ ] App/tests/test_ainser64.c
- [ ] App/tests/test_din_midi.c
- [ ] App/tests/test_bellows.c

**Files to Modify:**
- [ ] Services/test/test.h (add lifecycle functions)
- [ ] Services/test/test.c (implement stop flag, pause/resume)
- [ ] Services/test/test_cli.c (enhance commands)
- [ ] App/tests/module_tests.c (refactor to dispatcher)

## Success Criteria

1. ✅ Test stop works gracefully (no forced termination)
2. ✅ Individual test files < 500 lines each
3. ✅ module_tests.c < 500 lines (dispatcher only)
4. ✅ New tests easy to add (register + implement)
5. ✅ Test results properly tracked
6. ✅ CLI commands intuitive and complete

## Priority for This Session

Given time constraints, focus on:
1. **Infrastructure**: Add stop flag, result tracking to test.c/test.h
2. **Documentation**: Create this plan + example test file
3. **Proof of Concept**: Extract 2-3 tests to separate files
4. **Enhanced CLI**: Update stop/status commands

Full migration can be completed in follow-up sessions.
