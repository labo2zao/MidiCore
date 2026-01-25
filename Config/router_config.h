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

  // USB Device MIDI (4 ports - cables 0-3)
  ROUTER_NODE_USB_PORT0 = 8,   // USB Device MIDI Port 1 (cable 0)
  ROUTER_NODE_USB_PORT1 = 9,   // USB Device MIDI Port 2 (cable 1)
  ROUTER_NODE_USB_PORT2 = 10,  // USB Device MIDI Port 3 (cable 2)
  ROUTER_NODE_USB_PORT3 = 11,  // USB Device MIDI Port 4 (cable 3)

  // USB Host MIDI (OTG FS)
  ROUTER_NODE_USBH_IN   = 12,  // USB Host MIDI IN
  ROUTER_NODE_USBH_OUT  = 13,  // USB Host MIDI OUT

  // Logical / Internal nodes
  ROUTER_NODE_LOOPER    = 14,  // Internal looper engine
  ROUTER_NODE_KEYS      = 15,  // Custom keybed (AINSER/Hall)
} router_node_id_t;

#define ROUTER_CHMASK_ALL 0xFFFFu

// Legacy aliases for backward compatibility
#define ROUTER_NODE_USB_IN   ROUTER_NODE_USB_PORT0  // Default to port 0
#define ROUTER_NODE_USB_OUT  ROUTER_NODE_USB_PORT0  // Default to port 0
