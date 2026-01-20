/**
 * @file livefx.h
 * @brief Live FX system for real-time MIDI manipulation
 * 
 * Provides transpose, velocity scaling, and force-to-scale effects
 * that can be applied to tracks in real-time during performance.
 */

#pragma once
#include <stdint.h>
#include "Services/router/router.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LIVEFX_MAX_TRACKS 4

/**
 * @brief LiveFX configuration per track
 */
typedef struct {
  int8_t transpose;        // -12 to +12 semitones
  uint8_t vel_scale;       // Velocity scale: 0-200% (128 = 100%)
  uint8_t force_scale;     // Force-to-scale: 0=off, 1=on
  uint8_t scale_type;      // Scale index (see scale.h)
  uint8_t scale_root;      // Root note (C=0, C#=1, ..., B=11)
  uint8_t enabled;         // 0=bypass, 1=active
} livefx_config_t;

/**
 * @brief Initialize LiveFX system
 */
void livefx_init(void);

/**
 * @brief Set transpose amount for a track
 * @param track Track index (0-3)
 * @param semitones Transpose amount (-12 to +12)
 */
void livefx_set_transpose(uint8_t track, int8_t semitones);

/**
 * @brief Get transpose amount for a track
 */
int8_t livefx_get_transpose(uint8_t track);

/**
 * @brief Set velocity scale for a track
 * @param track Track index (0-3)
 * @param scale Velocity scale (0-200%, 128 = 100%)
 */
void livefx_set_velocity_scale(uint8_t track, uint8_t scale);

/**
 * @brief Get velocity scale for a track
 */
uint8_t livefx_get_velocity_scale(uint8_t track);

/**
 * @brief Set force-to-scale for a track
 * @param track Track index (0-3)
 * @param scale_type Scale type index
 * @param root Root note (0-11)
 * @param enable 1 to enable, 0 to disable
 */
void livefx_set_force_scale(uint8_t track, uint8_t scale_type, uint8_t root, uint8_t enable);

/**
 * @brief Get force-to-scale configuration
 */
void livefx_get_force_scale(uint8_t track, uint8_t* scale_type, uint8_t* root, uint8_t* enable);

/**
 * @brief Enable/disable LiveFX for a track
 * @param track Track index (0-3)
 * @param enable 1 to enable, 0 to bypass
 */
void livefx_set_enabled(uint8_t track, uint8_t enable);

/**
 * @brief Check if LiveFX is enabled for a track
 */
uint8_t livefx_get_enabled(uint8_t track);

/**
 * @brief Apply LiveFX to a MIDI message
 * @param track Track index (0-3)
 * @param msg Input/output MIDI message
 * @return 0 on success, -1 if message should be filtered
 */
int livefx_apply(uint8_t track, router_msg_t* msg);

/**
 * @brief Get configuration for a track
 */
const livefx_config_t* livefx_get_config(uint8_t track);

#ifdef __cplusplus
}
#endif
