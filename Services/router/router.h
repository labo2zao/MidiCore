#pragma once
#include <stdint.h>
#include "Config/router_config.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  ROUTER_MSG_1B = 1,
  ROUTER_MSG_2B = 2,
  ROUTER_MSG_3B = 3,
  ROUTER_MSG_SYSEX = 0xF0
} router_msg_type_t;

typedef struct {
  router_msg_type_t type;
  uint8_t b0, b1, b2;
  const uint8_t* data;
  uint16_t len;
} router_msg_t;

typedef int (*router_send_fn_t)(uint8_t out_node, const router_msg_t* msg);

void router_init(router_send_fn_t send_cb);
void router_set_route(uint8_t in_node, uint8_t out_node, uint8_t enable);
uint8_t router_get_route(uint8_t in_node, uint8_t out_node);

void router_set_chanmask(uint8_t in_node, uint8_t out_node, uint16_t chmask);
uint16_t router_get_chanmask(uint8_t in_node, uint8_t out_node);

void router_set_label(uint8_t in_node, uint8_t out_node, const char* label);
const char* router_get_label(uint8_t in_node, uint8_t out_node);

void router_process(uint8_t in_node, const router_msg_t* msg);

#ifdef __cplusplus
}
#endif
