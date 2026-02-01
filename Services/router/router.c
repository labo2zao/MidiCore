/**
 * @file router.c
 * @brief MIDI Router - MIOS32-inspired implementation
 * 
 * This router is inspired by the MIOS32 MIDI router architecture:
 * - Supports channel voice messages with per-channel filtering
 * - Supports SysEx with "forward once per destination" optimization
 * - Prevents loopback (USB→USB, DIN→same DIN port)
 * - Thread-safe with mutex protection
 * 
 * Reference: https://github.com/midibox/mios32/blob/master/modules/midi_router/midi_router.c
 */

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

/* Router state - starts as NOT ready */
static router_send_fn_t g_send = 0;  /* NULL until router_init() called */
static volatile uint8_t g_router_ready = 0;  /* Flag: 1 = initialized and ready */

/**
 * @brief Check if router is initialized and ready to process messages
 * 
 * USB callbacks can fire during MX_USB_DEVICE_Init() BEFORE router_init()
 * is called. Callers MUST check router_is_ready() before calling router_process()
 * in ISR/callback context to avoid HardFault.
 * 
 * @return 1 if router is ready, 0 if not yet initialized
 */
uint8_t router_is_ready(void) {
  return g_router_ready;
}
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
  // Guard against double init (can happen if called from both main.c and app_init.c)
  static uint8_t s_initialized = 0;
  if (s_initialized && g_send == send_cb) {
    return;  // Already initialized with same callback
  }
  s_initialized = 1;
  
  g_send = send_cb;
  memset(g_routes, 0, sizeof(g_routes));
  for (uint8_t i=0;i<ROUTER_NUM_NODES;i++) {
    for (uint8_t j=0;j<ROUTER_NUM_NODES;j++) {
      g_routes[i][j].chmask = ROUTER_CHMASK_ALL;
    }
  }
  // Lazy creation: mutex will be created on first use (after scheduler starts)
  g_router_mutex = NULL;
  
  // Mark router as ready - MUST be last!
  g_router_ready = 1;
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

/**
 * @brief Get port mask for "forward once" optimization (MIOS32-style)
 * 
 * Returns a bitmask for the given output node, used to track which
 * destinations have already received a SysEx/Realtime message.
 * This prevents duplicate forwarding when multiple routes point to same port.
 * 
 * @param out_node Output node ID
 * @return Bitmask (1 << node) or 0 if invalid
 */
static inline uint16_t router_get_port_mask(uint8_t out_node) {
  if (out_node < ROUTER_NUM_NODES) {
    return (uint16_t)(1u << out_node);
  }
  return 0;
}

/**
 * @brief Check if loopback should be blocked
 * 
 * Prevents:
 * - DIN_INx → DIN_OUTx on same port (hardware loopback)
 * - USB_PORTx → USB_PORTx (bidirectional loopback)
 * 
 * @param in_node Input node
 * @param out_node Output node
 * @return 1 if loopback should be blocked, 0 otherwise
 */
static inline uint8_t router_is_loopback(uint8_t in_node, uint8_t out_node) {
  // DIN loopback: DIN_IN1→DIN_OUT1, etc.
  if (in_node >= ROUTER_NODE_DIN_IN1 && in_node <= ROUTER_NODE_DIN_IN4 &&
      out_node >= ROUTER_NODE_DIN_OUT1 && out_node <= ROUTER_NODE_DIN_OUT4) {
    uint8_t in_port = (uint8_t)(in_node - ROUTER_NODE_DIN_IN1);
    uint8_t out_port = (uint8_t)(out_node - ROUTER_NODE_DIN_OUT1);
    if (in_port == out_port) return 1;
  }
  
  // USB loopback: USB_PORT0→USB_PORT0, etc.
  if (in_node >= ROUTER_NODE_USB_PORT0 && in_node <= ROUTER_NODE_USB_PORT3 &&
      out_node >= ROUTER_NODE_USB_PORT0 && out_node <= ROUTER_NODE_USB_PORT3) {
    if (in_node == out_node) return 1;
  }
  
  return 0;
}

void router_process(uint8_t in_node, const router_msg_t* msg) {
  router_tap_hook(in_node, msg);

  if (!msg || in_node >= ROUTER_NUM_NODES) return;
  if (!g_send) return;
  
  /* =========================================================================
   * MIOS32-style: Filter MidiCore/MIOS protocol SysEx from routing
   * These are device management messages, NOT music data
   * ========================================================================= */
  if (msg->type == ROUTER_MSG_SYSEX && msg->data && msg->len >= 5) {
    // Check for MidiCore/MIOS32 manufacturer ID: F0 00 00 7E
    if (msg->data[0] == 0xF0 && msg->data[1] == 0x00 && 
        msg->data[2] == 0x00 && msg->data[3] == 0x7E) {
      uint8_t device_id = msg->data[4];
      // Block ALL MidiCore/MIOS protocol messages from routing:
      // 0x32 = MidiCore query/response
      // 0x40 = MIOS32 bootloader protocol
      if (device_id == 0x32 || device_id == 0x40) {
        return;  // Don't route - handled internally
      }
    }
  }

  /* Determine message type for routing logic */
  uint8_t status = msg->b0;
  uint8_t is_chan_voice = is_channel_voice(status);
  uint8_t is_sysex = (msg->type == ROUTER_MSG_SYSEX);
  uint8_t is_realtime = (status >= 0xF8);  // Clock, Start, Stop, Continue, etc.
  
  /* Channel mask bit for channel voice messages */
  uint16_t chan_bit = is_chan_voice ? msg_channel_bit(msg) : 0;

  /* Take snapshot of routes under mutex protection */
  route_t snap[ROUTER_NUM_NODES];
  ensure_mutex();
  if (g_router_mutex) osMutexAcquire(g_router_mutex, osWaitForever);
  for (uint8_t out = 0; out < ROUTER_NUM_NODES; out++) {
    snap[out] = g_routes[in_node][out];
  }
  if (g_router_mutex) osMutexRelease(g_router_mutex);

  /* =========================================================================
   * MIOS32-style: "Forward once per destination" optimization
   * 
   * For SysEx and Realtime messages, use a bitmask to track which destinations
   * have already received the message. This prevents duplicate forwarding
   * when multiple routing rules point to the same output port.
   * 
   * Reference: MIOS32 midi_router.c uses sysex_dst_fwd_done bitmask
   * ========================================================================= */
  uint16_t dst_fwd_done = 0;  // Bitmask of already-forwarded destinations

  for (uint8_t out = 0; out < ROUTER_NUM_NODES; out++) {
    /* Skip if route not enabled */
    if (!snap[out].enabled) continue;
    
    /* Skip if channel voice message and channel not in mask */
    if (is_chan_voice && ((snap[out].chmask & chan_bit) == 0)) continue;
    
    /* Skip loopback routes */
    if (router_is_loopback(in_node, out)) continue;
    
    /* =========================================================================
     * MIOS32-style: SysEx and Realtime - forward only ONCE per destination
     * 
     * This is critical for:
     * 1. SysEx: Prevents duplicate patch dumps when multiple routes exist
     * 2. Realtime (Clock): Prevents tempo doubling when merging
     * ========================================================================= */
    if (is_sysex || is_realtime) {
      uint16_t mask = router_get_port_mask(out);
      if (mask && (dst_fwd_done & mask)) {
        continue;  // Already forwarded to this destination
      }
      dst_fwd_done |= mask;  // Mark as forwarded
    }
    
    /* Create a copy of the message for potential transformation */
    router_msg_t transformed_msg = *msg;
    
    /* Call transform hook (weak, can be implemented elsewhere for LiveFX, etc.) */
    router_transform_hook(out, &transformed_msg);
    
    /* Send to destination */
    (void)g_send(out, &transformed_msg);
  }
}
