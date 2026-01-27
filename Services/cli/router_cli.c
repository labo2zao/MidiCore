/**
 * @file router_cli.c
 * @brief CLI commands for MIDI Router control
 * 
 * Provides UART terminal commands to control MIDI routing matrix:
 * - Enable/disable routes between nodes
 * - Configure channel masks
 * - Set route labels
 * - Display routing configuration
 * - Test routing paths
 */

#include "router_cli.h"
#include "cli.h"
#include "Services/router/router.h"
#include "Config/router_config.h"
#include <string.h>
#include <stdlib.h>

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * @brief Parse node number from string (0-15)
 */
static int parse_node(const char* str) {
  int node = atoi(str);
  if (node < 0 || node >= ROUTER_NUM_NODES) {
    cli_error("Invalid node number: %s (must be 0-%d)\n", str, ROUTER_NUM_NODES - 1);
    return -1;
  }
  return node;
}

/**
 * @brief Parse channel mask from string
 * Supports: "all", "1", "1-16", "1,2,3", "1-8,10"
 */
static uint16_t parse_channel_mask(const char* str) {
  if (strcasecmp(str, "all") == 0) {
    return 0xFFFF;  // All 16 channels
  }
  
  // Simple single channel (1-16)
  if (strlen(str) <= 2 && str[0] >= '1' && str[0] <= '9') {
    int ch = atoi(str);
    if (ch >= 1 && ch <= 16) {
      return (uint16_t)(1 << (ch - 1));
    }
  }
  
  // For now, just support simple cases
  // TODO: Add support for ranges like "1-8" and lists like "1,2,3"
  cli_warning("Advanced channel mask parsing not yet implemented\n");
  cli_warning("Using 'all' channels (0xFFFF)\n");
  return 0xFFFF;
}

/**
 * @brief Print routing matrix
 */
static void print_routing_matrix(void) {
  cli_printf("\nMIDI Routing Matrix (%dx%d nodes):\n", ROUTER_NUM_NODES, ROUTER_NUM_NODES);
  cli_printf("=================================================\n");
  
  // Header row
  cli_printf("IN\\OUT ");
  for (int out = 0; out < ROUTER_NUM_NODES; out++) {
    cli_printf(" %2d", out);
  }
  cli_printf("\n");
  cli_printf("-------");
  for (int out = 0; out < ROUTER_NUM_NODES; out++) {
    cli_printf("---");
  }
  cli_printf("\n");
  
  // Data rows
  for (int in = 0; in < ROUTER_NUM_NODES; in++) {
    cli_printf("  %2d   ", in);
    for (int out = 0; out < ROUTER_NUM_NODES; out++) {
      uint8_t enabled = router_get_route((uint8_t)in, (uint8_t)out);
      cli_printf("  %c", enabled ? 'X' : '.');
    }
    cli_printf("\n");
  }
  cli_printf("\n");
}

/**
 * @brief Print detailed route information
 */
static void print_route_info(uint8_t in_node, uint8_t out_node) {
  uint8_t enabled = router_get_route(in_node, out_node);
  uint16_t chmask = router_get_chanmask(in_node, out_node);
  const char* label = router_get_label(in_node, out_node);
  
  cli_printf("Route: %d -> %d\n", in_node, out_node);
  cli_printf("  Status: %s\n", enabled ? "ENABLED" : "DISABLED");
  cli_printf("  Channel Mask: 0x%04X (", chmask);
  
  // Print enabled channels
  int first = 1;
  for (int ch = 0; ch < 16; ch++) {
    if (chmask & (1 << ch)) {
      if (!first) cli_printf(",");
      cli_printf("%d", ch + 1);
      first = 0;
    }
  }
  if (first) cli_printf("none");
  cli_printf(")\n");
  
  if (label && label[0]) {
    cli_printf("  Label: %s\n", label);
  }
}

// =============================================================================
// COMMAND HANDLERS
// =============================================================================

/**
 * @brief Router command handler
 * Usage: router <subcommand> [args...]
 */
static cli_result_t cmd_router(int argc, char* argv[]) {
  if (argc < 2) {
    cli_error("Missing subcommand\n");
    cli_printf("Usage: router <matrix|enable|disable|channel|label|info|test>\n");
    cli_printf("  router matrix              - Display routing matrix\n");
    cli_printf("  router enable IN OUT       - Enable route from IN to OUT\n");
    cli_printf("  router disable IN OUT      - Disable route from IN to OUT\n");
    cli_printf("  router channel IN OUT MASK - Set channel mask (e.g., 'all', '1', '1-16')\n");
    cli_printf("  router label IN OUT TEXT   - Set route label\n");
    cli_printf("  router info IN OUT         - Show detailed route info\n");
    cli_printf("  router test IN             - Test routing from input node\n");
    return CLI_INVALID_ARGS;
  }
  
  const char* subcmd = argv[1];
  
  // router matrix
  if (strcasecmp(subcmd, "matrix") == 0) {
    print_routing_matrix();
    return CLI_OK;
  }
  
  // router enable IN OUT
  else if (strcasecmp(subcmd, "enable") == 0) {
    if (argc < 4) {
      cli_error("Missing arguments\n");
      cli_printf("Usage: router enable IN OUT\n");
      return CLI_INVALID_ARGS;
    }
    
    int in_node = parse_node(argv[2]);
    int out_node = parse_node(argv[3]);
    if (in_node < 0 || out_node < 0) return CLI_INVALID_ARGS;
    
    router_set_route((uint8_t)in_node, (uint8_t)out_node, 1);
    cli_success("Enabled route: %d -> %d\n", in_node, out_node);
    return CLI_OK;
  }
  
  // router disable IN OUT
  else if (strcasecmp(subcmd, "disable") == 0) {
    if (argc < 4) {
      cli_error("Missing arguments\n");
      cli_printf("Usage: router disable IN OUT\n");
      return CLI_INVALID_ARGS;
    }
    
    int in_node = parse_node(argv[2]);
    int out_node = parse_node(argv[3]);
    if (in_node < 0 || out_node < 0) return CLI_INVALID_ARGS;
    
    router_set_route((uint8_t)in_node, (uint8_t)out_node, 0);
    cli_success("Disabled route: %d -> %d\n", in_node, out_node);
    return CLI_OK;
  }
  
  // router channel IN OUT MASK
  else if (strcasecmp(subcmd, "channel") == 0) {
    if (argc < 5) {
      cli_error("Missing arguments\n");
      cli_printf("Usage: router channel IN OUT MASK\n");
      cli_printf("  MASK: 'all', '1', '1-16', etc.\n");
      return CLI_INVALID_ARGS;
    }
    
    int in_node = parse_node(argv[2]);
    int out_node = parse_node(argv[3]);
    if (in_node < 0 || out_node < 0) return CLI_INVALID_ARGS;
    
    uint16_t chmask = parse_channel_mask(argv[4]);
    router_set_chanmask((uint8_t)in_node, (uint8_t)out_node, chmask);
    cli_success("Set channel mask for route %d -> %d: 0x%04X\n", in_node, out_node, chmask);
    return CLI_OK;
  }
  
  // router label IN OUT TEXT
  else if (strcasecmp(subcmd, "label") == 0) {
    if (argc < 5) {
      cli_error("Missing arguments\n");
      cli_printf("Usage: router label IN OUT TEXT\n");
      return CLI_INVALID_ARGS;
    }
    
    int in_node = parse_node(argv[2]);
    int out_node = parse_node(argv[3]);
    if (in_node < 0 || out_node < 0) return CLI_INVALID_ARGS;
    
    // Join remaining args as label
    char label[64] = {0};
    for (int i = 4; i < argc && strlen(label) < sizeof(label) - 2; i++) {
      if (i > 4) strcat(label, " ");
      strncat(label, argv[i], sizeof(label) - strlen(label) - 1);
    }
    
    router_set_label((uint8_t)in_node, (uint8_t)out_node, label);
    cli_success("Set label for route %d -> %d: '%s'\n", in_node, out_node, label);
    return CLI_OK;
  }
  
  // router info IN OUT
  else if (strcasecmp(subcmd, "info") == 0) {
    if (argc < 4) {
      cli_error("Missing arguments\n");
      cli_printf("Usage: router info IN OUT\n");
      return CLI_INVALID_ARGS;
    }
    
    int in_node = parse_node(argv[2]);
    int out_node = parse_node(argv[3]);
    if (in_node < 0 || out_node < 0) return CLI_INVALID_ARGS;
    
    print_route_info((uint8_t)in_node, (uint8_t)out_node);
    return CLI_OK;
  }
  
  // router test IN
  else if (strcasecmp(subcmd, "test") == 0) {
    if (argc < 3) {
      cli_error("Missing node number\n");
      cli_printf("Usage: router test IN\n");
      return CLI_INVALID_ARGS;
    }
    
    int in_node = parse_node(argv[2]);
    if (in_node < 0) return CLI_INVALID_ARGS;
    
    cli_printf("Testing routes from input node %d:\n", in_node);
    cli_printf("Active outputs:\n");
    
    int count = 0;
    for (int out = 0; out < ROUTER_NUM_NODES; out++) {
      if (router_get_route((uint8_t)in_node, (uint8_t)out)) {
        uint16_t chmask = router_get_chanmask((uint8_t)in_node, (uint8_t)out);
        const char* label = router_get_label((uint8_t)in_node, (uint8_t)out);
        cli_printf("  -> %2d: channels=0x%04X", out, chmask);
        if (label && label[0]) {
          cli_printf(" (%s)", label);
        }
        cli_printf("\n");
        count++;
      }
    }
    
    if (count == 0) {
      cli_warning("  No active routes from node %d\n", in_node);
    } else {
      cli_success("Found %d active route(s)\n", count);
    }
    
    return CLI_OK;
  }
  
  else {
    cli_error("Unknown subcommand: %s\n", subcmd);
    return CLI_INVALID_ARGS;
  }
}

// =============================================================================
// REGISTRATION
// =============================================================================

/**
 * @brief Register router CLI commands
 */
int router_cli_register(void) {
  return cli_register_command(
    "router",
    cmd_router,
    "Control MIDI routing matrix",
    "router <matrix|enable|disable|channel|label|info|test> [args...]",
    "midi"
  );
}
