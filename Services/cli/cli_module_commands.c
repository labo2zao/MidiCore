/**
 * @file cli_module_commands.c
 * @brief CLI commands for module control implementation
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
    cli_error("Missing subcommand\n");
    cli_printf("Usage: module <list|info|enable|disable|status|get|set|params> [args...]\n");
    return CLI_INVALID_ARGS;
  }

  const char* subcmd = argv[1];

  // module list [category]
  if (strcasecmp(subcmd, "list") == 0) {
    if (argc > 2) {
      // List by category (not implemented yet - would need category string parsing)
      cli_error("Category filtering not yet implemented\n");
      return CLI_ERROR;
    } else {
      module_registry_print_modules();
    }
    return CLI_OK;
  }

  // module info <name>
  else if (strcasecmp(subcmd, "info") == 0) {
    if (argc < 3) {
      cli_error("Missing module name\n");
      return CLI_INVALID_ARGS;
    }
    module_registry_print_module(argv[2]);
    return CLI_OK;
  }

  // module enable <name> [track]
  else if (strcasecmp(subcmd, "enable") == 0) {
    if (argc < 3) {
      cli_error("Missing module name\n");
      return CLI_INVALID_ARGS;
    }
    
    uint8_t track = 0xFF; // Default to global
    if (argc > 3) {
      track = (uint8_t)atoi(argv[3]);
    }
    
    int result = module_registry_enable(argv[2], track);
    if (result == 0) {
      if (track == 0xFF) {
        cli_success("Enabled module: %s\n", argv[2]);
      } else {
        cli_success("Enabled module: %s (track %d)\n", argv[2], track);
      }
    } else {
      cli_error("Failed to enable module: %s\n", argv[2]);
    }
    return result == 0 ? CLI_OK : CLI_ERROR;
  }

  // module disable <name> [track]
  else if (strcasecmp(subcmd, "disable") == 0) {
    if (argc < 3) {
      cli_error("Missing module name\n");
      return CLI_INVALID_ARGS;
    }
    
    uint8_t track = 0xFF;
    if (argc > 3) {
      track = (uint8_t)atoi(argv[3]);
    }
    
    int result = module_registry_disable(argv[2], track);
    if (result == 0) {
      if (track == 0xFF) {
        cli_success("Disabled module: %s\n", argv[2]);
      } else {
        cli_success("Disabled module: %s (track %d)\n", argv[2], track);
      }
    } else {
      cli_error("Failed to disable module: %s\n", argv[2]);
    }
    return result == 0 ? CLI_OK : CLI_ERROR;
  }

  // module status <name> [track]
  else if (strcasecmp(subcmd, "status") == 0) {
    if (argc < 3) {
      cli_error("Missing module name\n");
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
    
    if (track == 0xFF) {
      cli_printf("Module %s: %s\n", argv[2], status_str);
    } else {
      cli_printf("Module %s (track %d): %s\n", argv[2], track, status_str);
    }
    return CLI_OK;
  }

  // module get <name> <param> [track]
  else if (strcasecmp(subcmd, "get") == 0) {
    if (argc < 4) {
      cli_error("Missing module name or parameter\n");
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
        switch (param->type) {
          case PARAM_TYPE_BOOL:
            cli_printf("%s.%s = %s\n", argv[2], argv[3], value.bool_val ? "true" : "false");
            break;
          case PARAM_TYPE_INT:
            cli_printf("%s.%s = %ld\n", argv[2], argv[3], (long)value.int_val);
            break;
          case PARAM_TYPE_FLOAT:
            cli_printf("%s.%s = %.3f\n", argv[2], argv[3], value.float_val);
            break;
          case PARAM_TYPE_STRING:
            cli_printf("%s.%s = %s\n", argv[2], argv[3], 
                      value.string_val ? value.string_val : "(null)");
            break;
          case PARAM_TYPE_ENUM:
            cli_printf("%s.%s = %ld\n", argv[2], argv[3], (long)value.int_val);
            break;
        }
      }
    } else {
      cli_error("Failed to get parameter: %s.%s\n", argv[2], argv[3]);
    }
    return result == 0 ? CLI_OK : CLI_ERROR;
  }

  // module set <name> <param> <value> [track]
  else if (strcasecmp(subcmd, "set") == 0) {
    if (argc < 5) {
      cli_error("Missing module name, parameter, or value\n");
      return CLI_INVALID_ARGS;
    }
    
    uint8_t track = 0xFF;
    if (argc > 5) {
      track = (uint8_t)atoi(argv[5]);
    }
    
    // Get parameter descriptor to know type
    const module_param_t* param = module_registry_get_param_descriptor(argv[2], argv[3]);
    if (!param) {
      cli_error("Parameter not found: %s.%s\n", argv[2], argv[3]);
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
      cli_success("Set %s.%s = %s\n", argv[2], argv[3], argv[4]);
    } else {
      cli_error("Failed to set parameter: %s.%s\n", argv[2], argv[3]);
    }
    return result == 0 ? CLI_OK : CLI_ERROR;
  }

  // module params <name>
  else if (strcasecmp(subcmd, "params") == 0) {
    if (argc < 3) {
      cli_error("Missing module name\n");
      return CLI_INVALID_ARGS;
    }
    module_registry_print_params(argv[2]);
    return CLI_OK;
  }

  else {
    cli_error("Unknown subcommand: %s\n", subcmd);
    return CLI_INVALID_ARGS;
  }
}

// =============================================================================
// INITIALIZATION
// =============================================================================

int cli_module_commands_init(void)
{
  dbg_printf("[CLI-MOD] cli_module_commands_init called\r\n");
  int result = cli_register_command("module", cmd_module,
                              "Module control and configuration",
                              "module <list|info|enable|disable|status|get|set|params> [args...]",
                              "modules");
  dbg_printf("[CLI-MOD] cli_register_command returned %d\r\n", result);
  
  // Register stack monitor CLI commands
#if MODULE_ENABLE_STACK_MONITOR
  extern int stack_monitor_cli_init(void);
  dbg_printf("[CLI-MOD] Registering stack monitor CLI commands\r\n");
  stack_monitor_cli_init();
#endif
  
  return result;
}
