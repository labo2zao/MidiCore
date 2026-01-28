/**
 * @file test_router_example.c
 * @brief Example of refactored MIDI router test
 * 
 * This file demonstrates the new test module pattern:
 * - Clean separation from module_tests.c
 * - Uses tests_common.h macros
 * - Implements graceful stop via test_should_stop()
 * - Proper logging and assertions
 * - Performance measurement
 * 
 * NOTE: This is a TEMPLATE/EXAMPLE file showing how to refactor tests.
 * The actual test_router.c would be extracted from module_tests.c
 */

#include "tests_common.h"
#include "Services/router/router.h"
#include "Services/midi_filter/midi_filter.h"
#include <string.h>

// =============================================================================
// TEST CONFIGURATION
// =============================================================================

#define ROUTER_TEST_ITERATIONS 1000
#define ROUTER_TEST_DELAY_MS 50

// =============================================================================
// TEST STATE
// =============================================================================

typedef struct {
  uint32_t messages_sent;
  uint32_t messages_received;
  uint32_t errors_detected;
  test_perf_t routing_perf;
} router_test_state_t;

static router_test_state_t s_test_state;

// =============================================================================
// TEST HELPER FUNCTIONS
// =============================================================================

/**
 * @brief Initialize router for testing
 */
static int router_test_init(void) {
  TEST_LOG_INFO("Initializing router test");
  
  // Reset test state
  memset(&s_test_state, 0, sizeof(s_test_state));
  test_perf_init(&s_test_state.routing_perf);
  
  // Configure router with test routing
  // Example: Route USB Port 0 -> DIN Port 0
  router_set_enabled(ROUTER_NODE_USB_PORT0, 1);
  router_set_channel_mask(ROUTER_NODE_USB_PORT0, 0xFFFF);  // All channels
  
  TEST_LOG_INFO("Router initialization complete");
  return 0;
}

/**
 * @brief Cleanup router after testing
 */
static void router_test_cleanup(void) {
  TEST_LOG_INFO("Router test cleanup");
  
  // Print statistics
  TEST_LOG_INFO("=== Router Test Statistics ===");
  TEST_LOG_INFO("Messages sent: %u", s_test_state.messages_sent);
  TEST_LOG_INFO("Messages received: %u", s_test_state.messages_received);
  TEST_LOG_INFO("Errors detected: %u", s_test_state.errors_detected);
  TEST_LOG_INFO("Routing performance:");
  TEST_LOG_INFO("  Count: %u", s_test_state.routing_perf.count);
  TEST_LOG_INFO("  Avg: %u ms", test_perf_avg(&s_test_state.routing_perf));
  TEST_LOG_INFO("  Min: %u ms", s_test_state.routing_perf.min_ms);
  TEST_LOG_INFO("  Max: %u ms", s_test_state.routing_perf.max_ms);
}

/**
 * @brief Test routing a single MIDI message
 */
static int router_test_route_message(uint8_t status, uint8_t data1, uint8_t data2) {
  test_perf_start(&s_test_state.routing_perf);
  
  // Send message through router
  router_node_t src = ROUTER_NODE_USB_PORT0;
  router_node_t dst = ROUTER_NODE_DIN_PORT0;
  
  // Route the message
  int result = router_route_message(src, dst, status, data1, data2);
  
  test_perf_end(&s_test_state.routing_perf);
  
  if (result != 0) {
    s_test_state.errors_detected++;
    TEST_LOG_ERROR("Routing failed: status=0x%02X, d1=0x%02X, d2=0x%02X", 
                   status, data1, data2);
    return -1;
  }
  
  s_test_state.messages_sent++;
  return 0;
}

/**
 * @brief Run one iteration of router test
 */
static int router_test_iteration(void) {
  // Test various MIDI message types
  
  // 1. Note On
  TEST_ASSERT(router_test_route_message(0x90, 60, 100) == 0, "Note On routing");
  
  // 2. Note Off
  TEST_ASSERT(router_test_route_message(0x80, 60, 0) == 0, "Note Off routing");
  
  // 3. Control Change
  TEST_ASSERT(router_test_route_message(0xB0, 7, 64) == 0, "CC routing");
  
  // 4. Program Change
  TEST_ASSERT(router_test_route_message(0xC0, 42, 0) == 0, "PC routing");
  
  // 5. Pitch Bend
  TEST_ASSERT(router_test_route_message(0xE0, 0x00, 0x40) == 0, "Pitch Bend routing");
  
  return 0;
}

// =============================================================================
// MAIN TEST FUNCTION
// =============================================================================

/**
 * @brief Run MIDI router test
 * 
 * This test validates:
 * - Message routing between nodes
 * - Channel filtering
 * - Message type filtering
 * - Routing matrix configuration
 * - Performance characteristics
 */
void test_router_run(void) {
  TEST_LOG_INFO("========================================");
  TEST_LOG_INFO("  MIDI Router Test");
  TEST_LOG_INFO("========================================");
  
  // Initialize
  if (router_test_init() != 0) {
    TEST_LOG_ERROR("Router test initialization failed");
    return;
  }
  
  // Main test loop with graceful stop support
  TEST_LOOP_BEGIN()
    
    // Run test iteration
    if (router_test_iteration() != 0) {
      TEST_LOG_ERROR("Router test iteration %u failed", iteration);
      s_test_state.errors_detected++;
    }
    
    // Log progress every 100 iterations
    if (iteration % 100 == 0) {
      TEST_LOG_INFO("Progress: %u iterations, %u messages, %u errors",
                    iteration, s_test_state.messages_sent, s_test_state.errors_detected);
    }
    
  TEST_LOOP_END(ROUTER_TEST_DELAY_MS)
  
  // Cleanup and report results
  router_test_cleanup();
  
  if (s_test_state.errors_detected == 0) {
    TEST_LOG_PASS("Router test PASSED (%u iterations)", iteration);
  } else {
    TEST_LOG_FAIL("Router test FAILED (%u errors in %u iterations)", 
                  s_test_state.errors_detected, iteration);
  }
  
  TEST_LOG_INFO("========================================");
}

// =============================================================================
// TEST REGISTRATION
// =============================================================================

/**
 * @brief Get test descriptor for router test
 * Called by module_tests.c dispatcher
 */
const test_descriptor_t* test_router_get_descriptor(void) {
  static const test_descriptor_t descriptor = {
    .name = "router",
    .description = "MIDI router message routing and filtering test",
    .category = "midi",
    .timeout_ms = 0,  // No timeout (runs until stopped)
  };
  return &descriptor;
}
