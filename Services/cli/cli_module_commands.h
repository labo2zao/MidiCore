/**
 * @file cli_module_commands.h
 * @brief CLI commands for module control
 * 
 * Provides CLI commands for discovering, controlling, and configuring
 * all registered modules via the module registry.
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register module control commands with CLI
 * 
 * Registers the following commands:
 * - module list [category]       - List all modules or by category
 * - module info <name>            - Show module information
 * - module enable <name> [track]  - Enable a module
 * - module disable <name> [track] - Disable a module
 * - module status <name> [track]  - Show module status
 * - module get <name> <param> [track] - Get parameter value
 * - module set <name> <param> <value> [track] - Set parameter value
 * - module params <name>          - List module parameters
 * 
 * @return 0 on success, negative on error
 */
int cli_module_commands_init(void);

#ifdef __cplusplus
}
#endif
