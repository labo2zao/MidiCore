/**
 * @file cli_module_commands.c
 * @brief CLI commands for module control implementation
 * 
 * MIOS32-STYLE: NO printf / snprintf / vsnprintf
 * Uses only fixed-string output: cli_puts, cli_putc, cli_print_u32, cli_newline
 */

#include "Config/module_config.h"
#include "cli_module_commands.h"
#include "cli.h"
#include "Services/module_registry/module_registry.h"
#include "Services/midicore_hooks/midicore_hooks.h"
#include <string.h>
#include <stdlib.h>

// =============================================================================
// COMMAND HANDLERS
// =============================================================================

static cli_result_t cmd_module(int argc, char* argv[])
{
  if (argc < 2) {
    cli_error("Missing subcommand");
    cli_puts("Usage: module <list|info|enable|disable|status|get|set|params> [args...]");
    cli_newline();
    return CLI_INVALID_ARGS;
  }

  const char* subcmd = argv[1];

  // module list [category]
  if (strcasecmp(subcmd, "list") == 0) {
    if (argc > 2) {
      // List by category (not implemented yet - would need category string parsing)
      cli_error("Category filtering not yet implemented");
      return CLI_ERROR;
    } else {
      module_registry_print_modules();
    }
    return CLI_OK;
  }

  // module info <name>
  else if (strcasecmp(subcmd, "info") == 0) {
    if (argc < 3) {
      cli_error("Missing module name");
      return CLI_INVALID_ARGS;
    }
    module_registry_print_module(argv[2]);
    return CLI_OK;
  }

  // module enable <name> [track]
  else if (strcasecmp(subcmd, "enable") == 0) {
    if (argc < 3) {
      cli_error("Missing module name");
      return CLI_INVALID_ARGS;
    }
    
    uint8_t track = 0xFF; // Default to global
    if (argc > 3) {
      track = (uint8_t)atoi(argv[3]);
    }
    
    int result = module_registry_enable(argv[2], track);
    if (result == 0) {
      cli_puts("Enabled module: ");
      cli_puts(argv[2]);
      if (track != 0xFF) {
        cli_puts(" (track ");
        cli_print_u32(track);
        cli_putc(')');
      }
      cli_newline();
    } else {
      cli_error("Failed to enable module");
    }
    return result == 0 ? CLI_OK : CLI_ERROR;
  }

  // module disable <name> [track]
  else if (strcasecmp(subcmd, "disable") == 0) {
    if (argc < 3) {
      cli_error("Missing module name");
      return CLI_INVALID_ARGS;
    }
    
    uint8_t track = 0xFF;
    if (argc > 3) {
      track = (uint8_t)atoi(argv[3]);
    }
    
    int result = module_registry_disable(argv[2], track);
    if (result == 0) {
      cli_puts("Disabled module: ");
      cli_puts(argv[2]);
      if (track != 0xFF) {
        cli_puts(" (track ");
        cli_print_u32(track);
        cli_putc(')');
      }
      cli_newline();
    } else {
      cli_error("Failed to disable module");
    }
    return result == 0 ? CLI_OK : CLI_ERROR;
  }

  // module status <name> [track]
  else if (strcasecmp(subcmd, "status") == 0) {
    if (argc < 3) {
      cli_error("Missing module name");
      return CLI_INVALID_ARGS;
    }
    
    uint8_t track = 0xFF;
    if (argc > 3) {
      track = (uint8_t)atoi(argv[3]);
    }
    
    module_status_t status = module_registry_get_status(argv[2], track);
    const char* status_str = "Unknown";
    switch (status) {
      case MODULE_STATUS_DISABLED: status_str = "Disabled"; break;
      case MODULE_STATUS_ENABLED: status_str = "Enabled"; break;
      case MODULE_STATUS_ERROR: status_str = "Error"; break;
    }
    
    cli_puts("Module ");
    cli_puts(argv[2]);
    if (track != 0xFF) {
      cli_puts(" (track ");
      cli_print_u32(track);
      cli_putc(')');
    }
    cli_puts(": ");
    cli_puts(status_str);
    cli_newline();
    return CLI_OK;
  }

  // module get <name> <param> [track]
  else if (strcasecmp(subcmd, "get") == 0) {
    if (argc < 4) {
      cli_error("Missing module name or parameter");
      return CLI_INVALID_ARGS;
    }
    
    uint8_t track = 0xFF;
    if (argc > 4) {
      track = (uint8_t)atoi(argv[4]);
    }
    
    param_value_t value;
    int result = module_registry_get_param(argv[2], argv[3], track, &value);
    if (result == 0) {
      // Get parameter descriptor to know type
      const module_param_t* param = module_registry_get_param_descriptor(argv[2], argv[3]);
      if (param) {
        cli_puts(argv[2]);
        cli_putc('.');
        cli_puts(argv[3]);
        cli_puts(" = ");
        switch (param->type) {
          case PARAM_TYPE_BOOL:
            cli_puts(value.bool_val ? "true" : "false");
            break;
          case PARAM_TYPE_INT:
          case PARAM_TYPE_ENUM:
            cli_print_i32(value.int_val);
            break;
          case PARAM_TYPE_FLOAT:
            /* Float output - print integer part only for MIOS32 compatibility */
            cli_print_i32((int32_t)value.float_val);
            break;
          case PARAM_TYPE_STRING:
            cli_puts(value.string_val ? value.string_val : "(null)");
            break;
        }
        cli_newline();
      }
    } else {
      cli_error("Failed to get parameter");
    }
    return result == 0 ? CLI_OK : CLI_ERROR;
  }

  // module set <name> <param> <value> [track]
  else if (strcasecmp(subcmd, "set") == 0) {
    if (argc < 5) {
      cli_error("Missing module name, parameter, or value");
      return CLI_INVALID_ARGS;
    }
    
    uint8_t track = 0xFF;
    if (argc > 5) {
      track = (uint8_t)atoi(argv[5]);
    }
    
    // Get parameter descriptor to know type
    const module_param_t* param = module_registry_get_param_descriptor(argv[2], argv[3]);
    if (!param) {
      cli_error("Parameter not found");
      return CLI_ERROR;
    }
    
    param_value_t value;
    switch (param->type) {
      case PARAM_TYPE_BOOL:
        value.bool_val = (strcasecmp(argv[4], "true") == 0 || 
                         strcasecmp(argv[4], "1") == 0 ||
                         strcasecmp(argv[4], "on") == 0) ? 1 : 0;
        break;
      case PARAM_TYPE_INT:
      case PARAM_TYPE_ENUM:
        value.int_val = atoi(argv[4]);
        break;
      case PARAM_TYPE_FLOAT:
        value.float_val = atof(argv[4]);
        break;
      case PARAM_TYPE_STRING:
        value.string_val = argv[4];
        break;
    }
    
    int result = module_registry_set_param(argv[2], argv[3], track, &value);
    if (result == 0) {
      cli_puts("Set ");
      cli_puts(argv[2]);
      cli_putc('.');
      cli_puts(argv[3]);
      cli_puts(" = ");
      cli_puts(argv[4]);
      cli_newline();
    } else {
      cli_error("Failed to set parameter");
    }
    return result == 0 ? CLI_OK : CLI_ERROR;
  }

  // module params <name>
  else if (strcasecmp(subcmd, "params") == 0) {
    if (argc < 3) {
      cli_error("Missing module name");
      return CLI_INVALID_ARGS;
    }
    module_registry_print_params(argv[2]);
    return CLI_OK;
  }

  else {
    cli_error("Unknown subcommand");
    return CLI_INVALID_ARGS;
  }
}

// =============================================================================
// INITIALIZATION
// =============================================================================

int cli_module_commands_init(void)
{
  /* MIOS32-STYLE: Silent initialization - no dbg_printf */
  int result = cli_register_command("module", cmd_module,
                              "Module control and configuration",
                              "module <list|info|enable|disable|status|get|set|params> [args...]",
                              "modules");
  
  // Register stack monitor CLI commands
#if MODULE_ENABLE_STACK_MONITOR
  extern int stack_monitor_cli_init(void);
  stack_monitor_cli_init();
#endif
  
  return result;
}
