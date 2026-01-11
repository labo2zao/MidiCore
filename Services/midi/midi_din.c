// MIDI DIN input/output service.
//
// Notes:
// - Implements a small state machine per port with running-status support.
// - SysEx is forwarded in chunks via ROUTER_MSG_SYSEX.
// - Uses the HAL UART backend (interrupt RX ring buffers).

#include "midi_din.h"

#include <string.h>

#include "Hal/uart_midi/hal_uart_midi.h"
#include "Services/router/router.h"

#ifndef MIDI_DIN_SYSEX_CHUNK_SIZE
#define MIDI_DIN_SYSEX_CHUNK_SIZE 64
#endif

typedef struct {
  // Parser state
  uint8_t running_status;
  uint8_t msg[3];
  uint8_t idx;
  uint8_t expected;
  uint8_t in_sysex;

  // SysEx chunk buffer
  uint8_t sysex_buf[MIDI_DIN_SYSEX_CHUNK_SIZE];
  uint16_t sysex_len;

  // Debug stats
  midi_din_stats_t stats;
} midi_din_port_ctx_t;

static midi_din_port_ctx_t g_ctx[MIDI_DIN_PORTS];

static uint8_t midi_expected_len(uint8_t status)
{
  if (status < 0x80)
    return 0;

  if (status < 0xF0) {
    switch (status & 0xF0) {
      case 0xC0: // Program Change
      case 0xD0: // Channel Pressure
        return 2;
      default:
        return 3;
    }
  }

  // System Common + Realtime
  switch (status) {
    case 0xF0: // SysEx start (variable)
      return 0;
    case 0xF1: // MTC Quarter Frame
      return 2;
    case 0xF2: // Song Position
      return 3;
    case 0xF3: // Song Select
      return 2;
    case 0xF6: // Tune Request
      return 1;
    case 0xF7: // End of SysEx
      return 1;

    // Realtime messages (can appear anytime)
    case 0xF8:
    case 0xF9:
    case 0xFA:
    case 0xFB:
    case 0xFC:
    case 0xFD:
    case 0xFE:
    case 0xFF:
      return 1;
    default:
      return 1;
  }
}

static void dispatch_short_msg(uint8_t port, const uint8_t* bytes, uint8_t len)
{
  router_msg_t msg;
  memset(&msg, 0, sizeof(msg));
  msg.src = (router_node_t)(ROUTER_NODE_DIN_IN1 + port);

  if (len == 1) {
    msg.type = ROUTER_MSG_1B;
    msg.data.msg1.b0 = bytes[0];
  } else if (len == 2) {
    msg.type = ROUTER_MSG_2B;
    msg.data.msg2.b0 = bytes[0];
    msg.data.msg2.b1 = bytes[1];
  } else { // 3
    msg.type = ROUTER_MSG_3B;
    msg.data.msg3.b0 = bytes[0];
    msg.data.msg3.b1 = bytes[1];
    msg.data.msg3.b2 = bytes[2];
  }

  // Stats
  midi_din_port_ctx_t* c = &g_ctx[port];
  c->stats.rx_msgs++;
  c->stats.last_len = len;
  memset(c->stats.last_bytes, 0, sizeof(c->stats.last_bytes));
  memcpy(c->stats.last_bytes, bytes, len);

  router_dispatch(&msg);
}

static void dispatch_sysex_chunk(uint8_t port, const uint8_t* data, uint16_t len, uint8_t is_last)
{
  if (len == 0)
    return;

  router_msg_t msg;
  memset(&msg, 0, sizeof(msg));
  msg.src = (router_node_t)(ROUTER_NODE_DIN_IN1 + port);
  msg.type = ROUTER_MSG_SYSEX;
  msg.data.sysex.data = data;
  msg.data.sysex.len = len;
  msg.data.sysex.is_last = is_last;

  midi_din_port_ctx_t* c = &g_ctx[port];
  c->stats.rx_sysex_chunks++;
  c->stats.last_len = 0;
  memset(c->stats.last_bytes, 0, sizeof(c->stats.last_bytes));

  router_dispatch(&msg);
}

static void sysex_reset(midi_din_port_ctx_t* c)
{
  c->in_sysex = 0;
  c->sysex_len = 0;
}

static void sysex_push_byte(uint8_t port, midi_din_port_ctx_t* c, uint8_t b)
{
  if (c->sysex_len < MIDI_DIN_SYSEX_CHUNK_SIZE) {
    c->sysex_buf[c->sysex_len++] = b;
  } else {
    // Flush full chunk, then restart.
    dispatch_sysex_chunk(port, c->sysex_buf, c->sysex_len, 0);
    c->sysex_len = 0;
    c->sysex_buf[c->sysex_len++] = b;
  }
}

static void process_byte(uint8_t port, uint8_t b)
{
  midi_din_port_ctx_t* c = &g_ctx[port];
  c->stats.rx_bytes++;

  // Realtime messages can occur anywhere and should be dispatched immediately.
  if (b >= 0xF8) {
    dispatch_short_msg(port, &b, 1);
    return;
  }

  // SysEx handling
  if (c->in_sysex) {
    sysex_push_byte(port, c, b);
    if (b == 0xF7) {
      // End of SysEx, flush remaining and reset.
      dispatch_sysex_chunk(port, c->sysex_buf, c->sysex_len, 1);
      sysex_reset(c);
    }
    return;
  }

  if (b & 0x80) {
    // Status byte
    uint8_t exp = midi_expected_len(b);

    if (b == 0xF0) {
      // Start SysEx
      sysex_reset(c);
      c->in_sysex = 1;
      sysex_push_byte(port, c, b);
      return;
    }

    if (exp == 1) {
      // 1-byte system message
      dispatch_short_msg(port, &b, 1);
      // Running status is cleared by System Common messages (0xF0..0xF7)
      if (b >= 0xF0 && b <= 0xF7)
        c->running_status = 0;
      c->idx = 0;
      c->expected = 0;
      return;
    }

    // Channel voice and some system common messages
    c->msg[0] = b;
    c->idx = 1;
    c->expected = exp;

    if (b < 0xF0)
      c->running_status = b;
    else
      c->running_status = 0;

    return;
  }

  // Data byte
  if (c->expected == 0) {
    // Use running status if available (channel voice)
    if (c->running_status) {
      c->msg[0] = c->running_status;
      c->idx = 1;
      c->expected = midi_expected_len(c->running_status);
    } else {
      // Stray data byte: ignore
      c->stats.rx_stray_data++;
      return;
    }
  }

  if (c->idx < sizeof(c->msg))
    c->msg[c->idx++] = b;

  if (c->expected && c->idx >= c->expected) {
    dispatch_short_msg(port, c->msg, c->expected);
    c->idx = 0;
    c->expected = 0;
  }
}

void midi_din_init(void)
{
  memset(g_ctx, 0, sizeof(g_ctx));
  hal_uart_midi_init();
}

void midi_din_tick(void)
{
  for (uint8_t port = 0; port < MIDI_DIN_PORTS; ++port) {
    while (hal_uart_midi_rx_available(port)) {
      uint8_t b = hal_uart_midi_read_byte(port);
      process_byte(port, b);
    }
  }
}

void midi_din_send(uint8_t port, const uint8_t* data, uint16_t len)
{
  if (port >= MIDI_DIN_PORTS || data == NULL || len == 0)
    return;

  // Stats
  g_ctx[port].stats.tx_bytes += len;

  (void)hal_uart_midi_send_bytes(port, data, len);
}

void midi_din_get_stats(uint8_t port, midi_din_stats_t* out)
{
  if (!out) return;
  memset(out, 0, sizeof(*out));
  if (port >= MIDI_DIN_PORTS) return;
  *out = g_ctx[port].stats;
  out->rx_drops = hal_uart_midi_rx_drops(port);
}
