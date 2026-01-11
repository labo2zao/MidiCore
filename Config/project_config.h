#pragma once

// Project-wide feature toggles (safe defaults).

// Enable USB MIDI Host wrapper (requires CubeMX USB Host middleware + OTG FS in Host mode).
// If the USB Host middleware is not present, the code builds as a stub anyway.
#define ENABLE_USBH_MIDI 1

// Router node ids for USB host (must match your router DSL config if you use it)
#define ROUTER_NODE_USBH_IN   12
#define ROUTER_NODE_USBH_OUT  13

// -----------------------------------------------------------------------------
// Debug helpers
// -----------------------------------------------------------------------------

// When enabled, a low-priority task prints the raw ADC values of all AINSER64
// channels to USART1 (115200) at a fixed interval.
// Set to 0 for normal operation.
#define DEBUG_AIN_RAW_DUMP 1

// Period for the raw dump task.
#define DEBUG_AIN_RAW_DUMP_PERIOD_MS 250
