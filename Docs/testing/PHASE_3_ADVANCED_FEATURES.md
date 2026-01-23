# Phase 3: Runtime Configuration & Advanced Testing Features

## Overview

Phase 3 adds enterprise-grade testing features including runtime configuration, performance benchmarking, test timeout control, and result logging. These features enable flexible, automated, and production-ready testing.

## New Features

### 1. Runtime Test Configuration

Configure tests dynamically without recompilation using SD card configuration files.

**Configuration File Format (`test_config.txt`):**
```ini
# MidiCore Test Configuration

[execution]
timeout_ms=30000              # Test timeout (0 = no timeout)
enable_benchmarking=1         # Enable performance measurement
enable_logging=1              # Log results to SD card
abort_on_failure=0            # Stop on first failure
verbose_output=1              # Detailed UART output
run_count=1                   # Number of times to run each test

[tests]
enable_oled=1                 # Enable OLED_SSD1322 test
enable_patch_sd=1             # Enable PATCH_SD test
```

**API Usage:**
```c
// Initialize with defaults
test_config_init();

// Load from SD card
test_config_load("0:/test_config.txt");

// Get current configuration
const test_exec_config_t* config = test_config_get_exec();

// Modify configuration
test_exec_config_t new_config = *config;
new_config.timeout_ms = 60000;
test_config_set_exec(&new_config);

// Enable/disable specific tests
test_config_enable_test(MODULE_TEST_OLED_SSD1322_ID, 1);
test_config_enable_test(MODULE_TEST_PATCH_SD_ID, 0);

// Save configuration
test_config_save("0:/test_config.txt");
```

### 2. Performance Benchmarking

Automatic performance measurement with detailed metrics and reporting.

**Features:**
- Millisecond-precision timing
- Start/end timestamps
- Duration calculation
- UART reporting
- CSV export for analysis

**API Usage:**
```c
// Manual benchmarking
test_perf_start(MODULE_TEST_PATCH_SD_ID);
// ... run test ...
test_perf_metrics_t metrics = test_perf_end(MODULE_TEST_PATCH_SD_ID, result);

// Get metrics
const test_perf_metrics_t* m = test_perf_get(MODULE_TEST_PATCH_SD_ID);
dbg_printf("Duration: %lu ms\n", m->duration_ms);

// Print report to UART
test_perf_report(MODULE_TEST_ALL_ID);  // All tests
test_perf_report(MODULE_TEST_PATCH_SD_ID);  // Single test

// Save to SD card (CSV format)
test_perf_save("0:/performance.csv");
```

**Performance Report Example:**
```
==============================================
       PERFORMANCE REPORT
==============================================
OLED_SSD1322         : 12450 ms
PATCH_SD             : 8320 ms
ALL                  : 21100 ms
==============================================
```

**CSV Export Example (`performance.csv`):**
```csv
# MidiCore Test Performance Metrics
# Timestamp: 1234567 ms

Test,Duration_ms,Start_ms,End_ms
OLED_SSD1322,12450,100,12550
PATCH_SD,8320,12550,20870
```

### 3. Test Timeout Control

Watchdog-style timeout protection for long-running or hung tests.

**Features:**
- Configurable timeout per test
- Watchdog reset for incremental progress
- Timeout detection
- Remaining time queries

**API Usage:**
```c
// Initialize timeout (30 seconds)
test_timeout_init(30000);

// In test loop - reset periodically
while (running) {
  // Do work...
  test_timeout_reset();  // Reset watchdog
  
  // Check if timed out
  if (test_timeout_expired()) {
    dbg_print("Test timed out!\n");
    break;
  }
  
  // Check remaining time
  uint32_t remaining = test_timeout_remaining();
  if (remaining < 5000) {
    dbg_print("Less than 5 seconds remaining...\n");
  }
}
```

### 4. Result Logging

Persistent logging of test results to SD card for analysis and debugging.

**Features:**
- Timestamped log entries
- Status tracking (PASS/FAIL/SKIP/TIMEOUT)
- Performance metrics included
- Auto-sync to SD card
- Human-readable format

**API Usage:**
```c
// Initialize logging
test_log_init("0:/test_results.log");

// Log test result
test_result_extended_t result;
// ... run test ...
test_log_result(&result);

// Log custom message
test_log_message("Starting test suite...");

// Close log
test_log_close();
```

**Log File Example (`test_results.log`):**
```
# MidiCore Test Log
# Timestamp: 1234567 ms

[1234567 ms] Starting test suite...

[1234567 ms] Test: OLED_SSD1322
  Status: PASS
  Duration: 12450 ms

[1246017 ms] Test: PATCH_SD
  Status: PASS
  Duration: 8320 ms

[1254337 ms] Test: PATCH_SD
  Status: FAIL (code -1)
  Duration: 850 ms
```

### 5. Enhanced Test Runner

Advanced test execution with full configuration support.

**API:**
```c
// Run with custom configuration
int test_run_configured(
  const test_exec_config_t* config,     // Execution config (NULL = default)
  const test_selection_t* selection,    // Test selection (NULL = all enabled)
  test_result_extended_t* results,      // Result array (NULL = don't save)
  uint32_t max_results                  // Array size
);

// Run single test with timeout
int test_run_single_timed(
  module_test_t test_id,                // Test to run
  uint32_t timeout_ms,                  // Timeout (0 = no timeout)
  test_result_extended_t* result        // Output result
);
```

**Usage Examples:**

**Simple - Use Defaults:**
```c
test_config_init();
test_run_configured(NULL, NULL, NULL, 0);
```

**Custom Configuration:**
```c
test_exec_config_t config = {
  .timeout_ms = 60000,
  .enable_benchmarking = 1,
  .enable_logging = 1,
  .abort_on_failure = 0,
  .verbose_output = 1
};

test_result_extended_t results[10];
int count = test_run_configured(&config, NULL, results, 10);

dbg_printf("Ran %d tests\n", count);
for (int i = 0; i < count; i++) {
  dbg_printf("%s: %s (%lu ms)\n",
             results[i].test_name,
             results[i].result == 0 ? "PASS" : "FAIL",
             results[i].metrics.duration_ms);
}
```

**Selective Testing:**
```c
test_selection_t selection = {0};
selection.test_enabled[MODULE_TEST_OLED_SSD1322_ID] = 1;
selection.test_enabled[MODULE_TEST_PATCH_SD_ID] = 0;
selection.run_count = 3;  // Run 3 times

test_run_configured(NULL, &selection, NULL, 0);
```

**Single Test with Timeout:**
```c
test_result_extended_t result;
test_run_single_timed(MODULE_TEST_PATCH_SD_ID, 30000, &result);

if (result.timed_out) {
  dbg_print("Test timed out!\n");
} else if (result.result == 0) {
  dbg_print("Test passed!\n");
} else {
  dbg_printf("Test failed: %d\n", result.result);
}
```

## Integration with Existing Tests

Phase 3 features integrate seamlessly:

```c
// In MODULE_TEST_ALL or custom test runner
test_config_init();
test_config_load("0:/test_config.txt");

// Enable logging
test_exec_config_t config = *test_config_get_exec();
config.enable_logging = 1;
config.enable_benchmarking = 1;
test_config_set_exec(&config);

// Run tests
test_result_extended_t results[10];
int count = test_run_configured(NULL, NULL, results, 10);

// Save metrics
test_perf_save("0:/performance.csv");
test_log_close();

// Report
test_perf_report(MODULE_TEST_ALL_ID);
```

## CI/CD Integration

### GitHub Actions Example

```yaml
name: Hardware Test with Benchmarking

on: [push, pull_request]

jobs:
  test:
    runs-on: self-hosted  # Runner with STM32 hardware
    steps:
      - uses: actions/checkout@v2
      
      - name: Prepare SD card
        run: |
          echo "[execution]" > test_config.txt
          echo "timeout_ms=60000" >> test_config.txt
          echo "enable_benchmarking=1" >> test_config.txt
          echo "enable_logging=1" >> test_config.txt
          echo "abort_on_failure=1" >> test_config.txt
          cp test_config.txt /mnt/sdcard/
      
      - name: Build test firmware
        run: make CFLAGS+="-DMODULE_TEST_ALL"
      
      - name: Flash to device
        run: make flash
      
      - name: Run tests
        run: |
          timeout 120 cat /dev/ttyUSB0 > test_output.txt &
          sleep 90
      
      - name: Collect results
        run: |
          cp /mnt/sdcard/test_results.log ./
          cp /mnt/sdcard/performance.csv ./
      
      - name: Check results
        run: |
          if grep -q "ALL TESTS PASSED" test_output.txt; then
            echo "✅ All tests passed"
            cat performance.csv
            exit 0
          else
            echo "❌ Tests failed"
            cat test_results.log
            exit 1
          fi
      
      - name: Upload artifacts
        uses: actions/upload-artifact@v2
        with:
          name: test-results
          path: |
            test_output.txt
            test_results.log
            performance.csv
```

### Jenkins Pipeline Example

```groovy
pipeline {
  agent { label 'stm32-hardware' }
  
  stages {
    stage('Prepare') {
      steps {
        sh '''
          cat > test_config.txt << EOF
[execution]
timeout_ms=60000
enable_benchmarking=1
enable_logging=1
abort_on_failure=0
EOF
          cp test_config.txt /mnt/sdcard/
        '''
      }
    }
    
    stage('Build') {
      steps {
        sh 'make clean'
        sh 'make CFLAGS+="-DMODULE_TEST_ALL"'
      }
    }
    
    stage('Flash') {
      steps {
        sh 'make flash'
      }
    }
    
    stage('Test') {
      steps {
        sh '''
          timeout 120 cat /dev/ttyUSB0 > test_output.txt &
          PID=$!
          sleep 90
          kill $PID || true
        '''
      }
    }
    
    stage('Collect Results') {
      steps {
        sh 'cp /mnt/sdcard/test_results.log ./ || true'
        sh 'cp /mnt/sdcard/performance.csv ./ || true'
      }
    }
    
    stage('Analyze') {
      steps {
        script {
          def passed = sh(
            script: 'grep -q "ALL TESTS PASSED" test_output.txt',
            returnStatus: true
          ) == 0
          
          if (passed) {
            echo "✅ All tests passed"
            sh 'cat performance.csv'
          } else {
            error("❌ Tests failed - check test_results.log")
          }
        }
      }
    }
  }
  
  post {
    always {
      archiveArtifacts artifacts: '*.txt,*.log,*.csv', allowEmptyArchive: true
    }
  }
}
```

## Performance Analysis

### Analyzing Benchmark Data

The CSV export can be analyzed with tools like Python/pandas:

```python
import pandas as pd
import matplotlib.pyplot as plt

# Load performance data
df = pd.read_csv('performance.csv', comment='#')

# Calculate statistics
print("Performance Statistics:")
print(df['Duration_ms'].describe())

# Plot duration by test
df.plot(x='Test', y='Duration_ms', kind='bar')
plt.title('Test Execution Time')
plt.ylabel('Duration (ms)')
plt.show()

# Identify slow tests
slow_threshold = df['Duration_ms'].mean() + df['Duration_ms'].std()
slow_tests = df[df['Duration_ms'] > slow_threshold]
print("Slow tests:")
print(slow_tests)
```

## Best Practices

### 1. Configuration Management
- Store test configs in version control
- Use different configs for dev/CI/production
- Document configuration options
- Validate configs before use

### 2. Benchmarking
- Run benchmarks multiple times for consistency
- Track performance over time
- Set performance budgets/thresholds
- Investigate performance regressions

### 3. Timeouts
- Set reasonable timeouts (2-3x expected duration)
- Use watchdog resets in long operations
- Log timeout occurrences
- Investigate timeout root causes

### 4. Logging
- Enable logging for CI/CD runs
- Rotate logs to prevent SD card fill
- Include timestamps for correlation
- Log both successes and failures

### 5. Result Analysis
- Automate result parsing in CI/CD
- Track test reliability metrics
- Generate trend reports
- Alert on consistent failures

## Troubleshooting

### Config Not Loading
- Check filename exactly matches (case-sensitive)
- Verify FAT32 filesystem
- Check file format (key=value pairs)
- Enable verbose output to see errors

### Benchmarking Inaccurate
- Ensure `osKernelGetTickCount()` working
- Check for timer overflow (rare)
- Verify no blocking in measurement code
- Compare with external timing

### Timeouts Not Working
- Verify timeout_ms > 0
- Check `osKernelGetTickCount()` available
- Ensure watchdog reset called periodically
- Check for infinite loops

### Logging Fails
- Verify SD card writable
- Check sufficient free space
- Ensure FATFS initialized
- Close log files properly

## Version History

- **v1.0** (2026-01-23) - Phase 3 initial release
  - Runtime configuration
  - Performance benchmarking
  - Test timeout control
  - Result logging
  - Enhanced test runner

## Related Documentation

- [MODULE_TEST_PATCH_SD.md](MODULE_TEST_PATCH_SD.md) - Phase 1 tests
- [MODULE_TEST_ALL.md](MODULE_TEST_ALL.md) - Phase 2 runner
- [README_MODULE_TESTING.md](README_MODULE_TESTING.md) - Main testing guide
