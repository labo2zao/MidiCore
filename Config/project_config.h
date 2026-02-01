#pragma once

// Project-wide feature toggles (safe defaults).

// Enable USB MIDI Host wrapper (requires CubeMX USB Host middleware + OTG FS in Host mode).
// If the USB Host middleware is not present, the code builds as a stub anyway.
#define ENABLE_USBH_MIDI 1

// Note: Router node IDs (ROUTER_NODE_USBH_IN, ROUTER_NODE_USBH_OUT, etc.) 
// are now defined in router_config.h as enum values

// -----------------------------------------------------------------------------
// Panic / Fault Behavior
// -----------------------------------------------------------------------------

// PANIC_AUTO_RESET controls what happens on stack overflow or malloc failure:
//   0 = HALT for debugging (system stops, attach debugger to inspect)
//   1 = AUTO-RESET for production (system recovers automatically)
//
// For debugging crashes: Set to 0, then attach debugger when system halts
// For production use: Set to 1 for automatic recovery
#ifndef PANIC_AUTO_RESET
#define PANIC_AUTO_RESET 0  // Default: HALT for debugging
#endif

// -----------------------------------------------------------------------------
// Debug helpers
// -----------------------------------------------------------------------------

// When enabled, a low-priority task prints the raw ADC values of all AINSER64
// channels to USART1 (115200) at a fixed interval.
// Set to 0 for normal operation.
#define DEBUG_AIN_RAW_DUMP 1

// Period for the raw dump task.
#define DEBUG_AIN_RAW_DUMP_PERIOD_MS 250
