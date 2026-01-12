#pragma once
#include <stdint.h>

#define ROUTER_NUM_NODES 16

typedef enum {
  // DIN physical ports
  ROUTER_NODE_DIN_IN1  = 0,
  ROUTER_NODE_DIN_IN2  = 1,
  ROUTER_NODE_DIN_IN3  = 2,
  ROUTER_NODE_DIN_IN4  = 3,

  ROUTER_NODE_DIN_OUT1 = 4,
  ROUTER_NODE_DIN_OUT2 = 5,
  ROUTER_NODE_DIN_OUT3 = 6,
  ROUTER_NODE_DIN_OUT4 = 7,

  // USB device MIDI
  ROUTER_NODE_USB_IN   = 8,
  ROUTER_NODE_USB_OUT  = 9,

  // Reserved / logical nodes
  ROUTER_NODE_RESERVED10 = 10,
  ROUTER_NODE_RESERVED11 = 11,
  ROUTER_NODE_RESERVED12 = 12,
  ROUTER_NODE_RESERVED13 = 13,
  ROUTER_NODE_RESERVED14 = 14,
  ROUTER_NODE_RESERVED15 = 15,
} router_node_id_t;

#define ROUTER_CHMASK_ALL 0xFFFFu

// Custom logical node for keybed (AINSER/Hall).
#define ROUTER_NODE_KEYS ROUTER_NODE_RESERVED11

// USB Host MIDI (OTG FS) nodes (aliases onto reserved slots).
#define ROUTER_NODE_USBH_IN   12
#define ROUTER_NODE_USBH_OUT  13

// Logical node used as source for internal looper engine
#define ROUTER_NODE_LOOPER ROUTER_NODE_RESERVED10
