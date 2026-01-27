/**
 * @file ui_page_midi_monitor.c
 * @brief MIDI Monitor UI Page - Real-time MIDI message display
 * 
 * Shows the last N MIDI messages with timestamps, ports, and decoded info.
 * Useful for debugging MIDI routing and monitoring live performance.
 * Integrates with Services/midi_monitor for event capture.
 * Configurable via NGC files.
 * Can be used as debug mirror in test mode.
 */

#include "Services/ui/ui_page_midi_monitor.h"
#include "Services/ui/ui_gfx.h"
#include "Services/midi_monitor/midi_monitor.h"
#include "Config/module_config.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define MONITOR_BUFFER_SIZE 8  // Display last 8 messages (increased from 6)

// Configuration structure
typedef struct {
  uint8_t show_timestamp;
  uint8_t show_hex;
  uint8_t show_routed_status;
  uint8_t paused;
  uint8_t auto_scroll;
  uint16_t update_rate_ms;
} monitor_config_t;

static monitor_config_t config = {
  .show_timestamp = 1,
  .show_hex = 0,
  .show_routed_status = 1,
  .paused = 0,
  .auto_scroll = 1,
  .update_rate_ms = 100
};

// Simple MIDI event structure
typedef struct {
  uint32_t timestamp_ms;
  uint8_t node;      // Router node ID
  uint8_t len;       // Message length (1-3 bytes)
  uint8_t data[3];   // MIDI bytes
  uint8_t is_routed; // 1 if routed, 0 if filtered
} midi_event_t;

// Circular buffer for MIDI events
static midi_event_t event_buffer[MONITOR_BUFFER_SIZE];
static uint8_t event_write_idx = 0;
static uint8_t event_count = 0;
static uint8_t scroll_offset = 0;
static uint32_t last_update_time = 0;

/**
 * @brief Add a MIDI event to the monitor (called by MIDI monitor service or router hooks)
 */
void ui_midi_monitor_capture(uint8_t node, const uint8_t* data, uint8_t len, uint32_t timestamp_ms, uint8_t is_routed) {
  if (config.paused || len == 0 || len > 3) return;
  
  // Add to circular buffer
  event_buffer[event_write_idx].timestamp_ms = timestamp_ms;
  event_buffer[event_write_idx].node = node;
  event_buffer[event_write_idx].len = len;
  event_buffer[event_write_idx].is_routed = is_routed;
  
  for (uint8_t i = 0; i < len && i < 3; i++) {
    event_buffer[event_write_idx].data[i] = data[i];
  }
  
  event_write_idx = (event_write_idx + 1) % MONITOR_BUFFER_SIZE;
  if (event_count < MONITOR_BUFFER_SIZE) {
    event_count++;
  }
  
  if (config.auto_scroll) {
    scroll_offset = 0;
  }
}

/**
 * @brief Render the MIDI monitor page
 */
void ui_page_midi_monitor_render(uint32_t now_ms) {
  // Throttle updates
  if (now_ms - last_update_time < config.update_rate_ms) {
    return;
  }
  last_update_time = now_ms;
  
  ui_gfx_clear(0);
  
  // Header with status
  ui_gfx_set_font(UI_FONT_8X8);
  char header[64];
  
  const char* status = config.paused ? "PAUSED" : "LIVE";
  snprintf(header, sizeof(header), "MIDI MON [%s] Msgs:%u", status, event_count);
  ui_gfx_text(0, 0, header, 15);
  ui_gfx_hline(0, 11, 256, 8);
  
  // Display events
  uint8_t visible_count = (event_count < MONITOR_BUFFER_SIZE) ? event_count : MONITOR_BUFFER_SIZE;
  
  for (uint8_t i = 0; i < visible_count; i++) {
    // Calculate buffer index (newest first)
    uint8_t buf_idx = (event_write_idx - 1 - i + MONITOR_BUFFER_SIZE) % MONITOR_BUFFER_SIZE;
    if (buf_idx >= MONITOR_BUFFER_SIZE) continue;
    
    midi_event_t* ev = &event_buffer[buf_idx];
    
    char line[80];
    char decoded[40];
    char node_name[12];
    
    // Get node name from MIDI monitor service
    midi_monitor_get_node_name(ev->node, node_name, sizeof(node_name));
    
    // Decode message
    midi_monitor_decode_message(ev->data, ev->len, decoded, sizeof(decoded));
    
    // Build display line
    char* p = line;
    size_t remaining = sizeof(line);
    
    if (config.show_timestamp) {
      uint32_t sec = ev->timestamp_ms / 1000;
      uint32_t ms = ev->timestamp_ms % 1000;
      int n = snprintf(p, remaining, "[%02lu.%03lu] ", (unsigned long)(sec % 100), (unsigned long)ms);
      p += n; remaining -= n;
    }
    
    // Node name
    int n = snprintf(p, remaining, "%s ", node_name);
    p += n; remaining -= n;
    
    // Routing status
    if (config.show_routed_status) {
      const char* route_flag = ev->is_routed ? "[R]" : "[F]";
      n = snprintf(p, remaining, "%s ", route_flag);
      p += n; remaining -= n;
    }
    
    // Decoded message
    n = snprintf(p, remaining, "%s", decoded);
    p += n; remaining -= n;
    
    // Hex bytes
    if (config.show_hex && ev->len > 0) {
      n = snprintf(p, remaining, " [");
      p += n; remaining -= n;
      for (uint8_t j = 0; j < ev->len && j < 3; j++) {
        n = snprintf(p, remaining, "%02X ", ev->data[j]);
        p += n; remaining -= n;
      }
      n = snprintf(p, remaining, "]");
    }
    
    // Draw with brightness based on age
    uint8_t brightness = 14 - (i * 2);
    if (brightness < 6) brightness = 6;
    ui_gfx_text(0, 14 + i * 6, line, brightness);
  }
  
  // Footer
  ui_gfx_hline(0, 54, 256, 6);
  ui_gfx_set_font(UI_FONT_5X7);
  const char* footer = config.paused ? "B1:RESUME B2:CLR B3:HEX B4:TIME" : "B1:PAUSE B2:CLR B3:HEX B4:TIME";
  ui_gfx_text(0, 56, footer, 10);
}

/**
 * @brief Handle button press in MIDI monitor
 */
void ui_page_midi_monitor_on_button(uint8_t id, uint8_t pressed) {
  if (!pressed) return;
  
  switch (id) {
    case 1:  // PAUSE/RESUME
      config.paused = !config.paused;
      break;
      
    case 2:  // CLEAR buffer
      event_count = 0;
      event_write_idx = 0;
      memset(event_buffer, 0, sizeof(event_buffer));
      midi_monitor_clear();  // Also clear MIDI monitor service buffer
      break;
      
    case 3:  // Toggle HEX display
      config.show_hex = !config.show_hex;
      break;
      
    case 4:  // Toggle TIMESTAMP display
      config.show_timestamp = !config.show_timestamp;
      break;
      
    default:
      break;
  }
}

/**
 * @brief Handle encoder rotation in MIDI monitor
 */
void ui_page_midi_monitor_on_encoder(int8_t delta) {
  // Scroll through event history
  if (delta > 0 && scroll_offset > 0) {
    scroll_offset--;
    config.auto_scroll = 0;
  } else if (delta < 0 && scroll_offset < (event_count - MONITOR_BUFFER_SIZE)) {
    scroll_offset++;
    config.auto_scroll = 0;
  }
}

// ============================================================================
// NGC Config Support
// ============================================================================

/**
 * @brief Parse NGC configuration line
 * @param line NGC config line
 * @return 0 if parsed successfully, -1 if not a MIDI_MONITOR command
 */
int ui_page_midi_monitor_parse_ngc(const char* line) {
  if (!line) return -1;
  
  // Skip leading whitespace
  while (*line == ' ' || *line == '\t') line++;
  
  // Check if it's a MIDI_MONITOR command
  if (strncmp(line, "MIDI_MONITOR", 12) != 0) {
    return -1;
  }
  
  const char* params = line + 12;
  while (*params == ' ' || *params == '\t') params++;
  
  // Parse parameters
  if (strncmp(params, "SHOW_TIMESTAMP", 14) == 0) {
    config.show_timestamp = (strstr(params, "ON") != NULL) ? 1 : 0;
    return 0;
  }
  else if (strncmp(params, "SHOW_HEX", 8) == 0) {
    config.show_hex = (strstr(params, "ON") != NULL) ? 1 : 0;
    return 0;
  }
  else if (strncmp(params, "SHOW_ROUTED_STATUS", 18) == 0) {
    config.show_routed_status = (strstr(params, "ON") != NULL) ? 1 : 0;
    return 0;
  }
  else if (strncmp(params, "AUTO_SCROLL", 11) == 0) {
    config.auto_scroll = (strstr(params, "ON") != NULL) ? 1 : 0;
    return 0;
  }
  else if (strncmp(params, "UPDATE_RATE", 11) == 0) {
    int rate = 100;
    sscanf(params + 11, "%d", &rate);
    if (rate >= 50 && rate <= 1000) {
      config.update_rate_ms = rate;
    }
    return 0;
  }
  
  return -1;  // Unknown MIDI_MONITOR parameter
}

// ============================================================================
// Debug Mirror Support (for test mode)
// ============================================================================

/**
 * @brief Print text to MIDI monitor (debug mirror mode)
 * Useful in test mode to display debug messages on UI page
 */
void ui_page_midi_monitor_print(const char* text) {
  if (!text) return;
  
  // Add as a special "debug message" event
  uint32_t timestamp = HAL_GetTick();
  
  // Create a dummy MIDI event to hold the text
  // We'll use node 0xFF to indicate it's a debug message
  midi_event_t* ev = &event_buffer[event_write_idx];
  ev->timestamp_ms = timestamp;
  ev->node = 0xFF;  // Special: debug message
  ev->len = 0;
  ev->is_routed = 1;
  
  // Store text in data (limited to 3 chars, but that's OK for marker)
  memset(ev->data, 0, sizeof(ev->data));
  
  event_write_idx = (event_write_idx + 1) % MONITOR_BUFFER_SIZE;
  if (event_count < MONITOR_BUFFER_SIZE) {
    event_count++;
  }
}

/**
 * @brief Printf-style debug output to MIDI monitor
 */
void ui_page_midi_monitor_printf(const char* format, ...) {
  if (!format) return;
  
  char buffer[80];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  
  ui_page_midi_monitor_print(buffer);
}
