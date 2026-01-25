// hal_ainser64_hw_step.h
// AINSER64 (MBHP_AINSER64) driver backend for this project.
//
// Hardware model (MIOS32-compatible):
// - MCP3208 (12-bit SPI ADC)
// - 74HC595 loaded via MOSI during each 3-byte MCP transaction
// - the 74HC595 latch is wired to the ADC CS (RC) line, so each CS rising edge
//   updates the multiplexer address lines (A0..A2) and the green "LINK" LED.
//
// This file intentionally mimics the logic found in MIOS32 modules/ainser/ainser.c
// (see https://github.com/midibox/mios32 ) while keeping a simpler public API.

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Init the AINSER64 hardware backend.
// Returns 0 on success.
int32_t hal_ainser64_init(void);

// Read one mux step (0..7) for the given module/bank.
// - module/bank: currently only 0 is supported in this project (single CS line).
// - step: 0..7 (mux address)
// - out8: receives 8 raw 12-bit values (0..4095), one per MCP3208 channel.
// 
// IMPORTANT: Call this function continuously without delays between steps to maintain
// stable ADC readings. Delays between steps can cause discontinuous values and noise.
// This matches MIOS32 behavior where all channels are scanned in rapid succession.
// The LED will also exhibit smooth PWM breathing when scanned continuously.
//
// Returns 0 on success.
int32_t hal_ainser64_read_bank_step(uint8_t module, uint8_t step, uint16_t out8[8]);

// Optional: enable/disable LINK LED modulation (default: enabled).
// The LED uses MIOS32-style PWM breathing effect that requires continuous scanning.
void hal_ainser64_set_link_led_enable(uint8_t enable);

// Optional: set the mux step->connector mapping.
// If map is NULL, the default MIOS32 mapping is restored.
// map[step] gives the logical "port" index 0..7 for this mux address.
void hal_ainser64_set_mux_port_map(const uint8_t map[8]);

#ifdef __cplusplus
}
#endif
