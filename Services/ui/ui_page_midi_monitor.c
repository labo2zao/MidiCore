/**
 * @file ui_page_midi_monitor.c
 * @brief MIDI Monitor UI Page - Real-time MIDI message display
 * 
 * Shows the last N MIDI messages with timestamps, ports, and decoded info.
 * Useful for debugging MIDI routing and monitoring live performance.
 */

#include "Services/ui/ui_page_midi_monitor.h"
#include "Services/ui/ui_gfx.h"
#include <stdio.h>
#include <string.h>

#define MONITOR_BUFFER_SIZE 6  // Display last 6 messages

// Simple MIDI event structure
typedef struct {
  uint32_t timestamp_ms;
  uint8_t port;      // 0=IN1, 1=OUT1, etc.
  uint8_t len;       // Message length (1-3 bytes)
  uint8_t data[3];   // MIDI bytes
} midi_event_t;

// Circular buffer for MIDI events
static midi_event_t event_buffer[MONITOR_BUFFER_SIZE];
static uint8_t event_count = 0;
static uint8_t paused = 0;
static uint8_t scroll_offset = 0;

/**
 * @brief Add a MIDI event to the monitor (to be called by MIDI router)
 * 
 * Note: This is a placeholder. In a full implementation, this would be
 * called from the MIDI router when messages are received/sent.
 */
void ui_midi_monitor_capture(uint8_t port, const uint8_t* data, uint8_t len, uint32_t timestamp_ms) {
  if (paused || len == 0 || len > 3) return;
  
  // Add to circular buffer
  uint8_t idx = event_count % MONITOR_BUFFER_SIZE;
  event_buffer[idx].timestamp_ms = timestamp_ms;
  event_buffer[idx].port = port;
  event_buffer[idx].len = len;
  for (uint8_t i = 0; i < len && i < 3; i++) {
    event_buffer[idx].data[i] = data[i];
  }
  
  event_count++;
}

/**
 * @brief Decode MIDI message to human-readable string (compact, LoopA-style)
 */
static void decode_midi_message(const uint8_t* data, uint8_t len, char* out, size_t out_size) {
  if (len == 0) {
    snprintf(out, out_size, "Empty");
    return;
  }
  
  uint8_t status = data[0];
  uint8_t type = status & 0xF0;
  uint8_t channel = (status & 0x0F) + 1;
  
  switch (type) {
    case 0x80:  // Note Off
      if (len >= 3) {
        snprintf(out, out_size, "Off C%u N%u V%u", channel, data[1], data[2]);
      }
      break;
    case 0x90:  // Note On
      if (len >= 3) {
        snprintf(out, out_size, "On  C%u N%u V%u", channel, data[1], data[2]);
      }
      break;
    case 0xB0:  // Control Change
      if (len >= 3) {
        snprintf(out, out_size, "CC  C%u #%u=%u", channel, data[1], data[2]);
      }
      break;
    case 0xC0:  // Program Change
      if (len >= 2) {
        snprintf(out, out_size, "PC  C%u P%u", channel, data[1]);
      }
      break;
    case 0xE0:  // Pitch Bend
      if (len >= 3) {
        int bend = ((int)data[2] << 7) | data[1];
        snprintf(out, out_size, "PB  C%u %+d", channel, bend - 8192);
      }
      break;
    case 0xF0:  // System message
      if (status == 0xF0) {
        snprintf(out, out_size, "SysEx...");
      } else {
        snprintf(out, out_size, "Sys:0x%02X", status);
      }
      break;
    default:
      snprintf(out, out_size, "0x%02X", status);
      break;
  }
}

/**
 * @brief Render the MIDI monitor page
 */
void ui_page_midi_monitor_render(uint32_t now_ms) {
  (void)now_ms;
  
  ui_gfx_clear(0);
  
  // Header with 8x8 font for better readability
  ui_gfx_set_font(UI_FONT_8X8);
  char header[64];
  snprintf(header, sizeof(header), "MIDI %s Events:%u", 
           paused ? "[PAUSED]" : "[LIVE]", event_count);
  ui_gfx_text(0, 0, header, 15);
  ui_gfx_hline(0, 11, 256, 8);
  
  // Display event list with better spacing (9px per line instead of 8px)
  uint8_t display_count = (event_count < MONITOR_BUFFER_SIZE) ? event_count : MONITOR_BUFFER_SIZE;
  uint8_t start_idx = (event_count > MONITOR_BUFFER_SIZE) ? 
                      (event_count - MONITOR_BUFFER_SIZE) : 0;
  
  for (uint8_t i = 0; i < display_count; i++) {
    uint8_t buf_idx = (start_idx + i) % MONITOR_BUFFER_SIZE;
    midi_event_t* ev = &event_buffer[buf_idx];
    
    char line[64];
    char decoded[32];
    decode_midi_message(ev->data, ev->len, decoded, sizeof(decoded));
    
    // LoopA-style: More compact format - time + decoded message
    uint32_t sec = ev->timestamp_ms / 1000;
    uint32_t ms = ev->timestamp_ms % 1000;
    
    // Show time, port, and decoded message (more readable, less hex clutter)
    snprintf(line, sizeof(line), "%02u.%03u P%u %s",
             sec % 100, ms, ev->port, decoded);
    
    // Use 8x8 font with varying brightness (recent = brighter)
    uint8_t brightness = 12 - (display_count - i - 1);
    if (brightness < 8) brightness = 8;
    ui_gfx_text(0, 14 + i * 9, line, brightness);
  }
  
  // Footer with smaller 5x7 font
  ui_gfx_hline(0, 54, 256, 6);
  ui_gfx_set_font(UI_FONT_5X7);
  ui_gfx_text(0, 56, "B1:PAUSE B2:CLEAR B3:FILT B4:SAVE", 10);
}

/**
 * @brief Handle button press in MIDI monitor
 */
void ui_page_midi_monitor_on_button(uint8_t id, uint8_t pressed) {
  if (!pressed) return;
  
  switch (id) {
    case 1:  // PAUSE/RESUME
      paused = !paused;
      break;
      
    case 2:  // CLEAR buffer
      event_count = 0;
      memset(event_buffer, 0, sizeof(event_buffer));
      break;
      
    case 3:  // FILTER (placeholder - not implemented yet)
      break;
      
    case 4:  // SAVE (placeholder - not implemented yet)
      break;
      
    default:
      break;
  }
}

/**
 * @brief Handle encoder rotation in MIDI monitor
 */
void ui_page_midi_monitor_on_encoder(int8_t delta) {
  // Scroll through event history (not implemented - buffer is small)
  if (delta > 0) {
    scroll_offset++;
  } else if (delta < 0 && scroll_offset > 0) {
    scroll_offset--;
  }
}
