/**
 * @file test.c
 * @brief Test Module Implementation
 */

#include "Services/test/test.h"
#include "App/tests/module_tests.h"
#include "App/tests/test_debug.h"
#include "Config/module_config.h"

#ifdef MODULE_ENABLE_CLI
#include "Services/module_registry/module_registry.h"
#endif

#include <string.h>
#include <stdio.h>

// =============================================================================
// STATE
// =============================================================================

static uint8_t g_test_initialized = 0;
static test_config_t g_test_config = {
  .enabled = 1,
  .auto_run = 0,
  .timeout_ms = 30000,  // 30 seconds default
  .verbose = 1
};

static test_result_t g_current_result = {0};
static test_result_t g_last_result = {0};

// =============================================================================
// TEST DESCRIPTORS
// =============================================================================

typedef struct {
  const char* name;
  const char* description;
  module_test_t test_id;
} test_descriptor_t;

static const test_descriptor_t g_test_descriptors[] = {
  { "ainser64", "Test AINSER64 analog inputs", MODULE_TEST_AINSER64_ID },
  { "srio", "Test SRIO digital inputs", MODULE_TEST_SRIO_ID },
  { "srio_dout", "Test SRIO digital outputs (LEDs)", MODULE_TEST_SRIO_DOUT_ID },
  { "midi_din", "Test MIDI DIN I/O", MODULE_TEST_MIDI_DIN_ID },
  { "router", "Test MIDI router", MODULE_TEST_ROUTER_ID },
  { "looper", "Test looper recording/playback", MODULE_TEST_LOOPER_ID },
  { "lfo", "Test LFO module", MODULE_TEST_LFO_ID },
  { "humanizer", "Test Humanizer module", MODULE_TEST_HUMANIZER_ID },
  { "ui", "Test UI/OLED general", MODULE_TEST_UI_ID },
  { "ui_song", "Test Song Mode UI page", MODULE_TEST_UI_PAGE_SONG_ID },
  { "ui_midi_monitor", "Test MIDI Monitor UI page", MODULE_TEST_UI_PAGE_MIDI_MONITOR_ID },
  { "ui_sysex", "Test SysEx UI page", MODULE_TEST_UI_PAGE_SYSEX_ID },
  { "ui_config", "Test Config Editor UI page", MODULE_TEST_UI_PAGE_CONFIG_ID },
  { "ui_livefx", "Test LiveFX UI page", MODULE_TEST_UI_PAGE_LIVEFX_ID },
  { "ui_rhythm", "Test Rhythm Trainer UI page", MODULE_TEST_UI_PAGE_RHYTHM_ID },
  { "ui_humanizer", "Test Humanizer/LFO UI page", MODULE_TEST_UI_PAGE_HUMANIZER_ID },
  { "patch_sd", "Test patch loading from SD", MODULE_TEST_PATCH_SD_ID },
  { "pressure", "Test pressure sensor I2C", MODULE_TEST_PRESSURE_ID },
  { "breath", "Test breath controller", MODULE_TEST_BREATH_ID },
  { "usb_host_midi", "Test USB Host MIDI", MODULE_TEST_USB_HOST_MIDI_ID },
  { "usb_device_midi", "Test USB Device MIDI", MODULE_TEST_USB_DEVICE_MIDI_ID },
  { "oled_ssd1322", "Test OLED SSD1322 driver", MODULE_TEST_OLED_SSD1322_ID },
  { "footswitch", "Test footswitch mapping", MODULE_TEST_FOOTSWITCH_ID },
  { "gdb_debug", "Test GDB debug / UART", MODULE_TEST_GDB_DEBUG_ID },
};

#define TEST_DESCRIPTOR_COUNT (sizeof(g_test_descriptors) / sizeof(g_test_descriptors[0]))

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

static const test_descriptor_t* find_test_descriptor(const char* name)
{
  if (!name) return NULL;
  
  for (uint32_t i = 0; i < TEST_DESCRIPTOR_COUNT; i++) {
    if (strcmp(g_test_descriptors[i].name, name) == 0) {
      return &g_test_descriptors[i];
    }
  }
  return NULL;
}

static uint32_t get_tick_ms(void)
{
  // Use FreeRTOS tick count (assuming 1ms tick)
  extern uint32_t osKernelGetTickCount(void);
  return osKernelGetTickCount();
}

// =============================================================================
// API - INITIALIZATION
// =============================================================================

int test_init(void)
{
  if (g_test_initialized) {
    return 0;  // Already initialized
  }
  
  // Initialize test framework
  module_tests_init();
  
  // Clear results
  memset(&g_current_result, 0, sizeof(g_current_result));
  memset(&g_last_result, 0, sizeof(g_last_result));
  
  g_test_initialized = 1;
  
  // Register with module registry if available
#ifdef MODULE_ENABLE_CLI
  test_register_with_registry();
#endif
  
  if (g_test_config.verbose) {
    dbg_print("Test module initialized\r\n");
  }
  
  return 0;
}

uint8_t test_is_initialized(void)
{
  return g_test_initialized;
}

// =============================================================================
// API - TEST EXECUTION
// =============================================================================

int test_run(const char* test_name, int32_t duration_ms)
{
  if (!g_test_initialized) {
    return -1;
  }
  
  if (!g_test_config.enabled) {
    dbg_print("ERROR: Test module is disabled\r\n");
    return -2;
  }
  
  if (g_current_result.status == TEST_STATUS_RUNNING) {
    dbg_print("ERROR: A test is already running\r\n");
    return -3;
  }
  
  // Find test descriptor
  const test_descriptor_t* desc = find_test_descriptor(test_name);
  if (!desc) {
    dbg_print("ERROR: Test not found: ");
    dbg_print(test_name);
    dbg_print("\r\n");
    return -4;
  }
  
  // Initialize result
  memset(&g_current_result, 0, sizeof(g_current_result));
  strncpy(g_current_result.test_name, test_name, TEST_MAX_NAME_LEN - 1);
  g_current_result.status = TEST_STATUS_RUNNING;
  g_current_result.start_time_ms = get_tick_ms();
  
  if (g_test_config.verbose) {
    dbg_print("\r\n========================================\r\n");
    dbg_print("Running test: ");
    dbg_print(test_name);
    dbg_print("\r\n");
    dbg_print("Description: ");
    dbg_print(desc->description);
    dbg_print("\r\n");
    
    // Buffer duration message to avoid fragmentation
    char dur_buf[60];
    if (duration_ms > 0) {
      snprintf(dur_buf, sizeof(dur_buf), "Duration: %lu ms\r\n", (unsigned long)duration_ms);
    } else if (duration_ms == 0) {
      snprintf(dur_buf, sizeof(dur_buf), "Duration: Single iteration\r\n");
    } else {
      snprintf(dur_buf, sizeof(dur_buf), "Duration: Infinite (until stopped)\r\n");
    }
    dbg_print(dur_buf);
    dbg_print("========================================\r\n\r\n");
  }
  
  // Run the test (note: most tests run forever, so this may not return)
  int result = module_tests_run(desc->test_id);
  
  // If we get here, the test completed
  g_current_result.status = (result == 0) ? TEST_STATUS_PASSED : TEST_STATUS_FAILED;
  g_current_result.duration_ms = get_tick_ms() - g_current_result.start_time_ms;
  
  // Save last result
  memcpy(&g_last_result, &g_current_result, sizeof(test_result_t));
  
  if (g_test_config.verbose) {
    // Buffer complete completion message to avoid fragmentation
    char buf[150];
    snprintf(buf, sizeof(buf), 
             "\r\n========================================\r\n"
             "Test completed: %s\r\nStatus: %s\r\nDuration: %lu ms\r\n"
             "========================================\r\n\r\n",
             test_name,
             (result == 0) ? "PASSED" : "FAILED",
             (unsigned long)g_current_result.duration_ms);
    dbg_print(buf);
  }
  
  return result;
}

int test_stop(void)
{
  if (!g_test_initialized) {
    return -1;
  }
  
  if (g_current_result.status != TEST_STATUS_RUNNING) {
    dbg_print("WARNING: No test is currently running\r\n");
    return -2;
  }
  
  // Note: Most tests run in infinite loops, so stopping them
  // is not trivial. This would require additional infrastructure
  // in module_tests.c to support graceful termination.
  
  dbg_print("WARNING: Test stopping not fully implemented\r\n");
  dbg_print("Please reset the device to stop the current test\r\n");
  
  return 0;
}

uint8_t test_is_running(void)
{
  return (g_current_result.status == TEST_STATUS_RUNNING);
}

// =============================================================================
// API - TEST STATUS & RESULTS
// =============================================================================

int test_get_status(test_result_t* result)
{
  if (!result) return -1;
  if (!g_test_initialized) return -2;
  
  memcpy(result, &g_current_result, sizeof(test_result_t));
  return 0;
}

int test_get_last_result(test_result_t* result)
{
  if (!result) return -1;
  if (!g_test_initialized) return -2;
  
  memcpy(result, &g_last_result, sizeof(test_result_t));
  return 0;
}

int test_clear_results(void)
{
  if (!g_test_initialized) return -1;
  
  memset(&g_current_result, 0, sizeof(g_current_result));
  memset(&g_last_result, 0, sizeof(g_last_result));
  
  if (g_test_config.verbose) {
    dbg_print("Test results cleared\r\n");
  }
  
  return 0;
}

// =============================================================================
// API - TEST DISCOVERY
// =============================================================================

uint32_t test_get_count(void)
{
  return TEST_DESCRIPTOR_COUNT;
}

const char* test_get_name(uint32_t index)
{
  if (index >= TEST_DESCRIPTOR_COUNT) {
    return NULL;
  }
  return g_test_descriptors[index].name;
}

const char* test_get_description(const char* test_name)
{
  const test_descriptor_t* desc = find_test_descriptor(test_name);
  return desc ? desc->description : NULL;
}

// =============================================================================
// API - CONFIGURATION
// =============================================================================

int test_set_enabled(uint8_t enabled)
{
  g_test_config.enabled = enabled ? 1 : 0;
  
  if (g_test_config.verbose) {
    dbg_print("Test module ");
    dbg_print(enabled ? "enabled" : "disabled");
    dbg_print("\r\n");
  }
  
  return 0;
}

uint8_t test_get_enabled(void)
{
  return g_test_config.enabled;
}

int test_set_verbose(uint8_t verbose)
{
  g_test_config.verbose = verbose ? 1 : 0;
  return 0;
}

uint8_t test_get_verbose(void)
{
  return g_test_config.verbose;
}

int test_set_timeout(uint32_t timeout_ms)
{
  g_test_config.timeout_ms = timeout_ms;
  return 0;
}

uint32_t test_get_timeout(void)
{
  return g_test_config.timeout_ms;
}

// =============================================================================
// API - MODULE REGISTRY INTEGRATION
// =============================================================================

#ifdef MODULE_ENABLE_CLI

// Parameter getter/setter wrappers for module registry

static int test_param_get_enabled(uint8_t track, param_value_t* out)
{
  (void)track;  // Unused - test module is global
  if (!out) return -1;
  out->bool_val = g_test_config.enabled;
  return 0;
}

static int test_param_set_enabled(uint8_t track, const param_value_t* val)
{
  (void)track;  // Unused
  if (!val) return -1;
  return test_set_enabled(val->bool_val);
}

static int test_param_get_verbose(uint8_t track, param_value_t* out)
{
  (void)track;
  if (!out) return -1;
  out->bool_val = g_test_config.verbose;
  return 0;
}

static int test_param_set_verbose(uint8_t track, const param_value_t* val)
{
  (void)track;
  if (!val) return -1;
  return test_set_verbose(val->bool_val);
}

static int test_param_get_timeout(uint8_t track, param_value_t* out)
{
  (void)track;
  if (!out) return -1;
  out->int_val = (int32_t)g_test_config.timeout_ms;
  return 0;
}

static int test_param_set_timeout(uint8_t track, const param_value_t* val)
{
  (void)track;
  if (!val) return -1;
  return test_set_timeout((uint32_t)val->int_val);
}

int test_register_with_registry(void)
{
  module_descriptor_t desc = {0};
  
  // Module metadata
  strncpy(desc.name, "test", MODULE_REGISTRY_MAX_NAME_LEN - 1);
  strncpy(desc.description, "Module testing service", MODULE_REGISTRY_MAX_DESC_LEN - 1);
  desc.category = MODULE_CATEGORY_SYSTEM;
  
  // Module control functions
  desc.init = test_init;
  desc.enable = NULL;  // Use parameter instead
  desc.disable = NULL;
  desc.get_status = NULL;
  
  // Flags
  desc.is_global = 1;
  desc.has_per_track_state = 0;
  
  // Parameters
  desc.param_count = 3;
  
  // Parameter 0: enabled
  strncpy(desc.params[0].name, "enabled", MODULE_REGISTRY_MAX_NAME_LEN - 1);
  strncpy(desc.params[0].description, "Enable test module", MODULE_REGISTRY_MAX_DESC_LEN - 1);
  desc.params[0].type = PARAM_TYPE_BOOL;
  desc.params[0].get_value = test_param_get_enabled;
  desc.params[0].set_value = test_param_set_enabled;
  
  // Parameter 1: verbose
  strncpy(desc.params[1].name, "verbose", MODULE_REGISTRY_MAX_NAME_LEN - 1);
  strncpy(desc.params[1].description, "Verbose output", MODULE_REGISTRY_MAX_DESC_LEN - 1);
  desc.params[1].type = PARAM_TYPE_BOOL;
  desc.params[1].get_value = test_param_get_verbose;
  desc.params[1].set_value = test_param_set_verbose;
  
  // Parameter 2: timeout
  strncpy(desc.params[2].name, "timeout_ms", MODULE_REGISTRY_MAX_NAME_LEN - 1);
  strncpy(desc.params[2].description, "Test timeout (ms)", MODULE_REGISTRY_MAX_DESC_LEN - 1);
  desc.params[2].type = PARAM_TYPE_INT;
  desc.params[2].min = 1000;
  desc.params[2].max = 300000;  // 5 minutes max
  desc.params[2].get_value = test_param_get_timeout;
  desc.params[2].set_value = test_param_set_timeout;
  
  return module_registry_register(&desc);
}

#else

int test_register_with_registry(void)
{
  // Module registry not enabled
  return 0;
}

#endif  // MODULE_ENABLE_CLI
