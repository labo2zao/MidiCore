# Stack Monitor Testing Procedure

## Overview

This document provides step-by-step procedures for testing the FreeRTOS stack monitoring implementation in MidiCore firmware.

## Prerequisites

- MidiCore firmware built with `MODULE_ENABLE_STACK_MONITOR=1`
- STM32F407VGT6 target board
- UART or USB CDC connection for CLI
- Serial terminal (115200 baud, 8N1)
- Optional: ST-Link debugger for advanced testing

## Test Phases

### Phase 1: Basic Functionality Tests

#### Test 1.1: Stack Monitor Initialization

**Objective:** Verify stack monitor initializes and creates monitoring task.

**Procedure:**
1. Flash firmware to target
2. Connect serial terminal
3. Reset board
4. Watch boot messages

**Expected Output:**
```
[INIT] Initializing stack monitor...
[STACK] Initializing stack monitor...
[STACK] Stack monitor initialized (interval=5000ms, warn=20%, crit=5%)
[STACK] Monitor task started
```

**Pass Criteria:**
- Stack monitor init message appears
- No error messages
- Monitor task created successfully

#### Test 1.2: CLI Command Registration

**Objective:** Verify stack CLI commands are registered.

**Procedure:**
```
> list
```

**Expected Output:**
```
[stack] (3 commands)
  stack, stack_all, stack_monitor
```

**Pass Criteria:**
- All 3 stack commands appear in list
- Commands in "stack" category

#### Test 1.3: Query Current Task

**Objective:** Test `stack` command for current task.

**Procedure:**
```
> stack
```

**Expected Output:**
```
CliTask       :  3584/ 5120 bytes ( 70% used,  30% free) [OK]
```

**Pass Criteria:**
- Shows CliTask (since command runs in CLI task)
- Values are reasonable (used < total)
- Status is OK

#### Test 1.4: Query Specific Task

**Objective:** Test `stack` command with task name.

**Procedure:**
```
> stack defaultTask
> stack CliTask
> stack AinTask
```

**Expected Output:**
```
Task: defaultTask
  Stack size:    12288 bytes (3072 words)
  Used:          8192 bytes (67%)
  Free:          4096 bytes (33%)
  High-water:    4096 bytes
  Status:        OK
```

**Pass Criteria:**
- Each task shows valid data
- Used + Free = Stack size
- Status makes sense for usage
- High-water mark ≤ Stack size

#### Test 1.5: Query All Tasks

**Objective:** Test `stack_all` command.

**Procedure:**
```
> stack_all
```

**Expected Output:**
```
=== Stack Usage Report (11 tasks) ===
Task            Used         Total    Used%   Free%  Status
--------------- ------------ -------- ------- ------ ------
defaultTask         8192 B  12288 B      67%     33% OK
CliTask             3584 B   5120 B      70%     30% OK
AinTask              450 B   1024 B      44%     56% OK
...
```

**Pass Criteria:**
- All expected tasks present (at least 9-11)
- All show OK status in normal operation
- Percentages add to 100%
- No tasks show CRITICAL or OVERFLOW

#### Test 1.6: Verbose Output

**Objective:** Test verbose mode.

**Procedure:**
```
> stack_all -v
```

**Expected Output:**
```
defaultTask
  High-water mark: 4096 bytes (1024 words)
CliTask
  High-water mark: 1536 bytes (384 words)
...
```

**Pass Criteria:**
- High-water marks shown for each task
- Values are reasonable

---

### Phase 2: Monitor Control Tests

#### Test 2.1: View Monitor Stats

**Objective:** Test statistics reporting.

**Procedure:**
```
> stack_monitor stats
```

**Expected Output:**
```
=== Stack Monitor Statistics ===
Total checks:    24
Warnings:        0
Critical alerts: 0
Overflows:       0
Last check:      120523 ms
Interval:        5000 ms
Warn threshold:  20%
Crit threshold:  5%
```

**Pass Criteria:**
- Total checks > 0 (increases over time)
- Warnings/Critical/Overflows = 0 in normal operation
- Interval matches configuration

#### Test 2.2: View Configuration

**Objective:** Test config display.

**Procedure:**
```
> stack_monitor config
```

**Expected Output:**
```
Stack Monitor Configuration:
  Status:            Running
  Interval:          5000 ms
  Warning threshold: 20%
  Critical threshold: 5%
```

**Pass Criteria:**
- Shows current configuration
- Status is "Running"
- Values match defaults

#### Test 2.3: Change Interval

**Objective:** Test runtime configuration.

**Procedure:**
```
> stack_monitor config interval 10000
> stack_monitor config
```

**Expected Output:**
```
✓ Monitor interval set to 10000 ms

Stack Monitor Configuration:
  Interval:          10000 ms
  ...
```

**Pass Criteria:**
- Confirmation message
- Config shows new value
- Monitor continues to work

#### Test 2.4: Change Thresholds

**Objective:** Test threshold configuration.

**Procedure:**
```
> stack_monitor config warning 25
> stack_monitor config critical 10
> stack_monitor config
```

**Expected Output:**
```
✓ Warning threshold set to 25%
✓ Critical threshold set to 10%

Stack Monitor Configuration:
  Warning threshold: 25%
  Critical threshold: 10%
```

**Pass Criteria:**
- Thresholds updated successfully
- Values persist during session

#### Test 2.5: Stop/Start Monitor

**Objective:** Test start/stop control.

**Procedure:**
```
> stack_monitor stop
> stack_monitor stats
[Wait 10 seconds]
> stack_monitor stats
> stack_monitor start
[Wait 10 seconds]
> stack_monitor stats
```

**Expected Output:**
```
✓ Stack monitoring stopped

[After wait: total_checks doesn't increase]

✓ Stack monitoring started

[After wait: total_checks increases]
```

**Pass Criteria:**
- Stop prevents new checks
- Start resumes checking
- No errors or crashes

#### Test 2.6: Force Immediate Check

**Objective:** Test on-demand checking.

**Procedure:**
```
> stack_monitor stats    # Note total_checks
> stack_monitor check
> stack_monitor stats    # Check increased
```

**Expected Output:**
```
Total checks:    100
...

Forcing immediate stack check...
✓ Stack check completed

Total checks:    101
...
```

**Pass Criteria:**
- Check executes immediately
- Total checks increments
- Returns quickly (< 1 second)

#### Test 2.7: CSV Export

**Objective:** Test telemetry export.

**Procedure:**
```
> stack_monitor export
```

**Expected Output:**
```
Exporting stack data as CSV...

task_name,used_bytes,total_bytes,used_pct,free_pct,hwm_bytes,status
defaultTask,8192,12288,67,33,4096,0
CliTask,3584,5120,70,30,1536,0
...
```

**Pass Criteria:**
- Valid CSV format
- Header row present
- All tasks exported
- Can be parsed by spreadsheet

---

### Phase 3: Stress Tests

#### Test 3.1: Normal Operation Monitoring

**Objective:** Verify monitor during normal use.

**Procedure:**
1. Let system run for 10 minutes
2. Exercise features:
   - Play notes (AIN → MIDI)
   - Run CLI commands
   - Load patches
   - Trigger looper
3. Periodically check stacks:
   ```
   > stack_all
   ```
4. Check final stats:
   ```
   > stack_monitor stats
   ```

**Expected Results:**
- All tasks remain OK status
- No warnings/critical/overflows
- High-water marks stable (don't grow indefinitely)

**Pass Criteria:**
- Zero warnings after 10 minutes
- All tasks show healthy usage (< 80%)

#### Test 3.2: CLI Stress Test

**Objective:** Stress CliTask stack.

**Procedure:**
```
> help
> module list
> module info arpeggiator
> module params looper
> stack_all -v
> router matrix
> config list
[Repeat 20 times]
> stack CliTask
```

**Expected Results:**
```
Task: CliTask
  Used:          3800 B (74%)
  Free:          1320 B (26%)
  Status:        OK
```

**Pass Criteria:**
- CliTask used% stays < 80%
- No overflow or critical
- High-water mark stabilizes

#### Test 3.3: Deep Call Chain Test

**Objective:** Test defaultTask during deep initialization.

**Procedure:**
1. Modify code to call `app_init_and_start()` again (simulation)
2. Or: Create test function with deep recursion
3. Monitor defaultTask

**Expected Results:**
- defaultTask high-water mark measures deep usage
- Should be < 10KB (within 12KB allocation)

**Note:** This requires code modification - skip for basic testing.

---

### Phase 4: Failure Mode Tests

#### Test 4.1: Warning Threshold Test

**Objective:** Trigger warning alert.

**Procedure:**
1. Reduce warning threshold:
   ```
   > stack_monitor config warning 90
   ```
2. Wait for next check (or force):
   ```
   > stack_monitor check
   ```
3. Check output and stats:
   ```
   > stack_monitor stats
   ```

**Expected Output:**
```
[STACK] WARNING: Task 'CliTask' stack usage high: 74% used (3800/5120 bytes)

Warnings:        1
```

**Pass Criteria:**
- Warning message appears in log
- Statistics show warning count increased
- System continues to run

#### Test 4.2: Critical Threshold Test

**Objective:** Trigger critical alert.

**Procedure:**
1. Reduce critical threshold:
   ```
   > stack_monitor config critical 95
   ```
2. Force check:
   ```
   > stack_monitor check
   ```

**Expected Output:**
```
[STACK] CRITICAL: Task 'CliTask' stack nearly full: 74% used (3800/5120 bytes)!

Critical alerts: 1
```

**Pass Criteria:**
- Critical message appears
- Statistics updated
- System stable

#### Test 4.3: Intentional Overflow Test (DANGEROUS)

**⚠️ WARNING: This test can crash the system. Only perform with debugger attached.**

**Objective:** Verify overflow detection.

**Procedure:**
1. Create test task with very small stack (256 bytes)
2. Task allocates large local array (512 bytes)
3. Monitor should detect overflow

**Expected Result:**
```
[STACK] OVERFLOW: Task 'TestTask' stack corrupted!
```

**Alternative (Safer):**
Reduce an existing task's stack in code, rebuild, and test.

**Skip this test unless specifically needed.**

---

### Phase 5: Integration Tests

#### Test 5.1: Stack Monitor + Module System

**Objective:** Verify stack monitor integrates with module commands.

**Procedure:**
```
> module list
> module info stack_monitor   # (if registered)
> stack_all
```

**Pass Criteria:**
- Commands work together
- No interference between systems

#### Test 5.2: Stack Monitor + Config System

**Objective:** Verify monitor doesn't affect config save/load.

**Procedure:**
```
> config save 0:/test.ini
> stack_monitor check
> config load 0:/test.ini
> stack_all
```

**Pass Criteria:**
- Config operations succeed
- Stack data remains consistent
- No corruption

#### Test 5.3: Long-Duration Stability

**Objective:** Verify no memory leaks or drift over time.

**Procedure:**
1. Run system for 1 hour
2. Export CSV at start:
   ```
   > stack_monitor export > /tmp/stack_start.csv
   ```
3. Exercise system continuously
4. Export CSV at end:
   ```
   > stack_monitor export > /tmp/stack_end.csv
   ```
5. Compare high-water marks

**Expected Result:**
- High-water marks don't grow indefinitely
- Used bytes stable (±10%)
- No overflow

**Pass Criteria:**
- All tasks show < 2% drift in used%
- No new warnings/critical after initial settling

---

## Test Results Template

### Test Session Information
- **Date**: YYYY-MM-DD
- **Firmware Version**: vX.Y.Z
- **Build Configuration**: Debug / Release / Production
- **Hardware**: STM32F407VGT6 / Other
- **Tester**: Name

### Results Summary

| Test ID | Test Name | Result | Notes |
|---------|-----------|--------|-------|
| 1.1 | Stack Monitor Init | PASS/FAIL | |
| 1.2 | CLI Registration | PASS/FAIL | |
| 1.3 | Query Current Task | PASS/FAIL | |
| 1.4 | Query Specific Task | PASS/FAIL | |
| 1.5 | Query All Tasks | PASS/FAIL | |
| 1.6 | Verbose Output | PASS/FAIL | |
| 2.1 | View Stats | PASS/FAIL | |
| 2.2 | View Config | PASS/FAIL | |
| 2.3 | Change Interval | PASS/FAIL | |
| 2.4 | Change Thresholds | PASS/FAIL | |
| 2.5 | Stop/Start | PASS/FAIL | |
| 2.6 | Force Check | PASS/FAIL | |
| 2.7 | CSV Export | PASS/FAIL | |
| 3.1 | Normal Operation | PASS/FAIL | Duration: ___min |
| 3.2 | CLI Stress | PASS/FAIL | Iterations: ___ |
| 3.3 | Deep Call Chain | PASS/FAIL | Peak usage: ___% |
| 4.1 | Warning Alert | PASS/FAIL | |
| 4.2 | Critical Alert | PASS/FAIL | |
| 4.3 | Overflow Test | SKIP/PASS/FAIL | |
| 5.1 | Module Integration | PASS/FAIL | |
| 5.2 | Config Integration | PASS/FAIL | |
| 5.3 | Long-Duration | PASS/FAIL | Duration: ___hr |

### Overall Result
- [ ] All tests passed
- [ ] Minor issues (list below)
- [ ] Major issues (list below)

### Issues Found

1. **Issue**: Description
   - **Severity**: Critical / Major / Minor
   - **Reproduction**: Steps
   - **Workaround**: If any

### Stack Usage Snapshot

Paste output from `stack_all` command:
```
[Paste here]
```

### Recommendations
- Any suggested changes to stack sizes
- Configuration adjustments
- Code improvements

---

## Automated Testing (Future)

### Unit Tests

Potential test cases for automated framework:

```c
// Test stack_monitor_get_info()
void test_get_info(void) {
    stack_info_t info;
    int result = stack_monitor_get_info(NULL, &info);
    assert(result == 0);
    assert(info.used_bytes < info.stack_size_bytes);
    assert(info.used_percent + info.free_percent == 100);
}

// Test alert thresholds
void test_thresholds(void) {
    stack_monitor_set_warning_threshold(50);
    stack_monitor_set_critical_threshold(90);
    
    // Create test task with known high usage
    // Verify alert fires
}

// Test CSV export format
void test_csv_export(void) {
    // Capture CSV output
    // Parse and validate format
    // Check all expected fields present
}
```

### CI/CD Integration

```yaml
# .github/workflows/test.yml
- name: Build with stack monitor
  run: make clean && make STACK_MONITOR=1

- name: Flash firmware
  run: st-flash write build/firmware.bin 0x8000000

- name: Run stack tests
  run: python tests/test_stack_monitor.py

- name: Check for overflows
  run: |
    if grep "OVERFLOW" test_output.log; then
      echo "Stack overflow detected!"
      exit 1
    fi
```

---

## Troubleshooting

### Monitor Not Starting

**Symptom:** No "[STACK] Monitor task started" message

**Checks:**
1. `MODULE_ENABLE_STACK_MONITOR` defined as 1?
2. `stack_monitor_init()` called in app_init.c?
3. Enough heap for task creation (512 bytes stack)?

**Debug:**
```c
// Add trace in stack_monitor.c
dbg_printf("[STACK] About to create task...\r\n");
s_monitor_task_handle = osThreadNew(...);
if (s_monitor_task_handle == NULL) {
    dbg_printf("[STACK] ERROR: Task creation failed\r\n");
}
```

### CLI Commands Not Found

**Symptom:** "Command not found: stack"

**Checks:**
1. `stack_monitor_cli_init()` called?
2. CLI system initialized before stack monitor?
3. Check with `list` command

### Inaccurate Stack Sizes

**Symptom:** Stack size shows wrong value

**Root Cause:** CMSIS-RTOS2 doesn't provide direct access to stack size from TCB.

**Workaround:** Monitor still provides accurate high-water marks, which are most important.

### High-Water Mark Not Updating

**Symptom:** HWM doesn't change over time

**Possible Causes:**
1. Task not executing (check with debugger)
2. FreeRTOS not updating pattern
3. Task stack never used beyond initial allocation

**Check:**
```
> stack_monitor check    # Force update
> stack TaskName
```

---

## Appendix A: Sample Test Script

```bash
#!/bin/bash
# automated_stack_test.sh

SERIAL_PORT="/dev/ttyUSB0"
BAUD="115200"

echo "=== MidiCore Stack Monitor Test ==="
echo ""

# Helper function to send command and capture output
send_cmd() {
    echo "$ $1"
    echo "$1" > "$SERIAL_PORT"
    sleep 1
    # Read output (implementation depends on your setup)
}

# Test sequence
send_cmd "stack_all"
send_cmd "stack_monitor stats"
send_cmd "stack_monitor config"
send_cmd "stack_monitor check"
send_cmd "stack_monitor export"

echo ""
echo "=== Test Complete ==="
```

## Appendix B: Test Data Collection

### Before Fix (Hypothetical Baseline)

```
Task            Used%   Status   Notes
defaultTask       95%   CRITICAL Large locals on stack
CliTask           98%   CRITICAL Nested CLI buffers
```

### After Fix (Expected)

```
Task            Used%   Status   Notes
defaultTask       67%   OK       Config structs moved to static
CliTask           70%   OK       Increased to 5KB
AinTask           44%   OK       Normal
```

---

**Document Version**: 1.0  
**Last Updated**: 2025-01-30  
**Maintainer**: MidiCore Team
