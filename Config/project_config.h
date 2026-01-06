#pragma once

// Project-wide feature toggles (safe defaults).

// Enable USB MIDI Host wrapper (requires CubeMX USB Host middleware + OTG FS in Host mode).
// If the USB Host middleware is not present, the code builds as a stub anyway.
#define ENABLE_USBH_MIDI 1

// Router node ids for USB host (must match your router DSL config if you use it)
#define ROUTER_NODE_USBH_IN   12
#define ROUTER_NODE_USBH_OUT  13
