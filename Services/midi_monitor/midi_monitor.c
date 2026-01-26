/**
 * @file midi_monitor.c
 * @brief MIDI Monitor Service Implementation
 * 
 * Inspired by MIOS32 MIDI Monitor functionality.
 * Includes built-in OLED mirroring support for production and test modes.
 */

#include "midi_monitor.h"
#include "Services/router/router.h"
#include "App/tests/test_debug.h"
#include "Config/module_config.h"

// OLED mirroring support
#if MODULE_ENABLE_OLED && MODULE_ENABLE_UI
  #if defined(MODULE_TEST_MODE) || defined(MODULE_TEST_MIDI_DIN) || defined(MODULE_TEST_ROUTER)
    // Test mode: Use test OLED mirror
    #include "App/tests/test_oled_mirror.h"
    #define MIDI_MON_HAS_OLED_MIRROR 1
    #define MIDI_MON_USE_TEST_MIRROR 1
  #else
    // Production mode: Mirror to UI pages via label system
    // TODO: Implement UI label mirroring when UI label system is ready
    #define MIDI_MON_HAS_OLED_MIRROR 0
    #define MIDI_MON_USE_TEST_MIRROR 0
  #endif
#else
  #define MIDI_MON_HAS_OLED_MIRROR 0
  #define MIDI_MON_USE_TEST_MIRROR 0
#endif

#include <stdio.h>
#include <string.h>

// ============================================================================
// Private Data
// ============================================================================

static midi_monitor_event_t event_buffer[MIDI_MONITOR_BUFFER_SIZE];
static volatile uint16_t event_head = 0;
static volatile uint16_t event_tail = 0;
static volatile uint8_t buffer_full = 0;

static midi_monitor_stats_t stats;
static midi_monitor_config_t config;

#if MIDI_MON_HAS_OLED_MIRROR
static uint8_t oled_mirror_enabled = 0;
static uint8_t oled_mirror_initialized = 0;
#endif

// Ensure buffer size is power of 2 for efficient wrapping
_Static_assert((MIDI_MONITOR_BUFFER_SIZE & (MIDI_MONITOR_BUFFER_SIZE - 1)) == 0,
               "MIDI_MONITOR_BUFFER_SIZE must be power of 2");

// ============================================================================
// Private Functions
// ============================================================================

static inline uint16_t buffer_next(uint16_t idx)
{
  return (idx + 1) & (MIDI_MONITOR_BUFFER_SIZE - 1);
}

static inline uint16_t buffer_count(void)
{
  if (buffer_full) return MIDI_MONITOR_BUFFER_SIZE;
  if (event_head >= event_tail) return event_head - event_tail;
  return MIDI_MONITOR_BUFFER_SIZE - (event_tail - event_head);
}

static uint8_t message_passes_filter(uint8_t node, const uint8_t* data, uint8_t len)
{
  // Check node filter
  if (config.filter_node != 0xFF && config.filter_node != node)
    return 0;

  if (len == 0) return 0;

  uint8_t status = data[0];

  // Check channel filter (for channel messages)
  if (status < 0xF0) {
    uint8_t channel = status & 0x0F;
    if (config.filter_channel != 0xFF && config.filter_channel != channel)
      return 0;
  }

  // Check message type filter
  if (config.filter_msg_type != MIDI_MON_MSG_ALL) {
    uint8_t msg_type = status & 0xF0;
    
    // Realtime messages (0xF8-0xFF)
    if (status >= 0xF8) {
      if (!config.show_realtime) return 0;
      if (config.filter_msg_type != MIDI_MON_MSG_REALTIME) return 0;
    }
    // SysEx (0xF0)
    else if (status == 0xF0) {
      if (!config.show_sysex) return 0;
      if (config.filter_msg_type != MIDI_MON_MSG_SYSEX) return 0;
    }
    // System Common (0xF1-0xF7 except 0xF0)
    else if (status >= 0xF0 && status < 0xF8) {
      if (config.filter_msg_type != MIDI_MON_MSG_SYSTEM_COMMON) return 0;
    }
    // Channel messages
    else {
      if (msg_type != config.filter_msg_type) return 0;
    }
  }

  return 1;
}

static void print_to_uart(uint8_t node, const uint8_t* data, uint8_t len, uint32_t timestamp_ms, uint8_t is_routed)
{
  if (!config.uart_output) return;

  char node_name[16];
  char msg_decode[64];
  
  midi_monitor_get_node_name(node, node_name, sizeof(node_name));
  midi_monitor_decode_message(data, len, msg_decode, sizeof(msg_decode));

  // Format: [timestamp] NODE >> Message [ROUTED/FILTERED] | Raw bytes
  const char* route_flag = is_routed ? "[ROUTED]" : "[FILTERED]";
  dbg_printf("[%lu] %s >> %s %s | ", timestamp_ms, node_name, msg_decode, route_flag);
  
  for (uint8_t i = 0; i < len && i < 16; i++) {
    dbg_printf("%02X ", data[i]);
  }
  if (len > 16) {
    dbg_printf("... (%u bytes)", len);
  }
  dbg_printf("\r\n");

#if MIDI_MON_HAS_OLED_MIRROR
  // Mirror to OLED if enabled
  if (oled_mirror_enabled && oled_mirror_initialized) {
#if MIDI_MON_USE_TEST_MIRROR
    // Test mode: Use test_oled_mirror
    char oled_line[128];
    snprintf(oled_line, sizeof(oled_line), "[%lu] %s >> %s %s", 
             timestamp_ms, node_name, msg_decode, route_flag);
    oled_mirror_print(oled_line);
#else
    // Production mode: Mirror to UI label system
    // TODO: Call UI label update function when implemented
    // ui_label_set_text("midi_monitor", oled_line);
#endif
  }
#endif
}

// ============================================================================
// Public API Implementation
// ============================================================================

void midi_monitor_init(void)
{
  memset(&event_buffer, 0, sizeof(event_buffer));
  memset(&stats, 0, sizeof(stats));
  
  event_head = 0;
  event_tail = 0;
  buffer_full = 0;

  // Default configuration
  config.enabled = 1;
  config.filter_node = 0xFF;          // All nodes
  config.filter_channel = 0xFF;       // All channels
  config.filter_msg_type = MIDI_MON_MSG_ALL;
  config.show_sysex = 1;
  config.show_realtime = 1;
  config.uart_output = MIDI_MONITOR_ENABLE_UART_OUTPUT;

#if MIDI_MON_HAS_OLED_MIRROR
  // Auto-initialize and enable OLED mirroring when OLED is active
#if MIDI_MON_USE_TEST_MIRROR
  oled_mirror_init();
  oled_mirror_set_enabled(1);
  oled_mirror_initialized = 1;
  oled_mirror_enabled = 1;
#else
  // Production mode: OLED mirror controlled by UI
  oled_mirror_initialized = 1;
  oled_mirror_enabled = 1;  // Can be controlled via midi_monitor_set_oled_output()
#endif
#endif
}

void midi_monitor_capture_short(uint8_t node, const uint8_t* data, uint8_t len, uint32_t timestamp_ms, uint8_t is_routed)
{
  if (!config.enabled || !data || len == 0 || len > 3)
    return;

  // Apply filters
  if (!message_passes_filter(node, data, len))
    return;

  // Update statistics
  stats.total_messages++;
  
  if (len > 0) {
    uint8_t status = data[0];
    uint8_t msg_type = status & 0xF0;
    
    if (msg_type == 0x90 && len == 3 && data[2] > 0) {
      stats.note_on_count++;
    } else if (msg_type == 0x80 || (msg_type == 0x90 && len == 3 && data[2] == 0)) {
      stats.note_off_count++;
    } else if (msg_type == 0xB0) {
      stats.cc_count++;
    } else if (status >= 0xF8) {
      stats.realtime_count++;
    }
  }

  // Print to UART debug if enabled
  print_to_uart(node, data, len, timestamp_ms, is_routed);

  // Add to circular buffer
  if (buffer_full) {
    stats.dropped_messages++;
    return;
  }

  midi_monitor_event_t* event = &event_buffer[event_head];
  event->timestamp_ms = timestamp_ms;
  event->node = node;
  event->len = len;
  event->is_sysex = 0;
  event->is_routed = is_routed;
  event->sysex_total_len = 0;
  memcpy(event->data, data, len);

  event_head = buffer_next(event_head);
  if (event_head == event_tail) {
    buffer_full = 1;
  }
}

void midi_monitor_capture_sysex(uint8_t node, const uint8_t* data, uint16_t len, uint32_t timestamp_ms, uint8_t is_routed)
{
  if (!config.enabled || !data || len == 0)
    return;

  if (!config.show_sysex)
    return;

  // Apply filters (check first byte for F0)
  uint8_t first_byte = 0xF0;
  if (!message_passes_filter(node, &first_byte, 1))
    return;

  // Update statistics
  stats.total_messages++;
  stats.sysex_count++;

  // Print to UART if enabled (show first 16 bytes)
  print_to_uart(node, data, (len > 16) ? 16 : (uint8_t)len, timestamp_ms, is_routed);

  // Add to circular buffer (store first 16 bytes)
  if (buffer_full) {
    stats.dropped_messages++;
    return;
  }

  midi_monitor_event_t* event = &event_buffer[event_head];
  event->timestamp_ms = timestamp_ms;
  event->node = node;
  event->len = (len > 16) ? 16 : (uint8_t)len;
  event->is_sysex = 1;
  event->is_routed = is_routed;
  event->sysex_total_len = len;
  
  memcpy(event->data, data, event->len);
  
  event_head = buffer_next(event_head);
  if (event_head == event_tail) {
    buffer_full = 1;
  }
}

uint16_t midi_monitor_get_count(void)
{
  return buffer_count();
}

uint8_t midi_monitor_get_event(uint16_t index, midi_monitor_event_t* event)
{
  if (!event) return 0;
  
  uint16_t count = buffer_count();
  if (index >= count) return 0;

  // Calculate actual buffer index (oldest event is at tail)
  uint16_t buffer_idx = (event_tail + index) & (MIDI_MONITOR_BUFFER_SIZE - 1);
  
  memcpy(event, &event_buffer[buffer_idx], sizeof(midi_monitor_event_t));
  return 1;
}

void midi_monitor_clear(void)
{
  event_head = 0;
  event_tail = 0;
  buffer_full = 0;
}

void midi_monitor_get_stats(midi_monitor_stats_t* stats_out)
{
  if (stats_out) {
    memcpy(stats_out, &stats, sizeof(midi_monitor_stats_t));
  }
}

void midi_monitor_reset_stats(void)
{
  memset(&stats, 0, sizeof(stats));
}

void midi_monitor_get_config(midi_monitor_config_t* config_out)
{
  if (config_out) {
    memcpy(config_out, &config, sizeof(midi_monitor_config_t));
  }
}

void midi_monitor_set_config(const midi_monitor_config_t* config_in)
{
  if (config_in) {
    memcpy(&config, config_in, sizeof(midi_monitor_config_t));
  }
}

void midi_monitor_set_enabled(uint8_t enabled)
{
  config.enabled = enabled ? 1 : 0;
}

void midi_monitor_set_uart_output(uint8_t enabled)
{
  config.uart_output = enabled ? 1 : 0;
}

#if MIDI_MON_HAS_OLED_MIRROR
/**
 * @brief Enable/disable OLED mirroring
 * @param enabled 1 = enabled, 0 = disabled
 */
void midi_monitor_set_oled_output(uint8_t enabled)
{
  oled_mirror_enabled = enabled ? 1 : 0;
}

/**
 * @brief Check if OLED mirroring is enabled
 * @return 1 if enabled, 0 if disabled
 */
uint8_t midi_monitor_get_oled_output(void)
{
  return oled_mirror_enabled;
}

/**
 * @brief Update OLED display (call periodically in production mode)
 * In test mode, this is handled automatically by test_oled_mirror
 * In production mode, call this every 100ms or so to refresh UI
 */
void midi_monitor_update_oled(void)
{
#if MIDI_MON_USE_TEST_MIRROR
  if (oled_mirror_enabled && oled_mirror_initialized) {
    oled_mirror_update();
  }
#else
  // Production mode: Trigger UI label refresh
  // TODO: Call UI update function when implemented
  // ui_label_update("midi_monitor");
#endif
}
#endif

int midi_monitor_decode_message(const uint8_t* data, uint8_t len, char* out, size_t out_size)
{
  if (!data || !out || out_size == 0 || len == 0) {
    if (out && out_size > 0) out[0] = '\0';
    return 0;
  }

  uint8_t status = data[0];
  uint8_t msg_type = status & 0xF0;
  uint8_t channel = (status & 0x0F) + 1;  // 1-16

  // Realtime messages (0xF8-0xFF)
  if (status >= 0xF8) {
    const char* rt_names[] = {
      "Clock", "Tick", "Start", "Continue", "Stop", "?", "ActiveSense", "Reset"
    };
    uint8_t idx = status - 0xF8;
    return snprintf(out, out_size, "RT:%s", (idx < 8) ? rt_names[idx] : "Unknown");
  }

  // SysEx
  if (status == 0xF0) {
    if (len >= 5 && data[1] == 0x00 && data[2] == 0x00 && data[3] == 0x7E) {
      // MIOS32/Bootloader protocol
      uint8_t dev_id = data[4];
      if (dev_id == 0x32) return snprintf(out, out_size, "SysEx:MIOS32");
      if (dev_id == 0x40) return snprintf(out, out_size, "SysEx:Bootloader");
    }
    return snprintf(out, out_size, "SysEx (%u bytes)", len);
  }

  // System Common (0xF1-0xF7)
  if (status >= 0xF0 && status < 0xF8) {
    const char* sc_names[] = {
      "SysEx", "MTC", "SongPos", "SongSel", "?", "?", "TuneReq", "EOX"
    };
    uint8_t idx = status - 0xF0;
    return snprintf(out, out_size, "Sys:%s", (idx < 8) ? sc_names[idx] : "Unknown");
  }

  // Channel messages
  switch (msg_type) {
    case 0x80:  // Note Off
      if (len >= 3) {
        char note_name[4];
        midi_monitor_note_to_name(data[1], note_name);
        return snprintf(out, out_size, "NoteOff Ch:%u %s(%u) Vel:%u", 
                       channel, note_name, data[1], data[2]);
      }
      break;

    case 0x90:  // Note On
      if (len >= 3) {
        char note_name[4];
        midi_monitor_note_to_name(data[1], note_name);
        if (data[2] == 0) {
          return snprintf(out, out_size, "NoteOff Ch:%u %s(%u) Vel:0", 
                         channel, note_name, data[1]);
        }
        return snprintf(out, out_size, "NoteOn Ch:%u %s(%u) Vel:%u", 
                       channel, note_name, data[1], data[2]);
      }
      break;

    case 0xA0:  // Poly Pressure
      if (len >= 3) {
        return snprintf(out, out_size, "PolyPress Ch:%u Note:%u Val:%u", 
                       channel, data[1], data[2]);
      }
      break;

    case 0xB0:  // Control Change
      if (len >= 3) {
        return snprintf(out, out_size, "CC Ch:%u #%u=%u", 
                       channel, data[1], data[2]);
      }
      break;

    case 0xC0:  // Program Change
      if (len >= 2) {
        return snprintf(out, out_size, "ProgChg Ch:%u Prog:%u", 
                       channel, data[1]);
      }
      break;

    case 0xD0:  // Channel Pressure
      if (len >= 2) {
        return snprintf(out, out_size, "ChanPress Ch:%u Val:%u", 
                       channel, data[1]);
      }
      break;

    case 0xE0:  // Pitch Bend
      if (len >= 3) {
        int16_t bend = ((int16_t)data[2] << 7) | data[1];
        bend -= 8192;
        return snprintf(out, out_size, "PitchBend Ch:%u %+d", 
                       channel, bend);
      }
      break;
  }

  // Fallback: show raw bytes
  int written = snprintf(out, out_size, "Raw:");
  for (uint8_t i = 0; i < len && i < 3 && written < (int)out_size - 4; i++) {
    written += snprintf(out + written, out_size - written, " %02X", data[i]);
  }
  return written;
}

int midi_monitor_get_node_name(uint8_t node, char* out, size_t out_size)
{
  if (!out || out_size == 0) return 0;

  // Router node names (match Config/router_config.h)
  const char* names[] = {
    "DIN_IN1", "DIN_IN2", "DIN_IN3", "DIN_IN4",        // 0-3
    "DIN_OUT1", "DIN_OUT2", "DIN_OUT3", "DIN_OUT4",    // 4-7
    "USB_DEV0", "USB_DEV1", "USB_DEV2", "USB_DEV3",    // 8-11
    "USB_HOST_IN", "USB_HOST_OUT",                      // 12-13
    "LOOPER", "KEYS"                                     // 14-15
  };

  if (node < sizeof(names) / sizeof(names[0])) {
    return snprintf(out, out_size, "%s", names[node]);
  }

  return snprintf(out, out_size, "NODE_%u", node);
}

void midi_monitor_note_to_name(uint8_t note, char* out)
{
  if (!out) return;

  const char* note_names[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
  };

  uint8_t octave = (note / 12) - 1;  // C4 = MIDI note 60
  uint8_t note_idx = note % 12;

  snprintf(out, 4, "%s%u", note_names[note_idx], octave);
}
