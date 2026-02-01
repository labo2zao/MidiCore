/**
 * @file router_cli.c
 * @brief CLI commands for MIDI Router control
 * 
 * MIOS32-STYLE: NO printf / snprintf / vsnprintf
 * Uses only: cli_puts(), cli_putc(), cli_print_u32(), cli_print_hex16(), cli_newline()
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
    cli_error("Invalid node number");
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
  
  // Advanced parsing not yet implemented - return error indication
  cli_error("Unsupported mask format. Use: all, 1-16");
  return 0;  // Error - no channels selected
}

/**
 * @brief Print routing matrix
 */
static void print_routing_matrix(void) {
  cli_newline();
  cli_puts("MIDI Routing Matrix (");
  cli_print_u32(ROUTER_NUM_NODES);
  cli_puts("x");
  cli_print_u32(ROUTER_NUM_NODES);
  cli_puts(" nodes):");
  cli_newline();
  cli_puts("=================================================");
  cli_newline();
  
  // Header row
  cli_puts("IN\\OUT ");
  for (int out = 0; out < ROUTER_NUM_NODES; out++) {
    cli_puts(" ");
    if (out < 10) cli_putc(' ');
    cli_print_u32((uint32_t)out);
  }
  cli_newline();
  cli_puts("-------");
  for (int out = 0; out < ROUTER_NUM_NODES; out++) {
    cli_puts("---");
  }
  cli_newline();
  
  // Data rows
  for (int in = 0; in < ROUTER_NUM_NODES; in++) {
    cli_puts("  ");
    if (in < 10) cli_putc(' ');
    cli_print_u32((uint32_t)in);
    cli_puts("   ");
    for (int out = 0; out < ROUTER_NUM_NODES; out++) {
      uint8_t enabled = router_get_route((uint8_t)in, (uint8_t)out);
      cli_puts("  ");
      cli_putc(enabled ? 'X' : '.');
    }
    cli_newline();
  }
  cli_newline();
}

/**
 * @brief Print detailed route information
 */
static void print_route_info(uint8_t in_node, uint8_t out_node) {
  uint8_t enabled = router_get_route(in_node, out_node);
  uint16_t chmask = router_get_chanmask(in_node, out_node);
  const char* label = router_get_label(in_node, out_node);
  
  cli_puts("Route: ");
  cli_print_u32(in_node);
  cli_puts(" -> ");
  cli_print_u32(out_node);
  cli_newline();
  
  cli_puts("  Status: ");
  cli_puts(enabled ? "ENABLED" : "DISABLED");
  cli_newline();
  
  cli_puts("  Channel Mask: 0x");
  cli_print_hex16(chmask);
  cli_puts(" (");
  
  // Print enabled channels
  int first = 1;
  for (int ch = 0; ch < 16; ch++) {
    if (chmask & (1 << ch)) {
      if (!first) cli_putc(',');
      cli_print_u32((uint32_t)(ch + 1));
      first = 0;
    }
  }
  if (first) cli_puts("none");
  cli_puts(")");
  cli_newline();
  
  if (label && label[0]) {
    cli_puts("  Label: ");
    cli_puts(label);
    cli_newline();
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
    cli_error("Missing subcommand");
    cli_newline();
    cli_puts("Usage: router <matrix|enable|disable|channel|label|info|test>");
    cli_newline();
    cli_puts("  router matrix              - Display routing matrix");
    cli_newline();
    cli_puts("  router enable IN OUT       - Enable route from IN to OUT");
    cli_newline();
    cli_puts("  router disable IN OUT      - Disable route from IN to OUT");
    cli_newline();
    cli_puts("  router channel IN OUT MASK - Set channel mask");
    cli_newline();
    cli_puts("  router label IN OUT TEXT   - Set route label");
    cli_newline();
    cli_puts("  router info IN OUT         - Show detailed route info");
    cli_newline();
    cli_puts("  router test IN             - Test routing from input node");
    cli_newline();
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
      cli_error("Missing arguments");
      cli_newline();
      cli_puts("Usage: router enable IN OUT");
      cli_newline();
      return CLI_INVALID_ARGS;
    }
    
    int in_node = parse_node(argv[2]);
    int out_node = parse_node(argv[3]);
    if (in_node < 0 || out_node < 0) return CLI_INVALID_ARGS;
    
    router_set_route((uint8_t)in_node, (uint8_t)out_node, 1);
    cli_puts("Enabled route: ");
    cli_print_u32((uint32_t)in_node);
    cli_puts(" -> ");
    cli_print_u32((uint32_t)out_node);
    cli_newline();
    return CLI_OK;
  }
  
  // router disable IN OUT
  else if (strcasecmp(subcmd, "disable") == 0) {
    if (argc < 4) {
      cli_error("Missing arguments");
      cli_newline();
      cli_puts("Usage: router disable IN OUT");
      cli_newline();
      return CLI_INVALID_ARGS;
    }
    
    int in_node = parse_node(argv[2]);
    int out_node = parse_node(argv[3]);
    if (in_node < 0 || out_node < 0) return CLI_INVALID_ARGS;
    
    router_set_route((uint8_t)in_node, (uint8_t)out_node, 0);
    cli_puts("Disabled route: ");
    cli_print_u32((uint32_t)in_node);
    cli_puts(" -> ");
    cli_print_u32((uint32_t)out_node);
    cli_newline();
    return CLI_OK;
  }
  
  // router channel IN OUT MASK
  else if (strcasecmp(subcmd, "channel") == 0) {
    if (argc < 5) {
      cli_error("Missing arguments");
      cli_newline();
      cli_puts("Usage: router channel IN OUT MASK");
      cli_newline();
      cli_puts("  MASK: 'all', '1', '1-16', etc.");
      cli_newline();
      return CLI_INVALID_ARGS;
    }
    
    int in_node = parse_node(argv[2]);
    int out_node = parse_node(argv[3]);
    if (in_node < 0 || out_node < 0) return CLI_INVALID_ARGS;
    
    uint16_t chmask = parse_channel_mask(argv[4]);
    if (chmask == 0) {
      cli_error("Invalid channel mask");
      return CLI_INVALID_ARGS;
    }
    
    router_set_chanmask((uint8_t)in_node, (uint8_t)out_node, chmask);
    cli_puts("Set channel mask for route ");
    cli_print_u32((uint32_t)in_node);
    cli_puts(" -> ");
    cli_print_u32((uint32_t)out_node);
    cli_puts(": 0x");
    cli_print_hex16(chmask);
    cli_newline();
    return CLI_OK;
  }
  
  // router label IN OUT TEXT
  else if (strcasecmp(subcmd, "label") == 0) {
    if (argc < 5) {
      cli_error("Missing arguments");
      cli_newline();
      cli_puts("Usage: router label IN OUT TEXT");
      cli_newline();
      return CLI_INVALID_ARGS;
    }
    
    int in_node = parse_node(argv[2]);
    int out_node = parse_node(argv[3]);
    if (in_node < 0 || out_node < 0) return CLI_INVALID_ARGS;
    
    // Join remaining args as label (more efficient concatenation)
    char label[64] = {0};
    size_t label_len = 0;
    for (int i = 4; i < argc && label_len < sizeof(label) - 2; i++) {
      size_t arg_len = strlen(argv[i]);
      if (i > 4 && label_len < sizeof(label) - 1) {
        label[label_len++] = ' ';
      }
      size_t copy_len = arg_len;
      if (label_len + copy_len >= sizeof(label)) {
        copy_len = sizeof(label) - label_len - 1;
      }
      memcpy(label + label_len, argv[i], copy_len);
      label_len += copy_len;
    }
    label[label_len] = '\0';
    
    router_set_label((uint8_t)in_node, (uint8_t)out_node, label);
    cli_puts("Set label for route ");
    cli_print_u32((uint32_t)in_node);
    cli_puts(" -> ");
    cli_print_u32((uint32_t)out_node);
    cli_puts(": '");
    cli_puts(label);
    cli_puts("'");
    cli_newline();
    return CLI_OK;
  }
  
  // router info IN OUT
  else if (strcasecmp(subcmd, "info") == 0) {
    if (argc < 4) {
      cli_error("Missing arguments");
      cli_newline();
      cli_puts("Usage: router info IN OUT");
      cli_newline();
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
      cli_error("Missing node number");
      cli_newline();
      cli_puts("Usage: router test IN");
      cli_newline();
      return CLI_INVALID_ARGS;
    }
    
    int in_node = parse_node(argv[2]);
    if (in_node < 0) return CLI_INVALID_ARGS;
    
    cli_puts("Testing routes from input node ");
    cli_print_u32((uint32_t)in_node);
    cli_puts(":");
    cli_newline();
    cli_puts("Active outputs:");
    cli_newline();
    
    int count = 0;
    for (int out = 0; out < ROUTER_NUM_NODES; out++) {
      if (router_get_route((uint8_t)in_node, (uint8_t)out)) {
        uint16_t chmask = router_get_chanmask((uint8_t)in_node, (uint8_t)out);
        const char* label = router_get_label((uint8_t)in_node, (uint8_t)out);
        cli_puts("  -> ");
        if (out < 10) cli_putc(' ');
        cli_print_u32((uint32_t)out);
        cli_puts(": channels=0x");
        cli_print_hex16(chmask);
        if (label && label[0]) {
          cli_puts(" (");
          cli_puts(label);
          cli_puts(")");
        }
        cli_newline();
        count++;
      }
    }
    
    if (count == 0) {
      cli_warning("No active routes from this node");
      cli_newline();
    } else {
      cli_puts("Found ");
      cli_print_u32((uint32_t)count);
      cli_puts(" active route(s)");
      cli_newline();
    }
    
    return CLI_OK;
  }
  
  else {
    cli_error("Unknown subcommand");
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
