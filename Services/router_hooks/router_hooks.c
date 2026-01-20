/**
 * @file router_hooks.c
 * @brief Router integration hooks for LiveFX, MIDI Monitor, and SysEx capture
 */

#include "Services/router_hooks/router_hooks.h"
#include "Services/router/router.h"
#include "Services/livefx/livefx.h"
#include "Services/ui/ui_page_midi_monitor.h"
#include "Services/ui/ui_page_sysex.h"
#include "cmsis_os2.h"
#include <string.h>

// Track mapping for output nodes (which track's LiveFX to apply)
static uint8_t g_track_map[16] = {0};  // Default to track 0
static uint32_t g_timestamp_ms = 0;

/**
 * @brief Initialize router hooks
 */
void router_hooks_init(void) {
  memset(g_track_map, 0, sizeof(g_track_map));
}

/**
 * @brief Set track mapping for output nodes
 */
void router_hooks_set_track_map(uint8_t out_node, uint8_t track) {
  if (out_node < 16 && track < 4) {
    g_track_map[out_node] = track;
  }
}

/**
 * @brief Get track mapping for output node
 */
uint8_t router_hooks_get_track_map(uint8_t out_node) {
  if (out_node < 16) return g_track_map[out_node];
  return 0;
}

/**
 * @brief Router tap hook - called for incoming messages (before routing)
 * Captures messages for MIDI Monitor, SysEx viewer, and Looper
 */
void router_tap_hook(uint8_t in_node, const router_msg_t* msg) {
  if (!msg) return;
  
  // Update timestamp (approximate)
  g_timestamp_ms = osKernelGetTickCount();
  
  // Forward to looper (for recording)
  #if MODULE_ENABLE_LOOPER
  extern void looper_on_router_msg(uint8_t in_node, const router_msg_t* msg);
  looper_on_router_msg(in_node, msg);
  #endif
  
  // Capture for MIDI Monitor
  if (msg->type == ROUTER_MSG_SYSEX) {
    // SysEx message - capture for SysEx viewer
    if (msg->data && msg->len > 0) {
      ui_sysex_capture(msg->data, msg->len);
      
      // Also show in MIDI Monitor (first few bytes)
      uint8_t preview[3] = {0xF0, 0, 0};
      uint8_t preview_len = (msg->len >= 2) ? 3 : 1;
      if (msg->len >= 2) {
        preview[1] = msg->data[1];
        preview[2] = (msg->len >= 3) ? msg->data[2] : 0;
      }
      ui_midi_monitor_capture(in_node, preview, preview_len, g_timestamp_ms);
    }
  } else {
    // Standard MIDI message
    uint8_t data[3];
    uint8_t len = msg->type;
    data[0] = msg->b0;
    data[1] = msg->b1;
    data[2] = msg->b2;
    
    ui_midi_monitor_capture(in_node, data, len, g_timestamp_ms);
  }
}

/**
 * @brief Router transform hook - called for outgoing messages (after routing decision)
 * Applies LiveFX transformations based on track mapping
 */
void router_transform_hook(uint8_t out_node, router_msg_t* msg) {
  if (!msg) return;
  
  // Get the track for this output node
  uint8_t track = router_hooks_get_track_map(out_node);
  
  // Apply LiveFX if enabled for this track
  if (livefx_get_enabled(track)) {
    livefx_apply(track, msg);
  }
}
