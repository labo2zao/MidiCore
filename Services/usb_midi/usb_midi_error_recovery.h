#pragma once
#include <stdint.h>

/**
 * @file usb_midi_error_recovery.h
 * @brief Error recovery and fault detection for USB MIDI
 * 
 * Provides timeout detection, buffer overflow handling, and automatic
 * recovery mechanisms following MIOS32 defensive programming patterns.
 */

#ifdef __cplusplus
extern "C" {
#endif

// Error codes
typedef enum {
  USB_MIDI_ERR_NONE = 0,
  USB_MIDI_ERR_BUFFER_OVERFLOW = 1,      // SysEx buffer overflow
  USB_MIDI_ERR_INCOMPLETE_MESSAGE = 2,   // Message incomplete after timeout
  USB_MIDI_ERR_INVALID_CIN = 3,          // Invalid cable index number
  USB_MIDI_ERR_INVALID_CABLE = 4,        // Invalid cable number (>3)
  USB_MIDI_ERR_ENDPOINT_BUSY = 5,        // USB endpoint busy during TX
  USB_MIDI_ERR_MALFORMED_SYSEX = 6,      // SysEx without F0 or F7
  USB_MIDI_ERR_TIMEOUT = 7               // Message timeout
} usb_midi_error_t;

// Error recovery configuration
typedef struct {
  uint16_t sysex_timeout_ms;      // Timeout for incomplete SysEx (default: 1000ms)
  uint16_t message_timeout_ms;    // Timeout for incomplete messages (default: 100ms)
  uint8_t auto_recovery;          // 1 = auto-recover, 0 = report only
  uint8_t log_errors;             // 1 = log errors via callback, 0 = silent
  uint8_t reserved[4];
} usb_midi_error_config_t;

// Error statistics (per cable)
typedef struct {
  uint32_t buffer_overflows;      // SysEx buffer overflow count
  uint32_t incomplete_messages;   // Incomplete message timeouts
  uint32_t invalid_cin_count;     // Invalid CIN received
  uint32_t malformed_sysex;       // Malformed SysEx count
  uint32_t endpoint_busy_count;   // TX endpoint busy count
  uint32_t total_errors;          // Total error count
  uint32_t auto_recoveries;       // Successful auto-recovery count
  uint8_t cable_state;            // 0 = OK, 1 = Error, 2 = Recovering
  uint8_t last_error;             // usb_midi_error_t
  uint8_t reserved[2];
} usb_midi_error_stats_t;

// Error callback (optional - can be set for logging/monitoring)
typedef void (*usb_midi_error_callback_t)(uint8_t cable, usb_midi_error_t error, const char* description);

/**
 * @brief Initialize error recovery system
 */
void usb_midi_error_recovery_init(void);

/**
 * @brief Set error recovery configuration
 * @param config Pointer to configuration structure
 */
void usb_midi_error_set_config(const usb_midi_error_config_t* config);

/**
 * @brief Get current error recovery configuration
 * @param out Pointer to output configuration structure
 */
void usb_midi_error_get_config(usb_midi_error_config_t* out);

/**
 * @brief Get error statistics for a cable
 * @param cable Cable number (0-3)
 * @param out Pointer to output statistics structure
 */
void usb_midi_error_get_stats(uint8_t cable, usb_midi_error_stats_t* out);

/**
 * @brief Reset error statistics for a cable
 * @param cable Cable number (0-3), or 0xFF for all cables
 */
void usb_midi_error_reset_stats(uint8_t cable);

/**
 * @brief Register error callback for logging/monitoring
 * @param callback Function pointer to error callback, or NULL to disable
 */
void usb_midi_error_set_callback(usb_midi_error_callback_t callback);

/**
 * @brief Report an error (internal use)
 * @param cable Cable number (0-3)
 * @param error Error code
 * @param description Human-readable description (can be NULL)
 * 
 * Called internally when errors are detected. Triggers callback if registered.
 */
void usb_midi_error_report(uint8_t cable, usb_midi_error_t error, const char* description);

/**
 * @brief Check for SysEx buffer overflow
 * @param cable Cable number (0-3)
 * @param current_pos Current buffer position
 * @param buffer_size Maximum buffer size
 * @return 1 if overflow detected, 0 if OK
 * 
 * Called during SysEx reception to detect buffer overflows.
 */
uint8_t usb_midi_error_check_sysex_overflow(uint8_t cable, uint16_t current_pos, uint16_t buffer_size);

/**
 * @brief Update timeout detection (call from 1ms tick)
 * 
 * Checks for incomplete messages and SysEx timeouts.
 * Triggers auto-recovery if enabled.
 */
void usb_midi_error_tick_1ms(void);

/**
 * @brief Mark cable as active (message received)
 * @param cable Cable number (0-3)
 * 
 * Resets timeout counters for this cable.
 */
void usb_midi_error_mark_activity(uint8_t cable);

/**
 * @brief Recover from error state
 * @param cable Cable number (0-3)
 * 
 * Clears buffers, resets state, attempts to recover.
 * Called automatically if auto_recovery is enabled.
 */
void usb_midi_error_recover(uint8_t cable);

/**
 * @brief Validate CIN (Cable Index Number)
 * @param cin CIN byte (header byte from USB MIDI packet)
 * @return 1 if valid, 0 if invalid
 * 
 * Checks CIN for protocol compliance.
 */
uint8_t usb_midi_error_validate_cin(uint8_t cin);

/**
 * @brief Check USB endpoint status
 * @return 1 if busy, 0 if ready
 * 
 * Prevents transmission when endpoint is busy (prevents buffer overflow).
 */
uint8_t usb_midi_error_check_endpoint_busy(void);

#ifdef __cplusplus
}
#endif
