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

  uint8_t  last_len;         // 0,1,2,3
  uint8_t  last_bytes[3];    // last short message
  uint32_t rx_stray_data;    // stray data bytes with no running status
} midi_din_stats_t;

void midi_din_init(void);
void midi_din_tick(void);

// Send raw MIDI bytes out over the given DIN port.
void midi_din_send(uint8_t port, const uint8_t* data, uint16_t len);

// Snapshot stats for debugging/bring-up.
void midi_din_get_stats(uint8_t port, midi_din_stats_t* out);
