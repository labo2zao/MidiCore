/**
 * @file router_hooks.h
 * @brief Router integration hooks for LiveFX, MIDI Monitor, and SysEx capture
 * 
 * Implements router_tap_hook and router_transform_hook to integrate
 * various services with the MIDI router pipeline.
 */

#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize router hooks system
 */
void router_hooks_init(void);

/**
 * @brief Set track mapping for output nodes
 * @param out_node Output node index
 * @param track Track index (0-3) for LiveFX application
 */
void router_hooks_set_track_map(uint8_t out_node, uint8_t track);

/**
 * @brief Get track mapping for output node
 */
uint8_t router_hooks_get_track_map(uint8_t out_node);

#ifdef __cplusplus
}
#endif
