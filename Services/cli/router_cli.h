/**
 * @file router_cli.h
 * @brief CLI commands for MIDI Router control
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Register router CLI commands
 * @return 0 on success, negative on error
 */
int router_cli_register(void);

#ifdef __cplusplus
}
#endif
