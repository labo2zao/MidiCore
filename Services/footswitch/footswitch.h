// SPDX-License-Identifier: MIT
#pragma once

/**
 * @file footswitch.h
 * @brief Footswitch input service for MidiCore
 * 
 * Provides footswitch input handling with two modes:
 * - GPIO mode (default): Direct GPIO pin reading
 * - SRIO mode: Bit-bang SPI with 74HC165 shift register
 * 
 * Features:
 * - 8 footswitch inputs
 * - Software debouncing
 * - Press/release event detection
 * - Integration with looper module
 * - Configurable via FOOTSWITCH_USE_SRIO define
 * 
 * Hardware:
 * - GPIO mode: 8 GPIO pins (PE2, PE4-6, PB8-11), internal pull-ups
 * - SRIO mode: 1x 74HC165 shift register, bit-bang SPI (PB12/14/15)
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Constants
// =============================================================================

#define FOOTSWITCH_NUM_SWITCHES  8   ///< Number of footswitch inputs

// =============================================================================
// Types
// =============================================================================

/**
 * @brief Footswitch event types
 */
typedef enum {
    FOOTSWITCH_EVENT_NONE = 0,    ///< No event
    FOOTSWITCH_EVENT_PRESS,       ///< Button pressed
    FOOTSWITCH_EVENT_RELEASE      ///< Button released
} footswitch_event_t;

/**
 * @brief Footswitch event callback function
 * @param fs_num Footswitch number (0-7)
 * @param event Event type (press or release)
 */
typedef void (*footswitch_callback_t)(uint8_t fs_num, footswitch_event_t event);

// =============================================================================
// Public Functions
// =============================================================================

/**
 * @brief Initialize footswitch service
 * 
 * Configures GPIO pins or SRIO hardware depending on mode.
 * Must be called before using other footswitch functions.
 * 
 * @return 0 on success, negative on error
 */
int footswitch_init(void);

/**
 * @brief Scan footswitch inputs and detect events
 * 
 * Reads all footswitch inputs, performs debouncing, and detects
 * press/release events. Should be called periodically (e.g., every 10ms).
 * 
 * Events are processed via registered callback if available.
 * 
 * @return Number of events detected
 */
int footswitch_scan(void);

/**
 * @brief Register callback for footswitch events
 * @param callback Callback function to handle events (NULL to unregister)
 */
void footswitch_set_callback(footswitch_callback_t callback);

/**
 * @brief Get current state of a footswitch
 * @param fs_num Footswitch number (0-7)
 * @return true if pressed, false if released
 */
bool footswitch_is_pressed(uint8_t fs_num);

/**
 * @brief Get raw input state (bypasses debouncing)
 * @param fs_num Footswitch number (0-7)
 * @return true if pressed, false if released
 */
bool footswitch_read_raw(uint8_t fs_num);

#ifdef __cplusplus
}
#endif
