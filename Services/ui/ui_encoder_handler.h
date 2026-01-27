/**
 * @file ui_encoder_handler.h
 * @brief Rotary Encoder Handler for Module Control
 * 
 * Provides high-level encoder handling that integrates with the module
 * registry and UI navigation system. Maps encoder movements to parameter
 * changes and menu navigation.
 * 
 * Features:
 * - Acceleration for faster parameter changes
 * - Dead zone handling
 * - Value clamping to parameter ranges
 * - Context-aware encoder behavior
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Encoder behavior mode
 */
typedef enum {
  ENC_MODE_NAVIGATION,   // Navigate menus
  ENC_MODE_PARAM_EDIT,   // Edit parameter values
  ENC_MODE_VALUE_ADJUST, // Adjust numeric values
  ENC_MODE_LIST_SELECT   // Select from list
} encoder_mode_t;

/**
 * @brief Encoder configuration
 */
typedef struct {
  uint8_t acceleration_enabled;  // Enable acceleration
  uint8_t acceleration_factor;   // Acceleration multiplier (1-10)
  uint8_t dead_zone;             // Dead zone threshold (0-10)
  uint8_t detent_steps;          // Steps per detent (1, 2, or 4)
} encoder_config_t;

/**
 * @brief Initialize encoder handler
 * @param config Configuration (NULL for defaults)
 */
void ui_encoder_handler_init(const encoder_config_t* config);

/**
 * @brief Process encoder movement
 * @param enc_id Encoder ID (0 or 1)
 * @param delta Raw encoder delta from hardware
 * @param mode Current encoder mode
 * @return Processed delta value for application use
 */
int16_t ui_encoder_handler_process(uint8_t enc_id, int8_t delta, encoder_mode_t mode);

/**
 * @brief Reset encoder state (e.g., when changing context)
 * @param enc_id Encoder ID (0 or 1)
 */
void ui_encoder_handler_reset(uint8_t enc_id);

/**
 * @brief Get default encoder configuration
 * @param config Output configuration structure
 */
void ui_encoder_handler_get_defaults(encoder_config_t* config);

#ifdef __cplusplus
}
#endif
