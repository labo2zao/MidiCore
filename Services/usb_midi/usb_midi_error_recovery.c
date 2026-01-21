/**
 * @file usb_midi_error_recovery.c
 * @brief Error recovery implementation for USB MIDI
 * 
 * Following MIOS32 defensive programming patterns with timeout detection
 * and automatic recovery mechanisms.
 */

#include "Services/usb_midi/usb_midi_error_recovery.h"
#include "Services/usb_midi/usb_midi.h"
#include <string.h>

// External references to SysEx buffers (from usb_midi.c)
extern void usb_midi_sysex_reset_buffer(uint8_t cable);
extern uint16_t usb_midi_sysex_get_buffer_pos(uint8_t cable);

// Per-cable state for timeout detection
typedef struct {
  uint16_t inactive_ms;           // Milliseconds since last activity
  uint16_t sysex_active_ms;       // Milliseconds since SysEx started
  uint8_t has_incomplete_sysex;   // 1 = incomplete SysEx in progress
  uint8_t padding[3];
} cable_timeout_state_t;

// Module state
static struct {
  usb_midi_error_config_t config;
  usb_midi_error_stats_t stats[4];  // One per cable
  cable_timeout_state_t timeout[4]; // Timeout tracking per cable
  usb_midi_error_callback_t callback;
} error_state __attribute__((aligned(4)));

// Error descriptions for logging
static const char* error_descriptions[] = {
  "No error",
  "SysEx buffer overflow",
  "Incomplete message timeout",
  "Invalid CIN received",
  "Invalid cable number",
  "USB endpoint busy",
  "Malformed SysEx (missing F0 or F7)",
  "Message timeout"
};

void usb_midi_error_recovery_init(void) {
  memset(&error_state, 0, sizeof(error_state));
  
  // Default configuration (MIOS32-style conservative timeouts)
  error_state.config.sysex_timeout_ms = 1000;      // 1 second for SysEx
  error_state.config.message_timeout_ms = 100;     // 100ms for regular messages
  error_state.config.auto_recovery = 1;            // Auto-recover enabled
  error_state.config.log_errors = 0;               // Silent by default
}

void usb_midi_error_set_config(const usb_midi_error_config_t* config) {
  if (!config) return;
  memcpy(&error_state.config, config, sizeof(usb_midi_error_config_t));
}

void usb_midi_error_get_config(usb_midi_error_config_t* out) {
  if (!out) return;
  memcpy(out, &error_state.config, sizeof(usb_midi_error_config_t));
}

void usb_midi_error_get_stats(uint8_t cable, usb_midi_error_stats_t* out) {
  if (!out || cable > 3) return;
  memcpy(out, &error_state.stats[cable], sizeof(usb_midi_error_stats_t));
}

void usb_midi_error_reset_stats(uint8_t cable) {
  if (cable == 0xFF) {
    // Reset all cables
    memset(error_state.stats, 0, sizeof(error_state.stats));
    memset(error_state.timeout, 0, sizeof(error_state.timeout));
  } else if (cable < 4) {
    // Reset specific cable
    memset(&error_state.stats[cable], 0, sizeof(usb_midi_error_stats_t));
    memset(&error_state.timeout[cable], 0, sizeof(cable_timeout_state_t));
  }
}

void usb_midi_error_set_callback(usb_midi_error_callback_t callback) {
  error_state.callback = callback;
}

void usb_midi_error_report(uint8_t cable, usb_midi_error_t error, const char* description) {
  if (cable > 3) return;
  
  usb_midi_error_stats_t* stats = &error_state.stats[cable];
  
  // Update statistics based on error type
  switch (error) {
    case USB_MIDI_ERR_BUFFER_OVERFLOW:
      stats->buffer_overflows++;
      break;
    case USB_MIDI_ERR_INCOMPLETE_MESSAGE:
      stats->incomplete_messages++;
      break;
    case USB_MIDI_ERR_INVALID_CIN:
      stats->invalid_cin_count++;
      break;
    case USB_MIDI_ERR_MALFORMED_SYSEX:
      stats->malformed_sysex++;
      break;
    case USB_MIDI_ERR_ENDPOINT_BUSY:
      stats->endpoint_busy_count++;
      break;
    default:
      break;
  }
  
  stats->total_errors++;
  stats->last_error = error;
  stats->cable_state = 1; // Error state
  
  // Call registered callback if logging enabled
  if (error_state.config.log_errors && error_state.callback) {
    const char* desc = description ? description : error_descriptions[error];
    error_state.callback(cable, error, desc);
  }
  
  // Auto-recover if enabled
  if (error_state.config.auto_recovery) {
    usb_midi_error_recover(cable);
  }
}

uint8_t usb_midi_error_check_sysex_overflow(uint8_t cable, uint16_t current_pos, uint16_t buffer_size) {
  if (cable > 3) return 0;
  
  // Check if we're approaching or exceeding buffer limit
  if (current_pos >= buffer_size) {
    usb_midi_error_report(cable, USB_MIDI_ERR_BUFFER_OVERFLOW, "SysEx buffer full");
    return 1;
  }
  
  // Warn if getting close (90% full)
  if (current_pos > (buffer_size * 9 / 10)) {
    // Buffer nearly full but not yet overflow
    error_state.stats[cable].cable_state = 2; // Warning state
  }
  
  return 0;
}

void usb_midi_error_tick_1ms(void) {
  for (uint8_t cable = 0; cable < 4; cable++) {
    cable_timeout_state_t* timeout = &error_state.timeout[cable];
    usb_midi_error_stats_t* stats = &error_state.stats[cable];
    
    // Increment inactive timer
    if (timeout->inactive_ms < 0xFFFF) {
      timeout->inactive_ms++;
    }
    
    // Check for incomplete SysEx timeout
    if (timeout->has_incomplete_sysex) {
      timeout->sysex_active_ms++;
      
      if (timeout->sysex_active_ms > error_state.config.sysex_timeout_ms) {
        usb_midi_error_report(cable, USB_MIDI_ERR_INCOMPLETE_MESSAGE, "SysEx timeout");
        timeout->has_incomplete_sysex = 0;
        timeout->sysex_active_ms = 0;
      }
    }
    
    // Check for general message timeout (cable completely idle)
    if (timeout->inactive_ms > error_state.config.message_timeout_ms) {
      if (stats->cable_state == 2) { // Warning state
        // Cable has been idle long enough, likely recovered
        stats->cable_state = 0; // OK state
      }
    }
  }
}

void usb_midi_error_mark_activity(uint8_t cable) {
  if (cable > 3) return;
  
  cable_timeout_state_t* timeout = &error_state.timeout[cable];
  usb_midi_error_stats_t* stats = &error_state.stats[cable];
  
  // Reset inactive timer
  timeout->inactive_ms = 0;
  
  // If cable was in error state and now receiving data, mark as recovering
  if (stats->cable_state == 1) {
    stats->cable_state = 2; // Recovering
  }
}

void usb_midi_error_recover(uint8_t cable) {
  if (cable > 3) return;
  
  usb_midi_error_stats_t* stats = &error_state.stats[cable];
  cable_timeout_state_t* timeout = &error_state.timeout[cable];
  
  // Reset SysEx buffer for this cable (external function from usb_midi.c)
  // Note: This requires exposing usb_midi_sysex_reset_buffer() or equivalent
  // For now, we mark the state and the main RX handler will clean up
  
  timeout->has_incomplete_sysex = 0;
  timeout->sysex_active_ms = 0;
  timeout->inactive_ms = 0;
  
  stats->cable_state = 0; // OK state
  stats->auto_recoveries++;
}

uint8_t usb_midi_error_validate_cin(uint8_t cin) {
  uint8_t cin_low = cin & 0x0F;
  
  // Valid CINs per USB MIDI 1.0: 0x2-0xF (0x0, 0x1 are reserved)
  if (cin_low == 0x00 || cin_low == 0x01) {
    return 0; // Invalid
  }
  
  return 1; // Valid
}

uint8_t usb_midi_error_check_endpoint_busy(void) {
  // This would need access to USB device handle to check endpoint status
  // For now, return 0 (not busy) - actual implementation would check:
  // return (hUsbDeviceFS.ep_in[MIDI_EPIN_ADDR & 0xFU].status == USB_EP_TX_BUSY);
  return 0;
}

// Helper function to mark SysEx as active (called from usb_midi.c)
void usb_midi_error_sysex_started(uint8_t cable) {
  if (cable > 3) return;
  error_state.timeout[cable].has_incomplete_sysex = 1;
  error_state.timeout[cable].sysex_active_ms = 0;
}

// Helper function to mark SysEx as complete (called from usb_midi.c)
void usb_midi_error_sysex_completed(uint8_t cable) {
  if (cable > 3) return;
  error_state.timeout[cable].has_incomplete_sysex = 0;
  error_state.timeout[cable].sysex_active_ms = 0;
}
