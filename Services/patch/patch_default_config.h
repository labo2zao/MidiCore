/**
 * @file patch_default_config.h
 * @brief Default configuration embedded in firmware (compiled in)
 * 
 * This file contains the default configuration that is compiled into
 * the firmware. It's used when no config file exists on SD card,
 * providing a fully functional system without requiring SD card writes.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get default configuration as string array
 * @return Pointer to array of config lines (NULL-terminated)
 */
const char** patch_get_default_config_lines(void);

/**
 * @brief Get number of lines in default config
 * @return Number of config lines
 */
int patch_get_default_config_line_count(void);

/**
 * @brief Load default configuration into patch system (RAM only)
 * @return 0 on success, negative on error
 */
int patch_load_default_config(void);

#ifdef __cplusplus
}
#endif
