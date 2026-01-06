#include "Services/midi/midi_delayq.h"
#include <string.h>

#ifndef MIDI_DELAYQ_MAX
#define MIDI_DELAYQ_MAX 64
#endif

typedef struct {
  uint8_t used;
  uint8_t in_node;
  uint16_t due_ms;
  router_msg_t msg;
} item_t;

static item_t q[MIDI_DELAYQ_MAX];

void midi_delayq_init(void) {
  memset(q,0,sizeof(q));
}

void midi_delayq_send(uint8_t in_node, const router_msg_t* msg, uint16_t delay_ms) {
  if (!msg) return;
  if (delay_ms == 0) {
    router_process(in_node, msg);
    return;
  }
  for (uint32_t i=0;i<MIDI_DELAYQ_MAX;i++) {
    if (!q[i].used) {
      q[i].used = 1;
      q[i].in_node = in_node;
      q[i].due_ms = delay_ms;
      q[i].msg = *msg;
      return;
    }
  }
  // drop if full
}

void midi_delayq_tick_1ms(void) {
  for (uint32_t i=0;i<MIDI_DELAYQ_MAX;i++) {
    if (!q[i].used) continue;
    if (q[i].due_ms) q[i].due_ms--;
    if (q[i].due_ms == 0) {
      router_process(q[i].in_node, &q[i].msg);
      q[i].used = 0;
    }
  }
}
