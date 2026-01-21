#pragma once
#include <stdint.h>

/**
 * @file usb_midi_active_sensing.h
 * @brief MIDI Active Sensing support (0xFE)
 * 
 * Implements Active Sensing timeout detection for MIDI connections.
 * Per MIDI spec, Active Sensing (0xFE) should be sent every 300ms max.
 * If no message received for 300ms, connection is considered lost.
 * 
 * Features:
 * - Automatic timeout detection (configurable, default 300ms)
 * - Per-cable monitoring (4 USB MIDI cables)
 * - Optional Active Sensing transmission
 * - Connection state callbacks
 */

#ifdef __cplusplus
extern "C" {
#endif

// Active Sensing configuration
typedef struct {
  uint8_t enabled;                // 1 = enabled, 0 = disabled
  uint8_t send_active_sensing;    // 1 = send 0xFE periodically, 0 = don't send
  uint16_t timeout_ms;            // Timeout in milliseconds (default 300)
  uint16_t send_interval_ms;      // Interval for sending 0xFE (default 250ms)
  uint8_t cable_mask;             // Bit mask of cables to monitor (0x0F = all 4)
  uint8_t reserved;
} active_sensing_config_t;

// Connection state per cable
typedef struct {
  uint8_t is_connected;           // 1 = connected, 0 = timeout/disconnected
  uint8_t cable;                  // Cable number (0-3)
  uint16_t time_since_rx_ms;      // Time since last message (milliseconds)
  uint32_t active_sensing_count;  // Total Active Sensing messages received
  uint32_t timeout_count;         // Number of timeouts detected
  uint32_t last_message_time_ms;  // System time of last message
  uint8_t reserved[2];
} active_sensing_cable_state_t;

// Callback function type for connection state changes
typedef void (*active_sensing_callback_t)(uint8_t cable, uint8_t is_connected);

/**
 * @brief Initialize Active Sensing module
 */
void active_sensing_init(void);

/**
 * @brief Set Active Sensing configuration
 * @param config Pointer to configuration structure
 */
void active_sensing_set_config(const active_sensing_config_t* config);

/**
 * @brief Get Active Sensing configuration
 * @param out Pointer to output configuration structure
 */
void active_sensing_get_config(active_sensing_config_t* out);

/**
 * @brief Get cable connection state
 * @param cable Cable number (0-3)
 * @param out Pointer to output state structure
 * @return 0 on success, -1 if cable invalid
 */
int active_sensing_get_cable_state(uint8_t cable, active_sensing_cable_state_t* out);

/**
 * @brief Check if cable is connected
 * @param cable Cable number (0-3)
 * @return 1 if connected, 0 if disconnected/timeout
 */
uint8_t active_sensing_is_cable_connected(uint8_t cable);

/**
 * @brief Register callback for connection state changes
 * @param callback Function to call when connection state changes
 * 
 * Callback is called with (cable, is_connected) when:
 * - Connection established (first message received)
 * - Timeout detected (no message for timeout_ms)
 * - Connection restored (message received after timeout)
 */
void active_sensing_register_callback(active_sensing_callback_t callback);

/**
 * @brief Process received MIDI message (any type)
 * @param cable Cable number (0-3)
 * 
 * Updates last-activity timestamp for cable.
 * Resets timeout counter.
 * Called from USB MIDI RX path for all messages.
 */
void active_sensing_on_rx_message(uint8_t cable);

/**
 * @brief Process received Active Sensing message (0xFE)
 * @param cable Cable number (0-3)
 * 
 * Explicitly handles Active Sensing messages.
 * Updates statistics.
 */
void active_sensing_on_rx_active_sensing(uint8_t cable);

/**
 * @brief Send Active Sensing message (0xFE)
 * @param cable Cable number (0-3)
 * 
 * Sends Active Sensing on specified cable.
 * Called automatically if send_active_sensing is enabled.
 */
void active_sensing_send(uint8_t cable);

/**
 * @brief Update Active Sensing (call from 1ms timer)
 * 
 * Monitors timeouts for all cables.
 * Sends Active Sensing messages if configured.
 * Triggers callbacks on state changes.
 */
void active_sensing_tick_1ms(void);

/**
 * @brief Reset cable connection state
 * @param cable Cable number (0-3), or 0xFF for all cables
 * 
 * Resets timeout counters and marks cable as disconnected.
 * Use when manually disconnecting or reinitializing.
 */
void active_sensing_reset_cable(uint8_t cable);

/**
 * @brief Enable/disable Active Sensing
 * @param enabled 1 to enable, 0 to disable
 */
void active_sensing_set_enabled(uint8_t enabled);

/**
 * @brief Get enabled status
 * @return 1 if enabled, 0 if disabled
 */
uint8_t active_sensing_get_enabled(void);

#ifdef __cplusplus
}
#endif
