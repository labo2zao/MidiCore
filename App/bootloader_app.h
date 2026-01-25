/**
 * @file bootloader_app.h
 * @brief Example application integration with bootloader
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Check if SysEx message is a bootloader command
 * @param data SysEx data including F0 and F7
 * @param len Length of data
 * @return true if this is a bootloader message
 */
bool bootloader_app_is_bootloader_sysex(const uint8_t* data, uint32_t len);

/**
 * @brief Handle received SysEx message in application
 * @param data SysEx data including F0 and F7
 * @param len Length of data
 */
void bootloader_app_handle_sysex(const uint8_t* data, uint32_t len);

/**
 * @brief Enter bootloader mode via button press
 */
void bootloader_app_enter_via_button(void);

/**
 * @brief Get bootloader info string for display
 * @return Info string (static storage)
 */
const char* bootloader_app_get_info_string(void);

/**
 * @brief Router filter for bootloader messages
 * @param data Message data
 * @param len Message length
 * @return true to route normally, false to block routing
 */
bool bootloader_app_router_filter(const uint8_t* data, uint32_t len);

#ifdef __cplusplus
}
#endif
