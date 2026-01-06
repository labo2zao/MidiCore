#pragma once
#include <stdint.h>
#include "Services/router/router.h"

#ifdef __cplusplus
extern "C" {
#endif

void midi_delayq_init(void);
/** enqueue a msg to be routed from in_node after delay_ms (0=immediate). */
void midi_delayq_send(uint8_t in_node, const router_msg_t* msg, uint16_t delay_ms);
/** call at 1ms rate */
void midi_delayq_tick_1ms(void);

#ifdef __cplusplus
}
#endif
