#include "Services/router/router.h"
#include "cmsis_os2.h"
#include <string.h>

// Optional tap hook (e.g., looper recording). Weak so it can be implemented elsewhere.
__attribute__((weak)) void router_tap_hook(uint8_t in_node, const router_msg_t* msg) {
  (void)in_node; (void)msg;
}

// Optional transform hook (e.g., LiveFX, MIDI Monitor). Weak so it can be implemented elsewhere.
__attribute__((weak)) void router_transform_hook(uint8_t out_node, router_msg_t* msg) {
  (void)out_node; (void)msg;
}


#ifndef ROUTER_LABEL_MAX
#define ROUTER_LABEL_MAX 16
#endif

typedef struct {
  uint8_t enabled;
  uint16_t chmask;
  char label[ROUTER_LABEL_MAX];
} route_t;

static route_t g_routes[ROUTER_NUM_NODES][ROUTER_NUM_NODES];
static router_send_fn_t g_send = 0;
static osMutexId_t g_router_mutex;

static inline uint8_t is_channel_voice(uint8_t status) {
  uint8_t hi = status & 0xF0u;
  return (hi >= 0x80u && hi <= 0xE0u);
}

static inline uint16_t msg_channel_bit(const router_msg_t* msg) {
  uint8_t ch = (msg->b0 & 0x0Fu); // 0..15
  return (uint16_t)(1u << ch);
}

void router_init(router_send_fn_t send_cb) {
  g_send = send_cb;
  memset(g_routes, 0, sizeof(g_routes));
  for (uint8_t i=0;i<ROUTER_NUM_NODES;i++) {
    for (uint8_t j=0;j<ROUTER_NUM_NODES;j++) {
      g_routes[i][j].chmask = ROUTER_CHMASK_ALL;
    }
  }
  // Lazy creation: mutex will be created on first use (after scheduler starts)
  g_router_mutex = NULL;
}

// Helper: ensure mutex is created (call before first use)
static inline void ensure_mutex(void) {
  if (g_router_mutex == NULL) {
    const osMutexAttr_t attr = { .name = "router" };
    g_router_mutex = osMutexNew(&attr);
  }
}

void router_set_route(uint8_t in_node, uint8_t out_node, uint8_t enable) {
  if (in_node >= ROUTER_NUM_NODES || out_node >= ROUTER_NUM_NODES) return;
  ensure_mutex();
  if (g_router_mutex) osMutexAcquire(g_router_mutex, osWaitForever);
  g_routes[in_node][out_node].enabled = enable ? 1 : 0;
  if (g_router_mutex) osMutexRelease(g_router_mutex);
}

uint8_t router_get_route(uint8_t in_node, uint8_t out_node) {
  if (in_node >= ROUTER_NUM_NODES || out_node >= ROUTER_NUM_NODES) return 0;
  return g_routes[in_node][out_node].enabled;
}

void router_set_chanmask(uint8_t in_node, uint8_t out_node, uint16_t chmask) {
  if (in_node >= ROUTER_NUM_NODES || out_node >= ROUTER_NUM_NODES) return;
  ensure_mutex();
  if (g_router_mutex) osMutexAcquire(g_router_mutex, osWaitForever);
  g_routes[in_node][out_node].chmask = chmask;
  if (g_router_mutex) osMutexRelease(g_router_mutex);
}

uint16_t router_get_chanmask(uint8_t in_node, uint8_t out_node) {
  if (in_node >= ROUTER_NUM_NODES || out_node >= ROUTER_NUM_NODES) return ROUTER_CHMASK_ALL;
  return g_routes[in_node][out_node].chmask;
}

void router_set_label(uint8_t in_node, uint8_t out_node, const char* label) {
  if (in_node >= ROUTER_NUM_NODES || out_node >= ROUTER_NUM_NODES) return;
  if (!label) label = "";
  ensure_mutex();
  if (g_router_mutex) osMutexAcquire(g_router_mutex, osWaitForever);
  strncpy(g_routes[in_node][out_node].label, label, ROUTER_LABEL_MAX-1);
  g_routes[in_node][out_node].label[ROUTER_LABEL_MAX-1] = '\0';
  if (g_router_mutex) osMutexRelease(g_router_mutex);
}

const char* router_get_label(uint8_t in_node, uint8_t out_node) {
  if (in_node >= ROUTER_NUM_NODES || out_node >= ROUTER_NUM_NODES) return "";
  return g_routes[in_node][out_node].label;
}

void router_process(uint8_t in_node, const router_msg_t* msg) {
  router_tap_hook(in_node, msg);

  if (!msg || in_node >= ROUTER_NUM_NODES) return;
  if (!g_send) return;
  
  // Filter MIOS Studio bootloader protocol SysEx messages (F0 00 00 7E 40 ...)
  // Block bootloader messages (0x40) but allow query messages (0x32) to pass through
  if (msg->type == ROUTER_MSG_SYSEX && msg->data && msg->len >= 5) {
    // Check for MIOS32/Bootloader manufacturer ID: F0 00 00 7E
    if (msg->data[0] == 0xF0 && msg->data[1] == 0x00 && 
        msg->data[2] == 0x00 && msg->data[3] == 0x7E) {
      // Check device ID byte
      uint8_t device_id = msg->data[4];
      // 0x32 = MIOS32 query/response (ALLOW - MIOS Studio needs responses)
      // 0x40 = Bootloader protocol (BLOCK - only for bootloader mode)
      if (device_id == 0x40) {
        // Block bootloader messages from routing (they're only for bootloader mode)
        return;
      }
      // Let 0x32 (query) messages pass through - they need to be handled/responded to
    }
  }

  uint8_t status = msg->b0;
  uint8_t chan_voice = is_channel_voice(status);
  uint16_t bit = chan_voice ? msg_channel_bit(msg) : 0;

  route_t snap[ROUTER_NUM_NODES];

  ensure_mutex();
  if (g_router_mutex) osMutexAcquire(g_router_mutex, osWaitForever);
  for (uint8_t out=0; out<ROUTER_NUM_NODES; out++) snap[out] = g_routes[in_node][out];
  if (g_router_mutex) osMutexRelease(g_router_mutex);

  for (uint8_t out=0; out<ROUTER_NUM_NODES; out++) {
    if (!snap[out].enabled) continue;
    if (chan_voice && ((snap[out].chmask & bit) == 0)) continue;
    
    // CRITICAL FIX: Prevent hardware loopback - don't route DIN_INx back to DIN_OUTx on SAME port
    // Example: DIN_IN1 (node 0) should NOT route to DIN_OUT1 (node 4) to prevent feedback loops
    // Cross-port routing still works: DIN_IN1 → DIN_OUT2 (for MIDI merge/split)
    if (in_node >= ROUTER_NODE_DIN_IN1 && in_node <= ROUTER_NODE_DIN_IN4 &&
        out >= ROUTER_NODE_DIN_OUT1 && out <= ROUTER_NODE_DIN_OUT4) {
      uint8_t in_port = (uint8_t)(in_node - ROUTER_NODE_DIN_IN1);   // 0-3
      uint8_t out_port = (uint8_t)(out - ROUTER_NODE_DIN_OUT1);     // 0-3
      if (in_port == out_port) {
        // Block same-port loopback: DIN_IN1→DIN_OUT1, DIN_IN2→DIN_OUT2, etc.
        // This prevents infinite loops if hardware has MIDI cables connecting output back to input
        continue;
      }
    }
    
    // CRITICAL FIX: Prevent USB loopback - don't route USB_PORTx back to itself
    // USB ports are bidirectional, so routing USB_PORT0→USB_PORT0 would echo messages
    // back to MIOS Studio, causing crashes or infinite loops
    // Example: MIOS Studio sends query → MidiCore routes back → MIOS Studio receives corrupted echo → crash
    if (in_node >= ROUTER_NODE_USB_PORT0 && in_node <= ROUTER_NODE_USB_PORT3 &&
        out >= ROUTER_NODE_USB_PORT0 && out <= ROUTER_NODE_USB_PORT3) {
      if (in_node == out) {
        // Block USB self-loopback: USB_PORT0→USB_PORT0, USB_PORT1→USB_PORT1, etc.
        // This prevents MIOS Studio queries from being echoed back
        continue;
      }
    }
    
    // Create a copy of the message for potential transformation
    router_msg_t transformed_msg = *msg;
    
    // Call transform hook (weak, can be implemented elsewhere for LiveFX, etc.)
    router_transform_hook(out, &transformed_msg);
    
    (void)g_send(out, &transformed_msg);
  }
}
