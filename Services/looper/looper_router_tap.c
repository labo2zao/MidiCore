#include "Services/looper/looper.h"
void router_tap_hook(uint8_t in_node, const router_msg_t* msg) { looper_on_router_msg(in_node, msg); }
