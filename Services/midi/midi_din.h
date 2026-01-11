#pragma once

#include <stdint.h>

#include "Services/router/router.h"

#ifndef MIDI_DIN_PORTS
#define MIDI_DIN_PORTS 4
#endif

typedef struct {
  uint32_t rx_bytes;
  uint32_t tx_bytes;
  uint32_t rx_msgs;          // short channel/system messages (1..3 bytes)
  uint32_t rx_sysex_chunks;  // sysex chunks forwarded
  uint32_t rx_drops;         // bytes dropped in HAL ring buffers

  uint8_t last_len;          // 0,1,2,3
  uint8_t last[3];           // last short message
} midi_din_stats_t;

void midi_din_init(void);
void midi_din_tick(void);

// Send a router message out over the given DIN port.
// Supports ROUTER_MSG_1B/2B/3B and ROUTER_MSG_SYSEX.
void midi_din_send(uint8_t port, const router_msg_t* msg);

// Snapshot stats for debugging/bring-up.
void midi_din_get_stats(uint8_t port, midi_din_stats_t* out);
